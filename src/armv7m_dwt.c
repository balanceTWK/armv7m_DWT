#include "dwt_cfg.h"
#include "armv7m_dwt.h"

typedef volatile struct
{
    volatile uint32_t DHCSR;
    volatile uint32_t DCRSR;
    volatile uint32_t DCRDR;
    union    /* Manages vector catch behavior and DebugMonitor handling when debugging. */
    {
        volatile uint32_t all_in_one;
        struct
        {
            volatile uint32_t VC_CORERESET    : 1;
            volatile uint32_t                 : 3;
            volatile uint32_t VC_MMERR        : 1;
            volatile uint32_t VC_NOCPERR      : 1;
            volatile uint32_t VC_CHKERR       : 1;
            volatile uint32_t VC_STATERR      : 1;
            volatile uint32_t VC_BUSERR       : 1;
            volatile uint32_t VC_INTERR       : 1;
            volatile uint32_t VC_HARDERR      : 1;
            volatile uint32_t                 : 5;
            volatile uint32_t MON_EN          : 1;
            volatile uint32_t MON_PEND        : 1;
            volatile uint32_t MON_STEP        : 1;
            volatile uint32_t MON_REQ         : 1;
            volatile uint32_t                 : 4;
            volatile uint32_t TRCENA          : 1;
            volatile uint32_t                 : 7;
        };
    } DEMCR;
} arm_debug_system_reg_t;

#define DEBUG_SYSTEM_BASE_ADDRESS 0xE000EDF0
#define DEBUG_SYSTEM (*(arm_debug_system_reg_t *)DEBUG_SYSTEM_BASE_ADDRESS)

typedef volatile struct
{
    uint32_t FB_CTRL;
    union
    {
        void *address;
        struct
        {
            const uint32_t          : 29;
            const uint32_t RMPSPT   : 1;
            const uint32_t          : 2;
        };
    } FP_REMAP;
    union
    {
        uintptr_t address;
        struct
        {
            uint32_t ENABLE     : 1;
            uint32_t            : 1;
            uint32_t COMP       : 27;
            uint32_t            : 1;
            uint32_t REPLACE    : 2;
        };
    } FP_COMP[8];
} arm_fpb_reg_t;

#define FPB_BASE_ADDRESS 0xE0002000
#define FPB (*(arm_fpb_reg_t *)FPB_BASE_ADDRESS)

typedef volatile struct
{
    volatile uint32_t DWT_COMP;
    union
    {
        volatile uint32_t all_in_one;
        struct
        {
            volatile uint32_t MASK   : 5;
            volatile uint32_t        : 27;
        };
    } DWT_MASK;
    union
    {
        volatile uint32_t all_in_one;
        struct
        {
            volatile uint32_t FUNCTION   : 4;
            volatile uint32_t            : 1;
            volatile uint32_t EMITRANGE  : 1;
            volatile uint32_t            : 1;
            volatile uint32_t CYCMATCH   : 1;
            volatile uint32_t DATAVMATCH : 1;
            volatile uint32_t LNK1ENA    : 1;
            volatile uint32_t DATAVSIZE  : 2;
            volatile uint32_t DATAVADDR0 : 4;
            volatile uint32_t DATAVADDR1 : 4;
            volatile uint32_t            : 4;
            volatile uint32_t MATCHED    : 1;
            volatile uint32_t            : 7;
        };
    } DWT_FUNCTION;
    uint32_t RESERVERED;
}dwt_comparator_t;
typedef volatile struct
{
    union
    {
        volatile uint32_t all_in_one;
        struct
        {
            volatile uint32_t CYCCNTENA      : 1;
            volatile uint32_t POSTPRESET     : 4;
            volatile uint32_t POSTINIT       : 4;
            volatile uint32_t CYCTAP         : 1;
            volatile uint32_t SYNCTAP        : 2;
            volatile uint32_t PCSAMPLENA     : 1;
            volatile uint32_t                : 3;
            volatile uint32_t EXCTRCENA      : 1;
            volatile uint32_t CPIEVTENA      : 1;
            volatile uint32_t EXCEVTENA      : 1;
            volatile uint32_t SLEEPEVTENA    : 1;
            volatile uint32_t LSUEVTENA      : 1;
            volatile uint32_t FOLDEVTENA     : 1;
            volatile uint32_t CYCEVTENA      : 1;
            volatile uint32_t                : 1;
            volatile uint32_t NOPRFCNT       : 1;
            volatile uint32_t NOCYCCNT       : 1;
            volatile uint32_t NOEXTTRIG      : 1;
            volatile uint32_t NOTRCPKT       : 1;
            volatile uint32_t NUMCOMP        : 4;
        };
    } DWT_CTRL;
    volatile uint32_t DWT_CYCCNT;
    volatile uint32_t DWT_CPICNT;
    volatile uint32_t DWT_EXCCNT;
    volatile uint32_t DWT_SLEEPCNT;
    volatile uint32_t DWT_LSUCNT;
    volatile uint32_t DWT_FOLDCNT;
    volatile uint32_t DWT_PCSR;
    volatile dwt_comparator_t COMPARATOR[16];
} arm_dwt_reg_t;

