/*
 * gpio_mtest.c
 *
 *  Created on: 2015Äê1ÔÂ2ÈÕ
 *      Author: ma
 */


#include <stdbool.h>
#include <string.h>

#include <mocha.h>

#include "msp.h"




// These comments are copied from .... I have no idea.

//int main(int argc, char* argv[]) {
//    const UnitTest tests[] = {
//        unit_test_setup_teardown(test_find_item_by_value, create_key_values,
//                                 destroy_key_values),
//        unit_test_setup_teardown(test_sort_items_by_key, create_key_values,
//                                 destroy_key_values),
//    };
//    return run_tests(tests);
//}

//void test_sort_items_by_key(void **state) {
//    unsigned int i;
//    KeyValue * const kv = *state;
//    sort_items_by_key();
//    for (i = 1; i < sizeof(key_values) / sizeof(key_values[0]); i++) {
//        assert_true(kv[i - 1].key < kv[i].key);
//    }
//}



/*
 * These commented codes are original format, without macro
 *
 * to test gpio_clk_get, we need to mock _gpio_clk_enable
 * gpio_clk_enable
 */

//int __mock__gpio_clk_enable = 0;
//
//int __wrap__gpio_clk_enable(int index) {
//
//	extern int __real__gpio_clk_enable(int index) __weak;
//
//	if (__real__gpio_clk_enable && !__mock__gpio_clk_enable) {
//		return __real__gpio_clk_enable(index);
//	}
//
//	check_expected(index);
//	return mock();
//}

MOCHA(int, _gpio_clk_enable, int index) {		// This is mock function

	MOCHA_RETURN(_gpio_clk_enable, index);		// This macro call real function if mock off
	check_expected(index);						// check if passed argument are expected value
	return mock();								// return mocked return value predefined by test case
}

void test_gpio_clk_get(void** state) {			// This is a test case

	gpio_clock_t clk;							// set up stage
	memset(&clk, 0, sizeof(clk));

	MOCHA_ON(_gpio_clk_enable);					// turn on mocking function for _gpio_clk_enable

	EXPECT(_gpio_clk_enable, index, _gpio_reg2index(GPIOE));	// set expected argument value passing to mock
	RETURN(_gpio_clk_enable, 0);								// set value mock func should return

	gpio_clk_get(&clk, GPIOE, GPIO_PIN_6);		// call function under test

	assert_true(gpio_clk_status(&clk, GPIOE, GPIO_PIN_6) == 1);	// assert successful state

	MOCHA_OFF(_gpio_clk_enable);				// turn off mocking function for _gpio_clk_enable
}

MOCHA(int, _gpio_clk_disable, int index) {

	MOCHA_RETURN(_gpio_clk_disable, index);

	check_expected(index);
	return mock();
}

void test_alloc_gpio_handle_success(void **state) {

	gpio_config_t conf;
	gpio_clock_t clk;

	memset(&conf, 0xA5, sizeof(conf));
	memset(&clk, 0xB5, sizeof(clk));
	gpio_handle_t* h = alloc_gpio_handle(&conf, &clk);

	assert_non_null(h);
	assert_int_equal(conf.instance, h->instance);
	assert_memory_equal(&conf.init, &h->init, sizeof(conf.init));
	assert_int_equal(&clk, h->clk);

	free(h);
}

void test_alloc_gpio_handle_invalid_args(void **state) {

	gpio_config_t conf;
	gpio_clock_t clk;

	gpio_handle_t* h = 0;

	h = alloc_gpio_handle(0, &clk);
	assert_null(h);

	h = alloc_gpio_handle(&conf, 0);
	assert_null(h);
}

void test_gpioex_init_invalid_args(void **state) {

	int ret;
	gpio_clock_t	clk;

	const gpio_config_t* config = &PC6_As_Uart6Tx_DefaultConfig;
	gpio_handle_t 						ge;

	memset(&ge, 0xA5, sizeof(gpio_handle_t));

	ret = GPIOEX_Init(0, config->instance, &config->init, &clk);
	assert_int_equal(-EINVAL, ret);

	ret = GPIOEX_Init(&ge, 0, &config->init, &clk);
	assert_int_equal(-EINVAL, ret);

	ret = GPIOEX_Init(&ge, config->instance, 0, &clk);
	assert_int_equal(-EINVAL, ret);

	ret = GPIOEX_Init(&ge, config->instance, &config->init, 0);
	assert_int_equal(-EINVAL, ret);
}

int run_tests_gpio(void) {

	const UnitTest tests[] = {

		unit_test(test_gpio_clk_get),
		unit_test(test_gpioex_init_invalid_args),

		unit_test(test_alloc_gpio_handle_success),
		unit_test(test_alloc_gpio_handle_invalid_args),
	};

	return run_tests(tests);
}
