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

#include "wiced_bt_uuid.h"
#include "wiced_bt_ota_firmware_upgrade.h"
#include "app_ble.h"
#include "app_main.h"
#include "app_trace.h"
#ifdef FASTPAIR_ENABLE
#include "wiced_bt_gfps.h"
#endif
#include "app_nvram.h"
#include "wiced_app_cfg.h"
#ifdef APP_OFU_SUPPORT
#include "ofu/app_ofu_ble.h"
#endif
#include "bt_hs_spk_control.h"

/*
 * Definitions
 */
#define APP_BLE_HANDLE_GATT_SERVICE              1
#define APP_BLE_HANDLE_GAP_SERVICE               2
#define APP_BLE_HANDLE_CHAR_DEV_NAME             3
#define APP_BLE_HANDLE_CHAR_DEV_NAME_VAL         4
#define APP_BLE_HANDLE_CHAR_DEV_APPEARANCE       5
#define APP_BLE_HANDLE_CHAR_DEV_APPEARANCE_VAL   6

#define APP_BLE_HANDLE_FASTPAIR_SERVICE                             0x0070
#define APP_BLE_HANDLE_FASTPAIR_SERVICE_CHAR_KEY_PAIRING            0x0071
#define APP_BLE_HANDLE_FASTPAIR_SERVICE_CHAR_KEY_PAIRING_VAL        0x0072
#define APP_BLE_HANDLE_FASTPAIR_SERVICE_CHAR_KEY_PAIRING_CFG_DESC   0x0073
#define APP_BLE_HANDLE_FASTPAIR_SERVICE_CHAR_PASSKEY                0x0074
#define APP_BLE_HANDLE_FASTPAIR_SERVICE_CHAR_PASSKEY_VAL            0x0075
#define APP_BLE_HANDLE_FASTPAIR_SERVICE_CHAR_PASSKEY_CFG_DESC       0x0076
#define APP_BLE_HANDLE_FASTPAIR_SERVICE_CHAR_ACCOUNT_KEY            0x0077
#define APP_BLE_HANDLE_FASTPAIR_SERVICE_CHAR_ACCOUNT_KEY_VAL        0x0078
#define APP_BLE_HANDLE_FASTPAIR_SERVICE_CHAR_ACCOUNT_KEY_CFG_DESC   0x0079

#ifdef FASTPAIR_ENABLE
/* MODEL-specific definitions */
#if defined(CYW20721B2) || defined(CYW43012C0)
#define APP_BLE_FASTPAIR_MODEL_ID       0x82DA6E
#else
#define APP_BLE_FASTPAIR_MODEL_ID       0xCE948F //0xB49236 //0x000107 //0x140A02 // 0xCE948F
#endif

#if (APP_BLE_FASTPAIR_MODEL_ID == 0x82DA6E)
const uint8_t anti_spoofing_public_key[] =
{
    0x95, 0xcf, 0xdb, 0xae, 0xc0, 0xef, 0xc5, 0x1f, 0x39, 0x0f, 0x2a, 0xe0, 0x16, 0x5a, 0x2b, 0x59,
    0x62, 0xb2, 0xfe, 0x82, 0xfa, 0xf0, 0xd4, 0x1e, 0xa3, 0x4f, 0x07, 0x7e, 0xf7, 0x3d, 0xc0, 0x44,
    0x3d, 0xd0, 0x38, 0xb2, 0x31, 0x5d, 0xc6, 0x45, 0x72, 0x8a, 0x08, 0x0e, 0xc7, 0x4f, 0xc7, 0x76,
    0xd1, 0x19, 0xed, 0x8b, 0x17, 0x50, 0xb3, 0xa6, 0x94, 0x2e, 0xc8, 0x6b, 0xbb, 0x02, 0xc7, 0x4d
};

