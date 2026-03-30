#include "joycon_classic.h"

#include "app_bt_utils.h"
#include "cybsp_bt_config.h"
#include "cybt_platform_config.h"

#include "wiced_bt_stack.h"
#include "wiced_bt_dev.h"
#include "wiced_bt_cfg.h"
#include "wiced_bt_l2c.h"
#include "wiced_bt_sdp.h"
#include "wiced_bt_sdp_defs.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define JOYCON_HID_CONTROL_PSM      (0x0011u)
#define JOYCON_HID_INTERRUPT_PSM    (0x0013u)
#define JOYCON_SDP_DB_SIZE          (1024u)
#define JOYCON_HID_UUID_MASK        (1UL << BTM_EIR_UUID_SERVCLASS_HUMAN_INTERFACE)
#define COD_MAJOR_MASK              (0x1Fu)
#define COD_MAJOR_PERIPHERAL        (0x05u)

#define EIR_TYPE_COMPLETE_NAME      (0x09u)
#define EIR_TYPE_SHORT_NAME         (0x08u)

extern const cybt_platform_config_t cybsp_bt_platform_cfg;

static wiced_result_t app_bt_management_callback(wiced_bt_management_evt_t event, wiced_bt_management_evt_data_t *p_event_data);
static void app_bt_inquiry_result_cb(wiced_bt_dev_inquiry_scan_result_t *p_inquiry_result, uint8_t *p_eir_data);
static void app_bt_sdp_complete_cb(uint16_t sdp_result);
static void app_bt_remote_name_cb(wiced_bt_dev_remote_name_result_t *p_remote_name_result);

static void app_l2cap_connected_cb(wiced_bt_device_address_t bd_addr, uint16_t local_cid, uint16_t peer_mtu);
static void app_l2cap_disconnect_ind_cb(uint16_t local_cid, uint16_t reason, wiced_bool_t ack);
static void app_l2cap_disconnect_cfm_cb(uint16_t local_cid, uint16_t result);
static void app_l2cap_data_ind_cb(uint16_t local_cid, tDRB *p_drb);
static void app_l2cap_tx_complete_cb(uint16_t local_cid, void *p_data);

static void app_start_inquiry(void);
static void app_start_sdp(void);
static void app_open_hid_channels(void);
static void app_start_security(void);
static void app_try_send_enable_reports(void);
static void app_send_subcommand_03(void);
static wiced_bool_t app_name_looks_like_joycon(uint8_t *p_eir_data);
static wiced_bool_t app_is_joycon_candidate(const wiced_bt_dev_inquiry_scan_result_t *p_inquiry_result, uint8_t *p_eir_data);
static wiced_bool_t app_remote_name_looks_like_joycon(const uint8_t *p_name, uint16_t length);
static uint8_t app_ascii_tolower(uint8_t c);
static int joycon_log_printf(const char *format, ...);
static const char *app_hat_to_text(uint8_t hat);

#define printf joycon_log_printf

static wiced_bt_device_address_t joycon_bda = {0};
static wiced_bool_t joycon_bda_valid = WICED_FALSE;

static uint16_t control_cid = 0;
static uint16_t interrupt_cid = 0;
static uint16_t control_cid_pending = 0;
static uint16_t interrupt_cid_pending = 0;
static wiced_bool_t init_sent = WICED_FALSE;
static wiced_bool_t bt_enabled = WICED_FALSE;
static wiced_bool_t inquiry_requested = WICED_FALSE;
static uint16_t inquiry_retry_ticks = 0;
static wiced_bool_t inquiry_in_progress = WICED_FALSE;
static wiced_bool_t remote_name_in_progress = WICED_FALSE;
static wiced_bt_device_address_t remote_name_bda = {0};
static wiced_bool_t security_in_progress = WICED_FALSE;
static wiced_bt_device_link_keys_t paired_link_keys = {0};
static wiced_bool_t paired_link_keys_valid = WICED_FALSE;
static wiced_bt_local_identity_keys_t local_identity_keys = {0};
static wiced_bool_t local_identity_keys_valid = WICED_FALSE;
static wiced_bool_t joycon_paired = WICED_FALSE;
static wiced_bool_t input_only_mode = WICED_FALSE;
static wiced_bool_t force_log_output = WICED_FALSE;
static wiced_bool_t security_retry_requested = WICED_FALSE;
static uint16_t security_retry_ticks = 0;
static wiced_bool_t input_state_valid = WICED_FALSE;
static uint8_t last_btn0 = 0;
static uint8_t last_btn1 = 0;
static uint8_t last_hat = 0;
static uint8_t last_lx = 0;
static uint8_t last_ly = 0;
static uint8_t last_rx = 0;
static uint8_t last_ry = 0;

static uint8_t sdp_db_raw[JOYCON_SDP_DB_SIZE] = {0};
static wiced_bt_sdp_discovery_db_t *sdp_db = (wiced_bt_sdp_discovery_db_t *)sdp_db_raw;

static wiced_bt_l2cap_appl_information_t l2cap_cb = {
    .connected_cback = app_l2cap_connected_cb,
    .disconnect_indication_cback = app_l2cap_disconnect_ind_cb,
    .disconnect_confirm_cback = app_l2cap_disconnect_cfm_cb,
    .data_indication_cback = app_l2cap_data_ind_cb,
    .tx_complete_cback = app_l2cap_tx_complete_cb,
    .mtu = 672,
    .security_required = 0,
    .qos_present = 0,
    .flush_timeout_present = 0,
    .fcs_present = 0,
    .is_ob_only = 0,
    .qos = {0},
    .flush_timeout = 0,
    .fcr_allowed_modes = L2CAP_FCR_CHAN_OPT_BASIC,
    .fcr = {0},
    .fcs = 0
};

