/****************************************************************************
* File Name        : Cooktop_Demo_config.c
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

#include "Cooktop_Demo_config.h"

#include "mtb_ml.h"
#include "mtb_ml_model_16x8.h"
#include "AM_LSTM_tflm_model_int16x8.h"

#include "ifx_va_prms.h"
#include "ifx_sp_common_priv.h"

#include "Cooktop_Demo_U55_WWmodel.h"
#include "Cooktop_Demo_U55_CMDmodel.h"
#include "U55_NMBmodel.h"

#include "Cooktop_Demo.h"
#include "Cooktop_Demo_ifx_va_config_prms.h"

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
    .intent_name_list = Cooktop_Demo_intent_name_list,
    .variable_name_list = Cooktop_Demo_variable_name_list,
    .variable_phrase_list = Cooktop_Demo_variable_phrase_list,
    .unit_phrase_list = Cooktop_Demo_unit_phrase_list,
    .intent_map_array = Cooktop_Demo_intent_map_array,
    .intent_map_array_sizes = Cooktop_Demo_intent_map_array_sizes,
    .variable_phrase_sizes = Cooktop_Demo_variable_phrase_sizes,
    .unit_phrase_map_array = Cooktop_Demo_unit_phrase_map_array,
    .unit_phrase_map_array_sizes = Cooktop_Demo_unit_phrase_map_array_sizes,
    .NUM_UNIT_PHRASES = sizeof(Cooktop_Demo_unit_phrase_list),
};

// WW config
static mtb_wwd_conf_t ww_conf = {
    .ww_params = Cooktop_Demo_dfww_prms,
    .callback.cb_for_event = CY_EVENT_SOD,
    .callback.cb_function = Cooktop_Demo_wake_word_callback
};

// NLU config
static mtb_nlu_config_t nlu_conf = {
    .nlu_params = Cooktop_Demo_dfcmd_prms,
    .nlu_command_timeout = 5000,
};

static mtb_wwd_nlu_config_t ww_1_conf = {
    .ww_model_ptr = Cooktop_Demo_WWmodeldata,
    .cmd_model_ptr = Cooktop_Demo_CMDmodeldata,
    .nmb_model_ptr = NMBmodeldata,
    .wwd_nlu_buff_data = &wwd_nlu_buff,
    .sod_params = Cooktop_Demo_sod_prms,
    .hpf_params = Cooktop_Demo_pre_proc_hpf_prms,
    .denoise_params = Cooktop_Demo_denoise_prms,
    .ww_conf = &ww_conf,
    .nlu_conf.nlu_config = &nlu_conf,
    .nlu_conf.nlu_variable_data = &nlu_setup_array,
};

mtb_wwd_nlu_config_t *Cooktop_Demo_ww_nlu_configs[COOKTOP_DEMO_NO_OF_WAKE_WORD] = {&ww_1_conf
};

char *Cooktop_Demo_ww_str[COOKTOP_DEMO_NO_OF_WAKE_WORD] = {"Hey Cook Top"};

const char* Cooktop_Demo_intent_name_list[COOKTOP_DEMO_NUM_INTENTS] = {
    "SetPower",
    "SetHob",
    "SetTemp",
    "SetTimer",
    "OffMic",
};

const char* Cooktop_Demo_variable_name_list[COOKTOP_DEMO_NUM_VARIABLES] = {
    "OnOff",
    "Hob",
    "Temp",
    "Minutes",
};

const char* Cooktop_Demo_variable_phrase_list[COOKTOP_DEMO_NUM_VARIABLE_PHRASES] = {
    "on", "off", // OnOff
    "", // Hob
    "", // Temp
    "", // Minutes
};

const char* Cooktop_Demo_unit_phrase_list[COOKTOP_DEMO_NUM_UNIT_PHRASES] = {
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

const int Cooktop_Demo_intent_map_array[COOKTOP_DEMO_INTENT_MAP_ARRAY_TOTAL_SIZE] = {
    0, 1, 0, 0, // power (on)
    0, 1, 0, 1, // power (off) <end-silence>
    1, 1, 1, -1, // Set hob <numbers10> {}
    2, 1, 2, -1, // Set temp <numbers150to550by5> {degree,degrees}
    2, 1, 2, -1, // Set temp to <numbers150to550by5> {degree,degrees}
    2, 1, 2, -1, // Set temp at <numbers150to550by5> {degree,degrees}
    2, 1, 2, -1, // Set temperature <numbers150to550by5> {degree,degrees}
    2, 1, 2, -1, // Set temperature to <numbers150to550by5> {degree,degrees}
    2, 1, 2, -1, // Set temperature at <numbers150to550by5> {degree,degrees}
    3, 1, 3, -1, // Set timer to <numbers0to100by5> {minute,minutes}
    4, 0, // turn off microphone
    4, 0, // switch off microphone
    4, 0, // power off microphone
};

const int Cooktop_Demo_intent_map_array_sizes[COOKTOP_DEMO_NUM_COMMANDS] = {
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 2, 2, 2, 
};

const int Cooktop_Demo_variable_phrase_sizes[COOKTOP_DEMO_NUM_VARIABLES] = {
    2, // OnOff: (on,off)
    0, // Hob: None
    0, // Temp: None
    0, // Minutes: None
};

const int Cooktop_Demo_unit_phrase_map_array[COOKTOP_DEMO_UNIT_PHRASE_MAP_ARRAY_TOTAL_SIZE] = {
    0, // None
    0, // None
    1, 1, 13, // {}
    1, 2, 0, 1, // {degree,degrees}
    1, 2, 0, 1, // {degree,degrees}
    1, 2, 0, 1, // {degree,degrees}
    1, 2, 0, 1, // {degree,degrees}
    1, 2, 0, 1, // {degree,degrees}
    1, 2, 0, 1, // {degree,degrees}
    1, 2, 7, 8, // {minute,minutes}
    0, // None
    0, // None
    0, // None
};

const int Cooktop_Demo_unit_phrase_map_array_sizes[COOKTOP_DEMO_NUM_COMMANDS] = {
    1, 1, 3, 4, 4, 4, 4, 4, 4, 4, 1, 1, 1, 
};



__attribute__((weak)) void Cooktop_Demo_wake_word_callback(mtb_wwd_nlu_events_t event)
{

}
