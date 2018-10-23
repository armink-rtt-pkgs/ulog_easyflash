from building import *

cwd = GetCurrentDir()
src = Glob('quicklz.c')
CPPPATH = [cwd]

group = DefineGroup('EasyFlash', src, depend = ['PKG_USING_ULOG_EASYFLASH'], CPPPATH = CPPPATH)

Return('group')
