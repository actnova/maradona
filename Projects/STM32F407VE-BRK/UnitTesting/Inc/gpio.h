/**
  ******************************************************************************
  * File Name          : gpio.h
  * Date               : 11/06/2014 12:17:27
  * Description        : This file contains all the functions prototypes for 
  *                      the gpio  
  ******************************************************************************
  *
  * COPYRIGHT(c) 2014 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __gpio_H
#define __gpio_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include "stm32f4xx_hal.h"

/*
 * GPIOEX is a thin wrapper over original gpio interface.
 *
 * It encapsulates clock get/put logic.
 * if one gpio is requested for a bank, the clock is on, otherwise, the clock goes off.
 */
	 
typedef struct {

	uint16_t 				bits[8];	/** A to H **/
	
} gpio_man_t;

typedef enum {
	GPIOEX_STATE_RESET = 0,
	GPIOEX_STATE_SET,
} gpio_state_t;

typedef struct
{
	GPIO_TypeDef		*instance;
	GPIO_InitTypeDef	init;
} gpio_config_t;

/*
 * GPIO_TypeDef is actually the register map
 * GPIO_InitTypeDef is init configuration data
 *
 * clk points to clock provider
 */
	 
typedef struct
{
	GPIO_TypeDef  		*instance;
	GPIO_InitTypeDef	init;
	gpio_man_t			*man;
	gpio_state_t		state;

} gpio_handle_t;

extern gpio_man_t GPIO_Manager;

/** struct constructor **/
int GPIOEX_Init(gpio_handle_t* gpioex, GPIO_TypeDef* gpiox, const GPIO_InitTypeDef* init, gpio_man_t* clk);
int GPIOEX_InitByConfig(gpio_handle_t* gpioex, const gpio_config_t* config, gpio_man_t* clk);

gpio_handle_t* gpio_create_handle(const gpio_config_t* conf, gpio_man_t *man);
int gpio_destroy_handle(gpio_handle_t* h);

/** wrapper **/
void GPIOEX_HAL_Init(gpio_handle_t* gpioex);
void GPIOEX_HAL_DeInit(gpio_handle_t* gpioex);



/** in the following function, use only separate Pin defines, don't OR them **/

int gpio_man_init(gpio_man_t* man);
int gpio_request_pin(gpio_man_t* man, GPIO_TypeDef* gpio, uint32_t pin);
int gpio_release_pin(gpio_man_t* man, GPIO_TypeDef* gpio, uint32_t pin);
int	gpio_pin_in_use(gpio_man_t* man, GPIO_TypeDef* gpio, uint32_t pin);

/** gpio stub function **/
int _gpio_reg2index(GPIO_TypeDef* gpiox);
int _gpio_clk_enable(int index);
int _gpio_clk_disable(int index);


const extern gpio_config_t	PC6_As_Uart6Tx_DefaultConfig;
const extern gpio_config_t	PD6_As_Uart2Rx_DefaultConfig;
const extern gpio_config_t	PD5_As_Uart2Tx_DefaultConfig;

const extern gpio_handle_t	PC6_As_Uart6Tx_Default;
const extern gpio_handle_t	PD6_As_Uart2Rx_Default;
const extern gpio_handle_t	PD5_As_Uart2Tx_Default;


// void MX_GPIO_Init(void);
#ifdef __cplusplus
}
#endif
#endif /*__ pinoutConfig_H */

/**
  * @}
  */

/**
  * @}
  */



/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
