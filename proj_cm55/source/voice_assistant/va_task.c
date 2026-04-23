/******************************************************************************
* File Name : va_task.c
*
* Description :
* DEEPCRAFT(TM) Voice Assistant Task handler
********************************************************************************
* (c) 2026, Infineon Technologies AG, or an affiliate of Infineon
* Technologies AG. All rights reserved.
* This software, associated documentation and materials ("Software") is
* owned by Infineon Technologies AG or one of its affiliates ("Infineon")
* and is protected by and subject to worldwide patent protection, worldwide
* copyright laws, and international treaty provisions. Therefore, you may use
* this Software only as provided in the license agreement accompanying the
* software package from which you obtained this Software. If no license
* agreement applies, then any use, reproduction, modification, translation, or
* compilation of this Software is prohibited without the express written
* permission of Infineon.
*
* Disclaimer: UNLESS OTHERWISE EXPRESSLY AGREED WITH INFINEON, THIS SOFTWARE
* IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* INCLUDING, BUT NOT LIMITED TO, ALL WARRANTIES OF NON-INFRINGEMENT OF
* THIRD-PARTY RIGHTS AND IMPLIED WARRANTIES SUCH AS WARRANTIES OF FITNESS FOR A
* SPECIFIC USE/PURPOSE OR MERCHANTABILITY.
* Infineon reserves the right to make changes to the Software without notice.
* You are responsible for properly designing, programming, and testing the
* functionality and safety of your intended application of the Software, as
* well as complying with any legal requirements related to its use. Infineon
* does not guarantee that the Software will be free from intrusion, data theft
* or loss, or other breaches ("Security Breaches"), and Infineon shall have
* no liability arising out of any Security Breaches. Unless otherwise
* explicitly approved by Infineon, the Software may not be used in any
* application where a failure of the Product or any consequences of the use
* thereof can reasonably be expected to result in personal injury.
*******************************************************************************/

/*******************************************************************************
* Header Files
*******************************************************************************/

#include "va_task.h"
#include "led_pwm.h"
#include "cybsp.h"

/* RTOS includes */
#include "FreeRTOS.h"
#include "task.h"

#ifdef SHOW_MCPS
#include "profiler.h"
#endif /* SHOW_MCPS */
#include "retarget_io_init.h"
#ifdef ENABLE_VOICE_ID
#include "voice_id_task.h"
#endif /* ENABLE_VOICE_ID */
#include "app_logger.h"

/*****************************************************************************
 * Macros
 *****************************************************************************/

#define VA_TASK_STACK_SIZE                        (1024)
#define VA_TASK_PRIORITY                          (4)

#define VA_AUDIO_DATA_IN_BYTES                    (320)
#define VA_QUEUE_SIZE                             (VA_AUDIO_DATA_IN_BYTES)
#define VA_QUEUE_ELEMENTS                         (10)    

/* This is the maximum size of the command string that can be detected by the 
 * voice assistant. 
 */
#define COMMAND_STRING_SIZE                       (250U) 

/* Command timeout when using single commands */
#define CMD_TIMEOUT_SINGLE_CMD                    (5000U)

/* Command timeout when using multiple commands */
#define CMD_TIMEOUT_MULTI_CMD                     (20000U)

/* Choose one of the following options to run the voice-assistant: 
 * VA_MODE_WW_SINGLE_CMD : For every wake word, a single command is detected
 * VA_MODE_WW_MULTI_CMD  : For every wake word, multiple commands can be detected
 * VA_MODE_WW_ONLY       : Only wake word detection is performed
 * VA_MODE_CMD_ONLY      : Only command detection is performed
 */
#define RUNNING_MODE                              (VA_MODE_WW_SINGLE_CMD) 

/* Number of audio channels sampled from microphones and processed */
#define NUM_AUDIO_CHANNELS                        (1U)

/* How often to print the MCPS (multiply by 10 ms) */
#define PRINT_MCPS_COUNT                        (100u) 

/*******************************************************************************
* Global Variables
*******************************************************************************/
/* Breathing counter for the kit's blue LED */
static uint8_t breathing_counter;

