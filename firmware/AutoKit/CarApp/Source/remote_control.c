#include "remote_control.h"
#include "motor.h"
#include "app_bt_utils.h"

#include "cybsp.h"
#include "cybsp_bt_config.h"
#include "cybt_platform_config.h"
#include "cycfg_bt_settings.h"
#include "cycfg_gap.h"
#include "cycfg_gatt_db.h"

#include "wiced_bt_stack.h"
#include "wiced_bt_gatt.h"

#include "cyhal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern const cybt_platform_config_t cybsp_bt_platform_cfg;

static int printed = 0;
static int ble_mode_enabled = 1;
static int ble_initialized = 0;
static uint16_t connection_id = 0;
static int local_bda_valid = 0;
static wiced_bt_device_address_t local_bda = {0};

typedef void (*pfn_free_buffer_t)(uint8_t *);

static wiced_result_t app_bt_management_callback(wiced_bt_management_evt_t event, wiced_bt_management_evt_data_t *p_event_data);
static wiced_bt_gatt_status_t app_bt_gatt_event_callback(wiced_bt_gatt_evt_t event, wiced_bt_gatt_event_data_t *p_event_data);
static wiced_bt_gatt_status_t app_bt_connect_event_handler(wiced_bt_gatt_connection_status_t *p_conn_status);
static wiced_bt_gatt_status_t app_bt_server_event_handler(wiced_bt_gatt_event_data_t *p_data);
static wiced_bt_gatt_status_t app_bt_write_handler(wiced_bt_gatt_event_data_t *p_data);
static wiced_bt_gatt_status_t app_bt_gatt_req_read_handler(uint16_t conn_id,
                                                            wiced_bt_gatt_opcode_t opcode,
                                                            wiced_bt_gatt_read_t *p_read_req,
                                                            uint16_t len_requested);
static gatt_db_lookup_table_t *app_bt_find_by_handle(uint16_t handle);
static uint8_t *app_bt_alloc_buffer(uint16_t len);
static void app_bt_free_buffer(uint8_t *p_data);

void remote_control_init_ble(void)
{
    if (ble_initialized)
    {
        return;
    }

    cybt_platform_config_init(&cybsp_bt_platform_cfg);
    wiced_bt_stack_init(app_bt_management_callback, &wiced_bt_cfg_settings);
    ble_initialized = 1;
}

void remote_control_set_enabled(int enabled)
{
    ble_mode_enabled = enabled ? 1 : 0;
    if (!ble_mode_enabled)
    {
        motor_stop();
    }
}

void remote_control_enter(void)
{
    remote_control_set_enabled(1);
    motor_stop();
    printed = 0;
    printf("\r\nBLUETOOTH MODE\r\n");
    printf("Press SW2 to switch to self-driving.\r\n");

    if (local_bda_valid)
    {
        printf("Bluetooth Enabled\r\n");
        printf("Local Bluetooth Device Address: ");
        print_bd_address(local_bda);

        printf("Restart advertising...\r\n");
        wiced_bt_start_advertisements(BTM_BLE_ADVERT_UNDIRECTED_HIGH, 0, NULL);
    }
}

void remote_control_exit(void)
{
    remote_control_set_enabled(0);
    motor_stop();
    printed = 0;
    printf("\r\nEXIT BLUETOOTH MODE\r\n");
}

void remote_control_step(void)
{
    if (!printed)
    {
        printf("BT: waiting for controller writes...\r\n");
        fflush(stdout);
        printed = 1;
    }

    cyhal_system_delay_ms(50);
}

