#include <stdlib.h>
#include "pico/util/queue.h"
#include "control.h"

uint8_t ctrl_is_connected = 1;

absolute_time_t ctrl_connected_at;
absolute_time_t ctrl_disconnected_at;
absolute_time_t ctrl_last_command_at;

static queue_t *ctrl_ev_w_queue = NULL;
static queue_t *ctrl_ev_r_queue = NULL;
static uint8_t ctrl_is_discoverable = 1;


ctrl_ev_t ctrl_make_event(ctrl_ev_type_t ev_type, void *data) {
    ctrl_ev_t ev;
    
    ev.type = ev_type;
    ev.data = data;
    return ev;
}

void ctrl_init(queue_t *write_queue, queue_t *read_queue) {
    ctrl_ev_w_queue = write_queue;
    ctrl_ev_r_queue = read_queue;
    ctrl_disconnected_at = get_absolute_time();
    ctrl_last_command_at = ctrl_disconnected_at;
}

void ctrl_deinit() {
    ctrl_ev_w_queue = NULL;
    ctrl_ev_r_queue = NULL;
    ctrl_is_discoverable = 1;
    ctrl_is_connected = 0;
    ctrl_connected_at = nil_time;
    ctrl_disconnected_at = nil_time;
    ctrl_last_command_at = nil_time;
}

void ctrl_process_queue() {
    ctrl_ev_t ev;
    
    while (queue_try_remove(ctrl_ev_r_queue, &ev)) {
        switch (ev.type) {

            case CTRL_EV_CONNECTED:
                if (ctrl_is_connected) {
                    continue;
                }

                ctrl_is_connected = 1;
                ctrl_connected_at = get_absolute_time();
                ctrl_disconnected_at = nil_time;
                ctrl_make_discoverable(0);
            break;
                                          
            case CTRL_EV_DISCONNECTED:
                if (!ctrl_is_connected) {
                    continue;
                }
                ctrl_is_connected = 0;
                ctrl_disconnected_at = get_absolute_time();
                ctrl_connected_at = nil_time;
                ctrl_make_discoverable(1);
            break;
            
            default:
                break;

        }
    };
}

void ctrl_make_discoverable(uint8_t state) {
    if (state == ctrl_is_discoverable) {
        return;
    }
    uint8_t *state_ptr = malloc(sizeof(uint8_t));
    *state_ptr = state;
    ctrl_ev_t ev = ctrl_make_event(CTRL_EV_MAKE_DISCOVERABLE, state_ptr);
    
    if (!queue_try_add(ctrl_ev_w_queue, &ev)) {
        return;
    }
    
    ctrl_is_discoverable = state;
}

void ctrl_toggle_play_pause() {
    if (!ctrl_is_connected) {
        return;
    }
    ctrl_ev_t ev = ctrl_make_event(CTRL_EV_REQUEST_TOGGLE_PLAY, NULL);
    ctrl_last_command_at = get_absolute_time();
    
    queue_try_add(ctrl_ev_w_queue, &ev);
}

void ctrl_vol_up() {
    if (!ctrl_is_connected) {
        return;
    }
    ctrl_ev_t ev = ctrl_make_event(CTRL_EV_REQUEST_VOL_UP, NULL);
    ctrl_last_command_at = get_absolute_time();

    queue_try_add(ctrl_ev_w_queue, &ev);
}

void ctrl_vol_down() {
    if (!ctrl_is_connected) {
        return;
    }
    ctrl_ev_t ev = ctrl_make_event(CTRL_EV_REQUEST_VOL_DOWN, NULL);
    ctrl_last_command_at = get_absolute_time();

    queue_try_add(ctrl_ev_w_queue, &ev);
}