/* Required by the audio-voice-core to build */
uint8_t bf_coeffs[1];
uint32_t bf_coeffs_total_len;

#ifdef SHOW_MCPS
/* Variables used to print and calculate MCPS */
uint32_t show_count = 0;
uint32_t cpu_cycle_sum = 0;
#endif /* SHOW_MCPS */

TaskHandle_t rtos_va_task;
QueueHandle_t va_queue_handle;

uint8_t ptt_control_flag=0;
extern volatile uint8_t ptt_flag;

#ifdef ENABLE_VOICE_ID
extern volatile char voice_id_mode;
#endif /* ENABLE_VOICE_ID */

#ifdef USE_LED_DEMO
/*******************************************************************************
 * Function Name: led_demo
 *******************************************************************************
 * Summary:
 * Update the LED state and brightness based on the intent detected.
 *  
 * Parameters:
 *  intent: detected intent index
 *  brightness: LED brightness level (0-100)
  *
 * Return:
 *  void
 *
 *******************************************************************************/
static void led_demo(int intent, uint8_t brightness)
{
    /* LED PWM driver is used to control the LED brightness and state. */
    switch (intent)
    {
        case LED_DEMO_INTENT_TurnOnLight:
            led_pwm_on(LED_PWM_GREEN_LED);
            break;
        case LED_DEMO_INTENT_TurnOffLight:
            led_pwm_off(LED_PWM_GREEN_LED);
            break;
        case LED_DEMO_INTENT_IncreaseBrightness:
            led_pwm_set_brightness(LED_PWM_GREEN_LED, LED_PWM_MAX_BRIGHTNESS);
            break;
        case LED_DEMO_INTENT_DecreaseBrightness:
            led_pwm_set_brightness(LED_PWM_GREEN_LED, LED_PWM_MIN_BRIGHTNESS);
            break;
        case LED_DEMO_INTENT_SetBrightness:
            led_pwm_set_brightness(LED_PWM_GREEN_LED, brightness);
            break;
        case LED_DEMO_INTENT_ToggleLight:
            led_pwm_toggle(LED_PWM_GREEN_LED);
            break;
        default:
            break;
    }
}
#endif


#ifdef SHOW_MCPS
/*******************************************************************************
 * Function Name: print_mcps
 *******************************************************************************
 * Summary:
 * Prints the number of CPU cycles in Mega cycles per second (MCPS)
 *  
 * Parameters:
 *  
 * Return:
 *  void
 *
 *******************************************************************************/
static void print_mcps(void)
{
    cpu_cycle_sum += profiler_get_cycles();
    show_count++;
    if(show_count >= PRINT_MCPS_COUNT) 
    {
        app_log_print("Voice Assistant Profiler: %u MCPS\r\n", cpu_cycle_sum/1000000);
        show_count = 0;
        cpu_cycle_sum = 0;
    }
}
#endif /* SHOW_MCPS */

/*******************************************************************************
 * Function Name: print_voice_assistant_status
 *******************************************************************************
 * Summary:
 * Prints an error message if wake-word detection result is not successful. 
 * Or a detection message if wake-word is detected.
 * Also update the Green LED to indicate the voice assistant state.
 *  
 * Parameters:
 *  result: result of the voice-assistant operation
 *  event: state of the voice-assistant operation
 *  va_data: data detected from the voice-assistant operation
 *
 * Return:
 *  void
 *
 *******************************************************************************/