static const wiced_bt_cfg_rfcomm_t app_rfcomm_cfg = {
    .max_links = 0,
    .max_ports = 0
};

static const wiced_bt_cfg_avdt_t app_avdt_cfg = {
    .max_links = 0,
    .max_seps = 0
};

static const wiced_bt_cfg_avrc_t app_avrc_cfg = {
    .max_links = 0
};

static const wiced_bt_cfg_br_t app_br_cfg = {
    .br_max_simultaneous_links = 1,
    .br_max_rx_pdu_size = 1024,
    .device_class = {0, 0, 0},
    .rfcomm_cfg = app_rfcomm_cfg,
    .avdt_cfg = app_avdt_cfg,
    .avrc_cfg = app_avrc_cfg
};

static const wiced_bt_cfg_ble_scan_settings_t app_ble_scan_cfg = {
    .scan_mode = BTM_BLE_SCAN_MODE_ACTIVE,
    .high_duty_scan_interval = WICED_BT_CFG_DEFAULT_HIGH_DUTY_SCAN_INTERVAL,
    .high_duty_scan_window = WICED_BT_CFG_DEFAULT_HIGH_DUTY_SCAN_WINDOW,
    .high_duty_scan_duration = 0,
    .low_duty_scan_interval = WICED_BT_CFG_DEFAULT_LOW_DUTY_SCAN_INTERVAL,
    .low_duty_scan_window = WICED_BT_CFG_DEFAULT_LOW_DUTY_SCAN_WINDOW,
    .low_duty_scan_duration = 0,
    .high_duty_conn_scan_interval = WICED_BT_CFG_DEFAULT_HIGH_DUTY_CONN_SCAN_INTERVAL,
    .high_duty_conn_scan_window = WICED_BT_CFG_DEFAULT_HIGH_DUTY_CONN_SCAN_WINDOW,
    .high_duty_conn_duration = 0,
    .low_duty_conn_scan_interval = WICED_BT_CFG_DEFAULT_LOW_DUTY_CONN_SCAN_INTERVAL,
    .low_duty_conn_scan_window = WICED_BT_CFG_DEFAULT_LOW_DUTY_CONN_SCAN_WINDOW,
    .low_duty_conn_duration = 0,
    .conn_min_interval = WICED_BT_CFG_DEFAULT_CONN_MIN_INTERVAL,
    .conn_max_interval = WICED_BT_CFG_DEFAULT_CONN_MAX_INTERVAL,
    .conn_latency = WICED_BT_CFG_DEFAULT_CONN_LATENCY,
    .conn_supervision_timeout = WICED_BT_CFG_DEFAULT_CONN_SUPERVISION_TIMEOUT
};

static const wiced_bt_cfg_ble_advert_settings_t app_ble_adv_cfg = {
    .channel_map = (BTM_BLE_ADVERT_CHNL_37 | BTM_BLE_ADVERT_CHNL_38 | BTM_BLE_ADVERT_CHNL_39),
    .high_duty_min_interval = WICED_BT_CFG_DEFAULT_HIGH_DUTY_ADV_MIN_INTERVAL,
    .high_duty_max_interval = WICED_BT_CFG_DEFAULT_HIGH_DUTY_ADV_MAX_INTERVAL,
    .high_duty_duration = 0,
    .low_duty_min_interval = WICED_BT_CFG_DEFAULT_LOW_DUTY_ADV_MIN_INTERVAL,
    .low_duty_max_interval = WICED_BT_CFG_DEFAULT_LOW_DUTY_ADV_MAX_INTERVAL,
    .low_duty_duration = 0,
    .high_duty_directed_min_interval = WICED_BT_CFG_DEFAULT_HIGH_DUTY_DIRECTED_ADV_MIN_INTERVAL,
    .high_duty_directed_max_interval = WICED_BT_CFG_DEFAULT_HIGH_DUTY_DIRECTED_ADV_MAX_INTERVAL,
    .low_duty_directed_min_interval = WICED_BT_CFG_DEFAULT_LOW_DUTY_DIRECTED_ADV_MIN_INTERVAL,
    .low_duty_directed_max_interval = WICED_BT_CFG_DEFAULT_LOW_DUTY_DIRECTED_ADV_MAX_INTERVAL,
    .low_duty_directed_duration = 0,
    .high_duty_nonconn_min_interval = WICED_BT_CFG_DEFAULT_HIGH_DUTY_NONCONN_ADV_MIN_INTERVAL,
    .high_duty_nonconn_max_interval = WICED_BT_CFG_DEFAULT_HIGH_DUTY_NONCONN_ADV_MAX_INTERVAL,
    .high_duty_nonconn_duration = 0,
    .low_duty_nonconn_min_interval = WICED_BT_CFG_DEFAULT_LOW_DUTY_NONCONN_ADV_MIN_INTERVAL,
    .low_duty_nonconn_max_interval = WICED_BT_CFG_DEFAULT_LOW_DUTY_NONCONN_ADV_MAX_INTERVAL,
    .low_duty_nonconn_duration = 0
};

static const wiced_bt_cfg_ble_t app_ble_cfg = {
    .ble_max_simultaneous_links = 1,
    .ble_max_rx_pdu_size = 65,
    .appearance = 0,
    .rpa_refresh_timeout = 0,
    .host_addr_resolution_db_size = 5,
    .p_ble_scan_cfg = &app_ble_scan_cfg,
    .p_ble_advert_cfg = &app_ble_adv_cfg,
    .default_ble_power_level = 0
};

