#ifndef _DWT_CFG_H_
#define _DWT_CFG_H_

#define COMPARATOR_MAX 4

#ifdef __RTTHREAD__
#include <rtthread.h>

#define dwt_println(...)               rt_kprintf(__VA_ARGS__);

#else 

#endif

#endif /* _DWT_CFG_H_ */
