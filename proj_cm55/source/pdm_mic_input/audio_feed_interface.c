/******************************************************************************
* File Name : mic_data_feed.c
*
* Description :
* Feeds data received via PDM ISR/USB ISR to the audio pipeline on CM55
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



#include <stdint.h>
#include "audio_input_configuration.h"
#ifdef USE_AUDIO_ENHANCEMENT
#include "audio_enhancement_interface.h"
#else
#include "va_task.h"
#ifdef ENABLE_VOICE_ID
#include "voice_id_task.h"
#endif /* ENABLE_VOICE_ID */
#endif /* USE_AUDIO_ENHANCEMENT */
#include "app_logger.h"
#include "cyabs_rtos.h"


/*******************************************************************************
* Macros
*******************************************************************************/

/*******************************************************************************
* Global Variables
*******************************************************************************/

#ifdef ENABLE_VOICE_ID
extern QueueHandle_t vid_queue_handle;
#endif /* ENABLE_VOICE_ID */

#ifndef USE_AUDIO_ENHANCEMENT
extern QueueHandle_t va_queue_handle;
#endif /* USE_AUDIO_ENHANCEMENT*/

/*******************************************************************************
* Function Name: audio_mic_data_feed_cm55
********************************************************************************
* Summary:
* Receive PDM mic frames and feed it to the audio pipeline.
*
* Parameters:
*  audio_data - Pointer to audio buffer.
* Return:
*  None
*
*******************************************************************************/
void audio_mic_data_feed_cm55(int16_t *audio_data)
{

#ifdef USE_AUDIO_ENHANCEMENT
    cy_rslt_t result = CY_RSLT_SUCCESS;    
/* If Audio Enhancement is enabled, VA inferencing and Voice ID happens after AFE via AFE output callback */
    result=ae_feed(audio_data, NULL);
    if(CY_RSLT_SUCCESS != result)
    {
        app_log_print("Failed to feed audio frame to AFE - results %x \r\n ",result);
    }
#else
/* No Audio Enhancement, hence do VA inferencing directly */
    if (va_queue_handle !=NULL)
    {
        xQueueSend(va_queue_handle, (void*)audio_data, 0);
    }
#ifdef ENABLE_VOICE_ID 
    if (vid_queue_handle !=NULL)
    {
        xQueueSend(vid_queue_handle, (void*)audio_data, 0);
    }
#endif /* ENABLE_VOICE_ID */
#endif /* USE_AUDIO_ENHANCEMENT */
}


/* [] END OF FILE */