static const wiced_bt_cfg_gatt_t app_gatt_cfg = {
    .max_db_service_modules = 0,
    .max_eatt_bearers = 0
};

static const wiced_bt_cfg_l2cap_application_t app_l2cap_cfg = {
    .max_app_l2cap_psms = 2,
    .max_app_l2cap_channels = 2,
    .max_app_l2cap_le_fixed_channels = 0,
    .max_app_l2cap_br_edr_ertm_chnls = 0,
    .max_app_l2cap_br_edr_ertm_tx_win = 0
};

static const wiced_bt_cfg_settings_t app_bt_cfg_settings = {
    .device_name = (uint8_t *)"PSoC6-JoyConHost",
    .security_required = BTM_SEC_BEST_EFFORT,
    .p_br_cfg = &app_br_cfg,
    .p_ble_cfg = &app_ble_cfg,
    .p_gatt_cfg = &app_gatt_cfg,
    .p_isoc_cfg = NULL,
    .p_l2cap_app_cfg = &app_l2cap_cfg
};

static int joycon_log_printf(const char *format, ...)
{
    int result;
    va_list args;

    if ((input_only_mode == WICED_TRUE) && (force_log_output == WICED_FALSE))
    {
        return 0;
    }

    va_start(args, format);
    result = vprintf(format, args);
    va_end(args);

    return result;
}

static const char *app_hat_to_text(uint8_t hat)
{
    switch (hat & 0x0Fu)
    {
        case 0x00: return "UP";
        case 0x01: return "UP-RIGHT";
        case 0x02: return "RIGHT";
        case 0x03: return "DOWN-RIGHT";
        case 0x04: return "DOWN";
        case 0x05: return "DOWN-LEFT";
        case 0x06: return "LEFT";
        case 0x07: return "UP-LEFT";
        case 0x08: return "CENTER";
        default:   return "UNKNOWN";
    }
}

void joycon_classic_init(void)
{
    wiced_result_t result;
    int32_t mem_required;

    printf("[JOYCON] cybt_platform_config_init\r\n");
    cybt_platform_config_init(&cybsp_bt_platform_cfg);

    mem_required = wiced_bt_stack_get_dynamic_memory_size_for_config(&app_bt_cfg_settings);
    printf("[JOYCON] bt mem required=%ld, pool=%lu\r\n", (long)mem_required, (unsigned long)cybsp_bt_platform_cfg.task_mem_pool_size);

    result = wiced_bt_stack_init(app_bt_management_callback, &app_bt_cfg_settings);
    printf("[JOYCON] wiced_bt_stack_init result=%d\r\n", (int)result);
}

void joycon_classic_process(void)
{
    if (inquiry_retry_ticks > 0)
    {
        inquiry_retry_ticks--;
    }

    if (security_retry_ticks > 0)
    {
        security_retry_ticks--;
    }

    if (bt_enabled && security_retry_requested && (security_retry_ticks == 0))
    {
        security_retry_requested = WICED_FALSE;
        app_start_security();
    }

    if (bt_enabled && inquiry_requested && (inquiry_retry_ticks == 0))
    {
        app_start_inquiry();
    }
}

