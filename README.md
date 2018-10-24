# 基于 EasyFlash 的 ulog 日志后端实现
## 1、介绍

ulog 是 RT-Thread 全新的日志组件，其前后端分离式设计，使得更多的后端可以轻松的对接上去。该软件包就是 ulog 的后端实现，它的底层基于 Flash 闪存库 [EasyFlash](https://github.com/armink/EasyFlash) ，使得来自于 ulog 的日志可以保存在 Flash 上。其主要功能特点如下：

- 资源占用小，与 EasyFlash 的 LOG 功能无缝对接；
- 日志采用循环替换方式进行存储，当日志分区满了以后，会自动删除最久的日志；
- 已存储的历史日志支持读取到 RT-Thread 控制台中，便于阅读、调试；
- 可选择性的读取近期一部分日志到 RT-Thread 控制台中；
- 所有功能提供了 Finsh/MSH 命令，支持：日志读取，日志清理。

### 1.1 许可证

本软件包遵循 MIT 许可，详见 `LICENSE` 文件。

### 1.2 依赖

- RT-Thread 3.1.1+
- EasyFlash 3.0.0+

## 2、如何打开

使用本软件包需要在 RT-Thread 的包管理器中选择它，具体路径如下：

```
RT-Thread online packages
    tools packages --->
        [*] ulog_easyflash: the ulog backend implementation for EasyFlash
```

然后让 RT-Thread 的包管理器自动更新，或者使用 `pkgs --update` 命令更新包到 BSP 中。

## 3、使用说明

软件包的使用非常简单，只需要在应用层调用 `ulog_easyflash_backend_init()` 初始化函数即可。如果项目开启了组件自动初始化，甚至连这个函数都无需调用，软件包里已经为这个函数添加了组件初始化功能。下面重点介绍下 Finsh/MSH 命令如何使用

### 3.1 Finsh/MSH 命令的使用

命令的格式为 `ulog_flash <read|clean>` 

#### 3.1.1 历史日志读取

历史日志存到 Flash 后，可以通过下面的命令读取到控制台上，方便开发者回顾日志，分析问题。

- 读取全部的历史日志，输入命令：`ulog_flash read`

```shell
msh />ulog_flash read
10-23 10:28:51.618 D/example tshell: LOG_D(1): RT-Thread is an open source IoT operating system from China.
10-23 10:28:51.618 I/example tshell: LOG_I(1): RT-Thread is an open source IoT
......
msh />
```

- 读取近期的 200 字节日志，输入命令：`ulog_flash read 200`

```shell
msh />ulog_flash read 200
log_w(50): RT-Thread is an open source IoT operating system from China.
10-23 19:37:05.137 E/test tshell: ulog_e(50): RT-Thread is an open source IoT operating system from China.

msh />
```

#### 3.1.1 清空历史日志

当需要清空日志区的全部历史日志时，可以输入命令： `ulog_flash clean`

```shell
msh />ulog_flash clean
10-24 10:00:22.307 I/easyflash tshell: All logs which in flash is clean OK.
msh />
```

稍等片刻，日志清理完成将会显示清理成功的提示。

## 4、注意事项

暂无

## 5、联系方式 & 感谢

* 维护：[armink](https://github.com/armink)
* 主页：https://github.com/armink-rtt-pkgs/ulog_easyflash
