#include "cybsp.h"
#include "cyhal.h"
#include "cycfg.h"
#include "cy_retarget_io.h"
#include "wiced_bt_types.h"
#include "wiced_bt_ble.h"
#include "wiced_bt_gatt.h"
#include "wiced_bt_stack.h"
#include "bt_app.h"
#include "board.h"

/* ================= GLOBALS ================= */

volatile uint16_t bt_connection_id = 0;
uint16_t target_char_handle = 0;

/* ESP32 MAC */
uint8_t target_addr[6] = {0x68, 0xFE, 0x71, 0xFC, 0x5A, 0xDE};

/* 128-bit UUID (REVERSED) */
uint8_t target_uuid[16] = {
    0xf0, 0xde, 0xbc, 0x9a,
    0x78, 0x56,
    0x34, 0x12,
    0x78, 0x56,
    0x34, 0x12,
    0x01, 0xef, 0xcd, 0xab
};

/* ================= HELPERS ================= */

bool is_target_device(uint8_t *addr)
{
    for (int i = 0; i < 6; i++)
    {
        if (addr[i] != target_addr[i])
            return false;
    }
    return true;
}

bool uuid128_match(uint8_t *uuid)
{
    for (int i = 0; i < 16; i++)
    {
        if (uuid[i] != target_uuid[i])
            return false;
    }
    return true;
}

/* ================= FORWARD ================= */

void bt_app_init(void);
void scan_callback(wiced_bt_ble_scan_results_t *p_scan_result, uint8_t *p_adv_data);
wiced_bt_gatt_status_t bt_app_gatt_event_cb(wiced_bt_gatt_evt_t event,
                                            wiced_bt_gatt_event_data_t *p_data);

/* ================= SEND ================= */

void send_data(uint8_t value)
{
    if (bt_connection_id == 0 || target_char_handle == 0)
    {
        printf("Cannot send\r\n");
        return;
    }

    uint8_t buffer[1];
    buffer[0] = value;

    wiced_bt_gatt_client_send_write(
        bt_connection_id,
        GATT_WRITE_NO_RSP,
        target_char_handle,
        1,
        buffer,
        NULL
    );

    printf("Sent: %d\r\n", value);
}

/* ================= TASK ================= */

void bt_task(void *param)
{
    (void)param;

    for (;;)
    {
        vTaskDelay(2000);

        send_data(1);
    }
}

/* ================= INIT ================= */

void bt_app_init(void)
{
    printf("Starting CENTRAL...\r\n");

    wiced_bt_gatt_register(bt_app_gatt_event_cb);

    wiced_bt_ble_scan(BTM_BLE_SCAN_TYPE_HIGH_DUTY, WICED_TRUE, scan_callback);
}

/* ================= SCAN ================= */

void scan_callback(wiced_bt_ble_scan_results_t *p_scan_result, uint8_t *p_adv_data)
{
    if (p_scan_result != NULL)
    {
        printf("Found: ");
        for (int i = 0; i < 6; i++)
            printf("%02X:", p_scan_result->remote_bd_addr[i]);
        printf("\r\n");

        if (is_target_device(p_scan_result->remote_bd_addr))
        {
            printf("TARGET FOUND → CONNECT\r\n");

            wiced_bt_ble_scan(BTM_BLE_SCAN_TYPE_NONE, WICED_TRUE, scan_callback);

            wiced_bt_gatt_le_connect(
                p_scan_result->remote_bd_addr,
                p_scan_result->ble_addr_type,
                BLE_CONN_MODE_HIGH_DUTY,
                TRUE
            );
        }
    }
}

/* ================= MANAGEMENT ================= */

wiced_result_t bt_app_management_cb(wiced_bt_management_evt_t event,
                                   wiced_bt_management_evt_data_t *p_event_data)
{
    if (event == BTM_ENABLED_EVT)
    {
        if (WICED_BT_SUCCESS == p_event_data->enabled.status)
        {
            printf("Bluetooth ON\r\n");
            bt_app_init();
        }
    }
    return WICED_BT_SUCCESS;
}

/* ================= GATT ================= */

wiced_bt_gatt_status_t bt_app_gatt_event_cb(wiced_bt_gatt_evt_t event,
                                            wiced_bt_gatt_event_data_t *p_data)
{
    switch (event)
    {
        case GATT_CONNECTION_STATUS_EVT:

            if (p_data->connection_status.connected)
            {
                printf("Connected!\r\n");

                bt_connection_id = p_data->connection_status.conn_id;

                /* discover services */
                wiced_bt_gatt_client_send_discover(
                    bt_connection_id,
                    GATT_DISCOVER_SERVICES_ALL,
                    NULL
                );
            }
            else
            {
                printf("Disconnected!\r\n");

                bt_connection_id = 0;
                target_char_handle = 0;

                wiced_bt_ble_scan(BTM_BLE_SCAN_TYPE_HIGH_DUTY, WICED_TRUE, scan_callback);
            }
            break;

        case GATT_DISCOVERY_CPLT_EVT:

            printf("Discovery done → characteristics\r\n");

            wiced_bt_gatt_client_send_discover(
                bt_connection_id,
                GATT_DISCOVER_CHARACTERISTICS,
                NULL
            );
            break;

        case GATT_DISCOVERY_RESULT_EVT:

            if (p_data->discovery_result.discovery_type == GATT_DISCOVER_CHARACTERISTICS)
            {
                uint16_t handle =
                    p_data->discovery_result.discovery_data.characteristic_declaration.handle;

                if (p_data->discovery_result.discovery_data.characteristic_declaration.char_uuid.len == LEN_UUID_128)
                {
                    uint8_t *uuid =
                        p_data->discovery_result.discovery_data.characteristic_declaration.char_uuid.uu.uuid128;

                    printf("128-bit char found\r\n");

                    if (uuid128_match(uuid))
                    {
                        target_char_handle = handle;
                        printf("TARGET CHARACTERISTIC FOUND!\r\n");
                    }
                }
            }
            break;

        default:
            break;
    }

    return WICED_BT_GATT_SUCCESS;
}