static wiced_result_t app_bt_management_callback(wiced_bt_management_evt_t event, wiced_bt_management_evt_data_t *p_event_data)
{
    printf("[JOYCON] mgmt event: 0x%04X (%s)\r\n", (unsigned int)event, get_bt_event_name(event));

    switch (event)
    {
        case BTM_ENABLED_EVT:
            if (p_event_data->enabled.status == WICED_BT_SUCCESS)
            {
                bt_enabled = WICED_TRUE;
                inquiry_requested = WICED_TRUE;
                inquiry_retry_ticks = 1;
                printf("BT stack enabled, inquiry scheduled...\r\n");
                return WICED_BT_SUCCESS;
            }
            printf("BT stack init failed: %d\r\n", p_event_data->enabled.status);
            return WICED_BT_ERROR;

        case BTM_SECURITY_REQUEST_EVT:
            return WICED_BT_SUCCESS;

        case BTM_LOCAL_IDENTITY_KEYS_UPDATE_EVT:
            memcpy(&local_identity_keys, &p_event_data->local_identity_keys_update, sizeof(local_identity_keys));
            local_identity_keys_valid = WICED_TRUE;
            return WICED_BT_SUCCESS;

        case BTM_LOCAL_IDENTITY_KEYS_REQUEST_EVT:
            if (local_identity_keys_valid)
            {
                memcpy(&p_event_data->local_identity_keys_request, &local_identity_keys, sizeof(local_identity_keys));
                return WICED_BT_SUCCESS;
            }
            return WICED_BT_ERROR;

        case BTM_PAIRED_DEVICE_LINK_KEYS_UPDATE_EVT:
            memcpy(&paired_link_keys, &p_event_data->paired_device_link_keys_update, sizeof(paired_link_keys));
            paired_link_keys_valid = WICED_TRUE;
            if (joycon_bda_valid &&
                (memcmp(paired_link_keys.bd_addr, joycon_bda, BD_ADDR_LEN) == 0))
            {
                joycon_paired = WICED_TRUE;
            }
            printf("Stored BR/EDR link keys for ");
            print_bd_address(paired_link_keys.bd_addr);
            return WICED_BT_SUCCESS;

        case BTM_PAIRED_DEVICE_LINK_KEYS_REQUEST_EVT:
            if (paired_link_keys_valid &&
                (memcmp(p_event_data->paired_device_link_keys_request.bd_addr,
                        paired_link_keys.bd_addr,
                        BD_ADDR_LEN) == 0))
            {
                memcpy(&p_event_data->paired_device_link_keys_request,
                       &paired_link_keys,
                       sizeof(paired_link_keys));
                printf("Restored BR/EDR link keys for ");
                print_bd_address(p_event_data->paired_device_link_keys_request.bd_addr);
                return WICED_BT_SUCCESS;
            }
            return WICED_BT_ERROR;

        case BTM_USER_CONFIRMATION_REQUEST_EVT:
            printf("User confirmation request: just_works=%u, value=%lu\r\n",
                   (unsigned int)p_event_data->user_confirmation_request.just_works,
                   (unsigned long)p_event_data->user_confirmation_request.numeric_value);
            wiced_bt_dev_confirm_req_reply(WICED_BT_SUCCESS, p_event_data->user_confirmation_request.bd_addr);
            return WICED_BT_SUCCESS;

        case BTM_PIN_REQUEST_EVT:
        {
            uint8_t legacy_pin[] = {'0', '0', '0', '0'};
            printf("PIN request received, replying with default PIN\r\n");
            wiced_bt_dev_pin_code_reply(*(p_event_data->pin_request.bd_addr), WICED_BT_SUCCESS, (uint8_t)sizeof(legacy_pin), legacy_pin);
            return WICED_BT_SUCCESS;
        }

        case BTM_ENCRYPTION_STATUS_EVT:
            if ((p_event_data->encryption_status.bd_addr != NULL) &&
                (memcmp(p_event_data->encryption_status.bd_addr, joycon_bda, BD_ADDR_LEN) == 0))
            {
                printf("Encryption status for Joy-Con: result=%d\r\n", (int)p_event_data->encryption_status.result);
                if (p_event_data->encryption_status.result == WICED_BT_SUCCESS)
                {
                    security_in_progress = WICED_FALSE;
                    security_retry_requested = WICED_FALSE;
                    app_open_hid_channels();
                    return WICED_BT_SUCCESS;
                }

                if ((p_event_data->encryption_status.result == WICED_BT_PENDING) ||
                    (p_event_data->encryption_status.result == WICED_BT_BUSY))
                {
                    security_in_progress = WICED_FALSE;
                    security_retry_requested = WICED_TRUE;
                    security_retry_ticks = 10;
                    return WICED_BT_SUCCESS;
                }

                if (p_event_data->encryption_status.result == WICED_BT_NO_RESOURCES)
                {
                    security_in_progress = WICED_FALSE;
                    security_retry_requested = WICED_TRUE;
                    security_retry_ticks = 20;
                    return WICED_BT_SUCCESS;
                }

                if (p_event_data->encryption_status.result == WICED_BT_WRONG_MODE)
                {
                    security_in_progress = WICED_FALSE;
                    printf("Encryption skipped (no active ACL), opening HID channels\r\n");
                    security_retry_requested = WICED_FALSE;
                    app_open_hid_channels();
                    return WICED_BT_SUCCESS;
                }

                security_in_progress = WICED_FALSE;
                if (joycon_paired || paired_link_keys_valid)
                {
                    printf("Encryption status non-fatal after pairing, opening HID channels\r\n");
                    joycon_paired = WICED_TRUE;
                    security_retry_requested = WICED_FALSE;
                    app_open_hid_channels();
                    return WICED_BT_SUCCESS;
                }

                inquiry_requested = WICED_TRUE;
                inquiry_retry_ticks = 20;
                return WICED_BT_SUCCESS;
            }
            return WICED_BT_SUCCESS;

        case BTM_PAIRING_COMPLETE_EVT:
            if (p_event_data->pairing_complete.transport == BT_TRANSPORT_BR_EDR)
            {
                uint8_t br_status = p_event_data->pairing_complete.pairing_complete_info.br_edr.status;
                printf("BR/EDR pairing status: %u\r\n", (unsigned int)br_status);

                if ((p_event_data->pairing_complete.bd_addr != NULL) &&
                    (memcmp(p_event_data->pairing_complete.bd_addr, joycon_bda, BD_ADDR_LEN) == 0))
                {
                    if (br_status == 0u)
                    {
                        security_in_progress = WICED_FALSE;
                        joycon_paired = WICED_TRUE;
                        security_retry_requested = WICED_FALSE;
                        printf("Pairing complete, requesting BR/EDR encryption\r\n");
                        app_start_security();
                    }
                    else
                    {
                        joycon_paired = WICED_FALSE;
                        security_in_progress = WICED_FALSE;
                        security_retry_requested = WICED_FALSE;
                        inquiry_requested = WICED_TRUE;
                        inquiry_retry_ticks = 20;
                    }
                }
            }
            else
            {
                printf("LE pairing status: %d\r\n", p_event_data->pairing_complete.pairing_complete_info.ble.reason);
            }
            return WICED_BT_SUCCESS;

        case BTM_SECURITY_FAILED_EVT:
            printf("Security failed: status=%d hci_status=0x%02X\r\n",
                   (int)p_event_data->security_failed.status,
                   (unsigned int)p_event_data->security_failed.hci_status);
            if (memcmp(p_event_data->security_failed.bd_addr, joycon_bda, BD_ADDR_LEN) == 0)
            {
                security_in_progress = WICED_FALSE;
                security_retry_requested = WICED_FALSE;
                inquiry_requested = WICED_TRUE;
                inquiry_retry_ticks = 20;
            }
            return WICED_BT_SUCCESS;

        default:
            return WICED_BT_SUCCESS;
    }
}

