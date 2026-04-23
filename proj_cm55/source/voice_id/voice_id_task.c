/******************************************************************************
* File Name : voice_id_task.c
*
* Description :
* Voice ID handler
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
#ifdef ENABLE_VOICE_ID
#include "voice_id_task.h"
#include "cyabs_rtos.h"
#include "app_logger.h"

/*******************************************************************************
* Macros
*******************************************************************************/

#define VOICE_ID_TASK_STACK_SIZE                (1024)
#define VOICE_ID_TASK_PRIORITY                  (2)

#define VOICE_ID_QUEUE_SIZE                     (MONO_AUDIO_DATA_IN_BYTES)
#define VOICE_ID_QUEUE_ELEMENTS                 (10)                        

/*******************************************************************************
 * Global Variables
 *******************************************************************************/

/* Audio buffer used to generate an embedding from */
int16_t audio_for_embedding[FE_AUDIO_LEN];

/* Embeddings of enrolled users */
ifx_voice_id_embeddings_t embeddings_data = {0};

/* Embeddings during user verification */
float embedding[IFX_EMBEDDINGS_LENGTH_WORDS * IFX_NUM_VERIFICATION_EMBEDDINGS] = {0};

TaskHandle_t rtos_vid_task;
QueueHandle_t vid_queue_handle;
int32_t detected_user=-1;

extern volatile uint8_t enroll_flag;
extern volatile uint8_t erase_flag;

volatile char voice_id_mode = IFX_VOICE_ID_WAIT;



/*******************************************************************************
* Function Name: get_enrolled_users
********************************************************************************
* Summary:
* Get the number of enrolled users
*
* Parameters:
*  None
* 
* Return:
*  None
*
*******************************************************************************/

uint8_t get_enrolled_users(void)
{
    return embeddings_data.user_count;
}
void erase_enrolled_users(void)
{
    ifx_en_voice_id_status_t status;
    status = ifx_voice_id_clear_all_enrolled_users(&embeddings_data);
    if (status != IFX_VOICE_ID_SUCCESS) {
        app_log_print("Voice ID - Error in erasing enrolled users %s\r\n", ifx_voice_id_get_status_description(status));
    } else {
        ifx_storage_write(&embeddings_data);
        app_log_print("Voice ID - All enrollments erased successfully!\r\n");
        print_voice_id_info(&embeddings_data);
    }

}


/*******************************************************************************
* Function Name: voice_id_task_init
********************************************************************************
* Summary:
* Initialize Voice ID
*
* Parameters:
*  None
* 
* Return:
*  None
*
*******************************************************************************/

void voice_id_task_init(void)
{
    
    BaseType_t rtos_task_status;
    
    cy_rslt_t result;
    result = ifx_voice_id_init();

    if (CY_RSLT_SUCCESS != result) 
    {
        app_log_print("ERROR: initialization of the Voice ID failed!\r\n");
        CY_HALT();
    }

    result = ifx_storage_init();
    if (CY_RSLT_SUCCESS != result) 
    {
        app_log_print("ERROR: initializing flash SPI failed!\r\n");
        CY_HALT();
    }

    ifx_storage_read(&embeddings_data);
    
    print_voice_id_info(&embeddings_data);
    
    rtos_task_status = xTaskCreate(voice_id_task, "Voice_ID_task",
                        VOICE_ID_TASK_STACK_SIZE, NULL, VOICE_ID_TASK_PRIORITY,
                        &rtos_vid_task);

    if (pdPASS != rtos_task_status)
    {
        app_log_print("Voice ID task creation failed \r\n");
        CY_ASSERT(0);
    }

    vid_queue_handle = xQueueCreate(VOICE_ID_QUEUE_ELEMENTS, VOICE_ID_QUEUE_SIZE);
    if (vid_queue_handle == NULL)
    {
         app_log_print("Voice ID queue initialization failed \r\n");
         CY_ASSERT(0);
    }

#ifndef USE_KIT_PSE84_AI
    app_log_print("Voice ID - Press USER BTN2 for enrolling user \r\n");
    app_log_print("Voice ID - Press USER BTN1 and USER BTN2 together for deleting all users \r\n");
    app_log_print("Voice ID - Speak after Wake Word is detected for Voice Identification \r\n");
#else
    app_log_print("Voice ID - Press and hold USER BTN1, release based on UART notification for enrollment/erasing users \r\n");
#endif  /* USE_KIT_PSE84_AI */    
}


