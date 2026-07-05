#ifndef __COMMON_H__
#define __COMMON_H__

#include "sys.h"
#include "config.h"

#define XINRAW(p,n) P##p##in(n)
#define XINRAW_EXPAND(p,n) XINRAW(p,n) 

#define INIT_PORT_IN_FLOAT(p,n)  {\
	rcu_periph_clock_enable(RCU_GPIO##p);\
	gpio_mode_set(GPIO##p, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_##n);\
}

#define INIT_PORT_IN_FLOAT_EXPAND(p,n) INIT_PORT_IN_FLOAT(p,n)

#define GET_X_PORT(n)   X##n##_PORT
#define GET_X_PIN(n)    X##n##_PIN

#define XIN(n)  XINRAW_EXPAND(GET_X_PORT(n),GET_X_PIN(n))
#define INITX(n)    INIT_PORT_IN_FLOAT_EXPAND(GET_X_PORT(n),GET_X_PIN(n))

#endif
