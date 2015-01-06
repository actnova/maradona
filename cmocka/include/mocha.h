/*
 * mocha.h
 *
 *  Created on: 2015Äê1ÔÂ6ÈÕ
 *      Author: ma
 */

#ifndef CMOCKA_INCLUDE_MOCHA_H_
#define CMOCKA_INCLUDE_MOCHA_H_

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#ifdef USE_MOCHA

#define MOCHA(RT, NAME, ...) int __mock_##NAME = 0; \
        extern RT __real_##NAME(__VA_ARGS__) __attribute__((weak)); \
        RT __wrap_##NAME(__VA_ARGS__)

#define MOCHA_RETURN(NAME, ...) if (__real_##NAME && !__mock_##NAME) { return __real_##NAME(__VA_ARGS__); }

#else

/*
 * In this case,
 */

#define MOCHA(RT, NAME, ...) int __mock_##NAME = 0; \
        extern RT _disable_real_##NAME(__VA_ARGS__) __attribute__((weak)); \
        RT _disable_wrap_##NAME(__VA_ARGS__)

#define MOCHA_RETURN(NAME, ...) if (_disable_real_##NAME && !__mock_##NAME) { return _disable_real_##NAME(__VA_ARGS__); }

#endif

#define MOCHA_ON(NAME)  __mock_##NAME = 1;
#define MOCHA_OFF(NAME) __mock_##NAME = 0;

#define EXPECT(NAME, ...) expect_value(__wrap_##NAME, __VA_ARGS__)
#define RETURN(NAME, ...) will_return(__wrap_##NAME, __VA_ARGS__)


#endif /* CMOCKA_INCLUDE_MOCHA_H_ */
