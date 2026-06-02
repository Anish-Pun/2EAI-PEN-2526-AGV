#ifndef CTS_CLIENT_H
#define CTS_CLIENT_H

#include "wiced_bt_stack.h"

/* Bluetooth management callback */
wiced_result_t app_bt_management_callback(
        wiced_bt_management_evt_t event,
        wiced_bt_management_evt_data_t *p_event_data);

#endif