#define DWT_BASE_ADDRESS 0xE0001000
#define CORESIGHT_DWT (*(arm_dwt_reg_t *)DWT_BASE_ADDRESS)

#define CoreDebug_BASE      (0xE000EDF0UL)                            /*!< Core Debug Base Address */
#define CoreDebug           ((CoreDebug_Type *)     CoreDebug_BASE)   /*!< Core Debug configuration struct */

typedef volatile struct
{
    uint32_t can_cyccnt;
    uint32_t can_address_watch;
    uint32_t can_value_watch;
    uint32_t can_double_value_watch;
}armv7m_dwt_comparator;

armv7m_dwt_comparator dwt_comparator[COMPARATOR_MAX] = {0};

int dwt_show_support(void)
{
    dwt_println("\r\n");
    if(DEBUG_SYSTEM.DEMCR.TRCENA == 0)
    {
        dwt_println("=========================================\r\n");
        dwt_println("| !!!warning: DEBUG_SYSTEM is disable   |\r\n");
        dwt_println("| You may need to call dwt_init() first.|\r\n");
        dwt_println("=========================================\r\n");
        return -1;
    }
    dwt_println("========================== DWT channels SUPPORT ===========================\r\n");
    dwt_println("| %-*.*s | CYCCNT | address compare | vaule_0 compare | value_1 compare |\r\n", 8, 8, "channels");
    dwt_println("---------------------------------------------------------------------------\r\n");
    for (int i = 0; i < CORESIGHT_DWT.DWT_CTRL.NUMCOMP; i++)
    {
        dwt_println("| ch[%02d]   |",i);
        dwt_println(" %-*.*s |", 6, 6, dwt_comparator[i].can_cyccnt ? "YES":"NO");
        dwt_println(" %-*.*s |", 15, 15, dwt_comparator[i].can_address_watch ? "YES":"NO");
        dwt_println(" %-*.*s |", 15, 15, dwt_comparator[i].can_value_watch ? "YES":"NO");
        dwt_println(" %-*.*s |", 15, 15, dwt_comparator[i].can_double_value_watch ? "YES":"NO");
        dwt_println("\r\n");
    }
    dwt_println("===========================================================================\r\n");
    return 0;
}

