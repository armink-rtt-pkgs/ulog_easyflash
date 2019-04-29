/*
 * This file is part of the EasyFlash Library.
 *
 * Copyright (c) 2014-2018, Armink, <armink.ztl@gmail.com>
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
 * The ulog backend implementation for EasyFlash.
 * Created on: 2018-10-22
 */

#include <rtthread.h>
#include <easyflash.h>
#include <string.h>

#define LOG_TAG              "easyflash"
#include <ulog.h>

#ifdef ULOG_EASYFLASH_BACKEND_ENABLE

#if defined(ULOG_ASYNC_OUTPUT_BY_THREAD) && ULOG_ASYNC_OUTPUT_THREAD_STACK < 1024
#error "The thread stack size must more than 1024 when using async output by thread (ULOG_ASYNC_OUTPUT_BY_THREAD)"
#endif

static struct ulog_backend flash_backend;
static rt_uint32_t log_saving_lvl = LOG_FILTER_LVL_ALL;

/**
 * Read and output log to console.
 *
 * @param index index for saved log.
 *        Minimum index is 0.
 *        Maximum index is log used flash total size - 1.
 * @param size
 */
static void read_flash_log(size_t index, size_t size)
{
    /* 64 bytes buffer */
    uint32_t buf[16] = { 0 };
    size_t log_total_size = ef_log_get_used_size();
    size_t buf_size = sizeof(buf);
    size_t read_size = 0;

    /* word alignment for index and size */
    index = RT_ALIGN_DOWN(index, 4);
    size = RT_ALIGN_DOWN(size, 4);
    if (index + size > log_total_size)
    {
        rt_kprintf("The output position and size is out of bound. The max size is %d.\n", log_total_size);
        return;
    }

    while (1)
    {
        if (read_size + buf_size < size)
        {
            ef_log_read(index + read_size, buf, buf_size);
            rt_kprintf("%.*s", buf_size, buf);
            read_size += buf_size;
        }
        else
        {
            ef_log_read(index + read_size, buf, size - read_size);
            rt_kprintf("%.*s", size - read_size, buf);
            /* output newline sign */
            rt_kprintf(ULOG_NEWLINE_SIGN);
            break;
        }
    }
}

/**
 * Read and output all log which saved in flash.
 */
static void read_all_flash_log(void)
{
    read_flash_log(0, ef_log_get_used_size());
}


/**
 * Read and output recent log which saved in flash.
 *
 * @param size recent log size
 */
static void read_recent_flash_log(size_t size)
{
    size_t max_size = ef_log_get_used_size();

    if (size == 0)
    {
        return;
    }

    if (size > max_size)
    {
        rt_kprintf("The output size is out of bound. The max size is %d.\n", max_size);
    }
    else
    {
        read_flash_log(max_size - size, size);
    }
}

/**
 * clean all log which in flash
 */
void ulog_ef_log_clean(void)
{
    EfErrCode clean_result = EF_NO_ERR;

    /* clean all log which in flash */
    clean_result = ef_log_clean();

    if (clean_result == EF_NO_ERR)
    {
        LOG_I("All logs which in flash is clean OK.");
    }
    else
    {
        LOG_E("Clean logs which in flash has an error!");
    }
}

static void ulog_easyflash_backend_output(struct ulog_backend *backend, rt_uint32_t level, const char *tag, rt_bool_t is_raw,
        const char *log, size_t len)
{
    /* write some '\r' for word alignment */
    char write_overage_c[4] = { '\r', '\r', '\r', '\r' };
    size_t write_size_temp = 0;
    EfErrCode result = EF_NO_ERR;

    /* saving level filter for flash log */
    if (level <= log_saving_lvl)
    {
        /* calculate the word alignment write size */
        write_size_temp = RT_ALIGN_DOWN(len, 4);

        result = ef_log_write((uint32_t *) log, write_size_temp);
        /* write last word alignment data */
        if ((result == EF_NO_ERR) && (write_size_temp != len))
        {
            memcpy(write_overage_c, log + write_size_temp, len - write_size_temp);
            ef_log_write((uint32_t *) write_overage_c, sizeof(write_overage_c));
        }
    }
}

/**
 * Set flash log saving level. The log which level less than setting will stop saving to flash.
 *
 * @param level setting level
 */
void ulog_ef_log_lvl_set(rt_uint32_t level)
{
    log_saving_lvl = level;
}

int ulog_ef_backend_init(void)
{
    flash_backend.output = ulog_easyflash_backend_output;

    ulog_backend_register(&flash_backend, "easyflash", RT_TRUE);

    return 0;
}
INIT_APP_EXPORT(ulog_ef_backend_init);

#if defined(RT_USING_FINSH) && defined(FINSH_USING_MSH)
#include <finsh.h>
static void ulog_flash(uint8_t argc, char **argv)
{
    if (argc >= 2)
    {
        if (!strcmp(argv[1], "read"))
        {
            if (argc >= 3)
            {
                read_recent_flash_log(atol(argv[2]));
            }
            else
            {
                read_all_flash_log();
            }
        }
        else if (!strcmp(argv[1], "clean"))
        {
            ulog_ef_log_clean();
        }
        else
        {
            rt_kprintf("Please input ulog_flash <read|clean>.\n");
        }
    }
    else
    {
        rt_kprintf("Please input ulog_flash <read|clean>.\n");
    }
}
MSH_CMD_EXPORT(ulog_flash, ulog <read|clean> flash log by EasyFlash backend);
#endif /* defined(RT_USING_FINSH) && defined(FINSH_USING_MSH) */

#endif /* ULOG_EASYFLASH_BACKEND_ENABLE */
