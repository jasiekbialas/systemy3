#include <minix/drivers.h>
#include <minix/chardriver.h>
#include <sys/ioc_dfa.h>
#include <minix/ioctl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <minix/ds.h>


#define ONE_STATE_FUNCTION_SIZE 256
#define NO_OF_STATES 256
#define STATE_BITMAP 32
#define CURRENT_STATE_SIZE 1

#define AUTOMATA_SIZE ( CURRENT_STATE_SIZE + STATE_BITMAP + ONE_STATE_FUNCTION_SIZE * NO_OF_STATES )

#define BUFFER_SIZE 16384
#define YES 89
#define NO 78

/*
 * Function prototypes for the hello driver.
 */
static int dfa_open(devminor_t minor, int access, endpoint_t user_endpt);
static int dfa_close(devminor_t minor);

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
    .cdr_open	= dfa_open,
    .cdr_close	= dfa_close,
    .cdr_read	= dfa_read,
    .cdr_write   = dfa_write,
    .cdr_ioctl  = dfa_ioctl,
};


// current state | accepting states bitmap | transition function
static uint8_t automata[AUTOMATA_SIZE];
static char buffer[BUFFER_SIZE];

static uint8_t get_next_state(uint8_t state, uint8_t letter) {
    return automata[CURRENT_STATE_SIZE + STATE_BITMAP + state * ONE_STATE_FUNCTION_SIZE + letter];
}

static void set_transition(uint8_t from, uint8_t letter, uint8_t to) {
    automata[ CURRENT_STATE_SIZE + STATE_BITMAP + from * ONE_STATE_FUNCTION_SIZE + letter] = to;
}

static void add_accepting(uint8_t state) {
    uint8_t d = state / 8;
    uint8_t m = state % 8;

    automata[CURRENT_STATE_SIZE + d] |= (1 << m);
}

static void add_rejecting(uint8_t state) {
    uint8_t d = state / 8;
    uint8_t m = state % 8;

    automata[CURRENT_STATE_SIZE + d] &= ~(1 << m);
}

static uint8_t is_accepting(uint8_t state) {
    uint8_t d = state / 8;
    uint8_t m = state % 8;

    return automata[CURRENT_STATE_SIZE + d] & (1 << m);
}

static void set_current_state(uint8_t state) {
    automata[0] = state;
}

static uint8_t get_current_state() {
    return automata[0];
}


static int dfa_open(devminor_t UNUSED(minor), int UNUSED(access),
    endpoint_t UNUSED(user_endpt))
{
    printf("dfa_open()\n");
    return OK;
}

static int dfa_close(devminor_t UNUSED(minor))
{
    printf("dfa_close()\n");
    return OK;
}

static ssize_t dfa_read(devminor_t UNUSED(minor), u64_t UNUSED(position),
    endpoint_t endpt, cp_grant_id_t grant, size_t size, int UNUSED(flags),
    cdev_id_t UNUSED(id))
{

    size_t left = size;
    size_t send;
    int ret;

    printf("dfa_read()\n");

    if(is_accepting(get_current_state())) {
        memset(buffer, YES, BUFFER_SIZE);
    } else {
        memset(buffer, NO, BUFFER_SIZE);
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

    printf("dfa_write()\n");

    size_t left = size;
    while(left > 0) {
        read = MIN(left, BUFFER_SIZE);
        if ((ret = sys_safecopyfrom(endpt, grant, 0, (vir_bytes) buffer, read)) != OK) {
            return ret;
        }
        left-=read;
        for(size_t i = 0; i < read; i++) {
            set_current_state(get_next_state(get_current_state(), buffer[i]));
        }
    }
    return size;
}

static int dfa_ioctl(devminor_t UNUSED(minor), unsigned long request, endpoint_t endpt,
    cp_grant_id_t grant, int UNUSED(flags), endpoint_t user_endpt, cdev_id_t UNUSED(id))
{
    int rc;
    char buf[3];

    switch(request) {
    case DFAIOCRESET:
        set_current_state(0);
    case DFAIOCADD:
        rc = sys_safecopyfrom(endpt, grant, 0, (vir_bytes) buf, 3);
        if (rc == OK) {
            set_transition(buf[0], buf[1], buf[2]);
            set_current_state(0);
        }
        break;
    case DFAIOCACCEPT:
        rc = sys_safecopyfrom(endpt, grant, 0, (vir_bytes) buf, 1);
        if (rc == OK) {
            add_accepting(buf[0]);
        }
        break;
    case DFAIOCREJECT:
        rc = sys_safecopyfrom(endpt, grant, 0, (vir_bytes) buf, 1);
        if (rc == OK) {
            add_rejecting(buf[0]);
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

