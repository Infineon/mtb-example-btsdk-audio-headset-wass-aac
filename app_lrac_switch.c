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

#include "app_lrac.h"
#include "app_a2dp_sink.h"
#include "app_avrc_ct.h"
#include "app_handsfree.h"
#include "app_nvram.h"
#include "app_main.h"
#include "app_bt.h"
#include "app_volume.h"
#include "app_trace.h"
#include "wiced_bt_a2dp_sink.h"
#include "wiced_bt_avrc_ct.h"
#include "wiced_platform.h"
#include "wiced_platform_audio_common.h"
#include "wiced_memory.h"
#ifdef FASTPAIR_ENABLE
#include "wiced_bt_gfps.h"
#endif
#include "wiced_bt_ble.h"
#ifdef APP_OFU_SUPPORT
#include "ofu/app_ofu.h"
#endif


#ifdef FASTPAIR_ENABLE
/*
 * Local functions
 */
static wiced_bool_t app_lrac_gfps_provider_is_ready(void);
static wiced_result_t app_lrac_gfps_provider_switch_get(void *p_opaque, uint16_t *p_sync_data_len);
static wiced_result_t app_lrac_gfps_provider_switch_set(void *p_opaque, uint16_t sync_data_len);
#endif

/*
 * Types
 */
/* Switch Synchronization "Is Ready" function type */
typedef wiced_bool_t (app_lrac_switch_sync_is_ready_t)(void);

/* Switch Synchronization Data Get function type */
typedef wiced_result_t (app_lrac_switch_sync_get_t)(void *p_opaque, uint16_t *p_sync_data_len);

/* Switch Synchronization Data Set function type */
typedef wiced_result_t (app_lrac_switch_sync_set_t)(void *p_opaque, uint16_t sync_data_len);

/*
 * Structures
 */
typedef struct
{
    app_lrac_switch_sync_is_ready_t *p_ready;  /* Optional */
    app_lrac_switch_sync_get_t *p_get;
    app_lrac_switch_sync_set_t *p_set;
} app_lrac_switch_sync_fct_t;


#ifdef FASTPAIR_ENABLE
typedef struct __attribute__((packed))
{
    /* Account key database */
    wiced_bt_gfps_account_key_t account_key[FASTPAIR_ACCOUNT_KEY_NUM];

    /* Current GFPS discoverability */
    wiced_bool_t discoverability;

    /* Local LE RPA (Resolvable Private Address) */
    wiced_bt_device_address_t local_rpa;

    /* Local LE Key data. Use this structure to deliver the local IRK. */
    uint8_t local_key_data[BTM_SECURITY_LOCAL_KEY_DATA_LEN];

    /* Peer device's role */
    wiced_bt_lrac_role_t peer_role;
} app_lrac_gfps_provider_switch_data_t;
#endif

/*
 * Global variables
 */
const static app_lrac_switch_sync_fct_t app_lrac_switch_sync_fct[] =
{
    /*
     * Application's files
     */
    {
        .p_ready = app_lrac_ready_to_switch,
        .p_get = app_lrac_switch_get,
        .p_set = app_lrac_switch_set
    },
    {
        .p_ready = NULL,
        .p_get = platform_switch_get,
        .p_set = platform_switch_set
    },
#if defined (OTA_FW_UPGRADE) && (APP_OFU_SUPPORT)
    {
        .p_ready = app_ofu_ready_to_switch,
        .p_get = app_ofu_switch_get,
        .p_set = app_ofu_switch_set,
    },
#endif
#ifdef VOLUME_EFFECT
    {
        .p_ready = app_volume_effect_ready_to_switch,
        .p_get = NULL,
        .p_set = NULL,
    },
#endif
    /*
     * Libraries
     */
    {
        .p_ready = NULL,
        .p_get = wiced_bt_a2dp_sink_lrac_switch_get,
        .p_set = wiced_bt_a2dp_sink_lrac_switch_set
    },
    {
        .p_ready = NULL,
        .p_get = wiced_bt_avrc_ct_lrac_switch_get,
        .p_set = wiced_bt_avrc_ct_lrac_switch_set
    },
    {
        .p_ready = NULL,
        .p_get = wiced_bt_hfp_hf_lrac_switch_get,
        .p_set = wiced_bt_hfp_hf_lrac_switch_set
    },
#ifdef FASTPAIR_ENABLE
    {
        .p_ready = app_lrac_gfps_provider_is_ready,
        .p_get = app_lrac_gfps_provider_switch_get,
        .p_set = app_lrac_gfps_provider_switch_set
    },
#endif
    /*
     * Main application (probably safer to make it last)
     */
    {
        .p_ready = app_main_switch_is_ready,
        .p_get = app_main_switch_get,
        .p_set = app_main_switch_set
    },
};

