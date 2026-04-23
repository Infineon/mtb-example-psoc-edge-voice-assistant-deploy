/******************************************************************************
* File Name : profiler.c
*
* Description :
* Code for MCPS profiler
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
#include "cy_pdl.h"
#include "profiler.h"

/*******************************************************************************
* Macros
*******************************************************************************/
#define RESET_CYCLE_CNT (DWT->CYCCNT=0)
#define GET_CYCLE_CNT (DWT->CYCCNT)

/*******************************************************************************
* Function Prototypes
*******************************************************************************/

/*******************************************************************************
* Global Variables
*******************************************************************************/
static uint32_t profiler_cycles = 0;

/*******************************************************************************
* Function Name: Cy_Reset_Cycles
********************************************************************************
* Summary:
* Reset the DWT counter.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
uint32_t Cy_Reset_Cycles(void)
{
    /* Call DWTCyCNTInit before first call */
    return RESET_CYCLE_CNT;
}

/*******************************************************************************
* Function Name: Cy_Get_Cycles
********************************************************************************
* Summary:
* Return the current DWT counter.
*
* Parameters:
*  None
*
* Return:
*  Current DWT counter value.
*
*******************************************************************************/
uint32_t Cy_Get_Cycles(void)
{
    /* Call DWTCyCNTInit before first call */
    return GET_CYCLE_CNT;
}

/*******************************************************************************
* Function Name: profiler_init
********************************************************************************
* Summary:
* This function configures the DWT cycle counter.
*
* Parameters:
*  none
*
* Return:
*  None
*
*******************************************************************************/
 void profiler_init(void)
 {
    /* Disable TRC */
    CoreDebug->DEMCR &= ~CoreDebug_DEMCR_TRCENA_Msk; // ~0x01000000;
    /* Enable TRC */
    CoreDebug->DEMCR |=  CoreDebug_DEMCR_TRCENA_Msk; // 0x01000000;

    /* Disable clock cycle counter */
    DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk; //~0x00000001;
    /* Enable  clock cycle counter */
    DWT->CTRL |=  DWT_CTRL_CYCCNTENA_Msk; //0x00000001;
 }

/*******************************************************************************
* Function Name: profiler_start
********************************************************************************
* Summary:
* Start profiler.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void profiler_start(void)
{
    RESET_CYCLE_CNT;
}

/*******************************************************************************
* Function Name: profiler_stop
********************************************************************************
* Summary:
* Stop profiler.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void profiler_stop(void)
{
    profiler_cycles = GET_CYCLE_CNT;
}

/*******************************************************************************
* Function Name: cy_profiler_get_cycles
********************************************************************************
* Summary:
* Get profiling cycles.
*
* Parameters:
*  None
*
* Return:
*  Profiling cycles.
*
*******************************************************************************/
uint32_t profiler_get_cycles(void)
{
    return profiler_cycles;
}


/* [] END OF FILE */