const uint8_t anti_spoofing_private_key[] =
{   0x84, 0xee, 0x67, 0xc3, 0x67, 0xea, 0x57, 0x38, 0xa7, 0x7e, 0xe2, 0x4d, 0x68, 0xaa, 0x9c, 0xf0,
    0xc7, 0x9f, 0xc8, 0x07, 0x7e, 0x4e, 0x20, 0x35, 0x4c, 0x15, 0x43, 0x4d, 0xb5, 0xd2, 0xd1, 0xc3
};

#elif (APP_BLE_FASTPAIR_MODEL_ID == 0xCE948F)
const uint8_t anti_spoofing_public_key[] =
{
    0x0e, 0xe2, 0xbf, 0xe7, 0x96, 0xc6, 0xe1, 0x13, 0xf6, 0x57, 0x4a, 0xa8, 0x8c, 0x3a, 0x1b, 0x9c,
    0x67, 0x1e, 0x36, 0xdf, 0x62, 0x69, 0xd8, 0xe5, 0x07, 0xe6, 0x8a, 0x72, 0x66, 0x4c, 0x9c, 0x90,
    0xfc, 0xff, 0x00, 0x4f, 0x0f, 0x95, 0xde, 0x63, 0xe1, 0xc0, 0xbb, 0xa0, 0x75, 0xb1, 0xd2, 0x76,
    0xfd, 0xe9, 0x66, 0x25, 0x0d, 0x45, 0x43, 0x7d, 0x5b, 0xf9, 0xce, 0xc0, 0xeb, 0x11, 0x03, 0xbe
};

const uint8_t anti_spoofing_private_key[] =
{
    0x71, 0x11, 0x42, 0xb5, 0xe4, 0xa0, 0x6c, 0xa2, 0x8b, 0x74, 0xd4, 0x87, 0x7d, 0xac, 0x15, 0xc5,
    0x42, 0x38, 0x1d, 0xb7, 0xba, 0x21, 0x19, 0x60, 0x17, 0x67, 0xfc, 0xba, 0x67, 0x47, 0x44, 0xc6
};

#else   // APP_BLE_FASTPAIR_MODEL_ID
const uint8_t anti_spoofing_public_key[] =  "";
const uint8_t anti_spoofing_private_key[] = "";
#warning "No Anti-Spooging key"

#endif  // APP_BLE_FASTPAIR_MODEL_ID
#endif //FASTPAIR_ENABLE


/*
 * Structures
 */
typedef struct
{
    uint16_t conn_id;
    uint16_t mtu;
} app_ble_connection_t;

typedef struct
{
    app_ble_callback_t *p_callback;
    app_ble_connection_t connections[1];    /* Only one LE connection supported */
} app_ble_cb_t;

typedef struct
{
    uint16_t handle;
    uint16_t attr_len;
    void *p_attr;
} app_ble_attribute_t;

/*
 * Global variables
 */