#ifdef FASTPAIR_ENABLE
/*
 * app_lrac_gfps_provider_switch_get
 *
 * Get the necessary information used for GFPS provider module after role switch.
 */
static wiced_result_t app_lrac_gfps_provider_switch_get(void *p_opaque, uint16_t *p_sync_data_len)
{
    app_lrac_gfps_provider_switch_data_t *p_switch_data = (app_lrac_gfps_provider_switch_data_t *) p_opaque;

    /* Check parameter. */
    if (p_opaque == NULL)
    {
        return WICED_BT_BADARG;
    }

    if (p_sync_data_len == NULL)
    {
        return WICED_BT_BADARG;
    }

    if (*p_sync_data_len < sizeof(app_lrac_gfps_provider_switch_data_t))
    {
        return WICED_BT_BADARG;
    }

    /* Get account key list. */
    if (wiced_bt_gfps_provider_account_key_list_get((uint8_t *) p_switch_data->account_key) == WICED_FALSE)
    {
        return WICED_BT_ERROR;
    }

    /* Get current GFPS discoverability. */
    p_switch_data->discoverability = wiced_bt_gfps_provider_discoverability_get();

    /* Get local RPA. */
    memcpy((void *) p_switch_data->local_rpa,
           (void *) wiced_btm_get_private_bda(),
           sizeof(wiced_bt_device_address_t));

    /* Local LE Key data. */
    if (app_nvram_local_irk_get(p_switch_data->local_key_data) == WICED_FALSE)
    {
        return WICED_BT_ERROR;
    }

    /* Device's role */
    p_switch_data->peer_role = app_lrac_config_role_get();

    *p_sync_data_len = sizeof(app_lrac_gfps_provider_switch_data_t);

    return WICED_BT_SUCCESS;
}

static wiced_result_t app_lrac_gfps_provider_switch_set(void *p_opaque, uint16_t sync_data_len)
{
    app_lrac_gfps_provider_switch_data_t *p_switch_data = (app_lrac_gfps_provider_switch_data_t *) p_opaque;
    uint8_t local_key_data[BTM_SECURITY_LOCAL_KEY_DATA_LEN];

    /* Check parameter. */
    if (p_opaque == NULL)
    {
        return WICED_BT_BADARG;
    }

    if (sync_data_len != sizeof(app_lrac_gfps_provider_switch_data_t))
    {
        return WICED_BT_BADARG;
    }

    if (p_switch_data->peer_role != WICED_BT_LRAC_ROLE_PRIMARY)
    {
        return WICED_BT_SUCCESS;
    }

    /* Update account key list database. */
    if (wiced_bt_gfps_provider_account_key_list_update(p_switch_data->account_key) == WICED_FALSE)
    {
        return WICED_BT_ERROR;
    }

    /* Update local LE key data. */
    if (app_nvram_local_irk_get(local_key_data) == WICED_FALSE)
    {
        wiced_bt_ble_set_local_identity_key_data(p_switch_data->local_key_data);
        app_nvram_local_irk_update(p_switch_data->local_key_data);
    }
    else
    {
        if (memcmp((void *) local_key_data,
                   (void *) p_switch_data->local_key_data,
                   BTM_SECURITY_LOCAL_KEY_DATA_LEN) != 0)
        {
            wiced_bt_ble_set_local_identity_key_data(p_switch_data->local_key_data);
            app_nvram_local_irk_update(p_switch_data->local_key_data);
        }
    }

    /* Update local RPA. */
    wiced_bt_ble_set_resolvable_private_address(p_switch_data->local_rpa);

    /* Set GFPS discoverability. */
    wiced_bt_gfps_provider_discoverablility_set(p_switch_data->discoverability);

    return WICED_BT_SUCCESS;
}

/*
 * app_lrac_gfps_provider_is_ready
 *
 * Check if the GFPS provider module is ready for role switch now.
 */
static wiced_bool_t app_lrac_gfps_provider_is_ready(void)
{
    return !wiced_bt_gfps_provider_pairing_state_get();
}
#endif

