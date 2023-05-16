#include "pico/util/queue.h"
#include "btstack.h"
#include "control.h"
#include "debug.h"

static queue_t *ctrl_ev_w_queue = NULL;
static queue_t *ctrl_ev_r_queue = NULL;

static btstack_packet_callback_registration_t hci_event_callback_registration;

static uint16_t avrcp_cid = 0;
static bool     avrcp_connected = false;
static bd_addr_t remote_addr;
static bd_addr_t local_addr;

static uint8_t  sdp_avrcp_controller_service_buffer[200];
static uint8_t device_id_sdp_service_buffer[100];

static uint8_t playing_state = 1;

static void avrcp_packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    UNUSED(size);
    UNUSED(channel);

    bd_addr_t address;
    uint16_t local_cid;
    uint8_t  status = 0xFF;

    if (packet_type != HCI_EVENT_PACKET) {
        return;
    }

    uint8_t hci_event_type = hci_event_packet_get_type(packet);
    
    if (hci_event_type != HCI_EVENT_AVRCP_META) {
        return;
    }
    
#ifdef DEBUG_MODE
    char str_packet[64];
    hci_event_code_str(packet[2], str_packet);
    DEBUG("AVR Packet %s\n", str_packet);
#endif

    switch (packet[2]) {
        case AVRCP_SUBEVENT_CONNECTION_ESTABLISHED: {
            local_cid = avrcp_subevent_connection_established_get_avrcp_cid(packet);
            status = avrcp_subevent_connection_established_get_status(packet);
            if (status != ERROR_CODE_SUCCESS){
                avrcp_cid = 0;

                uint8_t *status_ptr = malloc(sizeof(uint8_t));
                *status_ptr = status;

                ctrl_ev_t ev = ctrl_make_event(CTRL_EV_REQ_CONNECTION_FAILED, status_ptr);
                queue_try_add(ctrl_ev_w_queue, &ev);
                return;
            }
            
            avrcp_cid = local_cid;
            avrcp_connected = true;
            avrcp_controller_pause(avrcp_cid);
            avrcp_subevent_connection_established_get_bd_addr(packet, address);
            DEBUG("AVRCP: Connected to %s, cid 0x%02x\n", bd_addr_to_str(address), avrcp_cid);
            return;
        }
        
        case AVRCP_SUBEVENT_CONNECTION_RELEASED: {
            DEBUG("AVRCP: Channel released: cid 0x%02x\n", avrcp_subevent_connection_released_get_avrcp_cid(packet));
            avrcp_cid = 0;
            avrcp_connected = false;
            return;
        }

        case AVRCP_SUBEVENT_NOTIFICATION_PLAYBACK_STATUS_CHANGED: {
            DEBUG("AVRCP Controller: Playback status changed %s\n", avrcp_play_status2str(avrcp_subevent_notification_playback_status_changed_get_play_status(packet)));
            uint8_t play_status = avrcp_subevent_notification_playback_status_changed_get_play_status(packet);
            DEBUG("PLAY STATUS: %d\n", play_status);
            break;
        }

        default:
            break;
        
    }
}

static void packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
    UNUSED(size);
    UNUSED(channel);
    
    uint16_t local_cid;
    uint8_t  status = 0xFF;
    hci_con_handle_t con_handle;
         
    if (packet_type != HCI_EVENT_PACKET) {
        return;
    }
    
    uint8_t hci_event_type = hci_event_packet_get_type(packet);

#ifdef DEBUG_MODE
    char str_packet[64];
    hci_event_code_str(hci_event_type, str_packet);
    DEBUG("Packet %s\n", str_packet);
#endif
    
    switch(hci_event_type){
        case BTSTACK_EVENT_STATE: {
            if (btstack_event_state_get_state(packet) != HCI_STATE_WORKING) {
                return;
            }
            gap_local_bd_addr(local_addr);
            DEBUG("BTstack up and running on %s.\n", bd_addr_to_str(local_addr));
            break;
        }
            
        case HCI_EVENT_PIN_CODE_REQUEST: {
            bd_addr_t address;
            hci_event_pin_code_request_get_bd_addr(packet, address);
            gap_pin_code_response(address, "0000");
            break;
        }

        case HCI_EVENT_USER_CONFIRMATION_REQUEST: {
            bd_addr_t address;
            hci_event_user_confirmation_request_get_bd_addr(packet, address);
            hci_send_cmd(&hci_user_confirmation_request_reply, address);
            break;
        }
                                                  
        case HCI_EVENT_SIMPLE_PAIRING_COMPLETE: {
            bd_addr_t *address = malloc(sizeof(bd_addr_t));
            hci_event_simple_pairing_complete_get_bd_addr(packet, remote_addr);

            bd_addr_copy(address[0], remote_addr);
            
            ctrl_ev_t paired_ev = ctrl_make_event(CTRL_EV_PAIRING_SUCCESS, address);
            queue_try_add(ctrl_ev_w_queue, &paired_ev);
            gap_discoverable_control(0);
            break;
        }
                                                
        case HCI_EVENT_CONNECTION_COMPLETE: {
            ctrl_ev_t ev = ctrl_make_event(CTRL_EV_CONNECTED, NULL);
            queue_try_add(ctrl_ev_w_queue, &ev);
            break;
        }
                                                
        case HCI_EVENT_DISCONNECTION_COMPLETE: {
            con_handle = hci_event_disconnection_complete_get_connection_handle(packet);

            uint8_t *reason_ptr = malloc(sizeof(uint8_t));
            *reason_ptr = hci_event_disconnection_complete_get_reason(packet);                    

            ctrl_ev_t ev = ctrl_make_event(CTRL_EV_DISCONNECTED, reason_ptr);
            queue_try_add(ctrl_ev_w_queue, &ev);
            break;
        }

        default:
            break;
    }
}