static void app_start_inquiry(void)
{
    wiced_bt_dev_inq_parms_t inq = {0};
    wiced_result_t inq_res;

    if (inquiry_in_progress)
    {
        return;
    }

    inquiry_requested = WICED_FALSE;
    inq.mode = BTM_GENERAL_INQUIRY;
    inq.duration = 5;
    inq.filter_cond_type = BTM_CLR_INQUIRY_FILTER;

    printf("Starting BR/EDR inquiry (5s)...\r\n");

    inq_res = wiced_bt_start_inquiry(&inq, app_bt_inquiry_result_cb);
    if ((inq_res == WICED_BT_SUCCESS) || (inq_res == WICED_BT_PENDING))
    {
        inquiry_in_progress = WICED_TRUE;
        return;
    }

    if (inq_res == WICED_BT_BUSY)
    {
        printf("Inquiry busy (res=%d), backing off...\r\n", (int)inq_res);
        inquiry_requested = WICED_TRUE;
        inquiry_retry_ticks = 50;
        return;
    }

    printf("Failed to start inquiry (res=%d), retrying...\r\n", (int)inq_res);
    if ((inq_res == WICED_BT_UNSUPPORTED) || (inq_res == WICED_BT_WRONG_MODE))
    {
        printf("[JOYCON] BR/EDR inquiry unsupported in current BT stack mode (check Bluetooth Configurator stack/device selection).\r\n");
    }
    inquiry_requested = WICED_TRUE;
    inquiry_retry_ticks = 100;
}

static void app_bt_inquiry_result_cb(wiced_bt_dev_inquiry_scan_result_t *p_inquiry_result, uint8_t *p_eir_data)
{
    wiced_bool_t name_match;
    wiced_bool_t hid_uuid_match;
    wiced_bool_t cod_peripheral;

    if (p_inquiry_result == NULL)
    {
        inquiry_in_progress = WICED_FALSE;

        if (remote_name_in_progress)
        {
            printf("Inquiry done, waiting for remote name response...\r\n");
            return;
        }

        if (!joycon_bda_valid)
        {
            printf("Inquiry done, Joy-Con not found. Retrying...\r\n");
            inquiry_requested = WICED_TRUE;
            inquiry_retry_ticks = 50;
        }
        return;
    }

    name_match = app_name_looks_like_joycon(p_eir_data);
    hid_uuid_match = ((p_inquiry_result->eir_uuid_mask & JOYCON_HID_UUID_MASK) != 0UL) ? WICED_TRUE : WICED_FALSE;
    cod_peripheral = (((uint8_t)(p_inquiry_result->dev_class[1] & COD_MAJOR_MASK)) == COD_MAJOR_PERIPHERAL) ? WICED_TRUE : WICED_FALSE;

    printf("Inquiry hit: ");
    print_bd_address(p_inquiry_result->remote_bd_addr);
    printf("  RSSI=%d, COD=%02X:%02X:%02X, EIR_HID=%u, name_match=%u\r\n",
           p_inquiry_result->rssi,
           p_inquiry_result->dev_class[0], p_inquiry_result->dev_class[1], p_inquiry_result->dev_class[2],
           (unsigned int)hid_uuid_match,
           (unsigned int)name_match);

    if (!app_is_joycon_candidate(p_inquiry_result, p_eir_data))
    {
        if (!cod_peripheral)
        {
            return;
        }

        if (remote_name_in_progress)
        {
            return;
        }

        memcpy(remote_name_bda, p_inquiry_result->remote_bd_addr, BD_ADDR_LEN);
        printf("Requesting remote name for candidate ");
        print_bd_address(remote_name_bda);
        wiced_result_t name_req_res = wiced_bt_dev_get_remote_name(remote_name_bda, app_bt_remote_name_cb);

        if ((name_req_res == WICED_BT_SUCCESS) || (name_req_res == WICED_BT_PENDING))
        {
            remote_name_in_progress = WICED_TRUE;
            return;
        }

        printf("Remote name request failed (res=%d)\r\n", (int)name_req_res);
        return;
    }
    else
    {
        printf("Joy-Con candidate accepted\r\n");
    }

    if (inquiry_in_progress)
    {
        (void)wiced_bt_cancel_inquiry();
        inquiry_in_progress = WICED_FALSE;
    }

    if (joycon_bda_valid && (memcmp(joycon_bda, p_inquiry_result->remote_bd_addr, BD_ADDR_LEN) == 0))
    {
        printf("Candidate already selected, waiting for SDP/L2CAP\r\n");
        return;
    }

    memcpy(joycon_bda, p_inquiry_result->remote_bd_addr, BD_ADDR_LEN);
    joycon_bda_valid = WICED_TRUE;

    printf("Joy-Con candidate found: ");
    print_bd_address(joycon_bda);
    app_start_sdp();
}

static wiced_bool_t app_is_joycon_candidate(const wiced_bt_dev_inquiry_scan_result_t *p_inquiry_result, uint8_t *p_eir_data)
{
    if ((p_inquiry_result == NULL) || (p_eir_data == NULL))
    {
        if ((p_inquiry_result != NULL) && ((p_inquiry_result->eir_uuid_mask & JOYCON_HID_UUID_MASK) != 0UL))
        {
            return WICED_TRUE;
        }
        return WICED_FALSE;
    }

    if (app_name_looks_like_joycon(p_eir_data))
    {
        return WICED_TRUE;
    }

    if ((p_inquiry_result->eir_uuid_mask & JOYCON_HID_UUID_MASK) != 0UL)
    {
        return WICED_TRUE;
    }

    return WICED_FALSE;
}

static uint8_t app_ascii_tolower(uint8_t c)
{
    if ((c >= (uint8_t)'A') && (c <= (uint8_t)'Z'))
    {
        return (uint8_t)(c + ((uint8_t)'a' - (uint8_t)'A'));
    }

    return c;
}