static const uint8_t app_ble_gatt_server_db[]=
{
        // Declare mandatory GATT service
        PRIMARY_SERVICE_UUID16(APP_BLE_HANDLE_GATT_SERVICE, UUID_SERVICE_GATT),

        // Declare mandatory GAP service. Device Name and Appearance are mandatory
        // characteristics of GAP service
        PRIMARY_SERVICE_UUID16(APP_BLE_HANDLE_GAP_SERVICE, UUID_SERVICE_GAP),

            // Declare mandatory GAP service characteristic: Dev Name
            CHARACTERISTIC_UUID16(APP_BLE_HANDLE_CHAR_DEV_NAME, APP_BLE_HANDLE_CHAR_DEV_NAME_VAL,
                UUID_CHARACTERISTIC_DEVICE_NAME, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE /*| LEGATTDB_PERM_AUTH_READABLE*/),

            // Declare mandatory GAP service characteristic: Appearance
            CHARACTERISTIC_UUID16(APP_BLE_HANDLE_CHAR_DEV_APPEARANCE, APP_BLE_HANDLE_CHAR_DEV_APPEARANCE_VAL,
                UUID_CHARACTERISTIC_APPEARANCE, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE /*| LEGATTDB_PERM_AUTH_READABLE*/),

#ifdef FASTPAIR_ENABLE
        // Declare Fast Pair service
        PRIMARY_SERVICE_UUID16(APP_BLE_HANDLE_FASTPAIR_SERVICE, WICED_BT_GFPS_UUID16),

            CHARACTERISTIC_UUID16_WRITABLE(APP_BLE_HANDLE_FASTPAIR_SERVICE_CHAR_KEY_PAIRING,
                                           APP_BLE_HANDLE_FASTPAIR_SERVICE_CHAR_KEY_PAIRING_VAL,
                                           WICED_BT_GFPS_UUID_CHARACTERISTIC_KEY_PAIRING,
                                           LEGATTDB_CHAR_PROP_WRITE | LEGATTDB_CHAR_PROP_NOTIFY,
                                           LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_REQ),

            CHAR_DESCRIPTOR_UUID16_WRITABLE(APP_BLE_HANDLE_FASTPAIR_SERVICE_CHAR_KEY_PAIRING_CFG_DESC,
                                            UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                                            LEGATTDB_PERM_AUTH_READABLE | LEGATTDB_PERM_WRITE_REQ),

            CHARACTERISTIC_UUID16_WRITABLE(APP_BLE_HANDLE_FASTPAIR_SERVICE_CHAR_PASSKEY,
                                           APP_BLE_HANDLE_FASTPAIR_SERVICE_CHAR_PASSKEY_VAL,
                                           WICED_BT_GFPS_UUID_CHARACTERISTIC_PASSKEY,
                                           LEGATTDB_CHAR_PROP_WRITE | LEGATTDB_CHAR_PROP_NOTIFY,
                                           LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_REQ),

            CHAR_DESCRIPTOR_UUID16_WRITABLE(APP_BLE_HANDLE_FASTPAIR_SERVICE_CHAR_PASSKEY_CFG_DESC,
                                            UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                                            LEGATTDB_PERM_AUTH_READABLE | LEGATTDB_PERM_WRITE_REQ),

            CHARACTERISTIC_UUID16_WRITABLE(APP_BLE_HANDLE_FASTPAIR_SERVICE_CHAR_ACCOUNT_KEY,
                                           APP_BLE_HANDLE_FASTPAIR_SERVICE_CHAR_ACCOUNT_KEY_VAL,
                                           WICED_BT_GFPS_UUID_CHARACTERISTIC_ACCOUNT_KEY,
                                           LEGATTDB_CHAR_PROP_WRITE,
                                           LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_REQ),

            CHAR_DESCRIPTOR_UUID16_WRITABLE(APP_BLE_HANDLE_FASTPAIR_SERVICE_CHAR_ACCOUNT_KEY_CFG_DESC,
                                            UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                                            LEGATTDB_PERM_AUTH_READABLE | LEGATTDB_PERM_WRITE_REQ),
#endif

        // Handle 0xff00: Broadcom vendor specific WICED Upgrade Service.
#ifdef OTA_SECURE_FIRMWARE_UPGRADE
        PRIMARY_SERVICE_UUID128(HANDLE_OTA_FW_UPGRADE_SERVICE, UUID_OTA_SEC_FW_UPGRADE_SERVICE),
#else
        PRIMARY_SERVICE_UUID128(HANDLE_OTA_FW_UPGRADE_SERVICE, UUID_OTA_FW_UPGRADE_SERVICE),
#endif
            // Handles 0xff03: characteristic WS Control Point, handle 0xff04 characteristic value.
            CHARACTERISTIC_UUID128_WRITABLE(HANDLE_OTA_FW_UPGRADE_CHARACTERISTIC_CONTROL_POINT, HANDLE_OTA_FW_UPGRADE_CONTROL_POINT,
                UUID_OTA_FW_UPGRADE_CHARACTERISTIC_CONTROL_POINT, LEGATTDB_CHAR_PROP_WRITE | LEGATTDB_CHAR_PROP_NOTIFY | LEGATTDB_CHAR_PROP_INDICATE,
                LEGATTDB_PERM_VARIABLE_LENGTH | LEGATTDB_PERM_WRITE_REQ /*| LEGATTDB_PERM_AUTH_WRITABLE*/),

                // Declare client characteristic configuration descriptor
                // Value of the descriptor can be modified by the client
                // Value modified shall be retained during connection and across connection
                // for bonded devices.  Setting value to 1 tells this application to send notification
                // when value of the characteristic changes.  Value 2 is to allow indications.
                CHAR_DESCRIPTOR_UUID16_WRITABLE(HANDLE_OTA_FW_UPGRADE_CLIENT_CONFIGURATION_DESCRIPTOR, UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                    LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_REQ /*| LEGATTDB_PERM_AUTH_WRITABLE */),

            // Handle 0xff07: characteristic WS Data, handle 0xff08 characteristic value. This
            // characteristic is used to send next portion of the FW Similar to the control point
            CHARACTERISTIC_UUID128_WRITABLE(HANDLE_OTA_FW_UPGRADE_CHARACTERISTIC_DATA, HANDLE_OTA_FW_UPGRADE_DATA,
                UUID_OTA_FW_UPGRADE_CHARACTERISTIC_DATA, LEGATTDB_CHAR_PROP_WRITE,
                LEGATTDB_PERM_VARIABLE_LENGTH | LEGATTDB_PERM_WRITE_REQ /*| LEGATTDB_PERM_AUTH_WRITABLE */),
};

