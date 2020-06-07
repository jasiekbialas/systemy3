#include <minix/drivers.h>
#include <minix/chardriver.h>
#include <sys/ioc_dfa.h>
#include <minix/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <minix/ds.h>


#define BUFFER_SIZE 65536
#define YES 89
#define NO 78

#define ONE_STATE_FUNCTION_SIZE 256
#define NO_OF_STATES 256

#define CURRENT_STATE_SIZE 1
#define BITMAP_SIZE 32
#define FUNCTION_SIZE (ONE_STATE_FUNCTION_SIZE * NO_OF_STATES)
#define AUTOMATA_SIZE (CURRENT_STATE_SIZE + BITMAP_SIZE + FUNCTION_SIZE)

#define CURRENT_STATE 0
#define BITMAP (CURRENT_STATE + CURRENT_STATE_SIZE)
#define FUNCTION (BITMAP + BITMAP_SIZE)

#define GET_CURRENT (automata[ CURRENT_STATE ])
#define SET_CURRENT(state) (automata[ CURRENT_STATE ] = state)

#define SET_TRANSITION(state, letter, value) (automata[FUNCTION + (state) * ONE_STATE_FUNCTION_SIZE + (letter)] = (value))
#define GET_NEXT(letter) (automata[FUNCTION + (GET_CURRENT) * ONE_STATE_FUNCTION_SIZE + (letter)])

#define DIVIDE_BY_8(num) ((num) >> 3)
#define MOD_8(num) ((num) & 7)

#define BITMAP_BYTE(state) (BITMAP + DIVIDE_BY_8(state))
#define GET_NTH_BIT_FROM_BYTE(byte, n) (byte & (1 << n));

#define SET_ACCEPTING(state) (automata[(BITMAP_BYTE(state))] |= (1 << (MOD_8(state))))
#define SET_REJECTING(state) (automata[(BITMAP_BYTE(state))] &= ~(1 << (MOD_8(state))))

#define IS_CURRENT_ACCEPTING (automata[(BITMAP_BYTE( GET_CURRENT ))] & (1 << GET_CURRENT))



// I'm holding all info about automata in this array, in this order:
// current state (1byte) | accepting states bitmap(32bytes) | transition function(256*256)bytes
static uint8_t automata[AUTOMATA_SIZE];
static uint8_t buffer[BUFFER_SIZE];

/*
 * Function prototypes for the hello driver.
 */

static ssize_t dfa_read(devminor_t minor, u64_t position, endpoint_t endpt,
    cp_grant_id_t grant, size_t size, int flags, cdev_id_t id);
static ssize_t dfa_write(devminor_t minor, u64_t position, endpoint_t endpt,
    cp_grant_id_t grant, size_t size, int flags, cdev_id_t id);
static int dfa_ioctl(devminor_t minor, unsigned long request, endpoint_t endpt,
    cp_grant_id_t grant, int flags, endpoint_t user_endpt, cdev_id_t id);

/* SEF functions and variables. */
static void sef_local_startup(void);
static int sef_cb_init(int type, sef_init_info_t *info);
static int sef_cb_lu_state_save(int);
static int lu_state_restore(void);


/* Entry points to the dfa driver. */
static struct chardriver dfa_tab =
{
    .cdr_read	= dfa_read,
    .cdr_write   = dfa_write,
    .cdr_ioctl  = dfa_ioctl,
};

static ssize_t dfa_read(devminor_t UNUSED(minor), u64_t UNUSED(position),
    endpoint_t endpt, cp_grant_id_t grant, size_t size, int UNUSED(flags),
    cdev_id_t UNUSED(id))
{

    size_t left = size;
    size_t send;
    int ret;

    send = MIN(left, BUFFER_SIZE);

    if(IS_CURRENT_ACCEPTING) {
        memset(buffer, YES, send);
    } else {
        memset(buffer, NO, send);
    }

    while(left > 0) {
        send = MIN(left, BUFFER_SIZE);
        if ((ret = sys_safecopyto(endpt, grant, 0, (vir_bytes) buffer, size)) != OK) {
            return ret;
        }
        left-=send;
    }

    return size;
}