int dwt_show_info(void)
{
    dwt_println("\r\n");
    if(DEBUG_SYSTEM.DEMCR.TRCENA == 0)
    {
        dwt_println("=========================================\r\n");
        dwt_println("| !!!warning: DEBUG_SYSTEM is disable   |\r\n");
        dwt_println("| You may need to call dwt_init() first.|\r\n");
        dwt_println("=========================================\r\n");
        return -1;
    }
    dwt_println("============= DWT channels table =============\r\n");
    dwt_println("| %-*.*s | value compare | address compare |\r\n", 8, 8, "channels");
    dwt_println("----------------------------------------------\r\n");
    for (int i = 0; i < CORESIGHT_DWT.DWT_CTRL.NUMCOMP; i++)
    {
        dwt_println("| ch[%02d]   |",i);
        if(CORESIGHT_DWT.COMPARATOR[i].DWT_FUNCTION.DATAVMATCH)
        {
            switch (CORESIGHT_DWT.COMPARATOR[i].DWT_FUNCTION.DATAVSIZE)
            {
            case SIZE_BYTE:
                dwt_println(" 0x%08X    |", (uint8_t)(CORESIGHT_DWT.COMPARATOR[i].DWT_COMP));
                break;
            case SIZE_HALF_WORD:
                dwt_println(" 0x%08X    |", (uint16_t)(CORESIGHT_DWT.COMPARATOR[i].DWT_COMP));
                break;
            case SIZE_WORD:
                dwt_println(" 0x%08X    |", (uint32_t)(CORESIGHT_DWT.COMPARATOR[i].DWT_COMP));
                break;
            default:
                dwt_println(" 0x%08X    |", (uint32_t)(CORESIGHT_DWT.COMPARATOR[i].DWT_COMP));
                break;
            }
            dwt_println(" ch[%02d] ch[%02d]   |",CORESIGHT_DWT.COMPARATOR[i].DWT_FUNCTION.DATAVADDR0, (CORESIGHT_DWT.COMPARATOR[i].DWT_FUNCTION.DATAVADDR0!=CORESIGHT_DWT.COMPARATOR[i].DWT_FUNCTION.DATAVADDR1) ? CORESIGHT_DWT.COMPARATOR[i].DWT_FUNCTION.DATAVADDR1:-1);
        }
        else
        {
            dwt_println(" NULL          |");
            if((CORESIGHT_DWT.COMPARATOR[i].DWT_FUNCTION.DATAVMATCH == 0)&&(CORESIGHT_DWT.COMPARATOR[i].DWT_FUNCTION.FUNCTION != 0))
            {
                dwt_println(" 0x%08X      |",CORESIGHT_DWT.COMPARATOR[i].DWT_COMP);
            }
            else
            {
                dwt_println(" NULL            |");
            }
        }
        dwt_println("\r\n");
    }
    dwt_println("==============================================\r\n");

}

int dwt_init(void)
{
    /* Global enable for all DWT and ITM features. */
    DEBUG_SYSTEM.DEMCR.TRCENA = 1;
    /* Enable the DebugMonitor exception. */
    DEBUG_SYSTEM.DEMCR.MON_EN = 1;
    CORESIGHT_DWT.DWT_CTRL.all_in_one = 0x00000000;

    for (int i = 0; i < CORESIGHT_DWT.DWT_CTRL.NUMCOMP; i++)
    {
        dwt_comparator[i].can_address_watch = 0;
        dwt_comparator[i].can_cyccnt = 0;
        dwt_comparator[i].can_value_watch = 0;
        dwt_comparator[i].can_double_value_watch = 0;
    }

    if(!CORESIGHT_DWT.DWT_CTRL.NOCYCCNT)
    {
        for (int i = 0; i < CORESIGHT_DWT.DWT_CTRL.NUMCOMP; i++)
        {
            CORESIGHT_DWT.COMPARATOR[i].DWT_FUNCTION.CYCMATCH = 1;
            if(CORESIGHT_DWT.COMPARATOR[i].DWT_FUNCTION.CYCMATCH)
            {
                dwt_comparator[i].can_cyccnt = 1;
                CORESIGHT_DWT.COMPARATOR[i].DWT_FUNCTION.CYCMATCH = 0;
            }
        }
    }
    for (int i = 0; i < CORESIGHT_DWT.DWT_CTRL.NUMCOMP; i++)
    {
        dwt_comparator[i].can_address_watch = 1;
    }
    if(CORESIGHT_DWT.DWT_CTRL.NOCYCCNT == 0)
    {
        for (int i = 0; i < CORESIGHT_DWT.DWT_CTRL.NUMCOMP; i++)
        {
            CORESIGHT_DWT.COMPARATOR[i].DWT_FUNCTION.DATAVMATCH = 1;
            if(CORESIGHT_DWT.COMPARATOR[i].DWT_FUNCTION.DATAVMATCH)
            {
                dwt_comparator[i].can_value_watch = 1;
                if(CORESIGHT_DWT.COMPARATOR[i].DWT_FUNCTION.LNK1ENA)
                {
                    dwt_comparator[i].can_double_value_watch = 1;
                }
                CORESIGHT_DWT.COMPARATOR[i].DWT_FUNCTION.DATAVMATCH = 0;
            }
        }
    }

    for (int i = 0; i < CORESIGHT_DWT.DWT_CTRL.NUMCOMP; i++)
    {
        CORESIGHT_DWT.COMPARATOR[i].DWT_FUNCTION.all_in_one = 0x00000000;
        CORESIGHT_DWT.COMPARATOR[i].DWT_COMP = 0;
        CORESIGHT_DWT.COMPARATOR[i].DWT_MASK.MASK = 0;    
    }
    dwt_show_support();
}