char app_ble_device_name[APP_MAIN_BT_DEV_NAME_LEN] = {0};
const uint8_t app_ble_appearance_name[2] = { BIT16_TO_8(APPEARANCE_GENERIC_MEDIA_PLAYER) };

const app_ble_attribute_t app_ble_attribute[] =
{
    {
        .handle = APP_BLE_HANDLE_CHAR_DEV_NAME_VAL,
        .attr_len = sizeof(app_ble_device_name),
        .p_attr = (void *)app_ble_device_name
    },
    {
        .handle = APP_BLE_HANDLE_CHAR_DEV_APPEARANCE_VAL,
        .attr_len = sizeof(app_ble_appearance_name),
        .p_attr = (void *)app_ble_appearance_name
    },
};

static app_ble_cb_t app_ble_cb;

static wiced_bt_ble_advert_elem_t app_ble_adv_elem = {0};

/*
 * Local functions
 */
static wiced_bt_gatt_status_t app_ble_gatt_callback(wiced_bt_gatt_evt_t event,
        wiced_bt_gatt_event_data_t *p_data);
static wiced_bt_gatt_status_t app_ble_gatt_attribute_request_handler(
        wiced_bt_gatt_attribute_request_t *p_data);
static wiced_bt_gatt_status_t app_ble_gatt_attribute_read_request_handler(uint16_t conn_id,
        wiced_bt_gatt_read_t *p_read_data);
static wiced_bt_gatt_status_t app_ble_gatt_attribute_write_request_handler(uint16_t conn_id,
        wiced_bt_gatt_write_t *p_write_data);

/*
 * app_ble_discoverabilty_change_callback
 *
 * callback function when the LE discoverability has been changed
 */
static void app_ble_discoverabilty_change_callback(wiced_bool_t discoverable)
{
#ifdef FASTPAIR_ENABLE
    wiced_bt_gfps_provider_discoverablility_set(discoverable);
#endif
}

#ifdef FASTPAIR_ENABLE
/*
 * Initiate Google Fast Pair Service Provider
 */
