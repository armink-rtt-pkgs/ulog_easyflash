# Ulog plugin based on EasyFlash

[中文页](README_ZH.md) | English

## 1. Introduction

ulog is a brand new log component of RT-Thread. Its front-end and back-end separation design allows more back-ends to be easily connected. This software package mainly implements the two functions of **ulog's Flash backend** and **ulog's filter parameter storage**. Its bottom layer is based on the Flash flash library [EasyFlash](https://github.com/armink) /EasyFlash) makes it easy to save logs and filter parameters from ulog on Flash. Its main features are as follows:

- Small resource occupation, seamless connection with ENV and LOG functions of EasyFlash;
- Logs are stored in a circular replacement method. When the log partition is full, the oldest log will be automatically deleted;
- The stored history log can be read into the RT-Thread console for easy reading and debugging;
- Optionally read a part of recent logs to the RT-Thread console;
- Automatically load the saved ulog filter parameters at startup;
- All functions provide Finsh/MSH commands, support: log reading, log cleaning, saving filter parameters.

### 1.1 License

This package complies with the MIT license, see the `LICENSE` file for details.

### 1.2 Dependency

- RT-Thread 3.1.1+
- EasyFlash 3.0.0+

## 2. How to open

To use this package, you need to select it in the package manager of RT-Thread. The specific path is as follows:

```
RT-Thread online packages
    tools packages --->
        [*] ulog_easyflash: The ulog flash plugin by EasyFlash.
            [*] Enable the flash backend for ulog
            [*] Save the ulog filter configuration to flash
            Version (latest) --->
```

- `Enable the flash backend for ulog`: Enable the flash backend function of ulog;
- `Save the ulog filter configuration to flash`: Turn on the function of saving ulog filter parameters.

Then let RT-Thread's package manager automatically update, or use the `pkgs --update` command to update the package to the BSP.

> **Note**:
>
>- If the LOG function of EasyFlash has not been enabled before, after opening this software package, you need to configure the log area size in the EasyFlash options. The configuration name is: Saved log area size.
>- Before using the function of saving filter parameters, ensure that the runtime filter function is enabled on the ulog side, and the configuration name is: Enable runtime log filter.

## 3. Instructions for use

### 3.1 Flash backend initialization

You need to call the initialization function `ulog_ef_backend_init()` at the application layer. If the project enables automatic component initialization, even this function does not need to be called. The component initialization function has been added to this function in the software package.

### 3.2 Set Flash log save level

Through this function: `void ulog_ef_log_lvl_set(rt_uint32_t level)`, you can set the log level you want to save to Flash. Logs below this level will be **discarded**. Example: To set to save only the logs at the warning level and above, you can execute the following code:

```c
ulog_ef_log_lvl_set(LOG_LVL_WARNING);
```

### 3.3 Load ulog filter parameters

Through this function: `void ulog_ef_filter_cfg_load(void)`, you can reprint the ulog filtering parameters already stored in Flash. If the project has enabled component auto-initialization, this function will run automatically upon power-up.

### 3.4 Save ulog filter parameters

Through this function: `void ulog_ef_filter_cfg_save(void)`, you can save the filtering parameters that ulog has set to flash. This function also has a corresponding Finsh/MSH command: `ulog_filter_save`. When you need to save, just execute it.

### 3.5 Use of Finsh/MSH commands

The command format related to Flash log is `ulog_flash <read|clean>`

#### 3.5.1 Historical log reading

After the history log is saved in Flash, it can be read to the console through the following command, which is convenient for developers to review the log and analyze the problem.

- To read all historical logs, enter the command: `ulog_flash read`

```shell
msh />ulog_flash read
10-23 10:28:51.618 D/example tshell: LOG_D(1): RT-Thread is an open source IoT operating system from China.
10-23 10:28:51.618 I/example tshell: LOG_I(1): RT-Thread is an open source IoT
......
msh />
```

- To read the recent 200-byte log, enter the command: `ulog_flash read 200`

```shell
msh />ulog_flash read 200
log_w(50): RT-Thread is an open source IoT operating system from China.
10-23 19:37:05.137 E/test tshell: ulog_e(50): RT-Thread is an open source IoT operating system from China.

msh />
```

#### 3.5.2 Clear history log

When you need to clear all historical logs in the log area, you can enter the command: `ulog_flash clean`

```shell
msh />ulog_flash clean
10-24 10:00:22.307 I/easyflash tshell: All logs which in flash is clean OK.
msh />
```

Wait for a while, the log cleaning is complete, a prompt of successful cleaning will be displayed.

#### 3.5.3 Save ulog filter parameters to Flash

In the debugging and development process, after the ulog filter parameters are set, if you need to save, you can enter the command: `ulog_filter_save`. After saving, restart and use the `ulog_filter` command again, you can see the previously saved filter parameters.

```shell
msh />ulog_filter_save
[Flash] (../packages/EasyFlash-latest/src/ef_env.c:821) Calculate ENV CRC32 number is 0x10C41F91.
[Flash] (../packages/EasyFlash-latest/src/ef_env.c:774) Erased ENV OK.
[Flash] (../packages/EasyFlash-latest/src/ef_env.c:788) Saved ENV OK.
msh />
```

## 4. Matters needing attention

- Before using this package, make sure that the dependent options are opened in advance, otherwise the options cannot be seen in menuconfig.

## 5. Contact & Thanks

* Maintenance: [armink](https://github.com/armink)
* Homepage: https://github.com/armink-rtt-pkgs/ulog_easyflash