static wiced_bool_t app_remote_name_looks_like_joycon(const uint8_t *p_name, uint16_t length)
{
    static const uint8_t joycon_left_name[] = "joy-con (l)";
    static const uint8_t joycon_right_name[] = "joy-con (r)";
    uint16_t actual_len;

    if ((p_name == NULL) || (length == 0))
    {
        return WICED_FALSE;
    }

    actual_len = length;
    while ((actual_len > 0) && ((p_name[actual_len - 1] == (uint8_t)' ') || (p_name[actual_len - 1] == 0u)))
    {
        actual_len--;
    }

    if (actual_len != (uint16_t)(sizeof(joycon_left_name) - 1u))
    {
        return WICED_FALSE;
    }

    wiced_bool_t left_match = WICED_TRUE;
    wiced_bool_t right_match = WICED_TRUE;

    for (uint16_t i = 0; i < actual_len; i++)
    {
        uint8_t c = app_ascii_tolower(p_name[i]);

        if (c != joycon_left_name[i])
        {
            left_match = WICED_FALSE;
        }

        if (c != joycon_right_name[i])
        {
            right_match = WICED_FALSE;
        }

        if ((left_match == WICED_FALSE) && (right_match == WICED_FALSE))
        {
            return WICED_FALSE;
        }
    }

    return (left_match || right_match) ? WICED_TRUE : WICED_FALSE;
}

static void app_bt_remote_name_cb(wiced_bt_dev_remote_name_result_t *p_remote_name_result)
{
    uint16_t name_len;

    remote_name_in_progress = WICED_FALSE;

    if (p_remote_name_result == NULL)
    {
        printf("Remote name callback with NULL result\r\n");
        return;
    }

    if (p_remote_name_result->status != WICED_BT_SUCCESS)
    {
        printf("Remote name request failed for ");
        print_bd_address(p_remote_name_result->bd_addr);
        printf("  status=0x%04X\r\n", p_remote_name_result->status);

        if (!inquiry_in_progress)
        {
            inquiry_requested = WICED_TRUE;
            inquiry_retry_ticks = 5;
        }
        return;
    }

    name_len = p_remote_name_result->length;
    if (name_len > BD_NAME_LEN)
    {
        name_len = BD_NAME_LEN;
    }

    if (!app_remote_name_looks_like_joycon(p_remote_name_result->remote_bd_name, name_len))
    {
        if (!inquiry_in_progress)
        {
            inquiry_requested = WICED_TRUE;
            inquiry_retry_ticks = 5;
        }
        return;
    }

    printf("Remote name: %.*s\r\n", (int)name_len, (const char *)p_remote_name_result->remote_bd_name);

    memcpy(joycon_bda, p_remote_name_result->bd_addr, BD_ADDR_LEN);
    joycon_bda_valid = WICED_TRUE;

    if (inquiry_in_progress)
    {
        (void)wiced_bt_cancel_inquiry();
        inquiry_in_progress = WICED_FALSE;
    }

    printf("Joy-Con accepted by remote name: ");
    print_bd_address(joycon_bda);
    app_start_sdp();
}

static wiced_bool_t app_name_looks_like_joycon(uint8_t *p_eir_data)
{
    uint8_t name_len = 0;
    uint8_t *p_name = NULL;

    if (p_eir_data == NULL)
    {
        return WICED_FALSE;
    }

    p_name = wiced_bt_dev_read_oob_data(p_eir_data, EIR_TYPE_COMPLETE_NAME, &name_len);
    if (p_name == NULL)
    {
        p_name = wiced_bt_dev_read_oob_data(p_eir_data, EIR_TYPE_SHORT_NAME, &name_len);
    }

    if ((p_name == NULL) || (name_len == 0))
    {
        return WICED_FALSE;
    }

    if (name_len >= 7)
    {
        for (uint8_t i = 0; i <= (uint8_t)(name_len - 7); i++)
        {
            if ((p_name[i] == 'J') && (p_name[i + 1] == 'o') && (p_name[i + 2] == 'y') &&
                (p_name[i + 3] == '-') && (p_name[i + 4] == 'C') && (p_name[i + 5] == 'o') &&
                (p_name[i + 6] == 'n'))
            {
                return WICED_TRUE;
            }
        }
    }

    return WICED_FALSE;
}

static void app_start_sdp(void)
{
    wiced_bt_uuid_t uuid_list[1];
    uint16_t attr_list[] = {
        ATTR_ID_PROTOCOL_DESC_LIST,
        ATTR_ID_ADDITION_PROTO_DESC_LISTS,
        ATTR_ID_HID_DESCRIPTOR_LIST
    };

    memset(sdp_db_raw, 0, sizeof(sdp_db_raw));

    uuid_list[0].len = LEN_UUID_16;
    uuid_list[0].uu.uuid16 = UUID_SERVCLASS_HUMAN_INTERFACE;

    if (!wiced_bt_sdp_init_discovery_db(sdp_db, sizeof(sdp_db_raw), 1, uuid_list,
                                        (uint16_t)(sizeof(attr_list) / sizeof(attr_list[0])), attr_list))
    {
        printf("SDP DB init failed\r\n");
        return;
    }

    printf("Starting SDP service search...\r\n");
    if (!wiced_bt_sdp_service_search_attribute_request(joycon_bda, sdp_db, app_bt_sdp_complete_cb))
    {
        printf("SDP request start failed\r\n");
    }
}