void bt_init(queue_t *write_queue, queue_t *read_queue) {
    ctrl_ev_w_queue = write_queue;
    ctrl_ev_r_queue = read_queue;

    l2cap_init();

    avrcp_init();
    avrcp_register_packet_handler(&avrcp_packet_handler);

    avrcp_controller_init();
    avrcp_controller_register_packet_handler(&avrcp_packet_handler);

    avrcp_target_init();
    avrcp_target_register_packet_handler(&avrcp_packet_handler);

    sdp_init();
    
    memset(sdp_avrcp_controller_service_buffer, 0, sizeof(sdp_avrcp_controller_service_buffer));
    uint16_t supported_features = AVRCP_FEATURE_MASK_CATEGORY_MONITOR_OR_AMPLIFIER;
    avrcp_controller_create_sdp_record(sdp_avrcp_controller_service_buffer, 0x10001, supported_features, "AVRCP Controller", "AVRCP");
    sdp_register_service(sdp_avrcp_controller_service_buffer);
    
    // Register Device ID (PnP) service SDP record
    memset(device_id_sdp_service_buffer, 0, sizeof(device_id_sdp_service_buffer));
    device_id_create_sdp_record(device_id_sdp_service_buffer, 0x10002, DEVICE_ID_VENDOR_ID_SOURCE_BLUETOOTH, BLUETOOTH_COMPANY_ID_BLUEKITCHEN_GMBH, 1, 1);
    sdp_register_service(device_id_sdp_service_buffer);

    gap_set_local_name("Pico Remote Control 00:00:00:00:00:00");

    // uint32_t class_of_device = 0x20050C;
    // uint32_t class_of_device = 0x200418;
    uint32_t class_of_device = 0x200408;
    gap_set_class_of_device(class_of_device);
    

    gap_ssp_set_io_capability(SSP_IO_CAPABILITY_NO_INPUT_NO_OUTPUT);

    // allow for role switch in general and sniff mode
    gap_set_default_link_policy_settings(LM_LINK_POLICY_ENABLE_ROLE_SWITCH | LM_LINK_POLICY_ENABLE_SNIFF_MODE);

    // allow for role switch on outgoing connections - this allows A2DP Source, e.g. smartphone, to become master when we re-connect to it
    gap_set_allow_role_switch(true);
    
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

#ifdef BT_DEBUG_MODE
    hci_dump_init(hci_dump_embedded_stdout_get_instance());
#endif

    hci_power_control(HCI_POWER_ON);
}

void bt_process_queue() {
    ctrl_ev_t ev;
    
    while (queue_try_remove(ctrl_ev_r_queue, &ev)) {
        switch (ev.type) {
            case CTRL_EV_MAKE_DISCOVERABLE: {
                uint8_t state = *(uint8_t *) ev.data;
                free(ev.data);

                DEBUG("Making discoverable %d\n", state);
                gap_discoverable_control(state);
                gap_connectable_control(state);
                break;
            }

            case CTRL_EV_REQUEST_CONNECTION: {
                if (!avrcp_cid) {
                    DEBUG("-------------CONNECTING-------------------\n");
                    uint8_t result = avrcp_connect(remote_addr, &avrcp_cid);
                    if (result != ERROR_CODE_SUCCESS) {
                        uint8_t *status_ptr = malloc(sizeof(uint8_t));
                        *status_ptr = result;

                        ctrl_ev_t con_failed_ev = ctrl_make_event(CTRL_EV_REQ_CONNECTION_FAILED, status_ptr);
                        queue_try_add(ctrl_ev_w_queue, &con_failed_ev);
                    }
                }
                break;
            }
                
            case CTRL_EV_REQUEST_TOGGLE_PLAY:
                if (playing_state) {
                    DEBUG("Requesting toggle play/pause %d\n", avrcp_controller_pause(avrcp_cid));
                    playing_state = 0;
                } else {
                    DEBUG("Requesting toggle play/pause %d\n", avrcp_controller_play(avrcp_cid));
                    playing_state = 1;
                }
            break;

            case CTRL_EV_REQUEST_VOL_UP:
                DEBUG("Requesting vol up %d\n", avrcp_controller_volume_up(avrcp_cid));
            break;
            case CTRL_EV_REQUEST_VOL_DOWN:
                // DEBUG("Requesting vol down %d\n", avrcp_controller_volume_down(avrcp_cid));
                DEBUG("Requesting vol down %d\n", avrcp_controller_mute(avrcp_cid));
                // DEBUG("Requesting vol down %d\n", avrcp_controller_set_absolute_volume(avrcp_cid, 0));
            break;
        }
    }
}