static wiced_result_t app_bt_management_callback(wiced_bt_management_evt_t event, wiced_bt_management_evt_data_t *p_event_data)
{
    wiced_result_t result = WICED_BT_ERROR;
    wiced_bt_device_address_t bda = {0};

    printf("Bluetooth Management Event: 0x%x %s\r\n", event, get_bt_event_name(event));

    switch (event)
    {
        case BTM_ENABLED_EVT:
            if (WICED_BT_SUCCESS == p_event_data->enabled.status)
            {
                printf("Bluetooth Enabled\r\n");

                wiced_bt_set_local_bdaddr((uint8_t *)cy_bt_device_address, BLE_ADDR_PUBLIC);
                wiced_bt_dev_read_local_addr(local_bda);
                local_bda_valid = 1;
                printf("Local Bluetooth Device Address: ");
                print_bd_address(local_bda);

                wiced_bt_gatt_register(app_bt_gatt_event_callback);
                wiced_bt_gatt_db_init(gatt_database, gatt_database_len, NULL);

                wiced_bt_ble_set_raw_advertisement_data(CY_BT_ADV_PACKET_DATA_SIZE,
                                                        cy_bt_adv_packet_data);
                wiced_bt_start_advertisements(BTM_BLE_ADVERT_UNDIRECTED_HIGH, 0, NULL);

                result = WICED_BT_SUCCESS;
            }
            else
            {
                printf("Failed to initialize Bluetooth controller and stack\r\n");
            }
            break;

        case BTM_PAIRING_IO_CAPABILITIES_BLE_REQUEST_EVT:
        case BTM_PAIRING_COMPLETE_EVT:
        case BTM_ENCRYPTION_STATUS_EVT:
        case BTM_SECURITY_REQUEST_EVT:
        case BTM_PAIRED_DEVICE_LINK_KEYS_UPDATE_EVT:
        case BTM_LOCAL_IDENTITY_KEYS_UPDATE_EVT:
        case BTM_BLE_SCAN_STATE_CHANGED_EVT:
            result = WICED_BT_SUCCESS;
            break;

        case BTM_PAIRED_DEVICE_LINK_KEYS_REQUEST_EVT:
        case BTM_LOCAL_IDENTITY_KEYS_REQUEST_EVT:
            result = WICED_BT_ERROR;
            break;

        case BTM_BLE_ADVERT_STATE_CHANGED_EVT:
                 printf("Advertisement State Change: %s\r\n",
                   get_bt_advert_mode_name(p_event_data->ble_advert_state_changed));
            result = WICED_BT_SUCCESS;
            break;

        case BTM_BLE_CONNECTION_PARAM_UPDATE:
                 printf("Connection parameter update status:%d, Connection Interval: %d, Connection Latency: %d, Connection Timeout: %d\r\n",
                   p_event_data->ble_connection_param_update.status,
                   p_event_data->ble_connection_param_update.conn_interval,
                   p_event_data->ble_connection_param_update.conn_latency,
                   p_event_data->ble_connection_param_update.supervision_timeout);
            result = WICED_BT_SUCCESS;
            break;

        case BTM_BLE_PHY_UPDATE_EVT:
                 printf("Selected TX PHY - %dM\r\nSelected RX PHY - %dM\r\n",
                   p_event_data->ble_phy_update_event.tx_phy,
                   p_event_data->ble_phy_update_event.rx_phy);
            result = WICED_BT_SUCCESS;
            break;

        default:
            break;
    }

    return result;
}

static wiced_bt_gatt_status_t app_bt_gatt_event_callback(wiced_bt_gatt_evt_t event,
                                                          wiced_bt_gatt_event_data_t *p_event_data)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_ERROR;

    switch (event)
    {
        case GATT_CONNECTION_STATUS_EVT:
            status = app_bt_connect_event_handler(&p_event_data->connection_status);
            break;

        case GATT_ATTRIBUTE_REQUEST_EVT:
            status = app_bt_server_event_handler(p_event_data);
            break;

        case GATT_GET_RESPONSE_BUFFER_EVT:
            p_event_data->buffer_request.buffer.p_app_rsp_buffer =
                app_bt_alloc_buffer(p_event_data->buffer_request.len_requested);
            p_event_data->buffer_request.buffer.p_app_ctxt = (void *)app_bt_free_buffer;
            status = WICED_BT_GATT_SUCCESS;
            break;

        case GATT_APP_BUFFER_TRANSMITTED_EVT:
        {
            pfn_free_buffer_t pfn_free = (pfn_free_buffer_t)p_event_data->buffer_xmitted.p_app_ctxt;
            if (pfn_free)
            {
                pfn_free(p_event_data->buffer_xmitted.p_app_data);
            }
            status = WICED_BT_GATT_SUCCESS;
            break;
        }

        default:
            status = WICED_BT_GATT_SUCCESS;
            break;
    }

    return status;
}

