from building import *

cwd = GetCurrentDir()
src = Glob('*.c')
CPPPATH = [cwd]

group = DefineGroup('EasyFlash', src, depend = ['PKG_USING_ULOG_EASYFLASH'], CPPPATH = CPPPATH)

Return('group')
