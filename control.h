#include "btstack.h"
#include "pico/util/queue.h"

#define PAIRING_TIMEOUT_MS 60 * 1000
#define DELAY_AVRCP_CONNECTION_AFTER_PAIR_MS 1000

typedef enum {
    CTRL_EV_MAKE_DISCOVERABLE,
    CTRL_EV_PAIRING_SUCCESS,
    CTRL_EV_PAIRING_ERROR,
    CTRL_EV_REQUEST_CONNECTION,
    CTRL_EV_REQ_CONNECTION_FAILED,
    CTRL_EV_CONNECTED,
    CTRL_EV_DISCONNECTED,
    CTRL_EV_REQUEST_TOGGLE_PLAY,
    CTRL_EV_REQUEST_VOL_UP,
    CTRL_EV_REQUEST_VOL_DOWN
} ctrl_ev_type_t;

typedef struct {
    ctrl_ev_type_t type;
    void *data;
} ctrl_ev_t;

ctrl_ev_t ctrl_make_event(ctrl_ev_type_t ev_type, void *data);

void ctrl_init(queue_t *write_queue, queue_t *read_queue);
void ctrl_process_queue();
void ctrl_make_discoverable(uint8_t state);
void ctrl_connect();
void ctrl_toggle_play_pause();
void ctrl_vol_up();
void ctrl_vol_down();