int dwt_deinit(void)
{
    CORESIGHT_DWT.DWT_CTRL.all_in_one = 0x00000000;
    for (int i = 0; i < CORESIGHT_DWT.DWT_CTRL.NUMCOMP; i++)
    {
        CORESIGHT_DWT.COMPARATOR[i].DWT_FUNCTION.all_in_one = 0x00000000;
        CORESIGHT_DWT.COMPARATOR[i].DWT_COMP = 0;
        CORESIGHT_DWT.COMPARATOR[i].DWT_MASK.MASK = 0;    
    }
    /* Disenable all DWT and ITM features. */
    DEBUG_SYSTEM.DEMCR.TRCENA = 0;
    /* Disenable the DebugMonitor exception. */
    DEBUG_SYSTEM.DEMCR.MON_EN = 0;
}

int data_address_watch_start(int ch, void *addr, uint32_t addr_mask, enum DWT_MODE mode)
{
    if(DEBUG_SYSTEM.DEMCR.TRCENA == 0)
    {
        dwt_println("=========================================\r\n");
        dwt_println("| !!!warning: DEBUG_SYSTEM is disable   |\r\n");
        dwt_println("| You may need to call dwt_init() first.|\r\n");
        dwt_println("=========================================\r\n");
        return -1;
    }
    if( ch >= CORESIGHT_DWT.DWT_CTRL.NUMCOMP)
    {
        dwt_println("[ERROR]: The chip only supports [0-%d] channels, or You are not use \"dwt_init()\" first to enable DWT !\r\n", CORESIGHT_DWT.DWT_CTRL.NUMCOMP-1);
        return -1;
    }
    if(addr_mask > 0x0000001F)
    {
        dwt_println("[ERROR]: addr_mask is not greater than 0x0000001F\r\n");
        return -1;
    }

    CORESIGHT_DWT.COMPARATOR[ch].DWT_COMP = (uint32_t)addr;
    CORESIGHT_DWT.COMPARATOR[ch].DWT_MASK.all_in_one = addr_mask;
    CORESIGHT_DWT.COMPARATOR[ch].DWT_FUNCTION.FUNCTION = mode;

    dwt_show_info();
    return 0;
}

int data_address_watch_stop(int ch)
{
    CORESIGHT_DWT.COMPARATOR[ch].DWT_FUNCTION.all_in_one = 0x00000000;
    CORESIGHT_DWT.COMPARATOR[ch].DWT_COMP = 0;
    CORESIGHT_DWT.COMPARATOR[ch].DWT_MASK.MASK = 0;
    
    return 0;
}

