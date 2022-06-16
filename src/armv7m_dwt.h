/*
 * This file is part of the armv7m_DWT Library.
 *
 * Copyright (c) 2021-2022, BalanceTWK (balancetwk@yeah.net)
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: Initialize function and other general function.
 * Created on: 2022-06-16
 */

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
