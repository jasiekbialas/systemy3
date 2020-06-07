#include <minix/drivers.h>
#include <minix/chardriver.h>
#include <sys/ioc_hello.h>
#include <minix/ioctl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <minix/ds.h>
#include "hello.h"


/*
 * Function prototypes for the hello driver.
 */
static int hello_open(devminor_t minor, int access, endpoint_t user_endpt);
static int hello_close(devminor_t minor);
static ssize_t hello_read(devminor_t minor, u64_t position, endpoint_t endpt,
    cp_grant_id_t grant, size_t size, int flags, cdev_id_t id);
static int hello_ioctl(devminor_t minor, unsigned long request, endpoint_t endpt,
    cp_grant_id_t grant, int flags, endpoint_t user_endpt, cdev_id_t id);

/* SEF functions and variables. */
static void sef_local_startup(void);
static int sef_cb_init(int type, sef_init_info_t *info);
static int sef_cb_lu_state_save(int);
static int lu_state_restore(void);

/* Entry points to the hello driver. */
static struct chardriver hello_tab =
{
    .cdr_open	= hello_open,
    .cdr_close	= hello_close,
    .cdr_read	= hello_read,
    .cdr_ioctl  = hello_ioctl,
};

/** State variable to count the number of times the device has been opened.
 * Note that this is not the regular type of open counter: it never decreases.
 */
static int open_counter;
static char hello_msg[HELLO_LEN];

static int hello_open(devminor_t UNUSED(minor), int UNUSED(access),
    endpoint_t UNUSED(user_endpt))
{
    printf("hello_open(). Called %d time(s).\n", ++open_counter);
    return OK;
}

static int hello_close(devminor_t UNUSED(minor))
{
    printf("hello_close()\n");
    return OK;
}

static ssize_t hello_read(devminor_t UNUSED(minor), u64_t position,
    endpoint_t endpt, cp_grant_id_t grant, size_t size, int UNUSED(flags),
    cdev_id_t UNUSED(id))
{
    u64_t dev_size;
    char *ptr;
    int ret;
    char *buf = hello_msg;

    printf("hello_read()\n");

    /* This is the total size of our device. */
    dev_size = (u64_t) strlen(buf);

    /* Check for EOF, and possibly limit the read size. */
    if (position >= dev_size) return 0;		/* EOF */
    if (position + size > dev_size)
        size = (size_t)(dev_size - position);	/* limit size */

    /* Copy the requested part to the caller. */
    ptr = buf + (size_t)position;
    if ((ret = sys_safecopyto(endpt, grant, 0, (vir_bytes) ptr, size)) != OK)
        return ret;

    /* Return the number of bytes read. */
    return size;
}

static int hello_ioctl(devminor_t UNUSED(minor), unsigned long request, endpoint_t endpt,
    cp_grant_id_t grant, int UNUSED(flags), endpoint_t user_endpt, cdev_id_t UNUSED(id))
{
    int rc;
    char buf[HELLO_LEN];

    switch(request) {
    case HIOCSETMSG:
        rc = sys_safecopyfrom(endpt, grant, 0, (vir_bytes) buf, HELLO_LEN);
        if (rc == OK) {
            strncpy(hello_msg, buf, HELLO_LEN);
            hello_msg[HELLO_LEN - 1] = 0; /* To make sure it's null-terminated. */
        }
        break;

    case HIOCGETMSG:
        rc = sys_safecopyto(endpt, grant, 0, (vir_bytes) hello_msg, HELLO_LEN);
        break;

    default:
        rc = ENOTTY;
    }

    return rc;
}

static int sef_cb_lu_state_save(int UNUSED(state)) {
/* Save the state. */
    ds_publish_u32("open_counter", open_counter, DSF_OVERWRITE);
    ds_publish_str("hello_msg", hello_msg, DSF_OVERWRITE);

    return OK;
}

static int lu_state_restore() {
/* Restore the state. */
    u32_t value;

    ds_retrieve_u32("open_counter", &value);
    ds_retrieve_str("hello_msg", hello_msg, HELLO_LEN);

    ds_delete_u32("open_counter");
    ds_delete_str("hello_msg");

    open_counter = (int) value;

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
/* Initialize the hello driver. */
    int do_announce_driver = TRUE;

    open_counter = 0;
    switch(type) {
        case SEF_INIT_FRESH:
            strncpy(hello_msg, HELLO_MESSAGE, HELLO_LEN);
            hello_msg[HELLO_LEN - 1] = 0;
            printf("%s", hello_msg);
        break;

        case SEF_INIT_LU:
            /* Restore the state. */
            lu_state_restore();
            do_announce_driver = FALSE;

            printf("%sHey, I'm a new version!\n", hello_msg);
        break;

        case SEF_INIT_RESTART:
            strncpy(hello_msg, HELLO_MESSAGE, HELLO_LEN);
            hello_msg[HELLO_LEN - 1] = 0;
            printf("%sHey, I've just been restarted!\n", hello_msg);
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
    chardriver_task(&hello_tab);
    return OK;
}