static void print_voice_assistant_status(cy_rslt_t result, va_event_t event, va_data_t *va_data)
{
    char command_text[COMMAND_STRING_SIZE] = {0};

    if (result == VA_RSLT_LICENSE_ERROR)
    {
        app_log_print("ERROR! Voice Assistant license expired!\r\n");
        handle_error();
    }
    else if ( result != VA_RSLT_SUCCESS )
    {
        app_log_print("Error! voice_assistant_process!! Error code=%d\r\n", result);
    }
    else
    {
        if ( event == VA_EVENT_WW_DETECTED )
        {
            if (RUNNING_MODE != VA_MODE_WW_ONLY)
            {
                breathing_counter = LED_PWM_MIN_BRIGHTNESS;
            }
            app_log_print("Wake-word detected!\r\n");
#ifdef ENABLE_VOICE_ID 
            if (get_enrolled_users()!=0){           
                voice_id_mode=IFX_VOICE_ID_VERIFY;
                app_log_print("Say a command \r\nVoice ID verification in progress ...\n\r");
            }
            else
            {
                app_log_print("Say a command \r\nNo user enrolled for Voice ID \r\n");
            }
            
#endif /* ENABLE_VOICE_ID */            
        }
        else if ( event == VA_EVENT_CMD_TIMEOUT )
        {
            if (RUNNING_MODE != VA_MODE_CMD_ONLY)
            {
                breathing_counter = 0;
                ptt_flag = 0;
                ptt_control_flag = 0;
                app_log_print("Command Timeout!\r\n");
            }
        }
        else if ( event == VA_EVENT_CMD_SILENCE_TIMEOUT )
        {
            if ((RUNNING_MODE != VA_MODE_WW_MULTI_CMD) && (RUNNING_MODE != VA_MODE_CMD_ONLY))
            {
                breathing_counter = 0;
                ptt_flag = 0;
                ptt_control_flag = 0;
                app_log_print("Pre Silence Timeout!\r\n");
            }
        }
        else if ( event == VA_EVENT_CMD_DETECTED )
        {
            if (RUNNING_MODE == VA_MODE_WW_SINGLE_CMD)
            {
                breathing_counter = 0;
            }
            app_log_print("Command detected: ");
            if (CY_RSLT_SUCCESS == voice_assistant_get_command(command_text))
            {
                app_log_print("%s\r\n\r\n", command_text);
            }

            if (va_data == NULL)
            {
                return;
            }

            app_log_print("Intent name: %s\r\n", MTB_NLU_INTENT_NAME_LIST(PROJECT_PREFIX)[va_data->intent_index] );

            if (va_data->num_var != 0)
            {
                app_log_print("Variable values: ");
                for (int i = 0; i < va_data->num_var; i++)
                {
                    if (va_data->variable[i].unit_idx < 0)
                    {
                        app_log_print("%s ", MTB_NLU_VARIABLE_PHRASE_LIST(PROJECT_PREFIX)[va_data->variable[i].value]);
                    }
                    else
                    {
                        app_log_print("%d ", va_data->variable[i].value);
                    }
                }
                app_log_print("\n\rVariable units : ");
                for (int i = 0; i < va_data->num_var; i++)
                {
                    if (va_data->variable[i].unit_idx < 0)
                    {
                        app_log_print("---");
                    }
                    else
                    {
                        app_log_print("%s", MTB_NLU_UNIT_PHRASE_LIST(PROJECT_PREFIX)[va_data->variable[i].unit_idx]);
                    }
                }
                app_log_print("\n\r");
            }
            app_log_print("\n\r");
            ptt_flag = 0;
            ptt_control_flag = 0;
        }
    }

    /* Update the Green LED state */
    if (breathing_counter == 0)
    {
        led_pwm_set_brightness(LED_PWM_BLUE_LED, LED_PWM_MAX_BRIGHTNESS);
        led_pwm_on(LED_PWM_BLUE_LED);
    }
    else
    {
        /* Increment twice for faster breathing */
        breathing_counter++;
        breathing_counter++;

        if (breathing_counter >= (2*LED_PWM_MAX_BRIGHTNESS))
        {
            breathing_counter = LED_PWM_MIN_BRIGHTNESS;
        }

        led_pwm_set_brightness(LED_PWM_BLUE_LED, abs(((int) (breathing_counter - LED_PWM_MAX_BRIGHTNESS))));
    }
}


/*******************************************************************************
* Function Name: voice_assistant_infer
********************************************************************************
* Summary:
* Wrapper for VA inferencing
*
* Parameters:
*  audio_frame - Mono Audio frame of 10ms at 16KHz sampling rate
* 
* Return:
*  None
*
*******************************************************************************/