static void app_ble_init_fast_pair(void)
{
    wiced_bt_gfps_provider_conf_t fastpair_conf = {0};

    /* set Tx power level data type in LE advertisement */
#if defined(CYW20719B2) || defined(CYW20721B2) || defined(CYW20819A1) || defined (CYW20820A1)
    fastpair_conf.ble_tx_pwr_level = wiced_bt_cfg_settings.default_ble_power_level;
#else
    fastpair_conf.ble_tx_pwr_level = 0;
#endif

    /* set GATT event callback */
    fastpair_conf.p_gatt_cb = app_ble_gatt_callback;

    /* set assigned handles for GATT attributes */
    fastpair_conf.gatt_db_handle.key_pairing_val        = APP_BLE_HANDLE_FASTPAIR_SERVICE_CHAR_KEY_PAIRING_VAL;
    fastpair_conf.gatt_db_handle.key_pairing_cfg_desc   = APP_BLE_HANDLE_FASTPAIR_SERVICE_CHAR_KEY_PAIRING_CFG_DESC;
    fastpair_conf.gatt_db_handle.passkey_val            = APP_BLE_HANDLE_FASTPAIR_SERVICE_CHAR_PASSKEY_VAL;
    fastpair_conf.gatt_db_handle.passkey_cfg_desc       = APP_BLE_HANDLE_FASTPAIR_SERVICE_CHAR_PASSKEY_CFG_DESC;
    fastpair_conf.gatt_db_handle.account_key_val        = APP_BLE_HANDLE_FASTPAIR_SERVICE_CHAR_ACCOUNT_KEY_VAL;

    /* model id */
    fastpair_conf.model_id = APP_BLE_FASTPAIR_MODEL_ID;

    /* anti-spoofing public key */
    memcpy((void *) &fastpair_conf.anti_spoofing_key.public[0],
           (void *) &anti_spoofing_public_key[0],
           WICED_BT_GFPS_ANTI_SPOOFING_KEY_LEN_PUBLIC);

    /* anti-spoofing private key */
    memcpy((void *) &fastpair_conf.anti_spoofing_key.private[0],
           (void *) &anti_spoofing_private_key[0],
           WICED_BT_GFPS_ANTI_SPOOFING_KEY_LEN_PRIVATE);

    /* Account Key Filter generate format */
    fastpair_conf.account_key_filter_generate_random = WICED_TRUE;;

    /* Account Key list size */
    fastpair_conf.account_key_list_size = FASTPAIR_ACCOUNT_KEY_NUM;

    /* NVRAM id for Account Key list */
    fastpair_conf.account_key_list_nvram_id = NVRAM_ID_GFPS_ACCOUNT_KEY;

    /* LE advertisement data appended to fast pair advertisement data */
    app_ble_adv_elem.advert_type    = BTM_BLE_ADVERT_TYPE_NAME_COMPLETE;
    app_ble_adv_elem.len            = strlen((const char *) app_ble_device_name);
    app_ble_adv_elem.p_data         = (uint8_t *)app_ble_device_name;

    fastpair_conf.appended_adv_data.p_elem      = &app_ble_adv_elem;
    fastpair_conf.appended_adv_data.elem_num    = 1;

    /* Initialize Google Fast Pair Service. */
    if (wiced_bt_gfps_provider_init(&fastpair_conf) == WICED_FALSE)
    {
        APP_TRACE_DBG("wiced_bt_gfps_provider_init fail\n");
    }

    /* Register the LE discoverability change callback. */
    bt_hs_spk_ble_discoverability_change_callback_register(&app_ble_discoverabilty_change_callback);
}

#else
/*
 * app_ble_advert_set_data
 */
static wiced_result_t app_ble_advert_set_data(void)
{
    wiced_bt_ble_advert_elem_t adv_elem[2];
    uint8_t num_elem = 0;
    uint8_t flag = BTM_BLE_GENERAL_DISCOVERABLE_FLAG | BTM_BLE_BREDR_NOT_SUPPORTED;
    wiced_result_t status;

    APP_TRACE_DBG("\n");

    adv_elem[num_elem].advert_type  = BTM_BLE_ADVERT_TYPE_FLAG;
    adv_elem[num_elem].len          = sizeof(uint8_t);
    adv_elem[num_elem].p_data       = &flag;
    num_elem++;

    if (strlen(app_ble_device_name) > 0)
    {
        adv_elem[num_elem].advert_type  = BTM_BLE_ADVERT_TYPE_NAME_COMPLETE;
        adv_elem[num_elem].len          = strlen(app_ble_device_name);
        adv_elem[num_elem].p_data       = (uint8_t*)app_ble_device_name;
        num_elem++;
    }

    status = wiced_bt_ble_set_raw_advertisement_data(num_elem, adv_elem);
    if (status != WICED_BT_SUCCESS)
    {
        APP_TRACE_ERR("wiced_bt_ble_set_raw_advertisement_data failed %d\n", status);
    }

    return status;
}

