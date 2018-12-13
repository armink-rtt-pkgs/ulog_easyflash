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
 * The ulog filter configuration store implement by EasyFlash.
 * Created on: 2018-11-08
 */

#include <rtthread.h>

#ifdef ULOG_EASYFLASH_CFG_SAVE_ENABLE

#include <easyflash.h>
#include <stdlib.h>
#include <string.h>

#define LOG_TAG              "easyflash"
#include <ulog.h>

#define ENV_FILTER_GLOBAL_LVL_NAME     "ulog.lvl"
#define ENV_FILTER_GLOBAL_TAG_NAME     "ulog.tag"
#define ENV_FILTER_GLOBAL_KW_NAME      "ulog.kw"
#define ENV_FILTER_TAG_LVL_NAME        "ulog.tag_lvl"

extern size_t ulog_ultoa(char *s, unsigned long int n);

/**
 * load the ulog configuration on flash
 *
 * @return result, 0 : success, else error
 *
 * @note don't using `&` and `##` on log tag definition when using this function.
 */
int ulog_ef_filter_cfg_load(void)
{
    char *value;

    /* restore the saving global level */
    if ((value = ef_get_env(ENV_FILTER_GLOBAL_LVL_NAME)) != NULL)
    {
        ulog_global_filter_lvl_set(atoi(value));
    }

    /* restore the saving global tag */
    if ((value = ef_get_env(ENV_FILTER_GLOBAL_TAG_NAME)) != NULL)
    {
        ulog_global_filter_tag_set(value);
    }

    /* restore the saving global kw */
    if ((value = ef_get_env(ENV_FILTER_GLOBAL_KW_NAME)) != NULL)
    {
        ulog_global_filter_kw_set(value);
    }

    /* restore the saving tag level list */
    if ((value = ef_get_env(ENV_FILTER_TAG_LVL_NAME)) != NULL)
    {
        char lvl_num[11], tag[ULOG_FILTER_TAG_MAX_LEN + 1], *lvl_pos, *next_node;
        rt_size_t node_len;
        /* decode every tag's level */
        while (1)
        {
            /* find every node */
            if ((next_node = strstr(value, "##")) != NULL)
            {
                node_len = next_node - value;
            }
            else
            {
                node_len = rt_strlen(value);
            }
            /* find level pos */
            lvl_pos = strstr(value, "&");
            if (lvl_pos != NULL && lvl_pos < value + node_len)
            {
                rt_strncpy(tag, value, lvl_pos - value);
                rt_strncpy(lvl_num, lvl_pos + 1, value + node_len - lvl_pos - 1);
                tag[lvl_pos - value] = '\0';
                lvl_num[value + node_len - lvl_pos - 1] = '\0';
                /* add a tag's level filter */
                ulog_tag_lvl_filter_set(tag, atoi(lvl_num));
            }
            else
            {
                LOG_W("Warning: tag's level decode failed!");
                break;
            }

            if (next_node)
            {
                value = next_node + 2;
            }
            else
            {
                break;
            }
        }
    }

    return 0;
}
INIT_APP_EXPORT(ulog_ef_filter_cfg_load);

/**
 * save the ulog filter configuration to flash
 *
 * @note don't using `&` and `##` on log tag definition when using this function.
 */
void ulog_ef_filter_cfg_save(void)
{
    unsigned char *cfgs = NULL;
    char lvl_num[11];

    /* set the global level env */
    {
        ulog_ultoa(lvl_num, ulog_global_filter_lvl_get());
        ef_set_env(ENV_FILTER_GLOBAL_LVL_NAME, lvl_num);
    }

    /* set the global tag env */
    if (rt_strlen(ulog_global_filter_tag_get()))
    {
        ef_set_env(ENV_FILTER_GLOBAL_TAG_NAME, ulog_global_filter_tag_get());
    }
    else if(ef_get_env(ENV_FILTER_GLOBAL_TAG_NAME))
    {    
        ef_del_env(ENV_FILTER_GLOBAL_TAG_NAME);
    }
    
    /* set the global kw env */
    if (rt_strlen(ulog_global_filter_kw_get()))
    {
        ef_set_env(ENV_FILTER_GLOBAL_KW_NAME, ulog_global_filter_kw_get());
    }
    else if(ef_get_env(ENV_FILTER_GLOBAL_KW_NAME))
    {    
        ef_del_env(ENV_FILTER_GLOBAL_KW_NAME);
    }
    
    /* set the tag's level env */
    {
        rt_slist_t *node;
        ulog_tag_lvl_filter_t tag_lvl = NULL;
        rt_size_t node_size, tag_len, lvl_len;
        int cfgs_size = 0;

        for (node = rt_slist_first(ulog_tag_lvl_list_get()); node; node = rt_slist_next(node))
        {
            tag_lvl = rt_slist_entry(node, struct ulog_tag_lvl_filter, list);
            ulog_ultoa(lvl_num, tag_lvl->level);
            tag_len = rt_strlen(tag_lvl->tag);
            lvl_len = rt_strlen(lvl_num);
            /* env string format: tag_name1&tag_lvl1##tag_name2&tag_lvl2## */
            node_size = tag_len + 1 + lvl_len + 2;
            cfgs_size += node_size;
            cfgs = (unsigned char *) rt_realloc(cfgs, cfgs_size);
            if (cfgs == NULL)
            {
                LOG_W("Warning: no memory for save cfgs");
                goto __exit;
            }
            rt_memcpy(cfgs + cfgs_size - node_size                        , tag_lvl->tag, tag_len);
            rt_memcpy(cfgs + cfgs_size - node_size + tag_len              , "&", 1);
            rt_memcpy(cfgs + cfgs_size - node_size + tag_len + 1          , lvl_num, lvl_len);
            rt_memcpy(cfgs + cfgs_size - node_size + tag_len + 1 + lvl_len, "##", 2);
        }

        if((cfgs)&&(cfgs_size>2))
        {    
            cfgs[cfgs_size - 2] = '\0';
            ef_set_env(ENV_FILTER_TAG_LVL_NAME, (char *)cfgs);
        }
        else if(ef_get_env(ENV_FILTER_TAG_LVL_NAME))
        {    
            ef_del_env(ENV_FILTER_TAG_LVL_NAME);
        }
    }

__exit:
    /* save the ulog filter env */
    ef_save_env();

    if (cfgs)
    {
        rt_free(cfgs);
    }
}
#if defined(RT_USING_FINSH) && defined(FINSH_USING_MSH)
#include <finsh.h>
MSH_CMD_EXPORT_ALIAS(ulog_ef_filter_cfg_save, ulog_filter_save, Save the ulog filter settings);
#endif /* defined(RT_USING_FINSH) && defined(FINSH_USING_MSH) */

#endif /* ULOG_EASYFLASH_CFG_SAVE_ENABLE */