static void app_start_security(void)
{
    wiced_result_t sec_res;
    wiced_result_t enc_res;

    if (!joycon_bda_valid)
    {
        return;
    }

    if (security_in_progress)
    {
        return;
    }

    if ((control_cid != 0) || (interrupt_cid != 0))
    {
        security_retry_requested = WICED_FALSE;
        return;
    }

    security_in_progress = WICED_TRUE;

    if (!joycon_paired)
    {
        sec_res = wiced_bt_dev_sec_bond(joycon_bda, BLE_ADDR_PUBLIC, BT_TRANSPORT_BR_EDR, 0, NULL);
        printf("Starting BR/EDR bonding, result=%d\r\n", (int)sec_res);

        if ((sec_res == WICED_BT_PENDING) || (sec_res == WICED_BT_BUSY))
        {
            return;
        }

        if (sec_res != WICED_BT_SUCCESS)
        {
            printf("Bonding start failed (res=%d), continuing with encryption request\r\n", (int)sec_res);
        }
    }
    else
    {
        sec_res = WICED_BT_SUCCESS;
    }

    enc_res = wiced_bt_dev_set_encryption(joycon_bda, BT_TRANSPORT_BR_EDR, NULL);
    printf("Requesting BR/EDR encryption, result=%d\r\n", (int)enc_res);

    if (enc_res == WICED_BT_SUCCESS)
    {
        security_in_progress = WICED_FALSE;
        security_retry_requested = WICED_FALSE;
        app_open_hid_channels();
        return;
    }

    if (enc_res == WICED_BT_PENDING)
    {
        return;
    }

    if ((enc_res == WICED_BT_BUSY) || (enc_res == WICED_BT_NO_RESOURCES))
    {
        security_in_progress = WICED_FALSE;
        security_retry_requested = WICED_TRUE;
        security_retry_ticks = 15;
        return;
    }

    if (enc_res == WICED_BT_WRONG_MODE)
    {
        security_in_progress = WICED_FALSE;
        security_retry_requested = WICED_FALSE;
        app_open_hid_channels();
        return;
    }

    security_in_progress = WICED_FALSE;
    security_retry_requested = WICED_FALSE;
    printf("Security start failed (res=%d), scheduling inquiry retry\r\n", (int)enc_res);
    inquiry_requested = WICED_TRUE;
    inquiry_retry_ticks = 20;
}

static void app_bt_sdp_complete_cb(uint16_t sdp_result)
{
    if (sdp_result != WICED_BT_SDP_SUCCESS)
    {
        printf("SDP failed: 0x%04X\r\n", sdp_result);
        joycon_bda_valid = WICED_FALSE;
        inquiry_requested = WICED_TRUE;
        inquiry_retry_ticks = 10;
        printf("Resuming inquiry after SDP failure...\r\n");
        return;
    }

    printf("SDP complete, starting security then HID L2CAP\r\n");
    app_start_security();
}

static void app_open_hid_channels(void)
{
    if ((control_cid_pending != 0) || (interrupt_cid_pending != 0))
    {
        return;
    }

    if ((control_cid != 0) && (interrupt_cid != 0))
    {
        return;
    }

    (void)wiced_bt_l2cap_register(JOYCON_HID_CONTROL_PSM, &l2cap_cb);
    (void)wiced_bt_l2cap_register(JOYCON_HID_INTERRUPT_PSM, &l2cap_cb);

    if (control_cid == 0)
    {
        control_cid_pending = wiced_bt_l2cap_connect_req(JOYCON_HID_CONTROL_PSM, joycon_bda);
        printf("L2CAP control connect requested. CID=%u\r\n", control_cid_pending);

        if (control_cid_pending == 0)
        {
            printf("L2CAP control connect request failed, retrying security/connect\r\n");
            security_in_progress = WICED_FALSE;
            inquiry_requested = WICED_TRUE;
            inquiry_retry_ticks = 10;
        }
        return;
    }

    if (interrupt_cid == 0)
    {
        interrupt_cid_pending = wiced_bt_l2cap_connect_req(JOYCON_HID_INTERRUPT_PSM, joycon_bda);
        printf("L2CAP interrupt connect requested. CID=%u\r\n", interrupt_cid_pending);

        if (interrupt_cid_pending == 0)
        {
            printf("L2CAP interrupt connect request failed, retrying security/connect\r\n");
            security_in_progress = WICED_FALSE;
            inquiry_requested = WICED_TRUE;
            inquiry_retry_ticks = 10;
        }
    }
}

static void app_l2cap_connected_cb(wiced_bt_device_address_t bd_addr, uint16_t local_cid, uint16_t peer_mtu)
{
    (void)bd_addr;
    (void)peer_mtu;

    if (local_cid == control_cid_pending)
    {
        control_cid = local_cid;
        control_cid_pending = 0;
        printf("Control channel connected (CID=%u)\r\n", control_cid);

        if ((interrupt_cid == 0) && (interrupt_cid_pending == 0))
        {
            app_open_hid_channels();
        }
    }
    else if (local_cid == interrupt_cid_pending)
    {
        interrupt_cid = local_cid;
        interrupt_cid_pending = 0;
        printf("Interrupt channel connected (CID=%u)\r\n", interrupt_cid);
    }
    else
    {
        printf("Unknown L2CAP channel connected (CID=%u)\r\n", local_cid);
    }

    app_try_send_enable_reports();
}

static void app_try_send_enable_reports(void)
{
    if ((control_cid != 0) && (interrupt_cid != 0) && (init_sent == WICED_FALSE))
    {
        app_send_subcommand_03();
        init_sent = WICED_TRUE;
        input_only_mode = WICED_TRUE;
    }
}

