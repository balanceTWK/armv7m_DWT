#include "dwt_cfg.h"

void DebugMon_Handler(void)
{
    dwt_println("DebugMon_Handler\r\n");
#ifdef PKG_USING_ARMV7M_DWT_TOOL_DEMO
    extern int armv7_dwt_rtt_demo(void);
    armv7_dwt_rtt_demo();
#endif
}
