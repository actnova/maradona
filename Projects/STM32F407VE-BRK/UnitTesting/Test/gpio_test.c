
#include <stdbool.h>
#include <string.h>
#include "stm32f4xx_hal.h"

#include "gpio.h"
#include "errno_ex.h"


static gpio_man_t	gclk;

TEST_GROUP(GPIO_Clock);

TEST_SETUP(GPIO_Clock)
{
	int i;
	
	__GPIOC_CLK_DISABLE();
	HAL_GPIO_DeInit(GPIOC, GPIO_PIN_6);

	for (i = 0; i < 8; i++)
	{
		gclk.bits[i] = 0;
	}
}

TEST_TEAR_DOWN(GPIO_Clock)
{
	int i;
	
	__GPIOC_CLK_DISABLE();
	HAL_GPIO_DeInit(GPIOC, GPIO_PIN_6);
	
	for (i = 0; i < 8; i++)
	{
		gclk.bits[i] = 0;
	}
}

//TEST(GPIO_Clock, ClockGet)
//{
//	gpio_clk_get(&gclk, GPIOC, GPIO_PIN_6);
//	TEST_ASSERT_TRUE(gpioc_clock_is_enabled());
//	TEST_ASSERT_TRUE(gpio_clk_status(&gclk, GPIOC, GPIO_PIN_6));
//}
//
//TEST(GPIO_Clock, ClockPut)
//{
//	gpio_clk_get(&gclk, GPIOC, GPIO_PIN_6);
//	gpio_clk_put(&gclk, GPIOC, GPIO_PIN_6);
//	TEST_ASSERT_FALSE(gpioc_clock_is_enabled());
//	TEST_ASSERT_FALSE(gpio_clk_status(&gclk, GPIOC, GPIO_PIN_6));
//}

//TEST(GPIO_Clock, ClockGet2Put1)
//{
//	gpio_request(&gclk, GPIOC, GPIO_PIN_6);
//	gpio_request(&gclk, GPIOC, GPIO_PIN_7);
//	gpio_release(&gclk, GPIOC, GPIO_PIN_7);
//	TEST_ASSERT_TRUE(gpioc_clock_is_enabled());
//	TEST_ASSERT_TRUE(gpio_in_use(&gclk, GPIOC, GPIO_PIN_6));
//	TEST_ASSERT_FALSE(gpio_in_use(&gclk, GPIOC, GPIO_PIN_7));
//}

TEST(GPIO_Clock, GPIOEX_HAL_Init)
{

	gpio_handle_t gpioex = PC6_As_Uart6Tx_Default;
	gpioex.man = &gclk;	// mock
	GPIOEX_HAL_Init(&gpioex);
	
	TEST_ASSERT_TRUE(gpio_pin_in_use(gpioex.man, GPIOC, gpioex.init.Pin));
	TEST_ASSERT_TRUE(gpio_mode_set(gpioex.instance, &gpioex.init));
	TEST_ASSERT_EQUAL(GPIOEX_STATE_SET, gpioex.state);
}

TEST(GPIO_Clock, GPIOEX_HAL_DeInit)
{
	gpio_handle_t gpioex = PC6_As_Uart6Tx_Default;
	gpioex.man = &gclk;	// mock
	GPIOEX_HAL_Init(&gpioex);
	GPIOEX_HAL_DeInit(&gpioex);
	
	TEST_ASSERT_FALSE(gpio_pin_in_use(gpioex.man, GPIOC, gpioex.init.Pin));
	TEST_ASSERT_TRUE(gpio_mode_reset(gpioex.instance, &gpioex.init));
	TEST_ASSERT_EQUAL(GPIOEX_STATE_RESET, gpioex.state);	
}


TEST_GROUP_RUNNER(GPIO_Clock)
{
	// RUN_TEST_CASE(GPIO_Clock, ClockGet);
	// RUN_TEST_CASE(GPIO_Clock, ClockPut);
	// RUN_TEST_CASE(GPIO_Clock, ClockGet2Put1);
	
	RUN_TEST_CASE(GPIO_Clock, GPIOEX_HAL_Init);
	RUN_TEST_CASE(GPIO_Clock, GPIOEX_HAL_DeInit);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TEST_GROUP(GPIOEX_Type);

TEST_SETUP(GPIOEX_Type){}
TEST_TEAR_DOWN(GPIOEX_Type){}

TEST(GPIOEX_Type, InitInvalidArgs)
{
	int ret;
	gpio_man_t	clk;

	const gpio_config_t* config = &PC6_As_Uart6Tx_DefaultConfig;
	gpio_handle_t 						ge;
	// TEST_ASSERT_NOT_NULL(&ge);
	memset(&ge, 0xA5, sizeof(gpio_handle_t));

	ret = GPIOEX_Init(0, config->instance, &config->init, &clk);
	TEST_ASSERT_EQUAL(-EINVAL, ret);

	ret = GPIOEX_Init(&ge, 0, &config->init, &clk);
	TEST_ASSERT_EQUAL(-EINVAL, ret);

	ret = GPIOEX_Init(&ge, config->instance, 0, &clk);
	TEST_ASSERT_EQUAL(-EINVAL, ret);

	ret = GPIOEX_Init(&ge, config->instance, &config->init, 0);
	TEST_ASSERT_EQUAL(-EINVAL, ret);
}
	
	
TEST(GPIOEX_Type, Init)
{
	int ret;
	gpio_man_t	clk;

	const gpio_config_t* config = &PC6_As_Uart6Tx_DefaultConfig;
	gpio_handle_t ge;

	memset(&ge, 0xA5, sizeof(gpio_handle_t));

	ret = GPIOEX_Init(&ge, config->instance, &config->init, &clk);

	TEST_ASSERT_EQUAL(0, ret);
	TEST_ASSERT_EQUAL_HEX32(config->instance, ge.instance);
	TEST_ASSERT_EQUAL_MEMORY(&config->init, &ge.init, sizeof(ge.init));
	TEST_ASSERT_EQUAL_HEX32(&clk, ge.man);
	TEST_ASSERT_EQUAL(GPIOEX_STATE_RESET, ge.state);
}

extern const gpio_config_t PC6_As_Uart6Tx_DefaultConfig;

//TEST(GPIOEX_Type, Dtor)
//{
//	GPIOEX_TypeDef* ge;
//	GPIO_ClockProviderTypeDef	clk;
//	
//	ge = GPIOEX_Init(PC6_As_Uart6Tx_Default.instance, &PC6_As_Uart6Tx_Default.init, &clk);

//	if (ge) GPIOEX_Dtor(ge);
//}
	
TEST_GROUP_RUNNER(GPIOEX_Type)
{
	// RUN_TEST_CASE(GPIOEX_Type, InitInvalidArgs);
	// RUN_TEST_CASE(GPIOEX_Type, Init);
	// RUN_TEST_CASE(GPIOEX_Type, InitByConfig); no test for wrapper function
	// RUN_TEST_CASE(GPIOEX_Type, Dtor);
}

TEST_GROUP_RUNNER(GPIO_All)
{
	RUN_TEST_GROUP(GPIO_Clock);
	RUN_TEST_GROUP(GPIOEX_Type);
}


	