static void app_send_subcommand_03(void)
{
    uint8_t report[] = {
        0xA2,       /* BT HID output report prefix */
        0x01,       /* Output report ID: subcommand */
        0x00,       /* packet number */
        0x00, 0x00, 0x00, 0x00, /* rumble data (left) */
        0x00, 0x00, 0x00, 0x00, /* rumble data (right) */
        0x03,       /* subcommand: set input report mode */
        0x30        /* standard full input report mode */
    };
    uint8_t write_res = wiced_bt_l2cap_data_write(control_cid, report, (uint16_t)sizeof(report), L2CAP_NON_FLUSHABLE_PACKET);

    printf("Sent subcommand 0x03 over control channel, write result=%u\r\n", write_res);
}

static void app_l2cap_disconnect_ind_cb(uint16_t local_cid, uint16_t reason, wiced_bool_t ack)
{
    printf("L2CAP disconnect ind: cid=%u reason=%u\r\n", local_cid, reason);

    if (ack)
    {
        (void)wiced_bt_l2cap_disconnect_rsp(local_cid);
    }

    if (local_cid == control_cid)
    {
        control_cid = 0;
    }
    if (local_cid == control_cid_pending)
    {
        control_cid_pending = 0;
    }
    if (local_cid == interrupt_cid)
    {
        interrupt_cid = 0;
    }
    if (local_cid == interrupt_cid_pending)
    {
        interrupt_cid_pending = 0;
    }

    if ((control_cid == 0) && (interrupt_cid == 0))
    {
        input_only_mode = WICED_FALSE;
        init_sent = WICED_FALSE;
        input_state_valid = WICED_FALSE;

        if ((control_cid_pending == 0) && (interrupt_cid_pending == 0) && joycon_bda_valid)
        {
            printf("HID channels closed, retrying security/connect\r\n");
            security_in_progress = WICED_FALSE;
            app_start_security();
        }
    }

    if (reason == L2CAP_CONN_SECURITY_BLOCK)
    {
        printf("L2CAP security block, retrying security\r\n");
        app_start_security();
    }
}

static void app_l2cap_disconnect_cfm_cb(uint16_t local_cid, uint16_t result)
{
    printf("L2CAP disconnect cfm: cid=%u result=%u\r\n", local_cid, result);

    if (local_cid == control_cid)
    {
        control_cid = 0;
    }
    if (local_cid == control_cid_pending)
    {
        control_cid_pending = 0;
    }
    if (local_cid == interrupt_cid)
    {
        interrupt_cid = 0;
    }
    if (local_cid == interrupt_cid_pending)
    {
        interrupt_cid_pending = 0;
    }

    if ((control_cid == 0) && (interrupt_cid == 0))
    {
        input_only_mode = WICED_FALSE;
        init_sent = WICED_FALSE;
        input_state_valid = WICED_FALSE;

        if ((control_cid_pending == 0) && (interrupt_cid_pending == 0) && joycon_bda_valid)
        {
            printf("HID channels closed (cfm), retrying security/connect\r\n");
            security_in_progress = WICED_FALSE;
            app_start_security();
        }
    }
}

static void app_l2cap_data_ind_cb(uint16_t local_cid, tDRB *p_drb)
{
    if ((local_cid == interrupt_cid) && (p_drb != NULL))
    {
        uint16_t payload_len = p_drb->drb_data_len;
        uint16_t payload_offset = p_drb->drb_data_offset;
        const uint8_t *payload = &p_drb->drb_data[payload_offset];
        if ((payload_len >= 13u) && (payload[0] == 0xA1u) && (payload[1] == 0x3Fu))
        {
            uint8_t btn0 = payload[2];
            uint8_t btn1 = payload[3];
            uint8_t hat = (uint8_t)(payload[4] & 0x0Fu);
            uint8_t lx = payload[5];
            uint8_t ly = payload[7];
            uint8_t rx = payload[9];
            uint8_t ry = payload[11];

            wiced_bool_t changed = (input_state_valid == WICED_FALSE) ||
                                   (btn0 != last_btn0) ||
                                   (btn1 != last_btn1) ||
                                   (hat != last_hat) ||
                                   (lx != last_lx) ||
                                   (ly != last_ly) ||
                                   (rx != last_rx) ||
                                   (ry != last_ry);

            if (changed)
            {
                uint8_t diff0 = (uint8_t)(btn0 ^ last_btn0);
                uint8_t diff1 = (uint8_t)(btn1 ^ last_btn1);

                force_log_output = WICED_TRUE;
                printf("IN b0=%02X b1=%02X hat=%s lx=%3u ly=%3u rx=%3u ry=%3u",
                       btn0,
                       btn1,
                       app_hat_to_text(hat),
                       (unsigned int)lx,
                       (unsigned int)ly,
                       (unsigned int)rx,
                       (unsigned int)ry);

                if (input_state_valid)
                {
                    printf(" chg0=%02X chg1=%02X", diff0, diff1);
                }
                printf("\r\n");
                force_log_output = WICED_FALSE;

                last_btn0 = btn0;
                last_btn1 = btn1;
                last_hat = hat;
                last_lx = lx;
                last_ly = ly;
                last_rx = rx;
                last_ry = ry;
                input_state_valid = WICED_TRUE;
            }
            return;
        }

        force_log_output = WICED_TRUE;
        printf("IN len=%u raw:", (unsigned int)payload_len);
        for (uint16_t i = 0; i < payload_len; i++)
        {
            printf(" %02X", payload[i]);
        }
        printf("\r\n");
        force_log_output = WICED_FALSE;
    }
}

static void app_l2cap_tx_complete_cb(uint16_t local_cid, void *p_data)
{
    (void)local_cid;
    (void)p_data;
}
