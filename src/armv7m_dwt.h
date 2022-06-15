#ifndef __ARMV7M_DWT_H__
#define __ARMV7M_DWT_H__

#include <stdint.h>

enum DWT_MODE
{
    READ_ONLY = 5,
    WRITE_ONLY,
    READ_WRITE,
    MAX_TYPE
};

enum DWT_DATA_SIZE
{
    SIZE_BYTE = 0,
    SIZE_HALF_WORD = 1,
    SIZE_WORD = 2,
    MAX_SIZE
};

int dwt_init(void);
int dwt_deinit(void);
int data_address_watch_start(int ch, void *addr, uint32_t addr_mask, enum DWT_MODE mode);
int data_address_watch_stop(int ch);
int data_value_watch_start(int ch, void *addr1, void *addr2, void *compare_value, enum DWT_DATA_SIZE data_size);
int data_value_watch_stop(int ch);

int dwt_show_support(void);
int dwt_show_info(void);

#endif /* __ARMV7M_DWT_H__ */