static ssize_t dfa_write(devminor_t UNUSED(minor), u64_t UNUSED(position),
    endpoint_t endpt, cp_grant_id_t grant, size_t size, int UNUSED(flags),
    cdev_id_t UNUSED(id))
{
    int ret;
    size_t read;

    size_t left = size;
    while(left > 0) {
        read = MIN(left, BUFFER_SIZE);
        if ((ret = sys_safecopyfrom(endpt, grant, 0, (vir_bytes) buffer, read)) != OK) {
            return ret;
        }
        left-=read;
        for(size_t i = 0; i < read; i++) {
           SET_CURRENT( GET_NEXT( buffer[i] ));
        }
    }
    return size;
}

static int dfa_ioctl(devminor_t UNUSED(minor), unsigned long request, endpoint_t endpt,
    cp_grant_id_t grant, int UNUSED(flags), endpoint_t user_endpt, cdev_id_t UNUSED(id))
{
    int rc;

    struct {
        uint8_t state1;
        uint8_t letter;
        uint8_t state2;
    } in;

    switch(request) {
    case DFAIOCRESET:
        SET_CURRENT(0);
    case DFAIOCADD:
        rc = sys_safecopyfrom(endpt, grant, 0, (vir_bytes) &in, 3);
        if (rc == OK) {
            SET_TRANSITION(in.state1, in.letter, in.state2);
            SET_CURRENT(0);
        }
        break;
    case DFAIOCACCEPT:
        rc = sys_safecopyfrom(endpt, grant, 0, (vir_bytes) &in, 1);
        if (rc == OK) {
            SET_ACCEPTING(in.state1);
        }
        break;
    case DFAIOCREJECT:
        rc = sys_safecopyfrom(endpt, grant, 0, (vir_bytes) &in, 1);
        if (rc == OK) {
            SET_REJECTING(in.state1);
        }
        break;
    }

    return rc;
}

static int sef_cb_lu_state_save(int UNUSED(state)) {
/* Save the state. */
    ds_publish_mem("automata_state", automata, AUTOMATA_SIZE, DSF_OVERWRITE);
    return OK;
}

static int lu_state_restore() {
/* Restore the state. */
    int value = AUTOMATA_SIZE;
    ds_retrieve_mem("automata_state", automata, &value);
    ds_delete_mem("automata_state");
    return OK;
}

static void sef_local_startup()
{
    /*
     * Register init callbacks. Use the same function for all event types
     */
    sef_setcb_init_fresh(sef_cb_init);
    sef_setcb_init_lu(sef_cb_init);
    sef_setcb_init_restart(sef_cb_init);

    /*
     * Register live update callbacks.
     */
    /* - Agree to update immediately when LU is requested in a valid state. */
    sef_setcb_lu_prepare(sef_cb_lu_prepare_always_ready);
    /* - Support live update starting from any standard state. */
    sef_setcb_lu_state_isvalid(sef_cb_lu_state_isvalid_standard);
    /* - Register a custom routine to save the state. */
    sef_setcb_lu_state_save(sef_cb_lu_state_save);

    /* Let SEF perform startup. */
    sef_startup();
}

static int sef_cb_init(int type, sef_init_info_t *UNUSED(info))
{
/* Initialize the dfa driver. */
    int do_announce_driver = TRUE;

    switch(type) {
        case SEF_INIT_RESTART:
        case SEF_INIT_FRESH:
            memset(automata, 0, AUTOMATA_SIZE);
            printf("dfa initiated\n");
        break;

        case SEF_INIT_LU:
            /* Restore the state. */
            lu_state_restore();
            do_announce_driver = FALSE;
            printf("Hey, I'm a new version of dfa!\n");
        break;
    }

    /* Announce we are up when necessary. */
    if (do_announce_driver) {
        chardriver_announce();
    }

    /* Initialization completed successfully. */
    return OK;
}

int main(void)
{
    /*
     * Perform initialization.
     */
    sef_local_startup();

    /*
     * Run the main loop.
     */
    chardriver_task(&dfa_tab);
    return OK;
}
