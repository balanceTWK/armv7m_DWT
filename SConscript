from building import *

cwd = GetCurrentDir()

src = []
path = []

src  += ['src/armv7m_dwt.c']
path += [cwd + '/src']

src  += ['port/DebugMon_Handler.c']
src  += ['port/dwt_rtt_port.c']
path += [cwd + '/port']


group = DefineGroup('arm_dwt', src, depend = ['PKG_USING_ARMV7M_DWT_TOOL'], CPPPATH = path)

Return('group')
