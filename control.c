#include <stdlib.h>
#include "btstack.h"
#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include "control.h"
#include "debug.h"

static queue_t *ctrl_ev_w_queue = NULL;
static queue_t *ctrl_ev_r_queue = NULL;

static uint8_t is_paired = 1;
static uint8_t is_discoverable = 0;

static uint8_t is_connected = 1;
static uint8_t connecting = 0;

static alarm_id_t check_pairing_status_alarm;

int64_t check_pairing_status(alarm_id_t id, void *user_data) {
    UNUSED(id);
    UNUSED(user_data);
    
    
    if (!is_paired) {
        ctrl_make_discoverable(0);
    }
    
    return 0;
}

ctrl_ev_t ctrl_make_event(ctrl_ev_type_t ev_type, void *data) {
    ctrl_ev_t ev;
    
    ev.type = ev_type;
    ev.data = data;
    return ev;
}

void ctrl_init(queue_t *write_queue, queue_t *read_queue) {
    ctrl_ev_w_queue = write_queue;
    ctrl_ev_r_queue = read_queue;
}

void ctrl_process_queue() {
    ctrl_ev_t ev;
    
    while (queue_try_remove(ctrl_ev_r_queue, &ev)) {
        switch (ev.type) {

            case CTRL_EV_PAIRING_SUCCESS: {
                DEBUG("Is paired with %s\n", bd_addr_to_str(ev.data));
                free(ev.data);
                ev.data = NULL;
                is_discoverable = 0;
                is_paired = 1;
                cancel_alarm(check_pairing_status_alarm);
                break;
            }

            case CTRL_EV_CONNECTED: {
                if (!is_paired) {
                    break;
                }

                ctrl_connect();
                is_connected = 1;
                break;
            }
                                          
            case CTRL_EV_DISCONNECTED: {
                uint8_t reason = *(uint8_t *) ev.data;
                free(ev.data);
                DEBUG("Connection lost. Reason: %d\n", reason);
                is_connected = 0;

                if (reason != ERROR_CODE_REMOTE_USER_TERMINATED_CONNECTION && reason != ERROR_CODE_PAIRING_NOT_ALLOWED) {
                    // RETRY CONNECTION
                }
                break;
            }
        }
    };
}

void ctrl_connect() {
    if (is_connected) {
        return;
    }

    ctrl_ev_t ev = ctrl_make_event(CTRL_EV_REQUEST_CONNECTION, NULL);
    if (!queue_try_add(ctrl_ev_w_queue, &ev)) {
        return;
    }
    
    connecting = 1;
}


void ctrl_make_discoverable(uint8_t state) {
    if (state && is_discoverable) {
        return;
    }
    uint8_t *state_ptr = malloc(sizeof(uint8_t));
    *state_ptr = state;
    ctrl_ev_t ev = ctrl_make_event(CTRL_EV_MAKE_DISCOVERABLE, state_ptr);
    
    if (!queue_try_add(ctrl_ev_w_queue, &ev)) {
        return;
    }
    
    is_discoverable = state;
    if (state) {
        check_pairing_status_alarm = add_alarm_in_ms(PAIRING_TIMEOUT_MS, check_pairing_status, NULL, false);
    }
}

void ctrl_toggle_play_pause() {
    ctrl_ev_t ev = ctrl_make_event(CTRL_EV_REQUEST_TOGGLE_PLAY, NULL);
    
    if (!queue_try_add(ctrl_ev_w_queue, &ev)) {
        return;
    }
}

void ctrl_vol_up() {
    ctrl_ev_t ev = ctrl_make_event(CTRL_EV_REQUEST_VOL_UP, NULL);

    if (!queue_try_add(ctrl_ev_w_queue, &ev)) {
        return;
    }
}

void ctrl_vol_down() {
    ctrl_ev_t ev = ctrl_make_event(CTRL_EV_REQUEST_VOL_DOWN, NULL);

    if (!queue_try_add(ctrl_ev_w_queue, &ev)) {
        return;
    }
}