void voice_assistant_infer(int16_t *audio_frame)
{
 #ifdef ENABLE_VOICE_ID
     if (voice_id_mode!=IFX_VOICE_ID_ENROLL)
     {
 #endif
        if (ptt_flag==1 && ptt_control_flag ==0)
        {
            app_log_print("Push to Talk Button detected! Say a command!\r\n\r\n");
            voice_assistant_change_state(VA_RUN_CMD);
            ptt_control_flag=1;
        }
#ifdef SHOW_MCPS
    profiler_start();
#endif /* SHOW_MCPS */    

        run_voice_assistant_process(audio_frame);

#ifdef SHOW_MCPS
    profiler_stop();
    print_mcps();
#endif /* SHOW_MCPS */        

 #ifdef ENABLE_VOICE_ID
     }
 #endif
}

/*******************************************************************************
 * Function Name: run_voice_assistant_process
 *******************************************************************************
 * Summary:
 * Run the voice assistant process and print any information in the terminal.
 *  
 * Parameters:
 *  audio_frame: pointer to the audio data frame
 *
 * Return:
 *  void
 *
 *******************************************************************************/
void run_voice_assistant_process(int16_t *audio_frame)
{
    va_rslt_t va_result;
    va_data_t va_data;
    va_event_t va_event;

 
    /* Process the audio data */
    va_result = voice_assistant_process(audio_frame, &va_event, &va_data);


    /* Print the status of the voice assistant */
    print_voice_assistant_status(va_result, va_event, &va_data);

    #ifdef USE_LED_DEMO
        /* Change the status of the LED if a command was detected */
        if (va_event == VA_EVENT_CMD_DETECTED)
        {
            led_demo(va_data.intent_index, va_data.variable[0].value);
        }
    #endif
}

