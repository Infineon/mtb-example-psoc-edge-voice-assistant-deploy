/******************************************************************************
* File Name : led_pwm.h
*
* Description :
* Header for LED PWM driver.
********************************************************************************
* (c) 2024-2026, Infineon Technologies AG, or an affiliate of Infineon
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
#ifndef _LED_PWM_H__
#define _LED_PWM_H__

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

#include "cy_result.h"

/*******************************************************************************
* Macros
*******************************************************************************/
#define LED_PWM_MAX_BRIGHTNESS              (100u)
#define LED_PWM_MIN_BRIGHTNESS              (1u)

#define LED_PWM_BLUE_LED                    (1u)    
#define LED_PWM_GREEN_LED                   (2u)

/*******************************************************************************
* Functions Prototypes
*******************************************************************************/
cy_rslt_t led_pwm_init(void);
void led_pwm_on(uint8_t led);
void led_pwm_off(uint8_t led);
void led_pwm_toggle(uint8_t led);
cy_rslt_t led_pwm_set_brightness(uint8_t led, uint8_t brightness);
cy_rslt_t led_pwm_get_brightness(uint8_t led, uint8_t *brightness);
void led_pwm_deinit(void);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _LED_PWM_H__*/

/* [] END OF FILE */