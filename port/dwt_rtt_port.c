#include <rtthread.h>
#include <rtdevice.h>
#include "armv7m_dwt.h"

#include <stdlib.h>

MSH_CMD_EXPORT(dwt_show_support, Shows DWT support information.);
MSH_CMD_EXPORT(dwt_show_info, Shows DWT usage information.);
MSH_CMD_EXPORT(dwt_init, Initialize DWT peripherals.);
MSH_CMD_EXPORT(dwt_deinit, Restore DWT peripherals.);

#ifdef PKG_USING_ARMV7M_DWT_DEMO

#define CALL_STACK_MAX_DEPTH 64
#define GET_THREAD_STACK_START         (uint32_t)(rt_thread_self()->stack_addr)
#define GET_THREAD_STACK_SIZE          rt_thread_self()->stack_size
#define CODE_FLASH_START 0x08000000
#define CODE_FLASH_END   (CODE_FLASH_START+(128*1024*1024))

/* include or export for supported cmb_get_msp, cmb_get_psp and cmb_get_sp function */
#if defined(__CC_ARM)
    static __inline __asm uint32_t cmb_get_msp(void) {
        mrs r0, msp
        bx lr
    }
    static __inline __asm uint32_t cmb_get_psp(void) {
        mrs r0, psp
        bx lr
    }
    static __inline __asm uint32_t cmb_get_sp(void) {
        mov r0, sp
        bx lr
    }
#elif defined(__CLANG_ARM)
    __attribute__( (always_inline) ) static __inline uint32_t cmb_get_msp(void) {
        uint32_t result;
        __asm volatile ("mrs %0, msp" : "=r" (result) );
        return (result);
    }
    __attribute__( (always_inline) ) static __inline uint32_t cmb_get_psp(void) {
        uint32_t result;
        __asm volatile ("mrs %0, psp" : "=r" (result) );
        return (result);
    }
    __attribute__( (always_inline) ) static __inline uint32_t cmb_get_sp(void) {
        uint32_t result;
        __asm volatile ("mov %0, sp" : "=r" (result) );
        return (result);
    }
#elif defined(__ICCARM__)
/* IAR iccarm specific functions */
/* Close Raw Asm Code Warning */  
#pragma diag_suppress=Pe940    
    static uint32_t cmb_get_msp(void)
    {
      __asm("mrs r0, msp");
      __asm("bx lr");        
    }
    static uint32_t cmb_get_psp(void)
    {
      __asm("mrs r0, psp");
      __asm("bx lr");        
    }
    static uint32_t cmb_get_sp(void)
    {
      __asm("mov r0, sp");
      __asm("bx lr");       
    }
#pragma diag_default=Pe940  
#elif defined(__GNUC__)
    __attribute__( ( always_inline ) ) static inline uint32_t cmb_get_msp(void) {
        register uint32_t result;
        __asm volatile ("MRS %0, msp\n" : "=r" (result) );
        return(result);
    }
    __attribute__( ( always_inline ) ) static inline uint32_t cmb_get_psp(void) {
        register uint32_t result;
        __asm volatile ("MRS %0, psp\n" : "=r" (result) );
        return(result);
    }
    __attribute__( ( always_inline ) ) static inline uint32_t cmb_get_sp(void) {
        register uint32_t result;
        __asm volatile ("MOV %0, sp\n" : "=r" (result) );
        return(result);
    }
#else
    #error "not supported compiler"
#endif

typedef struct
{
    uint32_t r0;// Register R0
    uint32_t r1;// Register R1
    uint32_t r2;// Register R2
    uint32_t r3;// Register R3
    uint32_t r12;// Register R12
    uint32_t lr;// Link register LR
    uint32_t pc;// Program counter PC
    uint32_t psr;// Program status word PSR
}context_reg;
#define BL_INS_MASK 0xF800
#define BL_INS_HIGH 0xF800
#define BL_INS_LOW 0xF000
#define BLX_INX_MASK 0xFF00
#define BLX_INX 0x4700