#endif

/*
 * app_ble_init
 */
wiced_result_t app_ble_init(app_ble_callback_t *p_callback)
{
    wiced_bt_gatt_status_t gatt_status;

    APP_TRACE_DBG("\n");

    memset(&app_ble_cb, 0, sizeof(app_ble_cb));

    app_ble_cb.p_callback = p_callback;

    /*  GATT DB Initialization */
    gatt_status = wiced_bt_gatt_db_init(app_ble_gatt_server_db, sizeof(app_ble_gatt_server_db));
    if (gatt_status != WICED_BT_GATT_SUCCESS)
    {
        APP_TRACE_ERR("wiced_bt_gatt_db_init failed %d\n", gatt_status);
        return WICED_BT_ERROR;
    }

#ifdef FASTPAIR_ENABLE
    /* Initiate Google Fast Pair Service Provider. */
    app_ble_init_fast_pair();

#else
    app_ble_advert_set_data();

    /* GATT registration */
    gatt_status = wiced_bt_gatt_register( app_ble_gatt_callback );
    WICED_BT_TRACE( "wiced_bt_gatt_register status %d\n", gatt_status );

#endif

    return WICED_BT_SUCCESS;
}

/*
 * app_ble_gatt_callback
 */
static wiced_bt_gatt_status_t app_ble_gatt_callback(wiced_bt_gatt_evt_t event,
        wiced_bt_gatt_event_data_t *p_data)
{
    app_ble_event_data_t event_data;
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    switch( event )
    {
    case GATT_CONNECTION_STATUS_EVT:
        if (p_data->connection_status.connected)
        {
            APP_TRACE_DBG("LE GATT Connection Up Bdaddr:%B AddrType:%d ConnId:%d\n",
                    p_data->connection_status.bd_addr,
                    p_data->connection_status.addr_type,
                    p_data->connection_status.conn_id);
            if (app_ble_cb.connections[0].conn_id != 0)
            {
                /* This application supports one LE Connection only */
                wiced_bt_gatt_disconnect(p_data->connection_status.conn_id);
            }
            else
            {
                app_ble_cb.connections[0].conn_id = p_data->connection_status.conn_id;
                app_ble_cb.connections[0].mtu = GATT_DEF_BLE_MTU_SIZE;

                if (app_ble_cb.p_callback == NULL)
                {
                    APP_TRACE_ERR("no callback\n");
                    return WICED_BT_GATT_INTERNAL_ERROR;
                }
#if defined (OTA_FW_UPGRADE) && (APP_OFU_SUPPORT)
                app_ofu_ble_connection_up(p_data->connection_status.conn_id,
                        p_data->connection_status.bd_addr);
#endif
                /* Send the event to the app */
                event_data.gatt_connected.conn_id = p_data->connection_status.conn_id;
                event_data.gatt_connected.status = WICED_BT_SUCCESS;
                memcpy(&event_data.gatt_connected.bdaddr, p_data->connection_status.bd_addr, BD_ADDR_LEN);
                app_ble_cb.p_callback(APP_BLE_EVENT_GATT_CONNECTED, &event_data);
            }
        }
        else
        {
            APP_TRACE_DBG("LE GATT Connection Down ConnId:%d\n",
                    p_data->connection_status.conn_id);

            if (app_ble_cb.connections[0].conn_id != p_data->connection_status.conn_id)
            {
                APP_TRACE_ERR("Unknown ConnId:%d\n", p_data->connection_status.conn_id);
                return WICED_BT_GATT_INTERNAL_ERROR;
            }
#if defined (OTA_FW_UPGRADE) && (APP_OFU_SUPPORT)
            app_ofu_ble_connection_down(p_data->connection_status.conn_id);
#endif
            app_ble_cb.connections[0].conn_id = 0;
            app_ble_cb.connections[0].mtu = GATT_DEF_BLE_MTU_SIZE;
        }
        break;

    case GATT_OPERATION_CPLT_EVT:
        APP_TRACE_DBG("LE GATT Operation Complete\n");
        break;

    case GATT_DISCOVERY_RESULT_EVT:
        APP_TRACE_DBG("LE GATT Discovery Result\n");
        break;

    case GATT_DISCOVERY_CPLT_EVT:
        APP_TRACE_DBG("LE GATT Discovery Complete\n");
        break;

    case GATT_ATTRIBUTE_REQUEST_EVT:
        status = app_ble_gatt_attribute_request_handler(&p_data->attribute_request);
        break;

    default:
        break;
    }

    return status;
}