/*******************************************************************************
 * Function Name: voice_assistant_task_init
 *******************************************************************************
 * Summary:
 * Initialized Voice Assistant task
 *
 * Parameters:
 *  void
 *
 * Return:
 *  void
 *
 *******************************************************************************/


 void voice_assistant_task_init(void)
 {
    va_rslt_t va_result;
        
    /* Initialize the voice assistant */
    va_result = voice_assistant_init(RUNNING_MODE);

    if (va_result != VA_RSLT_SUCCESS)
    {
        app_log_print("Error initializing the voice assistant. Error code=%d\r\n", va_result);
        handle_error();
    }
    else
    {
        app_log_print("Voice Assistant initialized!\r\n\r\n");
    }

    /* Set the command timeout based on running mode */
    if (RUNNING_MODE == VA_MODE_WW_SINGLE_CMD)
    {
        /* Set the command timeout for single commands */
        va_result = voice_assistant_set_command_timeout(CMD_TIMEOUT_SINGLE_CMD);
    }
    else if (RUNNING_MODE == VA_MODE_WW_MULTI_CMD)
    {
        /* Set the command timeout for multiple commands */
        va_result = voice_assistant_set_command_timeout(CMD_TIMEOUT_MULTI_CMD);
    }
    else if (RUNNING_MODE == VA_MODE_CMD_ONLY)
    {
        /* Set the command timeout for the maximum value */
        va_result = voice_assistant_set_command_timeout(CY_NLU_COMMAND_TIMEOUT_MAX);
    }

    if (va_result != VA_RSLT_SUCCESS)
    {
        app_log_print("Error setting timeout value. Error code=%d\r\n", va_result);
        handle_error();
    }

    /* Set the initial breathing counter value */
    if (RUNNING_MODE == VA_MODE_CMD_ONLY)
    {
        breathing_counter = LED_PWM_MIN_BRIGHTNESS;
    }
    else
    {
        breathing_counter = 0;
    }

    /* Initialize the LED PWM driver */
    led_pwm_init();

#ifdef USE_LED_DEMO
    app_log_print("Wake word: Okay Infineon \n\n\r");
    app_log_print("Example: Okay Infineon <switch on the light>\n\n\r");
    app_log_print("Commands: \r\n");
    app_log_print("1. Turn/Switch on the light \r\n");
    app_log_print("2. Turn/Switch off the light \r\n");
    app_log_print("3. Enable/Disable the light \r\n");
    app_log_print("4. Increase/Decrease the brightness \r\n");
    app_log_print("5. Make the light brighter/dimmer \r\n");
    app_log_print("6. Set/Adjust the brightness to <numbers 1 to 10> \r\n");
    app_log_print("7. Dim light to <numbers 1 to 10> \r\n");
    app_log_print("8. Flip/Toggle the light \r\n");
    app_log_print("9. Change the light state \r\n\r\n");
#else
    /* Print the instructions */
    if ((RUNNING_MODE == VA_MODE_WW_SINGLE_CMD) || (RUNNING_MODE == VA_MODE_WW_MULTI_CMD)) 
    {

        app_log_print("Say the wake-word \"%s\" followed by a command.\n\r\n\r", 
            MTB_WWD_NLU_CONFIG_WAKE_WORD_STR(PROJECT_PREFIX)[0]);
    } 
    else if (RUNNING_MODE == VA_MODE_WW_ONLY)
    {
        app_log_print("\n\rSay the wake-word \"%s\".\n\r\n\r", 
            MTB_WWD_NLU_CONFIG_WAKE_WORD_STR(PROJECT_PREFIX)[0]);
    } 
    else if (RUNNING_MODE == VA_MODE_CMD_ONLY)
    {
        app_log_print("\n\rSay a command.\n\r");
    }
#endif /* USE_LED_DEMO */

        /* Print the behavior of the Blue LED */
    app_log_print("Note:\r\n");
    app_log_print("a. BLUE User LED will be solid ON --> indicating waiting for wake word \r\n");
    if (RUNNING_MODE != VA_MODE_WW_ONLY)
    {
        app_log_print("b. On successful wake word detection BLUE User LED will start breathing, indicating waiting for command\r\n");
        app_log_print("c. After successful command detection and execution BLUE User LED will be solid again indicating step (a) \r\n");
        app_log_print("d. In case of timeout or silence after wake word detection BLUE User LED will be solid again indicating step (a) \r\n");
#ifndef ENABLE_VOICE_ID           
        app_log_print("e. For Push To Talk (PTT), Press USER BTN1 to skip Wake Word \r\n");
#else
        app_log_print("e. For Push To Talk (PTT), Press and hold USER BTN1, release after UART notification to skip Wake Word \r\n");
#endif /* USE_KIT_PSE84_AI */      


    }
    app_log_print("\n\r");
    
#ifndef USE_AUDIO_ENHANCEMENT 
    BaseType_t rtos_task_status;   
    rtos_task_status = xTaskCreate(voice_assistant_task, "voice_assistant_task",
                        VA_TASK_STACK_SIZE, NULL, VA_TASK_PRIORITY,
                        &rtos_va_task);

    if (pdPASS != rtos_task_status)
    {
        app_log_print("Voice Assistant task creation failed \r\n");
        CY_ASSERT(0);
    }

    va_queue_handle = xQueueCreate(VA_QUEUE_ELEMENTS, VA_QUEUE_SIZE);
    if (va_queue_handle == NULL)
    {
         app_log_print("Voice Assistant queue initialization failed \r\n");
         CY_ASSERT(0);
    }
#endif /* USE_AUDIO_ENHANCEMENT */    
    
 }
 
 
 /*******************************************************************************
 * Function Name: voice_assistant_task
 *******************************************************************************
 * Summary:
 * Voice assistant task if DEEPCRAFT(TM) Audio Enhancement is disabled
 *
 * Parameters:
 *  void
 *
 * Return:
 *  void
 *
 *******************************************************************************/

void voice_assistant_task(void * arg)
{
    char va_audio_data[VA_QUEUE_SIZE] = {0};
    while(1)
    {
        memset(va_audio_data, 0, VA_QUEUE_SIZE);
        if (pdTRUE == xQueueReceive(va_queue_handle, (void*)va_audio_data, portMAX_DELAY))
        {
            voice_assistant_infer((int16_t*)va_audio_data);
        }
    }
    
}
/* [] END OF FILE */