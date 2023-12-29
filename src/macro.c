/*
 * @Author: hxy
 * @Date: 2023-10-02 18:21:43
 * @LastEditTime: 2023-11-09 11:56:27
 * @Description: 处理宏
 */
#include "as.h"
#include "sb.h"
#include "macro.h"

htab_t macro_hash;  /* 表示宏定义的哈希表 */
int macro_defined;  /* 是否有宏被定义*/
static void
del_formal (formal_entry *formal)
{
    sb_kill (&formal->actual);
    sb_kill (&formal->def);
    sb_kill (&formal->name);
    free (formal);
}

/**
 * @Description: 释放分配给宏的内存
 */
static void
free_macro (macro_entry *macro)
{
    formal_entry *formal;

    for (formal = macro->formals; formal; ) {
        formal_entry *f;

        f = formal;
        formal = formal->next;
        del_formal (f);
    }
    htab_delete (macro->formal_hash);
    sb_kill (&macro->sub);
    free ((char *) macro->name);
    free (macro);
}

static void
macro_del_f (void *ent)
{
    string_tuple_t *tuple = ent;
    free_macro ((macro_entry *) tuple->value);
}

/**
 * @Description: 初始化宏处理
 */
void
macro_init (void)
{
    macro_hash = htab_create_alloc (16, hash_string_tuple, eq_string_tuple,
            macro_del_f, notes_calloc, NULL);
    macro_defined = 0;
}

void
macro_end (void) {
    htab_delete (macro_hash);
}