int data_value_watch_start(int ch, void *addr1, void *addr2, void *compare_value, enum DWT_DATA_SIZE data_size)
{
    if(DEBUG_SYSTEM.DEMCR.TRCENA == 0)
    {
        dwt_println("=========================================\r\n");
        dwt_println("| !!!warning: DEBUG_SYSTEM is disable   |\r\n");
        dwt_println("| You may need to call dwt_init() first.|\r\n");
        dwt_println("=========================================\r\n");
        return -1;
    }
    if(compare_value == NULL)
    {
        dwt_println("[ERROR]: compare_value is illegal\r\n");
        goto ___fail_exit;
    }

    CORESIGHT_DWT.COMPARATOR[ch].DWT_COMP = *((uint32_t*)compare_value);
    CORESIGHT_DWT.COMPARATOR[ch].DWT_FUNCTION.FUNCTION = WRITE_ONLY;
    CORESIGHT_DWT.COMPARATOR[ch].DWT_FUNCTION.DATAVSIZE = data_size;

    if((addr1)&&(addr2))
    {
        if(!dwt_comparator[ch].can_double_value_watch)
        {
            dwt_println("[ERROR]: ch[%d] is unsupported\r\n",ch);
            CORESIGHT_DWT.COMPARATOR[ch].DWT_COMP = 0x00000000;
            CORESIGHT_DWT.COMPARATOR[ch].DWT_FUNCTION.all_in_one = 0x00000000;
            CORESIGHT_DWT.COMPARATOR[ch].DWT_MASK.MASK = 0;
            goto ___fail_exit;
        }
        for (int ch_1 = 0; ch_1 < CORESIGHT_DWT.DWT_CTRL.NUMCOMP; ch_1++)
        {
            if(CORESIGHT_DWT.COMPARATOR[ch_1].DWT_FUNCTION.FUNCTION == 0)
            {
                CORESIGHT_DWT.COMPARATOR[ch].DWT_FUNCTION.DATAVMATCH = 1;
                CORESIGHT_DWT.COMPARATOR[ch].DWT_FUNCTION.DATAVADDR0 = ch_1;
                CORESIGHT_DWT.COMPARATOR[ch_1].DWT_COMP = (uint32_t)addr1;
                CORESIGHT_DWT.COMPARATOR[ch_1].DWT_FUNCTION.FUNCTION = READ_WRITE;
                CORESIGHT_DWT.COMPARATOR[ch_1].DWT_FUNCTION.DATAVSIZE = data_size;
                for (int ch_2 = 0; ch_2 < CORESIGHT_DWT.DWT_CTRL.NUMCOMP; ch_2++)
                {
                    if(CORESIGHT_DWT.COMPARATOR[ch_2].DWT_FUNCTION.FUNCTION == 0)
                    {
                        if(CORESIGHT_DWT.COMPARATOR[ch].DWT_FUNCTION.LNK1ENA)
                        {
                            CORESIGHT_DWT.COMPARATOR[ch].DWT_FUNCTION.DATAVADDR1 = ch_2;
                            CORESIGHT_DWT.COMPARATOR[ch_2].DWT_COMP = (uint32_t)addr2;
                            CORESIGHT_DWT.COMPARATOR[ch_2].DWT_FUNCTION.FUNCTION = READ_WRITE;
                            CORESIGHT_DWT.COMPARATOR[ch_2].DWT_FUNCTION.DATAVSIZE = data_size;
                            dwt_show_info();
                            return 0;
                        }
                        CORESIGHT_DWT.COMPARATOR[ch].DWT_COMP = 0x00000000;
                        CORESIGHT_DWT.COMPARATOR[ch].DWT_FUNCTION.all_in_one = 0x00000000;
                        CORESIGHT_DWT.COMPARATOR[ch].DWT_MASK.MASK = 0;

                        CORESIGHT_DWT.COMPARATOR[ch_1].DWT_COMP = 0x00000000;
                        CORESIGHT_DWT.COMPARATOR[ch_1].DWT_FUNCTION.all_in_one = 0x00000000;
                        CORESIGHT_DWT.COMPARATOR[ch_1].DWT_MASK.MASK = 0;
                        dwt_println("%s:%d\r\n",__FUNCTION__,__LINE__);
                        goto ___fail_exit;
                    }
                }
                CORESIGHT_DWT.COMPARATOR[ch].DWT_FUNCTION.all_in_one = 0x00000000;
                CORESIGHT_DWT.COMPARATOR[ch].DWT_COMP = 0x00000000;
                CORESIGHT_DWT.COMPARATOR[ch].DWT_MASK.MASK = 0;

                CORESIGHT_DWT.COMPARATOR[ch_1].DWT_FUNCTION.all_in_one = 0x00000000;
                CORESIGHT_DWT.COMPARATOR[ch_1].DWT_COMP = 0x00000000;
                CORESIGHT_DWT.COMPARATOR[ch_1].DWT_MASK.MASK = 0;
                dwt_println("%s:%d\r\n",__FUNCTION__,__LINE__);
                goto ___fail_exit;
            }
        }
        CORESIGHT_DWT.COMPARATOR[ch].DWT_COMP = 0x00000000;
        CORESIGHT_DWT.COMPARATOR[ch].DWT_FUNCTION.all_in_one = 0x00000000;
        CORESIGHT_DWT.COMPARATOR[ch].DWT_MASK.MASK = 0;
        dwt_println("%s:%d\r\n",__FUNCTION__,__LINE__);
        goto ___fail_exit;
    }
    else if ((addr1)||(addr2))
    {
        if(!dwt_comparator[ch].can_double_value_watch)
        {
            dwt_println("[ERROR]: ch[%d] is unsupported\r\n",ch);
            CORESIGHT_DWT.COMPARATOR[ch].DWT_COMP = 0x00000000;
            CORESIGHT_DWT.COMPARATOR[ch].DWT_FUNCTION.all_in_one = 0x00000000;
            CORESIGHT_DWT.COMPARATOR[ch].DWT_MASK.MASK = 0;
            goto ___fail_exit;
        }
        for (int ch_1 = 0; ch_1 < CORESIGHT_DWT.DWT_CTRL.NUMCOMP; ch_1++)
        {
            if(CORESIGHT_DWT.COMPARATOR[ch_1].DWT_FUNCTION.FUNCTION == 0)
            {
                CORESIGHT_DWT.COMPARATOR[ch].DWT_FUNCTION.DATAVMATCH = 1;
                CORESIGHT_DWT.COMPARATOR[ch].DWT_FUNCTION.DATAVADDR0 = ch_1;
                CORESIGHT_DWT.COMPARATOR[ch_1].DWT_COMP = ((uint32_t)addr1)|((uint32_t)addr2);
                CORESIGHT_DWT.COMPARATOR[ch_1].DWT_FUNCTION.FUNCTION = READ_WRITE;
                CORESIGHT_DWT.COMPARATOR[ch_1].DWT_FUNCTION.DATAVSIZE = data_size;
                dwt_show_info();
                return 0;
            }
        }
        CORESIGHT_DWT.COMPARATOR[ch].DWT_COMP = 0x00000000;
        CORESIGHT_DWT.COMPARATOR[ch].DWT_FUNCTION.all_in_one = 0x00000000;
        CORESIGHT_DWT.COMPARATOR[ch].DWT_MASK.MASK = 0;
        dwt_println("%s:%d\r\n",__FUNCTION__,__LINE__);
        goto ___fail_exit;
    }
___fail_exit:
    dwt_show_support();
    return -1;
}