/*
 * app_ble_advert_enable
 */
wiced_result_t app_ble_advert_enable(wiced_bt_ble_advert_mode_t adv_mode)
{
    wiced_result_t status;

    APP_TRACE_DBG("adv_mode:%d\n", adv_mode);

    status = wiced_bt_start_advertisements(adv_mode, 0, NULL);
    if (status != WICED_BT_SUCCESS)
    {
        APP_TRACE_ERR("wiced_bt_start_advertisements failed %d\n", status);
    }

    return status;
}

/*
 * app_ble_name_set
 */
wiced_result_t app_ble_name_set(char *p_ble_dev_name)
{
    char appended_ble_dev_name[] = " LE";
    int length,max_length;

    APP_TRACE_DBG("Name:%s\n", p_ble_dev_name);

    length = strlen(p_ble_dev_name);
    max_length = APP_MAIN_BT_DEV_NAME_LEN - strlen(appended_ble_dev_name);

    if (length >= max_length)
    {
        length = max_length - 1;
        APP_TRACE_ERR("Name tool long. Clipping length to %d\n", length);
    }

    memcpy(app_ble_device_name, p_ble_dev_name, length);
    /* Add LE for advertising */
    memcpy(app_ble_device_name + length, appended_ble_dev_name, strlen(appended_ble_dev_name));
    app_ble_device_name[length + strlen(appended_ble_dev_name)] = '\0';
    /* Update Advertising data with the new name */
    app_ble_adv_elem.len = strlen((const char *) app_ble_device_name);

#ifdef FASTPAIR_ENABLE
    wiced_bt_gfps_provider_advertisement_data_update(BTM_BLE_ADVERT_TYPE_NAME_COMPLETE,
                                                     strlen((const char *) app_ble_device_name),
                                                     (uint8_t *)app_ble_device_name);

    return WICED_BT_SUCCESS;

#else
    /* Update Advertising data with the new name */
    return app_ble_advert_set_data();

#endif

}

/*
 * app_ble_advert_state_changed
 */
void app_ble_advert_state_changed(wiced_bt_ble_advert_mode_t mode)
{
    app_ble_event_data_t event_data;

    if (app_ble_cb.p_callback == NULL)
    {
        APP_TRACE_ERR("no callback\n");
        return;
    }

    /* Send the event to the app */
    event_data.adv_changed.mode = mode;
    app_ble_cb.p_callback(APP_BLE_EVENT_ADV_CHANGED, &event_data);
}

/*
 * app_ble_gatt_attribute_request_handler
 */
static wiced_bt_gatt_status_t app_ble_gatt_attribute_request_handler(
        wiced_bt_gatt_attribute_request_t *p_data)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_INVALID_PDU;

    APP_TRACE_DBG("LE GATT Attribute Request con_id:%d req_type:%d\n",
            p_data->conn_id, p_data->request_type);

    switch ( p_data->request_type )
    {
    case GATTS_REQ_TYPE_READ:
        status = app_ble_gatt_attribute_read_request_handler(p_data->conn_id, &p_data->data.read_req);
        break;

    case GATTS_REQ_TYPE_WRITE:
        APP_TRACE_DBG("LE GATT Attribute Write Handle:0x%x length:%d\n",
                p_data->data.write_req.handle, p_data->data.write_req.val_len);
        status = app_ble_gatt_attribute_write_request_handler(p_data->conn_id, &p_data->data.write_req);
        break;

    case GATTS_REQ_TYPE_WRITE_EXEC:
        APP_TRACE_DBG("LE GATT Attribute Write Exec\n", p_data->data.exec_write);
        break;

    case GATTS_REQ_TYPE_MTU:
        APP_TRACE_DBG("LE GATT Attribute MTU:%d\n", p_data->data.mtu);
        break;

    case GATTS_REQ_TYPE_CONF:
        APP_TRACE_DBG("LE GATT Attribute Conf Handle:0x%x\n", p_data->data.handle);
        break;

   default:
        break;
    }

    return status;
}

