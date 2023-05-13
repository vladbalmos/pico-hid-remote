#include <stdio.h>
#include "btstack.h"
#include "debug.h"

void hci_event_code_str(uint8_t event_code, char* output) {
    switch(event_code) {
        case GAP_EVENT_PAIRING_STARTED:
            strcpy(output, "GAP_EVENT_PAIRING_STARTED");
            break;
        case GAP_EVENT_PAIRING_COMPLETE:
            strcpy(output, "GAP_EVENT_PAIRING_COMPLETE");
            break;
        case GAP_EVENT_SECURITY_LEVEL:
            strcpy(output, "GAP_EVENT_SECURITY_LEVEL");
            break;
        case BTSTACK_EVENT_STATE:
            strcpy(output, "BTSTACK_EVENT_STATE");
            break;
        case BTSTACK_EVENT_SCAN_MODE_CHANGED:
            strcpy(output, "BTSTACK_EVENT_SCAN_MODE_CHANGED");
            break;
        case BTSTACK_EVENT_NR_CONNECTIONS_CHANGED:
            strcpy(output, "BTSTACK_EVENT_NR_CONNECTIONS_CHANGED");
            break;
        case HCI_EVENT_NOP:
            strcpy(output, "HCI_EVENT_NOP");
            break;
        case HCI_EVENT_INQUIRY_COMPLETE:
            strcpy(output, "HCI_EVENT_INQUIRY_COMPLETE");
            break;
        case HCI_EVENT_INQUIRY_RESULT:
            strcpy(output, "HCI_EVENT_INQUIRY_RESULT");
            break;
        case HCI_EVENT_CONNECTION_COMPLETE:
            strcpy(output, "HCI_EVENT_CONNECTION_COMPLETE");
            break;
        case HCI_EVENT_CONNECTION_REQUEST:
            strcpy(output, "HCI_EVENT_CONNECTION_REQUEST");
            break;
        case HCI_EVENT_DISCONNECTION_COMPLETE:
            strcpy(output, "HCI_EVENT_DISCONNECTION_COMPLETE");
            break;
        case HCI_EVENT_AUTHENTICATION_COMPLETE_EVENT:
            strcpy(output, "HCI_EVENT_AUTHENTICATION_COMPLETE_EVENT");
            break;
        case HCI_EVENT_REMOTE_NAME_REQUEST_COMPLETE:
            strcpy(output, "HCI_EVENT_REMOTE_NAME_REQUEST_COMPLETE");
            break;
        case HCI_EVENT_ENCRYPTION_CHANGE:
            strcpy(output, "HCI_EVENT_ENCRYPTION_CHANGE");
            break;
        case HCI_EVENT_CHANGE_CONNECTION_LINK_KEY_COMPLETE:
            strcpy(output, "HCI_EVENT_CHANGE_CONNECTION_LINK_KEY_COMPLETE");
            break;
        case HCI_EVENT_MASTER_LINK_KEY_COMPLETE:
            strcpy(output, "HCI_EVENT_MASTER_LINK_KEY_COMPLETE");
            break;
        case HCI_EVENT_READ_REMOTE_SUPPORTED_FEATURES_COMPLETE:
            strcpy(output, "HCI_EVENT_READ_REMOTE_SUPPORTED_FEATURES_COMPLETE");
            break;
        case HCI_EVENT_READ_REMOTE_VERSION_INFORMATION_COMPLETE:
            strcpy(output, "HCI_EVENT_READ_REMOTE_VERSION_INFORMATION_COMPLETE");
            break;
        case HCI_EVENT_QOS_SETUP_COMPLETE:
            strcpy(output, "HCI_EVENT_QOS_SETUP_COMPLETE");
            break;
        case HCI_EVENT_COMMAND_COMPLETE:
            strcpy(output, "HCI_EVENT_COMMAND_COMPLETE");
            break;
        case HCI_EVENT_COMMAND_STATUS:
            strcpy(output, "HCI_EVENT_COMMAND_STATUS");
            break;
        case HCI_EVENT_HARDWARE_ERROR:
            strcpy(output, "HCI_EVENT_HARDWARE_ERROR");
            break;
        case HCI_EVENT_FLUSH_OCCURRED:
            strcpy(output, "HCI_EVENT_FLUSH_OCCURRED");
            break;
        case HCI_EVENT_ROLE_CHANGE:
            strcpy(output, "HCI_EVENT_ROLE_CHANGE");
            break;
        case HCI_EVENT_NUMBER_OF_COMPLETED_PACKETS:
            strcpy(output, "HCI_EVENT_NUMBER_OF_COMPLETED_PACKETS");
            break;
        case HCI_EVENT_MODE_CHANGE:
            strcpy(output, "HCI_EVENT_MODE_CHANGE");
            break;
        case HCI_EVENT_RETURN_LINK_KEYS:
            strcpy(output, "HCI_EVENT_RETURN_LINK_KEYS");
            break;
        case HCI_EVENT_PIN_CODE_REQUEST:
            strcpy(output, "HCI_EVENT_PIN_CODE_REQUEST");
            break;
        case HCI_EVENT_LINK_KEY_REQUEST:
            strcpy(output, "HCI_EVENT_LINK_KEY_REQUEST");
            break;
        case HCI_EVENT_LINK_KEY_NOTIFICATION:
            strcpy(output, "HCI_EVENT_LINK_KEY_NOTIFICATION");
            break;
        case HCI_EVENT_LOOPBACK_COMMAND:
            strcpy(output, "HCI_EVENT_LOOPBACK_COMMAND");
            break;
        case HCI_EVENT_DATA_BUFFER_OVERFLOW:
            strcpy(output, "HCI_EVENT_DATA_BUFFER_OVERFLOW");
            break;
        case HCI_EVENT_MAX_SLOTS_CHANGED:
            strcpy(output, "HCI_EVENT_MAX_SLOTS_CHANGED");
            break;
        case HCI_EVENT_READ_CLOCK_OFFSET_COMPLETE:
            strcpy(output, "HCI_EVENT_READ_CLOCK_OFFSET_COMPLETE");
            break;
        case HCI_EVENT_CONNECTION_PACKET_TYPE_CHANGED:
            strcpy(output, "HCI_EVENT_CONNECTION_PACKET_TYPE_CHANGED");
            break;
        case HCI_EVENT_QOS_VIOLATION:
            strcpy(output, "HCI_EVENT_QOS_VIOLATION");
            break;
        case HCI_EVENT_PAGE_SCAN_REPETITION_MODE_CHANGE:
            strcpy(output, "HCI_EVENT_PAGE_SCAN_REPETITION_MODE_CHANGE");
            break;
        case HCI_EVENT_FLOW_SPECIFICATION_COMPLETE:
            strcpy(output, "HCI_EVENT_FLOW_SPECIFICATION_COMPLETE");
            break;
        case HCI_EVENT_INQUIRY_RESULT_WITH_RSSI:
            strcpy(output, "HCI_EVENT_INQUIRY_RESULT_WITH_RSSI");
            break;
        case HCI_EVENT_READ_REMOTE_EXTENDED_FEATURES_COMPLETE:
            strcpy(output, "HCI_EVENT_READ_REMOTE_EXTENDED_FEATURES_COMPLETE");
            break;
        case HCI_EVENT_SYNCHRONOUS_CONNECTION_COMPLETE:
            strcpy(output, "HCI_EVENT_SYNCHRONOUS_CONNECTION_COMPLETE");
            break;
        case HCI_EVENT_SYNCHRONOUS_CONNECTION_CHANGED:
            strcpy(output, "HCI_EVENT_SYNCHRONOUS_CONNECTION_CHANGED");
            break;
        case HCI_EVENT_SNIFF_SUBRATING:
            strcpy(output, "HCI_EVENT_SNIFF_SUBRATING");
            break;
        case HCI_EVENT_EXTENDED_INQUIRY_RESPONSE:
            strcpy(output, "HCI_EVENT_EXTENDED_INQUIRY_RESPONSE");
            break;
        case HCI_EVENT_ENCRYPTION_KEY_REFRESH_COMPLETE:
            strcpy(output, "HCI_EVENT_ENCRYPTION_KEY_REFRESH_COMPLETE");
            break;
        case HCI_EVENT_IO_CAPABILITY_REQUEST:
            strcpy(output, "HCI_EVENT_IO_CAPABILITY_REQUEST");
            break;
        case HCI_EVENT_IO_CAPABILITY_RESPONSE:
            strcpy(output, "HCI_EVENT_IO_CAPABILITY_RESPONSE");
            break;
        case HCI_EVENT_USER_CONFIRMATION_REQUEST:
            strcpy(output, "HCI_EVENT_USER_CONFIRMATION_REQUEST");
            break;
        case HCI_EVENT_USER_PASSKEY_REQUEST:
            strcpy(output, "HCI_EVENT_USER_PASSKEY_REQUEST");
            break;
        case HCI_EVENT_REMOTE_OOB_DATA_REQUEST:
            strcpy(output, "HCI_EVENT_REMOTE_OOB_DATA_REQUEST");
            break;
        case HCI_EVENT_SIMPLE_PAIRING_COMPLETE:
            strcpy(output, "HCI_EVENT_SIMPLE_PAIRING_COMPLETE");
            break;
        case HCI_EVENT_LINK_SUPERVISION_TIMEOUT_CHANGED:
            strcpy(output, "HCI_EVENT_LINK_SUPERVISION_TIMEOUT_CHANGED");
            break;
        case HCI_EVENT_ENHANCED_FLUSH_COMPLETE:
            strcpy(output, "HCI_EVENT_ENHANCED_FLUSH_COMPLETE");
            break;
        case HCI_EVENT_USER_PASSKEY_NOTIFICATION:
            strcpy(output, "HCI_EVENT_USER_PASSKEY_NOTIFICATION");
            break;
        case HCI_EVENT_KEYPRESS_NOTIFICATION:
            strcpy(output, "HCI_EVENT_KEYPRESS_NOTIFICATION");
            break;
        case HCI_EVENT_REMOTE_HOST_SUPPORTED_FEATURES:
            strcpy(output, "HCI_EVENT_REMOTE_HOST_SUPPORTED_FEATURES");
            break;
        case HCI_EVENT_LE_META:
            strcpy(output, "HCI_EVENT_LE_META");
            break;
        case HCI_EVENT_NUMBER_OF_COMPLETED_DATA_BLOCKS:
            strcpy(output, "HCI_EVENT_NUMBER_OF_COMPLETED_DATA_BLOCKS");
            break;
        case HCI_EVENT_ENCRYPTION_CHANGE_V2:
            strcpy(output, "HCI_EVENT_ENCRYPTION_CHANGE_V2");
            break;
        case HCI_EVENT_VENDOR_SPECIFIC:
            strcpy(output, "HCI_EVENT_VENDOR_SPECIFIC");
            break;
        case HCI_EVENT_TRANSPORT_SLEEP_MODE:
            strcpy(output, "HCI_EVENT_TRANSPORT_SLEEP_MODE");
            break;
        case HCI_EVENT_TRANSPORT_USB_INFO:
            strcpy(output, "HCI_EVENT_TRANSPORT_USB_INFO");
            break;
        case HCI_EVENT_TRANSPORT_READY:
            strcpy(output, "HCI_EVENT_TRANSPORT_READY");
            break;
        case HCI_EVENT_TRANSPORT_PACKET_SENT:
            strcpy(output, "HCI_EVENT_TRANSPORT_PACKET_SENT");
            break;
        case HCI_EVENT_BIS_CAN_SEND_NOW:
            strcpy(output, "HCI_EVENT_BIS_CAN_SEND_NOW");
            break;
        case HCI_EVENT_CIS_CAN_SEND_NOW:
            strcpy(output, "HCI_EVENT_CIS_CAN_SEND_NOW");
            break;
        case HCI_EVENT_SCO_CAN_SEND_NOW:
            strcpy(output, "HCI_EVENT_SCO_CAN_SEND_NOW");
            break;
        case HCI_EVENT_META_GAP:
            strcpy(output, "HCI_EVENT_META_GAP");
            break;
        case HCI_EVENT_HSP_META:
            strcpy(output, "HCI_EVENT_HSP_META");
            break;
        case HCI_EVENT_HFP_META:
            strcpy(output, "HCI_EVENT_HFP_META");
            break;
        case HCI_EVENT_ANCS_META:
            strcpy(output, "HCI_EVENT_ANCS_META");
            break;
        case HCI_EVENT_AVDTP_META:
            strcpy(output, "HCI_EVENT_AVDTP_META");
            break;
        case HCI_EVENT_AVRCP_META:
            strcpy(output, "HCI_EVENT_AVRCP_META");
            break;
        case HCI_EVENT_GOEP_META:
            strcpy(output, "HCI_EVENT_GOEP_META");
            break;
        case HCI_EVENT_PBAP_META:
            strcpy(output, "HCI_EVENT_PBAP_META");
            break;
        case HCI_EVENT_HID_META:
            strcpy(output, "HCI_EVENT_HID_META");
            break;
        case HCI_EVENT_A2DP_META:
            strcpy(output, "HCI_EVENT_A2DP_META");
            break;
        case HCI_EVENT_HIDS_META:
            strcpy(output, "HCI_EVENT_HIDS_META");
            break;
        case HCI_EVENT_GATTSERVICE_META:
            strcpy(output, "HCI_EVENT_GATTSERVICE_META");
            break;
        case HCI_EVENT_BIP_META:
            strcpy(output, "HCI_EVENT_BIP_META");
            break;
        case HCI_EVENT_MAP_META:
            strcpy(output, "HCI_EVENT_MAP_META");
            break;
        case HCI_EVENT_MESH_META:
            strcpy(output, "HCI_EVENT_MESH_META");
            break;
        default: {
            char unknown_packet[32];
            sprintf(unknown_packet, "UNKNOWN %x\n", event_code);
            strcpy(output, unknown_packet);
        }
    }
}