int data_value_watch_stop(int ch)
{
    int ch0,ch1;
    ch0 = CORESIGHT_DWT.COMPARATOR[ch].DWT_FUNCTION.DATAVADDR0;
    ch1 = CORESIGHT_DWT.COMPARATOR[ch].DWT_FUNCTION.DATAVADDR1;

    if(CORESIGHT_DWT.COMPARATOR[ch].DWT_FUNCTION.DATAVMATCH)
    {
        CORESIGHT_DWT.COMPARATOR[ch0].DWT_FUNCTION.all_in_one = 0x00000000;
        CORESIGHT_DWT.COMPARATOR[ch0].DWT_COMP = 0;
        CORESIGHT_DWT.COMPARATOR[ch0].DWT_MASK.MASK = 0;    
        if(CORESIGHT_DWT.COMPARATOR[ch].DWT_FUNCTION.LNK1ENA)
        {
            CORESIGHT_DWT.COMPARATOR[ch1].DWT_FUNCTION.all_in_one = 0x00000000;
            CORESIGHT_DWT.COMPARATOR[ch1].DWT_COMP = 0;
            CORESIGHT_DWT.COMPARATOR[ch1].DWT_MASK.MASK = 0;            
        }
        CORESIGHT_DWT.COMPARATOR[ch].DWT_FUNCTION.all_in_one = 0x00000000;
        CORESIGHT_DWT.COMPARATOR[ch].DWT_COMP = 0;
        CORESIGHT_DWT.COMPARATOR[ch].DWT_MASK.MASK = 0;
    }
}