/*
 * app_ble_gatt_attribute_read_request_handler
 */
static wiced_bt_gatt_status_t app_ble_gatt_attribute_read_request_handler(uint16_t conn_id,
        wiced_bt_gatt_read_t *p_read_data)
{
    int index;
    app_ble_attribute_t *p_attr = NULL;
    int attr_len_to_copy;

    APP_TRACE_DBG("handle:0x%x\n", p_read_data->handle);

    /* First, check if this is basic Attribute handle (e.g. Name or Appearance) */
    for (index = 0 ; index < (sizeof(app_ble_attribute)/sizeof(app_ble_attribute[0])) ; index++)
    {
        if (app_ble_attribute[index].handle == p_read_data->handle)
        {
            p_attr = (app_ble_attribute_t *)&app_ble_attribute[index];
            break;
        }
    }

    /* If this Handle was not found in the Attribute Handle list, check if its OTA FW Upgrade */
    if (p_attr == NULL)
    {
#if defined (OTA_FW_UPGRADE) && (APP_OFU_SUPPORT)
        /* if read request is for the OTA FW upgrade service, pass it to the library to process */
        if ((p_read_data->handle > HANDLE_OTA_FW_UPGRADE_SERVICE) &&
            (p_read_data->handle <= HANDLE_OTA_FW_UPGRADE_APP_INFO))
        {
            return app_ofu_ble_read_handler(conn_id, p_read_data);
        }
        else
#endif
        {
            return WICED_BT_GATT_INVALID_HANDLE;
        }
    }

    /*
     * Handle basic Read Request
     * */
    attr_len_to_copy = p_attr->attr_len;

    APP_TRACE_DBG("conn_id:%d hdl:0x%x offset:%d len:%d\n", conn_id, p_read_data->handle,
            p_read_data->offset, attr_len_to_copy);

    if (p_read_data->offset >= p_attr->attr_len)
    {
        attr_len_to_copy = 0;
    }

    if (attr_len_to_copy != 0)
    {
        uint8_t *from;
        int to_copy = attr_len_to_copy - p_read_data->offset;

        if (to_copy > *p_read_data->p_val_len)
        {
            to_copy = *p_read_data->p_val_len;
        }

        from = ((uint8_t *)p_attr->p_attr) + p_read_data->offset;
        *p_read_data->p_val_len = to_copy;

        memcpy( p_read_data->p_val, from, to_copy);
    }

    return WICED_BT_GATT_SUCCESS;
}

/*
 * app_ble_gatt_attribute_write_request_handler
 */
static wiced_bt_gatt_status_t app_ble_gatt_attribute_write_request_handler(uint16_t conn_id,
        wiced_bt_gatt_write_t *p_write_data)
{
#if defined (OTA_FW_UPGRADE) && (APP_OFU_SUPPORT)
    /* if read request is for the OTA FW upgrade service, pass it to the library to process */
    if ((p_write_data->handle > HANDLE_OTA_FW_UPGRADE_SERVICE) &&
        (p_write_data->handle <= HANDLE_OTA_FW_UPGRADE_APP_INFO))
    {
        return app_ofu_ble_write_handler(conn_id, p_write_data);
    }
    else
#endif
    {
        APP_TRACE_ERR("invalid handle:0x%x\n", p_write_data->handle);
        return WICED_BT_GATT_INVALID_HANDLE;
    }
}
