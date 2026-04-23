/****************************************************************************
* File Name        : LED_Demo_config.c
*
* Description      : This source file contains the configuration object for WWD and NLU
*
* Related Document : See README.md
*
*****************************************************************************
* Copyright 2025, Cypress Semiconductor Corporation (an Infineon company)
* All rights reserved.
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
*****************************************************************************/

#include "LED_Demo_config.h"

#include "mtb_ml.h"
#include "mtb_ml_model_16x8.h"
#include "AM_LSTM_tflm_model_int16x8.h"

#include "ifx_va_prms.h"
#include "ifx_sp_common_priv.h"

#include "LED_Demo_U55_WWmodel.h"
#include "LED_Demo_U55_CMDmodel.h"
#include "U55_NMBmodel.h"

#include "LED_Demo.h"
#include "LED_Demo_ifx_va_config_prms.h"

/* Following am_tensor_arena has been counted as part of persistent memory total size */
/* Tensor_arena buffer must be in SOCMEM and aligned by 16 which are required by U55 */
static uint8_t am_tensor_arena[AM_LSTM_ARENA_SIZE] __attribute__((aligned(16)))
                                          __attribute__((section(".cy_socmem_data")));


static int16_t data_feed_int[N_SEQ * FEATURE_BUF_SZ] __attribute__((aligned(16)));
static float mtb_ml_input_buffer[N_SEQ * FEATURE_BUF_SZ];

static float xIn[FRAME_SIZE_16K] __attribute__((section(".wwd_nlu_data3")));
static float features[FEATURE_BUF_SZ] __attribute__((section(".wwd_nlu_data4")));
static float output_scores[(N_PHONEMES + 1) * (1 + AM_LOOKBACK)] __attribute__((section(".wwd_nlu_data5")));

//common buffers
static mtb_wwd_nlu_buff_t wwd_nlu_buff =
{
    .am_model_bin = { MTB_ML_MODEL_BIN_DATA(AM_LSTM) },
    .am_model_buffer = {
        .tensor_arena = am_tensor_arena,
        .tensor_arena_size = AM_LSTM_ARENA_SIZE
    },
    .data_feed_int = data_feed_int,
    .mtb_ml_input_buffer = mtb_ml_input_buffer,
    .output_scores = output_scores,
    .xIn = xIn,
    .features = features
};

// NLU setup array
static mtb_nlu_setup_array_t nlu_setup_array =
{
    .intent_name_list = LED_Demo_intent_name_list,
    .variable_name_list = LED_Demo_variable_name_list,
    .variable_phrase_list = LED_Demo_variable_phrase_list,
    .unit_phrase_list = LED_Demo_unit_phrase_list,
    .intent_map_array = LED_Demo_intent_map_array,
    .intent_map_array_sizes = LED_Demo_intent_map_array_sizes,
    .variable_phrase_sizes = LED_Demo_variable_phrase_sizes,
    .unit_phrase_map_array = LED_Demo_unit_phrase_map_array,
    .unit_phrase_map_array_sizes = LED_Demo_unit_phrase_map_array_sizes,
    .NUM_UNIT_PHRASES = sizeof(LED_Demo_unit_phrase_list),
};

// WW config
static mtb_wwd_conf_t ww_conf = {
    .ww_params = LED_Demo_dfww_prms,
    .callback.cb_for_event = CY_EVENT_SOD,
    .callback.cb_function = LED_Demo_wake_word_callback
};

// NLU config
static mtb_nlu_config_t nlu_conf = {
    .nlu_params = LED_Demo_dfcmd_prms,
    .nlu_command_timeout = 5000,
};

static mtb_wwd_nlu_config_t ww_1_conf = {
    .ww_model_ptr = LED_Demo_WWmodeldata,
    .cmd_model_ptr = LED_Demo_CMDmodeldata,
    .nmb_model_ptr = NMBmodeldata,
    .wwd_nlu_buff_data = &wwd_nlu_buff,
    .sod_params = LED_Demo_sod_prms,
    .hpf_params = LED_Demo_pre_proc_hpf_prms,
    .denoise_params = LED_Demo_denoise_prms,
    .ww_conf = &ww_conf,
    .nlu_conf.nlu_config = &nlu_conf,
    .nlu_conf.nlu_variable_data = &nlu_setup_array,
};

mtb_wwd_nlu_config_t *LED_Demo_ww_nlu_configs[LED_DEMO_NO_OF_WAKE_WORD] = {&ww_1_conf
};

char *LED_Demo_ww_str[LED_DEMO_NO_OF_WAKE_WORD] = {"OK Infineon"};

const char* LED_Demo_intent_name_list[LED_DEMO_NUM_INTENTS] = {
    "TurnOnLight",
    "IncreaseBrightness",
    "DecreaseBrightness",
    "SetBrightness",
    "ToggleLight",
    "TurnOffLight",
};

const char* LED_Demo_variable_name_list[LED_DEMO_NUM_VARIABLES] = {
    "Brightness",
};

const char* LED_Demo_variable_phrase_list[LED_DEMO_NUM_VARIABLE_PHRASES] = {
    "", // Brightness
};

const char* LED_Demo_unit_phrase_list[LED_DEMO_NUM_UNIT_PHRASES] = {
    "degree", "degrees", 
    "percent", 
    "level", "levels", 
    "hour", "hours", 
    "minute", "minutes", 
    "second", "seconds", 
    "day", "days", 
    "", 
    "AM", 
    "PM", 
};

const int LED_Demo_intent_map_array[LED_DEMO_INTENT_MAP_ARRAY_TOTAL_SIZE] = {
    0, 0, // turn on the light
    0, 0, // switch on the light
    0, 0, // enable the light
    1, 0, // make the light brighter
    1, 0, // increase the brightness
    2, 0, // make the light dimmer
    2, 0, // decrease the brightness
    3, 1, 0, -1, // set the brightness to <numbers10> {}
    3, 1, 0, -1, // adjust the brightness to <numbers10> {}
    3, 1, 0, -1, // dim light to <numbers10> {}
    4, 0, // flip the light
    4, 0, // toggle the light
    4, 0, // change the light state
    5, 0, // turn off the light
    5, 0, // switch off the light
    5, 0, // disable the light
};

const int LED_Demo_intent_map_array_sizes[LED_DEMO_NUM_COMMANDS] = {
    2, 2, 2, 2, 2, 2, 2, 4, 4, 4, 2, 2, 2, 2, 2, 2, 
};

const int LED_Demo_variable_phrase_sizes[LED_DEMO_NUM_VARIABLES] = {
    0, // Brightness: None
};

const int LED_Demo_unit_phrase_map_array[LED_DEMO_UNIT_PHRASE_MAP_ARRAY_TOTAL_SIZE] = {
    0, // None
    0, // None
    0, // None
    0, // None
    0, // None
    0, // None
    0, // None
    1, 1, 13, // {}
    1, 1, 13, // {}
    1, 1, 13, // {}
    0, // None
    0, // None
    0, // None
    0, // None
    0, // None
    0, // None
};

const int LED_Demo_unit_phrase_map_array_sizes[LED_DEMO_NUM_COMMANDS] = {
    1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 1, 1, 1, 1, 1, 1, 
};



__attribute__((weak)) void LED_Demo_wake_word_callback(mtb_wwd_nlu_events_t event)
{

}
