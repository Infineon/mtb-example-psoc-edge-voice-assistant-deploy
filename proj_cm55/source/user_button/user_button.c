/******************************************************************************
* File Name : user_button.c
*
* Description :
* Source file for user button handling
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
#include "user_button.h"
#include "app_logger.h"

/*****************************************************************************
 * Macros
 *****************************************************************************/

#define USER_BUTTON_TASK_NAME               ("user_button_task")
#define USER_BUTTON_TASK_STACK_SIZE         (128)
#define USER_BUTTON_TASK_PRIORITY           (CY_RTOS_PRIORITY_NORMAL)


/* Debounce counter for button presses (multiply by 10 ms) */
#define BUTTON_DEBOUNCE_COUNT               (10U)
#define TASK_WAIT_TIME_MS                   (20)

#ifdef USE_KIT_PSE84_AI
#define BUTTON_DEBOUNCE_THRESHOLD_LOW       (100U)
#define BUTTON_DEBOUNCE_PTT                 (200U)
#define BUTTON_DEBOUNCE_ENROLL              (300U)
#define BUTTON_DEBOUNCE_ERASE               (400U)
#define BUTTON_DEBOUNCE_THRESHOLD_HIGH      (500U)
#endif /* USE_KIT_PSE84_AI */

/*******************************************************************************
* Global Variables
*******************************************************************************/

TaskHandle_t rtos_user_btn_task;

#ifdef USE_KIT_PSE84_AI
int32_t button_press = 0;
int32_t button_release = 0;
int8_t ptt_mark = 0;
int8_t erase_mark = 0;
int8_t enroll_mark = 0;
int8_t nop_mark = 0;
#endif /* USE_KIT_PSE84_AI */
uint8_t user_button_1 = 0;
volatile uint8_t ptt_flag = 0;

#ifdef ENABLE_VOICE_ID
uint8_t user_button_2 = 0;
volatile uint8_t enroll_flag = 0;
volatile uint8_t erase_flag = 0;
#endif /* ENABLE_VOICE_ID */

/*******************************************************************************
* Function Name: user_button_init
********************************************************************************
* Summary:
* Initialize user button
*
* Parameters:
*  None
* 
* Return:
* None
*
*******************************************************************************/
void user_button_init()
{
    BaseType_t rtos_task_status;
    
    rtos_task_status = xTaskCreate(user_button_task, USER_BUTTON_TASK_NAME,
                        USER_BUTTON_TASK_STACK_SIZE, NULL, USER_BUTTON_TASK_PRIORITY,
                        &rtos_user_btn_task);

    if (pdPASS != rtos_task_status)
    {
        app_log_print("User button task creation failed \r\n");
        CY_ASSERT(0);
    }
    
    
}

/*******************************************************************************
* Function Name: process_user_button
********************************************************************************
* Summary:
* Process user button and set the functionality
*
* Parameters:
*  None
* 
* Return:
* None
*
*******************************************************************************/

#ifdef USE_KIT_PSE84_AI

