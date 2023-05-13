#include <stdio.h>
#include "btstack.h"
#include "debug.h"

static btstack_packet_callback_registration_t hci_event_callback_registration;

static uint16_t avrcp_cid = 0;
static bool     avrcp_connected = false;
static bd_addr_t remote_addr;
static bd_addr_t local_addr;

static uint8_t  sdp_avrcp_controller_service_buffer[200];
static uint8_t  device_id_sdp_service_buffer[100];


static void avrcp_packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    UNUSED(size);
    UNUSED(channel);

    bd_addr_t address;
    uint16_t local_cid;
    uint8_t  status = 0xFF;

    if (packet_type != HCI_EVENT_PACKET) {
        printf("not event packet\n");
        return;
    }

    uint8_t hci_event_type = hci_event_packet_get_type(packet);
    
    if (hci_event_type != HCI_EVENT_AVRCP_META) {
        printf("not meta packet\n");
        return;
    }
    
#ifdef BT_DEBUG_MODE
    char str_packet[64];
    hci_event_code_str(packet[2], str_packet);
    printf("AVR Packet %s\n", str_packet);
#endif

    switch (packet[2]) {
        case AVRCP_SUBEVENT_CONNECTION_ESTABLISHED: {
            local_cid = avrcp_subevent_connection_established_get_avrcp_cid(packet);
            status = avrcp_subevent_connection_established_get_status(packet);
            if (status != ERROR_CODE_SUCCESS){
                printf("AVRCP: Connection failed, status 0x%02x\n", status);
                avrcp_cid = 0;
                return;
            }
            
            avrcp_cid = local_cid;
            avrcp_connected = true;
            avrcp_controller_pause(avrcp_cid);
            avrcp_subevent_connection_established_get_bd_addr(packet, address);
            printf("AVRCP: Connected to %s, cid 0x%02x\n", bd_addr_to_str(address), avrcp_cid);
            return;
        }
        
        case AVRCP_SUBEVENT_CONNECTION_RELEASED:
            printf("AVRCP: Channel released: cid 0x%02x\n", avrcp_subevent_connection_released_get_avrcp_cid(packet));
            avrcp_cid = 0;
            avrcp_connected = false;
            return;
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

#ifdef BT_DEBUG_MODE
    char str_packet[64];
    hci_event_code_str(hci_event_type, str_packet);
    printf("Packet %s\n", str_packet);
#endif
    
    switch(hci_event_type){
        case BTSTACK_EVENT_STATE:
            if (btstack_event_state_get_state(packet) != HCI_STATE_WORKING) return;
            gap_local_bd_addr(local_addr);
            printf("BTstack up and running on %s.\n", bd_addr_to_str(local_addr));
            // uint8_t result = avrcp_connect(remote_addr, &avrcp_cid);
            // printf("Initial Conn result is: %x\n", result);
            break;
            
        case HCI_EVENT_PIN_CODE_REQUEST: {
            printf("Pin code request - using '0000'.\n");
            bd_addr_t address;
            hci_event_pin_code_request_get_bd_addr(packet, address);
            gap_pin_code_response(address, "0000");
            printf("Remote addr: %s\n", bd_addr_to_str(address));
            break;
        }

        case HCI_EVENT_USER_CONFIRMATION_REQUEST: {
            bd_addr_t address;
            printf("User Confirmation Request - auto-accepting.\n");
            hci_event_user_confirmation_request_get_bd_addr(packet, address);
            hci_send_cmd(&hci_user_confirmation_request_reply, address);
            printf("Remote addr: %s\n", bd_addr_to_str(address));
            break;
        }
                                                  
        case HCI_EVENT_SIMPLE_PAIRING_COMPLETE: {
            bd_addr_t address;
            hci_event_simple_pairing_complete_get_bd_addr(packet, address);
            printf("Remote addr: %s\n", bd_addr_to_str(address));
            if (!avrcp_cid) {
                uint8_t result = avrcp_connect(address, &avrcp_cid);
                printf("Pair Conn result is: %x\n", result);
            }
            break;
        }

        case HCI_EVENT_AUTHENTICATION_COMPLETE: {
            printf("Authentication complete.\n");
            break;
        }

        case HCI_EVENT_DISCONNECTION_COMPLETE:
            con_handle = hci_event_disconnection_complete_get_connection_handle(packet);
            printf("- Connection 0x%04x: disconnect, reason %02x\n", con_handle, hci_event_disconnection_complete_get_reason(packet));                    
            break;

        default:
            break;
    }
}

void bt_init() {
    l2cap_init();

    avrcp_init();
    avrcp_register_packet_handler(&avrcp_packet_handler);

    avrcp_controller_init();
    avrcp_controller_register_packet_handler(&avrcp_packet_handler);

    avrcp_target_init();
    avrcp_target_register_packet_handler(&avrcp_packet_handler);

    sdp_init();
    
    memset(sdp_avrcp_controller_service_buffer, 0, sizeof(sdp_avrcp_controller_service_buffer));
    uint16_t supported_features = AVRCP_FEATURE_MASK_CATEGORY_PLAYER_OR_RECORDER;
    avrcp_controller_create_sdp_record(sdp_avrcp_controller_service_buffer, 0x10001, supported_features, "AVRCP Controller", "Vlad");
    sdp_register_service(sdp_avrcp_controller_service_buffer);

    gap_set_local_name("Pico Remote Control 00:00:00:00:00:00");

    gap_discoverable_control(1);
    gap_connectable_control(1);

    uint32_t class_of_device = 0x20050C;
    gap_set_class_of_device(class_of_device);

    // allow for role switch in general and sniff mode
    gap_set_default_link_policy_settings( LM_LINK_POLICY_ENABLE_ROLE_SWITCH | LM_LINK_POLICY_ENABLE_SNIFF_MODE );

    // allow for role switch on outgoing connections - this allows A2DP Source, e.g. smartphone, to become master when we re-connect to it
    gap_set_allow_role_switch(true);
    
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

#ifdef BT_DEBUG_MODE
    hci_dump_init(hci_dump_embedded_stdout_get_instance());
#endif

    hci_power_control(HCI_POWER_ON);
    
}