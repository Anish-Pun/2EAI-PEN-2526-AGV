#include "wiced_bt_stack.h"
#include "cybsp.h"
#include "cyhal.h"
#include "cy_retarget_io.h"
#include <FreeRTOS.h>
#include <task.h>
#include "cycfg_bt_settings.h"
#include "cycfg_gap.h"
#include "wiced_bt_dev.h"
#include "app_bt_utils.h"
#include <stdlib.h>
#include "wiced_bt_uuid.h"
#include "wiced_bt_types.h"
#include <string.h>

/* ===== CONFIG ===== */
#define TARGET_NAME "CarApp"
#define UUID_SERVICE_CAR_CONTROL   0xFFF0
#define UUID_CHAR_COMMAND          0xFFF1

/* ===== GLOBALS ===== */
static uint16_t bt_connection_id = 0;
static uint16_t command_char_handle = 0;
static bool is_connecting = false;

/* ===== FORWARD ===== */
void scan_result_callback(wiced_bt_ble_scan_results_t *p_scan_result, uint8_t *p_adv_data);
static void ble_app_init(void);
static wiced_bt_gatt_status_t ble_app_connect_handler(wiced_bt_gatt_connection_status_t *p_conn_status);
static wiced_bt_gatt_status_t ble_app_gatt_event_callback(wiced_bt_gatt_evt_t event,
                                                          wiced_bt_gatt_event_data_t *p_event_data);
static wiced_bt_gatt_status_t discovery_result_handler(wiced_bt_gatt_discovery_result_t *result);
static wiced_bt_gatt_status_t discovery_complete_handler(wiced_bt_gatt_discovery_complete_t *complete);

void send_command(uint8_t cmd);

/*******************************************************************************
* MANAGEMENT CALLBACK
*******************************************************************************/
wiced_result_t app_bt_management_callback(wiced_bt_management_evt_t event,
                                          wiced_bt_management_evt_data_t *p_event_data)
{
    wiced_bt_device_address_t bda = { 0 };

    if (event == BTM_ENABLED_EVT)
    {
        wiced_bt_dev_read_local_addr(bda);
        printf("Local Bluetooth Address: ");
        print_bd_address(bda);

        ble_app_init();
    }

    return WICED_BT_SUCCESS;
}

/*******************************************************************************
* INIT
*******************************************************************************/
static void ble_app_init(void)
{
    wiced_bt_gatt_register(ble_app_gatt_event_callback);

    printf("\n=== CENTRAL START ===\n");
    printf("Scanning for %s...\n", TARGET_NAME);

    wiced_bt_ble_scan(BTM_BLE_SCAN_TYPE_HIGH_DUTY, WICED_TRUE, scan_result_callback);
}

/*******************************************************************************
* SCAN
*******************************************************************************/
void scan_result_callback(wiced_bt_ble_scan_results_t *p_scan_result, uint8_t *p_adv_data)
{
    if (p_scan_result == NULL || is_connecting)
        return;

    uint8_t len;
    uint8_t *name = wiced_bt_ble_check_advertising_data(
        p_adv_data,
        BTM_BLE_ADVERT_TYPE_NAME_COMPLETE,
        &len
    );

    if (name)
    {
        char dev_name[32] = {0};
        memcpy(dev_name, name, len);
        dev_name[len] = '\0';

        printf("Found device: %s\n", dev_name);

        if (strcmp(dev_name, TARGET_NAME) == 0)
        {
            printf(">>> FOUND — CONNECTING\n");

            is_connecting = true;

            wiced_bt_ble_scan(BTM_BLE_SCAN_TYPE_NONE, WICED_TRUE, NULL);

            wiced_bt_gatt_le_connect(
                p_scan_result->remote_bd_addr,
                p_scan_result->ble_addr_type,
                BLE_CONN_MODE_HIGH_DUTY,
                TRUE
            );
        }
    }
}

