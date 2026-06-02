#include <stdio.h>
#include <string.h>

#include "wiced_bt_stack.h"
#include "cycfg_bt_settings.h"
#include "cycfg_gap.h"
#include "app_bt_utils.h"

#include "cts_client.h"

static uint16_t connection_id = 0;

/* scan callback */
void scan_result_cback(
        wiced_bt_ble_scan_results_t *p_scan_result,
        uint8_t *p_adv_data)
{
    /* scan finished */
    if(p_scan_result == NULL)
    {
        printf("Scan complete\n");
        return;
    }

    printf("Device found: ");
    print_bd_address(p_scan_result->remote_bd_addr);
    printf("\n");

    /* ESP32 BLE MAC address */
    uint8_t esp32_mac[6] = {0x68,0xFE,0x71,0xFC,0x5A,0xDE};

    /* check if this is the ESP32 */
    if(memcmp(p_scan_result->remote_bd_addr, esp32_mac, 6) == 0)
    {
        printf("ESP32 gevonden -> verbinden\n");

        /* stop scanning */
        wiced_bt_ble_scan(
                BTM_BLE_SCAN_TYPE_NONE,
                WICED_FALSE,
                NULL);

        /* connect to ESP32 */
        wiced_bt_gatt_le_connect(
                p_scan_result->remote_bd_addr,
                p_scan_result->ble_addr_type,
                BLE_CONN_MODE_HIGH_DUTY,
                TRUE);
    }
}

/* gatt event handler */
static wiced_bt_gatt_status_t ble_app_gatt_event_callback(
        wiced_bt_gatt_evt_t event,
        wiced_bt_gatt_event_data_t *p_event_data)
{
    switch(event)
    {

        case GATT_CONNECTION_STATUS_EVT:

            if(p_event_data->connection_status.connected)
            {
                connection_id =
                    p_event_data->connection_status.conn_id;

                printf("Connected to ESP32\n");

                wiced_bt_gatt_discovery_param_t discovery;

                discovery.s_handle = 1;
                discovery.e_handle = 0xFFFF;

                wiced_bt_gatt_client_send_discover(
                        connection_id,
                        GATT_DISCOVER_SERVICES_ALL,
                        &discovery);
            }
            else
            {
                printf("Disconnected\n");
                connection_id = 0;
            }

        break;

        case GATTC_OPTYPE_NOTIFICATION:
        {
            wiced_bt_gatt_data_t notif =
                p_event_data->operation_complete.response_data.att_value;

            char buffer[32];

            memcpy(buffer, notif.p_data, notif.len);
            buffer[notif.len] = '\0';

            printf("Data received: %s\n", buffer);

            int speed;
            int steer;

            sscanf(buffer,"%d,%d",&speed,&steer);

            printf("Speed: %d\n", speed);
            printf("Steer: %d\n", steer);
        }
        break;

    }

    return WICED_BT_GATT_SUCCESS;
}

/* bluetooth management callback */
wiced_result_t app_bt_management_callback(
        wiced_bt_management_evt_t event,
        wiced_bt_management_evt_data_t *p_event_data)
{
    switch(event)
    {

        case BTM_ENABLED_EVT:

            if(p_event_data->enabled.status ==
               WICED_BT_SUCCESS)
            {
                printf("Bluetooth enabled\n");

                wiced_bt_gatt_register(
                        ble_app_gatt_event_callback);

                /* start scanning */
                wiced_bt_ble_scan(
                        BTM_BLE_SCAN_TYPE_HIGH_DUTY,
                        WICED_TRUE,
                        scan_result_cback);
            }

        break;

    }

    return WICED_BT_SUCCESS;
}