void process_user_button(void)
{

#ifdef ENABLE_VOICE_ID
/* AI kit has only one user-button so re-using it for 3 functionalities*/
    if (0 == Cy_GPIO_Read(CYBSP_USER_BTN1_PORT, CYBSP_USER_BTN1_NUM))
    {
        button_press++;
    }
    else
    {

        if (button_press>0)
        {
            button_release = 1;
        }
    }

   
    if (button_press>BUTTON_DEBOUNCE_THRESHOLD_LOW && ptt_mark==0)
    {
        printf("Release User-Button now for Push-to-Talk \r\n");
        ptt_mark=1;
    }
    if (button_press>BUTTON_DEBOUNCE_PTT && enroll_mark==0)
    {
        printf("Release User-Button now for Voice ID enrollment \r\n");
        enroll_mark=1;
    }
    if (button_press>BUTTON_DEBOUNCE_ENROLL && erase_mark==0)
    {
        printf("Release User-Button now for erasing all Voice ID enrollment\r\n");
        erase_mark=1;
    }
    if (button_press>BUTTON_DEBOUNCE_ERASE && nop_mark==0)
    {
        printf("Release User-Button now for no-action \r\n");
        nop_mark=1;
    }


    if (button_release==1)
    {
        if (button_press>BUTTON_DEBOUNCE_THRESHOLD_LOW && button_press<BUTTON_DEBOUNCE_PTT)
        {
            if (ptt_flag == 0)
            {
                ptt_flag = 1;
                app_log_print("Push-To-Talk Enabled \r\n");
            }                
        }
        else if (button_press>BUTTON_DEBOUNCE_PTT && button_press<BUTTON_DEBOUNCE_ENROLL)
        {
            if (enroll_flag == 0)
            {
                app_log_print("Voice ID Enroll Enabled \r\n");
                enroll_flag = 1;
            }            
        }
        else if (button_press>BUTTON_DEBOUNCE_ENROLL && button_press<BUTTON_DEBOUNCE_ERASE)
        {
            if (erase_flag == 0)
            {
                erase_flag = 1;
                app_log_print("Voice ID Erasing enrollments \r\n");
            }    
        }
        else if (button_press>BUTTON_DEBOUNCE_THRESHOLD_HIGH)
        {

        }
        button_release = 0;
        button_press = 0;
        ptt_mark=erase_mark=enroll_mark=nop_mark=0;
    }
#else
    if (0 == Cy_GPIO_Read(CYBSP_USER_BTN1_PORT, CYBSP_USER_BTN1_NUM))
    {
        user_button_1++;
    }
    else
    {
        user_button_1 = 0;
    }
    if (user_button_1 > BUTTON_DEBOUNCE_COUNT)
    {

        if (ptt_flag == 0)
        {
            user_button_1 = 0;
            ptt_flag = 1;
            app_log_print("ptt flag set \r\n");
        }
    }

#endif     /* ENABLE_VOICE_ID */

}

#else
void process_user_button(void)
{
    if (0 == Cy_GPIO_Read(CYBSP_USER_BTN1_PORT, CYBSP_USER_BTN1_NUM))
    {
        user_button_1++;
    }
    else
    {
        user_button_1 = 0;
    }
  
#ifdef ENABLE_VOICE_ID        
       if (0 == Cy_GPIO_Read(CYBSP_USER_BTN2_PORT, CYBSP_USER_BTN2_NUM))
    {
        user_button_2++;
    }
    else
    {
        user_button_2 = 0;
    }

    /* If the button is pressed for more than BUTTON_DEBOUNCE_COUNT, return true */

    if (user_button_1 > BUTTON_DEBOUNCE_COUNT && user_button_2 > BUTTON_DEBOUNCE_COUNT)
    {

        if (erase_flag == 0)
        {
            user_button_1 = 0;
            user_button_2 = 0;
            erase_flag = 1;
        }
    }
    else if (user_button_1 > BUTTON_DEBOUNCE_COUNT && user_button_2 ==0)
    {

        if (ptt_flag == 0)
        {
            user_button_1 = 0;
            ptt_flag = 1;
        }
    }
    
       else if (user_button_2 > BUTTON_DEBOUNCE_COUNT && user_button_1 ==0)
    {

        if (enroll_flag == 0)
        {
            user_button_2 = 0;
            enroll_flag = 1;
        }
    } 
#else
    if (user_button_1 > BUTTON_DEBOUNCE_COUNT)
    {

        if (ptt_flag == 0)
        {
            user_button_1 = 0;
            ptt_flag = 1;
        }
    }
#endif /* ENABLE_VOICE_ID */        
}

#endif /*USE_KIT_PSE84_AI*/

/*******************************************************************************
* Function Name: user_button_task
********************************************************************************
* Summary:
* User button task
*
* Parameters:
*  None
* 
* Return:
* None
*
*******************************************************************************/
void user_button_task(void* arg)
{
    
    while(1)
    {
        process_user_button();  
        vTaskDelay(TASK_WAIT_TIME_MS);
    }
    
    
}