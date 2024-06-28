/*
 * Copyright 2016-2024, Cypress Semiconductor Corporation (an Infineon company) or
 * an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
 *
 * This software, including source code, documentation and related
 * materials ("Software") is owned by Cypress Semiconductor Corporation
 * or one of its affiliates ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products.  Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 */

#pragma once

#include "wiced_bt_dev.h"
#include "wiced_bt_ble.h"
#include "wiced_bt_gatt.h"

/*
 * Definitions
 */
typedef enum
{
    APP_BLE_EVENT_ADV_CHANGED = 0,
    APP_BLE_EVENT_GATT_CONNECTED,
} app_ble_event_t;

/*
 * Structures
 */
typedef struct
{
    wiced_bt_ble_advert_mode_t mode;
} app_ble_event_adv_changed_t;

typedef struct
{
    wiced_result_t status;
    wiced_bt_device_address_t bdaddr;
    uint16_t conn_id;
} app_ble_event_gatt_connected_t;

typedef union
{
    app_ble_event_adv_changed_t adv_changed;
    app_ble_event_gatt_connected_t gatt_connected;
} app_ble_event_data_t;

typedef void (app_ble_callback_t)(app_ble_event_t event, app_ble_event_data_t *p_data);

/*
 * app_ble_init
 */
wiced_result_t app_ble_init(app_ble_callback_t *p_callack);

/*
 * app_ble_name_set
 */
wiced_result_t app_ble_name_set(char *p_ble_dev_name);

/*
 * app_ble_advert_enable
 */
wiced_result_t app_ble_advert_enable(wiced_bt_ble_advert_mode_t adv_mode);

/*
 * app_ble_advert_state_changed
 */
void app_ble_advert_state_changed(wiced_bt_ble_advert_mode_t mode);
