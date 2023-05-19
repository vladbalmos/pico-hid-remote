#include "pico/util/queue.h"
#include "hidremote.h"
#include "btstack.h"
#include "control.h"
#include "debug.h"

#define KEY_PLAY_PAUSE 0xCD
#define KEY_VOL_UP 0xE9
#define KEY_VOL_DOWN 0xEA

static queue_t *ctrl_ev_w_queue = NULL;
static queue_t *ctrl_ev_r_queue = NULL;

static btstack_packet_callback_registration_t hci_event_callback_registration;
static btstack_packet_callback_registration_t sm_event_callback_registration;
static uint8_t battery = 100;
static hci_con_handle_t con_handle = HCI_CON_HANDLE_INVALID;
static uint8_t send_keycode;

// https://github.com/adafruit/circuitpython/blob/9e4dea7b15f3b4108be00f7aaffaa24f38aca978/shared-module/usb_hid/Device.c
const uint8_t hid_descriptor[] = {
    0x05, 0x0C,        // Usage Page (Consumer)
    0x09, 0x01,        // Usage (Consumer Control)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x01,        //   Report ID (3)
    0x75, 0x10,        //   Report Size (16)
    0x95, 0x01,        //   Report Count (1)
    0x15, 0x01,        //   Logical Minimum (1)
    0x26, 0x8C, 0x02,  //   Logical Maximum (652)
    0x19, 0x01,        //   Usage Minimum (Consumer Control)
    0x2A, 0x8C, 0x02,  //   Usage Maximum (AC Send)
    0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              // End Collection
};

const uint8_t adv_data[] = {
    // Flags general discoverable, BR/EDR not supported
    0x02, BLUETOOTH_DATA_TYPE_FLAGS, 0x06,
    // Name
    0x0f, BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME, 'P', 'I', 'C', 'O', ' ', 'H', 'I', 'D', 'R', 'e', 'm', 'o', 't', 'e',
    // 16-bit Service UUIDs
    0x03, BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS, ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE & 0xff, ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE >> 8,
    // Appearance HID - Keyboard (Category 15, Sub-Category 1)
    0x03, BLUETOOTH_DATA_TYPE_APPEARANCE, 0xC1, 0x03,
};

const uint8_t adv_data_len = sizeof(adv_data);


static void send_report(uint8_t keycode){
    uint8_t report[] = { keycode, 0};
    hids_device_send_input_report(con_handle, report, sizeof(report));
}

static void packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
    UNUSED(channel);
    UNUSED(size);

    if (packet_type != HCI_EVENT_PACKET) {
        DEBUG("Unknown event\n");
        return;
    }

    switch (hci_event_packet_get_type(packet)) {
        case HCI_EVENT_DISCONNECTION_COMPLETE:
            con_handle = HCI_CON_HANDLE_INVALID;
            ctrl_ev_t ev = ctrl_make_event(CTRL_EV_DISCONNECTED, NULL);
            queue_try_add(ctrl_ev_w_queue, &ev);
            DEBUG("BT Disconnected\n");
            break;
        case SM_EVENT_JUST_WORKS_REQUEST:
            DEBUG("BT Authenticated\n");
            sm_just_works_confirm(sm_event_just_works_request_get_handle(packet));
            break;

        case HCI_EVENT_HIDS_META:
            switch (hci_event_hids_meta_get_subevent_code(packet)){
                case HIDS_SUBEVENT_INPUT_REPORT_ENABLE:
                    con_handle = hids_subevent_input_report_enable_get_con_handle(packet);
                    DEBUG("BT HID Connected\n");
                    ctrl_ev_t ev = ctrl_make_event(CTRL_EV_CONNECTED, NULL);
                    queue_try_add(ctrl_ev_w_queue, &ev);
                    break;
                case HIDS_SUBEVENT_CAN_SEND_NOW:
                    send_report(send_keycode);
                    break;
                default:
                    break;
            }
            break;

        default:
            break;
    }
}

void bt_init(queue_t *write_queue, queue_t *read_queue) {
    ctrl_ev_w_queue = write_queue;
    ctrl_ev_r_queue = read_queue;

    l2cap_init();

    sm_init();
    sm_set_io_capabilities(IO_CAPABILITY_NO_INPUT_NO_OUTPUT);
    sm_set_authentication_requirements(SM_AUTHREQ_SECURE_CONNECTION | SM_AUTHREQ_BONDING);

    // setup ATT server
    att_server_init(profile_data, NULL, NULL);

    // setup battery service
    battery_service_server_init(battery);

    // setup device information service
    device_information_service_server_init();

    // setup HID Device service
    hids_device_init(0, hid_descriptor, sizeof(hid_descriptor));

    // setup advertisements
    uint16_t adv_int_min = 0x0030;
    uint16_t adv_int_max = 0x0320;
    uint8_t adv_type = 0;
    bd_addr_t null_addr;
    memset(null_addr, 0, 6);
    gap_advertisements_set_params(adv_int_min, adv_int_max, adv_type, 0, null_addr, 0x07, 0x00);
    gap_advertisements_set_data(adv_data_len, (uint8_t*) adv_data);
    gap_advertisements_enable(1);

    // register for HCI events
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    // register for SM events
    sm_event_callback_registration.callback = &packet_handler;
    sm_add_event_handler(&sm_event_callback_registration);

    // register for HIDS
    hids_device_register_packet_handler(packet_handler);

#ifdef BT_DEBUG_MODE
    hci_dump_init(hci_dump_embedded_stdout_get_instance());
#endif

    hci_power_control(HCI_POWER_ON);
}

void bt_deinit() {
    sm_remove_event_handler(&sm_event_callback_registration);
    hci_remove_event_handler(&hci_event_callback_registration);
    gap_advertisements_enable(0);
    att_server_deinit();
    sm_deinit();
    l2cap_deinit();
    hci_power_control(HCI_POWER_OFF);
    
    ctrl_ev_w_queue = NULL;
    ctrl_ev_r_queue = NULL;
    con_handle = HCI_CON_HANDLE_INVALID;
    send_keycode = 0;
}

void bt_process_queue() {
    ctrl_ev_t ev;
    
    while (queue_try_remove(ctrl_ev_r_queue, &ev)) {
        switch (ev.type) {
            case CTRL_EV_MAKE_DISCOVERABLE: {
                uint8_t state = *(uint8_t *) ev.data;
                free(ev.data);
                gap_advertisements_enable(state);
                DEBUG("Making discoverable %d\n", state);
                break;
            }

            case CTRL_EV_REQUEST_TOGGLE_PLAY:
                DEBUG("Requesting toggle play/pause\n");
                send_keycode = KEY_PLAY_PAUSE;
                hids_device_request_can_send_now_event(con_handle);
                send_keycode = 0;
                hids_device_request_can_send_now_event(con_handle);
            break;

            case CTRL_EV_REQUEST_VOL_UP:
                DEBUG("Requesting vol up\n");
                send_keycode = KEY_VOL_UP;
                hids_device_request_can_send_now_event(con_handle);
                send_keycode = 0;
                hids_device_request_can_send_now_event(con_handle);
            break;

            case CTRL_EV_REQUEST_VOL_DOWN:
                DEBUG("Requesting vol down\n");
                send_keycode = KEY_VOL_DOWN;
                hids_device_request_can_send_now_event(con_handle);
                send_keycode = 0;
                hids_device_request_can_send_now_event(con_handle);
            break;
        }
    }
}