/*******************************************************************************
* Function Name: voice_id_task
********************************************************************************
* Summary:
* Voice ID task
*
* Parameters:
*  arg
* 
* Return:
*  None
*
*******************************************************************************/
void voice_id_task(void * arg)
{
    
    uint8_t new_user_idx = 0;
    int32_t max_idx = 0;
    uint32_t embedding_idx = 0;
    uint8_t enroll_init = 0;
    
    
    ifx_en_voice_id_status_t ret = IFX_VOICE_ID_SUCCESS;
    char vid_audio_data[VOICE_ID_QUEUE_SIZE] = {0};
 
    while(1)
    {
        
        memset(vid_audio_data, 0, VOICE_ID_QUEUE_SIZE);
        if (pdTRUE == xQueueReceive(vid_queue_handle, (void*)vid_audio_data, portMAX_DELAY))
        {
            if (erase_flag == 1)
            {
                app_log_print("Voice ID - Erasing all enrollments \r\n");
                voice_id_mode = IFX_VOICE_ID_WAIT;
                erase_enrolled_users();
                erase_flag = 0;
            }else if (enroll_flag==1)
            {
                voice_id_mode = IFX_VOICE_ID_ENROLL;
            } 
            
            if (voice_id_mode == IFX_VOICE_ID_ENROLL)
            {
                if (enroll_init == 0)
                {
                    if (embeddings_data.user_count < IFX_MAX_SUPPORTED_USERS) {
                        new_user_idx = embeddings_data.user_count; /* next free slot */
                    } else {
                        new_user_idx = IFX_MAX_SUPPORTED_USERS - 1U; /* reuse last slot when full */
                    }
                    embedding_idx = 0;
                      enroll_init = 1;
                      app_log_print("Voice ID - Enrolling User [%d], Speak for approx ~ %d seconds \r\n",new_user_idx, 2 * IFX_NUM_ENROLLMENT_EMBEDDINGS);
                }
                
                ret = ifx_voice_id_infer((int16_t*)vid_audio_data, embedding);
                app_log_print(".");
                if (ret == IFX_VOICE_ID_LIMIT)
                {
                    app_log_print("Voice ID - License limit reached - Please reset the board \r\n");
                    CY_ASSERT(0);
                }
                if (ret == IFX_VOICE_ID_INFERENCE_COMPLETE && embedding_idx < IFX_NUM_ENROLLMENT_EMBEDDINGS)
                {
                    ifx_voice_id_add_embedding(&embeddings_data, embedding, new_user_idx, (uint8_t)embedding_idx);
                    memset(&embedding, 0, sizeof(embedding));
                    embedding_idx++;
                }
                if (embedding_idx == IFX_NUM_ENROLLMENT_EMBEDDINGS)
                {
                    if (embeddings_data.user_count < IFX_MAX_SUPPORTED_USERS) {
                        embeddings_data.user_count++;
                    }

            /* Save embeddings to flash */
                    ifx_storage_write(&embeddings_data);
                    app_log_print("\r\n Voice ID - User [%d] enrolled. \r\n",new_user_idx);
                    voice_id_mode = IFX_VOICE_ID_WAIT;
                    enroll_flag=0;
                    enroll_init=0;
                }
                
            }
            else if (voice_id_mode == IFX_VOICE_ID_VERIFY)
            {
                ret = ifx_voice_id_infer((int16_t*)vid_audio_data, embedding);
                //app_log_print("Voice id inferencing %x \r\n",ret);
                if (ret == IFX_VOICE_ID_LIMIT)
                {
                    app_log_print("Voice ID limit reached - Please reset the board \r\n");
                    CY_ASSERT(0);
                }
                if (ret == IFX_VOICE_ID_INFERENCE_COMPLETE)
                {
                    app_log_print("Voice ID - Verifying User... \r\n");
                    max_idx=ifx_voice_id_verify(embedding,&embeddings_data);
                    if (max_idx>=0)
                    {
                        detected_user=max_idx;
                        app_log_print("Voice ID - Detected user is [%d] \r\n",detected_user);
                    } else
                    {
                        app_log_print("Voice ID - Unknown user detected \r\n");    
                    }
                    voice_id_mode = IFX_VOICE_ID_WAIT;
                }
            }            
        }
    }
}


/*******************************************************************************
* Function Name: print_voice_id_info
********************************************************************************
* Summary:
* Print Enrolment status
*
* Parameters:
*  embeddings
* 
* Return:
*  None
*
*******************************************************************************/

void print_voice_id_info(const ifx_voice_id_embeddings_t* embeddings)
{
   
    if (embeddings == NULL) {
        app_log_print("Enrolled Users:  (N/A)\r\n");
        app_log_print("Available Slots: (N/A)\r\n");   
    }
    else {
        uint32_t max_users = (uint32_t)IFX_MAX_SUPPORTED_USERS;
        uint32_t uc = (uint32_t)embeddings->user_count;
        uint32_t available = (uc < max_users) ? (max_users - uc) : 0U;

        app_log_print("Enrolled Users:  %u\r\n", (unsigned)uc);
        app_log_print("Available Slots: %u\r\n", (unsigned)available);
    }

}
#endif /* ENABLE_VOICE_ID */
/* [] END OF FILE */