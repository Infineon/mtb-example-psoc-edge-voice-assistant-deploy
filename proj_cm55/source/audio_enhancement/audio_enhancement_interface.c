/******************************************************************************
* File Name : audio_enhancement_interface.c
*
* Description :
* Wrapper for DEEPCRAFT(TM) Audio Enhancement
********************************************************************************
* (c) 2025-2026, Infineon Technologies AG, or an affiliate of Infineon
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

#include "audio_enhancement_interface.h"
#include "cyabs_rtos.h"

#include "va_task.h"
#include "audio_usb_send_utils.h"
#include "usb_audio_interface.h"
#ifdef ENABLE_VOICE_ID
#include "voice_id_task.h"
#endif /* ENABLE_VOICE_ID */
#include "app_logger.h"

/*******************************************************************************
* Macros
*******************************************************************************/
#define AE_USB_TASK_PRIORITY                            (4)
#define AE_USB_QUEUE_SIZE                               (MONO_AUDIO_DATA_IN_BYTES*7)
#define AE_USB_QUEUE_ELEMENTS                           (20)

#define AE_USB_STACK_SIZE                               (1024)

/*******************************************************************************
* Global Variables
*******************************************************************************/
#ifdef CY_AFE_ENABLE_TUNING_FEATURE
TaskHandle_t rtos_ae_usb_task;
QueueHandle_t ae_usb_queue_handle;
#endif /*CY_AFE_ENABLE_TUNING_FEATURE*/

#ifdef ENABLE_VOICE_ID
extern QueueHandle_t vid_queue_handle;
#endif /* ENABLE_VOICE_ID */

extern volatile uint8_t ptt_flag;

/*******************************************************************************
* Function Name: license_limitation_exit
********************************************************************************
* Summary:
*  This function is blocking call after Audio Front End license timeout
*
*******************************************************************************/
void license_limitation_exit()
{
    while(1){}
}


/*******************************************************************************
* Function Name: audio_enhancement_process_output
********************************************************************************
* Summary:
* Application specific AE output handler
*
* Parameters:
*  output_buffer - Buffers from AE processing.
* 
* Return:
*  None
*
*******************************************************************************/

void audio_enhancement_process_output(ae_buffer_info_t *output_buffer)
{
    
    BaseType_t ret = pdTRUE;
    int16_t * infer_buffer=output_buffer->output_buf;
    
    /* Use the output data from the audio enhancement with the voice-assistant */
    voice_assistant_infer(infer_buffer);
    
#ifdef ENABLE_VOICE_ID    
    if (vid_queue_handle !=NULL)
    {
        ret = xQueueSend(vid_queue_handle, (void*)infer_buffer, 0);
    }
    if (pdTRUE != ret)
    {
        //app_log_print(">>> Failed to send to Voice ID queue - Queue full \r\n");
    }
#endif /* ENABLE_VOICE_ID */

#ifdef CY_AFE_ENABLE_TUNING_FEATURE  
    if (ae_usb_queue_handle !=NULL)
    {
        ret = xQueueSend(ae_usb_queue_handle, (void*)output_buffer, 0);
    }
    if (pdTRUE != ret)
    {
        //app_log_print(">>> Send failed to AE USB task - Queue full \r\n");
    }  
#endif /* CY_AFE_ENABLE_TUNING_FEATURE */ 

}


/*******************************************************************************
* Function Name: ae_init
********************************************************************************
* Summary:
* Initialize DEEPCRAFT(TM) Audio Enhancement 
*
* Parameters:
*  channels - number of input channels
* 
* Return:
*  Result of AE initialization.
*
*******************************************************************************/

int ae_init(int channels)
{

    ae_rslt_t result = AE_RSLT_SUCCESS;
#ifdef CY_AFE_ENABLE_TUNING_FEATURE    
    BaseType_t rtos_task_status;
#endif /* CY_AFE_ENABLE_TUNING_FEATURE */
    result = audio_enhancement_init(channels);
    
    if (result != AE_RSLT_SUCCESS) 
    {
        app_log_print("DEEPCRAFT Audio Enhancement initialization failed \r\n");
    }
    else
    {
        app_log_print("DEEPCRAFT Audio Enhancement initialized \r\n");
    }

#ifdef CY_AFE_ENABLE_TUNING_FEATURE
    /* Enable USB interface*/
    usb_audio_interface_init();
    usb_send_out_dbg_init_channels();
    
    ae_usb_queue_handle = xQueueCreate(AE_USB_QUEUE_ELEMENTS, AE_USB_QUEUE_SIZE);
    if (ae_usb_queue_handle == NULL)
    {
         app_log_print("Init queue for AE USB streamer queue failed \r\n");
         CY_ASSERT(0);
    }

    rtos_task_status = xTaskCreate(ae_usb_task, "AE_USB_task",
                        AE_USB_STACK_SIZE, NULL, AE_USB_TASK_PRIORITY,
                        &rtos_ae_usb_task);

    if (pdPASS != rtos_task_status)
    {
        app_log_print("AE USB streamer task create failed \r\n");
        CY_ASSERT(0);
    }
#endif /* CY_AFE_ENABLE_TUNING_FEATURE */     
    return result;
}

/*******************************************************************************
* Function Name: ae_feed
********************************************************************************
* Summary:
*  Feed mic audio data and AEC reference to Audio Enhancement.
*
*******************************************************************************/

int ae_feed(void* audio_input, void* aec_buffer)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    result = audio_enhancement_feed_input((int16_t*)audio_input, (int16_t*)aec_buffer);
    
    if (AE_RSLT_LICENSE_ERROR == result)
    {
        app_log_print("CPU Halt: Audio Enhancement Restricted License Timeout - Reset the board \r\n");
        license_limitation_exit();
        return result;
    }

    if(AE_RSLT_SUCCESS != result)
    {
        app_log_print("Failed to feed data to AE \r\n");
        return result;
    }
    return result;
}

/*******************************************************************************
* Function Name: ae_usb_task
********************************************************************************
* Summary:
* AE USB streamer task.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
#ifdef CY_AFE_ENABLE_TUNING_FEATURE
void ae_usb_task(void *arg)
{

    char ae_usb_data[AE_USB_QUEUE_SIZE] = {0};
    ae_buffer_info_t *ae_usb_data_ptr= NULL;
     
    while(1)
    {
        memset(ae_usb_data, 0, AE_USB_QUEUE_SIZE);
        if (pdTRUE == xQueueReceive(ae_usb_queue_handle, (void*)ae_usb_data, portMAX_DELAY))
        {

            ae_usb_data_ptr = (ae_buffer_info_t*)ae_usb_data;
    
            usb_send_out_dbg_put(USB_CHANNEL_1,(int16_t *)ae_usb_data_ptr->dbg_output1);
            usb_send_out_dbg_put(USB_CHANNEL_2,(int16_t *)ae_usb_data_ptr->dbg_output2);      
            usb_send_out_dbg_put(USB_CHANNEL_3,(int16_t *)ae_usb_data_ptr->dbg_output3);
            usb_send_out_dbg_put(USB_CHANNEL_4,(int16_t *)ae_usb_data_ptr->dbg_output4);

        }
    }
}
#endif /* CY_AFE_ENABLE_TUNING_FEATURE*/
/* [] END OF FILE */