/*
 * app_lrac_switch_is_ready
 */
wiced_bool_t app_lrac_switch_is_ready(void)
{
    uint8_t nb_sync_get_fct;
    uint8_t tag;
    wiced_bool_t is_ready;

    nb_sync_get_fct = sizeof(app_lrac_switch_sync_fct) / sizeof(app_lrac_switch_sync_fct[0]);

    /* Call every "Is Ready"  functions to check if every module is ready to Switch */
    for (tag = 0 ; tag < nb_sync_get_fct ; tag++)
    {
        if (app_lrac_switch_sync_fct[tag].p_ready != NULL)
        {
            is_ready = app_lrac_switch_sync_fct[tag].p_ready();
            if (is_ready == WICED_FALSE)
            {
                APP_TRACE_ERR("Module %d is not ready to Switch\n", tag);
                return WICED_FALSE;
            }
        }
    }
    return WICED_TRUE;
}

/*
 * app_lrac_switch_data_collect
 */
wiced_result_t app_lrac_switch_data_collect(void)
{
    uint16_t sync_data_len;
    uint8_t tag;
    uint8_t nb_sync_get_fct;
    wiced_result_t status = WICED_BT_ERROR;
    wiced_bool_t last = WICED_FALSE;
    uint8_t *switch_data;


    switch_data = wiced_bt_lrac_share_buf_lock_and_get(WICED_BT_LRAC_SHARE_BUF_ID_SWITCH_COLLECT_BUF);
    if (switch_data == NULL)
    {
        APP_TRACE_ERR("Cannot allocate switch_data\n");
        return WICED_NO_MEMORY;
    }

    if (wiced_bt_lrac_share_buf_length() < WICED_BT_LRAC_SWITCH_BLOB_SIZE_MAX)
    {
        APP_TRACE_ERR("Share buffer too small\n");
        wiced_bt_lrac_share_buf_unlock(WICED_BT_LRAC_SHARE_BUF_ID_SWITCH_COLLECT_BUF);
        return WICED_NO_MEMORY;
    }

    nb_sync_get_fct = sizeof(app_lrac_switch_sync_fct) / sizeof(app_lrac_switch_sync_fct[0]);

    /* Collect Switch Synchronization data from LRAC Library */
    for (tag = 0 ; tag < nb_sync_get_fct ; tag++)
    {
        if (app_lrac_switch_sync_fct[tag].p_get)
        {
            sync_data_len = WICED_BT_LRAC_SWITCH_BLOB_SIZE_MAX;
            status = app_lrac_switch_sync_fct[tag].p_get(&switch_data[0], &sync_data_len);
            if (status == WICED_SUCCESS)
            {
                /* Check if this is the last tag */
                if (tag == (nb_sync_get_fct -1))
                    last = WICED_TRUE;

                /* Send Switch Data */
                status = wiced_bt_lrac_switch_data_rsp(last, tag, &switch_data[0], sync_data_len);
                if (status != WICED_BT_SUCCESS)
                {
                    APP_TRACE_ERR("wiced_bt_lrac_switch_data_rsp for tag:%d failed\n", tag);
                    break;
                }
            }
            else
            {
                APP_TRACE_ERR("p_get for tag:%d failed\n", tag);
                break;
            }
        }
    }

    wiced_bt_lrac_share_buf_unlock(WICED_BT_LRAC_SHARE_BUF_ID_SWITCH_COLLECT_BUF);

    return status;
}

/*
 * app_lrac_switch_data_apply
 */
wiced_result_t app_lrac_switch_data_apply(uint8_t tag, uint8_t *p_data, uint16_t length)
{
    uint8_t nb_sync_set_fct;
    wiced_result_t status;

    nb_sync_set_fct = sizeof(app_lrac_switch_sync_fct) / sizeof(app_lrac_switch_sync_fct[0]);

    if (tag < nb_sync_set_fct)
    {
        if (app_lrac_switch_sync_fct[tag].p_set)
        {
            status = app_lrac_switch_sync_fct[tag].p_set(p_data, length);
        }
        else
        {
            APP_TRACE_ERR("No Sync Set function for tag:%d\n", tag);
            status = WICED_BT_ERROR;
        }
    }
    else
    {
        APP_TRACE_ERR("Wrong tag:%d\n", tag);
        status = WICED_BT_ERROR;
    }
    return status;
}