/* check the disassembly instruction is 'BL' or 'BLX' */
static int disassembly_ins_is_bl_blx(uint32_t addr)
{
    uint16_t ins1 = *((uint16_t *)addr);
    uint16_t ins2 = *((uint16_t *)(addr + 2));

    if ((ins2 & BL_INS_MASK) == BL_INS_HIGH && (ins1 & BL_INS_MASK) == BL_INS_LOW)
    {
        return 1;
    }
    else if ((ins2 & BLX_INX_MASK) == BLX_INX)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void prepare_call_stack_buf(uint32_t *buf, size_t size, uint32_t sp) 
{
    uint32_t stack_start;
    uint32_t stack_size;
    uint32_t pc;
    size_t depth = 0;

    stack_start = GET_THREAD_STACK_START;
    stack_size = GET_THREAD_STACK_SIZE;
    for (; sp < stack_start + stack_size; sp += sizeof(size_t))
    {
        pc = *((uint32_t *) sp) - sizeof(size_t);
        if (pc % 2 == 0)
        {
            continue;
        }
        pc = *((uint32_t *)sp) - 1;
        if((pc < CODE_FLASH_START) || (pc > CODE_FLASH_END))
        {
            continue;
        }
        if (disassembly_ins_is_bl_blx(pc - sizeof(size_t)))
        {
            buf[depth++] = pc;
            if(depth ==size)
            {
                break;
            }
        }
    }
}

int armv7_dwt_rtt_demo(void)
{
    rt_kprintf("thread:%s psp:0x%08X\r\n",rt_thread_self()->name,cmb_get_psp());
#ifdef RT_USING_FINSH
    extern long list_thread(void);
    list_thread();
#endif
    uint32_t call_stack_buf[16] = {0};
    prepare_call_stack_buf(call_stack_buf,sizeof(call_stack_buf),cmb_get_psp());

    rt_kprintf("\r\naddr2line -e rtthread.elf -a -f");
    for (int i = 0; i < sizeof(call_stack_buf); i++)
    {
        if(call_stack_buf[i])
        {
            rt_kprintf(" %08X",call_stack_buf[i]);
        }
        else
        {
            break;
        }
    }
    rt_kprintf("\r\n");

    return 0;
}

char databuf[64];

int dwt_value_test(int argc, char **argv)
{
    int ch;
    int ret;
    volatile rt_uint8_t temp;
    temp = 0x55;
    databuf[1] = 0x00 ;
    databuf[2] = 0x00 ;

    if(argc == 2)
    {
        ch = strtol(argv[1], NULL, 0);
        ret = data_value_watch_start(ch, (void*)&databuf[1], (void*)&databuf[2], (void*)&temp, SIZE_BYTE);
        // ret = data_value_watch_start(ch, (void*)NULL, (void*)&databuf[2], (void*)&temp, SIZE_BYTE);
        if(ret != 0)
        {
            return 0;
        }

        rt_kprintf("1. temp = databuf[1];\r\n");
        rt_thread_mdelay(500);
        temp = databuf[1];
        rt_kprintf("2. temp = databuf[2];\r\n");
        rt_thread_mdelay(500);
        temp = databuf[2];

        rt_kprintf("3. databuf[1] = 0x55 ;\r\n");
        rt_thread_mdelay(500);
        databuf[1] = 0x55 ;
        rt_kprintf("4. databuf[2] = 0x55 ;\r\n");
        rt_thread_mdelay(500);
        databuf[2] = 0x55 ;

        rt_thread_mdelay(500);

        rt_kprintf("5. temp = databuf[1];\r\n");
        rt_thread_mdelay(500);
        temp = databuf[1];
        rt_kprintf("6. temp = databuf[2];\r\n");
        rt_thread_mdelay(500);
        temp = databuf[2];


        rt_kprintf("7. databuf[1] = 0x56 ;\r\n");
        rt_thread_mdelay(500);
        databuf[1] = 0x56 ;
        rt_kprintf("8. databuf[2] = 0x56 ;\r\n");
        rt_thread_mdelay(500);
        databuf[2] = 0x56 ;

        data_value_watch_stop(ch);
    }
    else
    {
        rt_kprintf("command: dwt_value_watch 1\r\n");
    }
    return 0;
}
MSH_CMD_EXPORT(dwt_value_test, dwt_value_test);

int dwt_address_test(int argc, char **argv)
{
    int ch;
    int ret;
    volatile rt_uint8_t temp;
    temp = 0x55;
    databuf[1] = 0x00 ;

    if(argc == 2)
    {
        ch = strtol(argv[1], NULL, 0);
        // ret = data_address_watch_start(ch, (void*)&databuf[1], 0x00000000, READ_ONLY);
        ret = data_address_watch_start(ch, (void*)&databuf[1], 0x00000000, READ_WRITE);
        // ret = data_address_watch_start(ch, (void*)&databuf[1], 0x00000000, WRITE_ONLY);
        if(ret != 0)
        {
            return 0;
        }
        rt_kprintf("1. temp = databuf[1];\r\n");
        rt_thread_mdelay(500);
        temp = databuf[1];

        
        rt_kprintf("2. databuf[1] = 0x55 ;\r\n");
        rt_thread_mdelay(500);
        databuf[1] = 0x55 ;

        data_address_watch_stop(ch);
    }
    else
    {
        rt_kprintf("command: dwt_value_watch 1\r\n");
    }
}
MSH_CMD_EXPORT(dwt_address_test, dwt_address_test);

#endif