static wiced_bt_gatt_status_t app_bt_connect_event_handler(wiced_bt_gatt_connection_status_t *p_conn_status)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_ERROR;

    if (NULL != p_conn_status)
    {
        if (p_conn_status->connected)
        {
            printf("GATT_CONNECTION_STATUS_EVT: Connect BDA ");
            print_bd_address(p_conn_status->bd_addr);
            printf("Connection ID %d\r\n", p_conn_status->conn_id);
            connection_id = p_conn_status->conn_id;
        }
        else
        {
            printf("Disconnected : BDA ");
            print_bd_address(p_conn_status->bd_addr);
                 printf("Connection ID '%d', Reason '%s'\r\n",
                   p_conn_status->conn_id,
                   get_bt_gatt_disconn_reason_name(p_conn_status->reason));

            connection_id = 0;
            wiced_bt_start_advertisements(BTM_BLE_ADVERT_UNDIRECTED_HIGH, 0, NULL);
        }

        status = WICED_BT_GATT_SUCCESS;
    }

    return status;
}

static wiced_bt_gatt_status_t app_bt_server_event_handler(wiced_bt_gatt_event_data_t *p_data)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_ERROR;
    wiced_bt_gatt_attribute_request_t *p_att_req = &p_data->attribute_request;

    switch (p_att_req->opcode)
    {
        case GATT_REQ_READ:
        case GATT_REQ_READ_BLOB:
            status = app_bt_gatt_req_read_handler(p_att_req->conn_id,
                                                  p_att_req->opcode,
                                                  &p_att_req->data.read_req,
                                                  p_att_req->len_requested);
            break;

        case GATT_REQ_WRITE:
            status = app_bt_write_handler(p_data);
            if (status == WICED_BT_GATT_SUCCESS)
            {
                wiced_bt_gatt_write_req_t *p_write_request = &p_att_req->data.write_req;
                wiced_bt_gatt_server_send_write_rsp(p_att_req->conn_id,
                                                    p_att_req->opcode,
                                                    p_write_request->handle);
            }
            break;

        case GATT_CMD_WRITE:
        case GATT_CMD_SIGNED_WRITE:
            status = app_bt_write_handler(p_data);
            break;

        case GATT_REQ_EXECUTE_WRITE:
            wiced_bt_gatt_server_send_execute_write_rsp(p_att_req->conn_id, p_att_req->opcode);
            status = WICED_BT_GATT_SUCCESS;
            break;

        case GATT_REQ_MTU:
            status = wiced_bt_gatt_server_send_mtu_rsp(p_att_req->conn_id,
                                                       p_att_req->data.remote_mtu,
                                                       wiced_bt_cfg_settings.p_ble_cfg->ble_max_rx_pdu_size);
            break;

        default:
            status = WICED_BT_GATT_SUCCESS;
            break;
    }

    return status;
}