/*******************************************************************************
* GATT CALLBACK
*******************************************************************************/
static wiced_bt_gatt_status_t
ble_app_gatt_event_callback(wiced_bt_gatt_evt_t event,
                           wiced_bt_gatt_event_data_t *p_data)
{
    switch (event)
    {
        case GATT_CONNECTION_STATUS_EVT:
            return ble_app_connect_handler(&p_data->connection_status);

        case GATT_DISCOVERY_RESULT_EVT:
            return discovery_result_handler(&p_data->discovery_result);

        case GATT_DISCOVERY_CPLT_EVT:
            return discovery_complete_handler(&p_data->discovery_complete);

        default:
            break;
    }

    return WICED_BT_GATT_SUCCESS;
}

/*******************************************************************************
* CONNECT
*******************************************************************************/
static wiced_bt_gatt_status_t
ble_app_connect_handler(wiced_bt_gatt_connection_status_t *p_conn_status)
{
    if (!p_conn_status)
        return WICED_BT_GATT_ERROR;

    if (p_conn_status->connected)
    {
        printf("=== CONNECTED ===\n");

        bt_connection_id = p_conn_status->conn_id;
        is_connecting = false;

        /* START SERVICE DISCOVERY */
        wiced_bt_gatt_discovery_param_t param;

        param.s_handle = 1;
        param.e_handle = 0xFFFF;
        param.uuid.len = LEN_UUID_16;
        param.uuid.uu.uuid16 = UUID_SERVICE_CAR_CONTROL;

        wiced_bt_gatt_client_send_discover(
            bt_connection_id,
            GATT_DISCOVER_SERVICES_BY_UUID,
            &param
        );
    }
    else
    {
        printf("=== DISCONNECTED ===\n");

        bt_connection_id = 0;
        is_connecting = false;

        printf("Restart scanning...\n");

        wiced_bt_ble_scan(BTM_BLE_SCAN_TYPE_HIGH_DUTY, WICED_TRUE, scan_result_callback);
    }

    return WICED_BT_GATT_SUCCESS;
}

/*******************************************************************************
* DISCOVERY RESULT
*******************************************************************************/
static wiced_bt_gatt_status_t
discovery_result_handler(wiced_bt_gatt_discovery_result_t *result)
{
    if (result->discovery_type == GATT_DISCOVER_CHARACTERISTICS)
    {
        if (result->discovery_data.characteristic_declaration.char_uuid.uu.uuid16
            == UUID_CHAR_COMMAND)
        {
            command_char_handle =
                result->discovery_data.characteristic_declaration.val_handle;

            printf("Command handle FOUND: %d\n", command_char_handle);
        }
    }

    return WICED_BT_GATT_SUCCESS;
}

/*******************************************************************************
* DISCOVERY COMPLETE
*******************************************************************************/
static wiced_bt_gatt_status_t
discovery_complete_handler(wiced_bt_gatt_discovery_complete_t *complete)
{
    if (complete->discovery_type == GATT_DISCOVER_SERVICES_BY_UUID)
    {
        /* NOW find characteristic */
        wiced_bt_gatt_discovery_param_t param;

        param.s_handle = 1;
        param.e_handle = 0xFFFF;
        param.uuid.len = LEN_UUID_16;
        param.uuid.uu.uuid16 = UUID_CHAR_COMMAND;

        wiced_bt_gatt_client_send_discover(
            bt_connection_id,
            GATT_DISCOVER_CHARACTERISTICS,
            &param
        );
    }

    else if (complete->discovery_type == GATT_DISCOVER_CHARACTERISTICS)
    {
        printf("=== READY TO SEND ===\n");

        /* TEST SEND AFTER DISCOVERY */
        send_command(0x01);
    }

    return WICED_BT_GATT_SUCCESS;
}

/*******************************************************************************
* SEND COMMAND
*******************************************************************************/
void send_command(uint8_t cmd)
{
    if (bt_connection_id == 0 || command_char_handle == 0)
    {
        printf("Cannot send yet!\n");
        return;
    }

    wiced_bt_gatt_write_hdr_t write_hdr;
    uint8_t *data = malloc(1);

    data[0] = cmd;

    write_hdr.handle = command_char_handle;
    write_hdr.len = 1;
    write_hdr.offset = 0;
    write_hdr.auth_req = GATT_AUTH_REQ_NONE;

    wiced_bt_gatt_client_send_write(
        bt_connection_id,
        GATT_REQ_WRITE,
        &write_hdr,
        data,
        NULL
    );

    printf("Sent command: %d\n", cmd);
}