static wiced_bt_gatt_status_t app_bt_write_handler(wiced_bt_gatt_event_data_t *p_data)
{
    wiced_bt_gatt_write_req_t *p_write_req = &p_data->attribute_request.data.write_req;
    wiced_bt_gatt_status_t status = WICED_BT_GATT_INVALID_HANDLE;

    for (int i = 0; i < app_gatt_db_ext_attr_tbl_size; i++)
    {
        if (app_gatt_db_ext_attr_tbl[i].handle == p_write_req->handle)
        {
            if (app_gatt_db_ext_attr_tbl[i].max_len >= p_write_req->val_len)
            {
                app_gatt_db_ext_attr_tbl[i].cur_len = p_write_req->val_len;
                memset(app_gatt_db_ext_attr_tbl[i].p_data, 0x00, app_gatt_db_ext_attr_tbl[i].max_len);
                memcpy(app_gatt_db_ext_attr_tbl[i].p_data, p_write_req->p_val, app_gatt_db_ext_attr_tbl[i].cur_len);

                if (memcmp(app_gatt_db_ext_attr_tbl[i].p_data,
                           p_write_req->p_val,
                           app_gatt_db_ext_attr_tbl[i].cur_len) == 0)
                {
                    status = WICED_BT_GATT_SUCCESS;
                }

                if (!ble_mode_enabled)
                {
                    motor_stop();
                    break;
                }

                switch (p_write_req->handle)
                {
                    case HDLC_PSOC_CAR_VALUE:
                    {
                        if (app_psoc_start[0] == 0)
                        {
                            int16_t accel_x = (int16_t)(((uint16_t)app_psoc_car[0] << 8) | app_psoc_car[1]);
                            int16_t accel_y = (int16_t)(((uint16_t)app_psoc_car[2] << 8) | app_psoc_car[3]);

                            if (accel_x > 5000)
                            {
                                motor_turn_left(80);
                            }
                            else if (accel_x < -7000)
                            {
                                motor_turn_right(80);
                            }
                            else if (accel_y > 6000)
                            {
                                motor_forward(80);
                            }
                            else
                            {
                                motor_stop();
                            }
                        }
                        else
                        {
                            motor_stop();
                        }
                        break;
                    }

                    case HDLC_PSOC_START_VALUE:
                        printf("button : %d\r\n", app_psoc_start[0]);
                        break;

                    default:
                        break;
                }
            }
            else
            {
                status = WICED_BT_GATT_INVALID_ATTR_LEN;
                printf("Invalid attribute length during GATT write\r\n");
            }
            break;
        }
    }

    if (WICED_BT_GATT_SUCCESS != status)
    {
        printf("GATT write failed: %d\r\n", status);
    }

    return status;
}

static wiced_bt_gatt_status_t app_bt_gatt_req_read_handler(uint16_t conn_id,
                                                            wiced_bt_gatt_opcode_t opcode,
                                                            wiced_bt_gatt_read_t *p_read_req,
                                                            uint16_t len_requested)
{
    gatt_db_lookup_table_t *puAttribute;
    uint16_t attr_len_to_copy, to_send;
    uint8_t *from;

    if ((puAttribute = app_bt_find_by_handle(p_read_req->handle)) == NULL)
    {
        wiced_bt_gatt_server_send_error_rsp(conn_id, opcode, p_read_req->handle, WICED_BT_GATT_INVALID_HANDLE);
        return WICED_BT_GATT_INVALID_HANDLE;
    }

    attr_len_to_copy = puAttribute->cur_len;

    if (p_read_req->offset >= puAttribute->cur_len)
    {
        wiced_bt_gatt_server_send_error_rsp(conn_id, opcode, p_read_req->handle, WICED_BT_GATT_INVALID_OFFSET);
        return WICED_BT_GATT_INVALID_OFFSET;
    }

    to_send = MIN(len_requested, attr_len_to_copy - p_read_req->offset);
    from = puAttribute->p_data + p_read_req->offset;
    return wiced_bt_gatt_server_send_read_handle_rsp(conn_id, opcode, to_send, from, NULL);
}

static gatt_db_lookup_table_t *app_bt_find_by_handle(uint16_t handle)
{
    for (int i = 0; i < app_gatt_db_ext_attr_tbl_size; i++)
    {
        if (app_gatt_db_ext_attr_tbl[i].handle == handle)
        {
            return (&app_gatt_db_ext_attr_tbl[i]);
        }
    }
    return NULL;
}

static uint8_t *app_bt_alloc_buffer(uint16_t len)
{
    uint8_t *p = (uint8_t *)malloc(len);
    return p;
}

static void app_bt_free_buffer(uint8_t *p_data)
{
    if (p_data != NULL)
    {
        free(p_data);
    }
}
