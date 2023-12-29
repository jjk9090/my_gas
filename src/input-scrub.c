/*
 * @Author: hxy
 * @Date: 2023-10-04 16:18:37
 * @LastEditTime: 2023-11-06 11:25:52
 * @Description: 请填写简介
 */
#include "as.h"
#include "app.h"
#include "filenames.h"
#include "input-file.h"
#include "sb.h"
#include "listing.h"
#define AFTER_SIZE  (1)
#define BEFORE_STRING ("\n")
#define AFTER_STRING ("\0")	/* memcpy of 0 chars might choke.  */
#define BEFORE_SIZE (1)

#ifndef TC_EOL_IN_INSN
#define TC_EOL_IN_INSN(P) 0
#endif
// sb的嵌套数
int macro_nest;
static bool is_linefile;
static const char *physical_input_file;
static const char *logical_input_file;
static unsigned int physical_input_line;
static unsigned int logical_input_line;
static sb from_sb;
static enum expansion from_sb_expansion = expanding_none;
static size_t sb_index = -1;  /* sb的第几层 */
static size_t buffer_length;  /* 输入缓冲区的大小 */
static char *buffer_start;	/*指向完整缓冲区区域的第一个字符 */

static char *partial_where;
static size_t partial_size;	/* 表示缓冲区中部分行中的字符数量 */


struct input_save {
    char *              buffer_start;
    char *              partial_where;
    size_t              partial_size;
    char                save_source[AFTER_SIZE];
    size_t              buffer_length;
    const char *        physical_input_file;
    const char *        logical_input_file;
    unsigned int        physical_input_line;
    unsigned int        logical_input_line;
    bool                is_linefile;
    size_t              sb_index;
    sb                  from_sb;
    enum expansion      from_sb_expansion; /* Should we do a conditional check?  */
    struct input_save * next_saved_file;	/* Chain of input_saves.  */
    char *              input_file_save;	/* Saved state of input routines.  */
    char *              saved_position;	/* Caller's saved position in buf.  */
};

static struct input_save *next_saved_file;
static char save_source[AFTER_SIZE];

static void
input_scrub_reinit (void) {
    input_file_begin ();	
    logical_input_line = -1u;
    logical_input_file = NULL;
    sb_index = -1;

    buffer_length = input_file_buffer_size () * 2;
    // 包括了前置字符串 BEFORE_STRING、后置字符串 AFTER_STRING、
    // 一个额外的字符用于保存换行符，并且可以容纳指定长度的源代码内容
    buffer_start = XNEWVEC (char, BEFORE_SIZE + AFTER_SIZE + 1 + buffer_length);
}
/*
* 对文件缓冲区的字符进行初始化
*/
void
input_scrub_begin (void) {
    know (strlen (BEFORE_STRING) == BEFORE_SIZE);
    know (strlen (AFTER_STRING) == AFTER_SIZE
        || (AFTER_STRING[0] == '\0' && AFTER_SIZE == 1));
    physical_input_file = NULL;
    next_saved_file = NULL;
    macro_nest = 0;

    // 初始化文件的输入缓冲区
    input_scrub_reinit ();
    /*
        do_scrub_begin 函数的作用是对当前汇编源文件进行词法分析的初始化处理。
        在这个函数中，将不同类型的字符，如空格、字母、数字、换行符等，
        标记上不同的词法类型，用于后续的词法分析。
    */
    do_scrub_begin (flag_m68k_mri);
}


void
as_report_context (void)
{
    const struct input_save *saved = next_saved_file;
    enum expansion expansion = from_sb_expansion;
    int indent = 1;

    if (!macro_nest)
        return;

    do {
        if (expansion != expanding_macro)
    /* Nothing.  */;
        else if (saved->logical_input_file != NULL
          && saved->logical_input_line != -1u)
    as_info_where (saved->logical_input_file, saved->logical_input_line,
            indent, _("macro invoked from here"));
        else
    as_info_where (saved->physical_input_file, saved->physical_input_line,
            indent, _("macro invoked from here"));

        expansion = saved->from_sb_expansion;
        ++indent;
      }
    while ((saved = saved->next_saved_file) != NULL);
}

const char *
as_where (unsigned int *linep) {
  const char *file = as_where_top (linep);

  if (macro_nest && is_linefile) {
        const struct input_save *saved = next_saved_file;
        enum expansion expansion = from_sb_expansion;

        do {
            if (expansion != expanding_macro)
                /* Nothing.  */;
            else if (saved->logical_input_file != NULL
                && (linep == NULL || saved->logical_input_line != -1u)){
                if (linep != NULL)
                *linep = saved->logical_input_line;
                file = saved->logical_input_file;
            }
            else if (saved->physical_input_file != NULL){
                if (linep != NULL)
                *linep = saved->physical_input_line;
                file = saved->physical_input_file;
            }

            expansion = saved->from_sb_expansion;
        } while ((saved = saved->next_saved_file) != NULL);
    }

  return file;
}

const char *
as_where_physical (unsigned int *linep)
{
    if (physical_input_file != NULL) {
        if (linep != NULL)
	        *linep = physical_input_line;
        return physical_input_file;
    }

    if (linep != NULL)
        *linep = 0;
    return NULL;
}

const char *
as_where_top (unsigned int *linep)
{
    if (logical_input_file != NULL
        && (linep == NULL || logical_input_line != -1u)) {
        if (linep != NULL)
            *linep = logical_input_line;
        return logical_input_file;
    }

    return as_where_physical (linep);
}

char *
input_scrub_new_file (const char *filename)
{
    input_file_open (filename, !flag_no_comments);
    physical_input_file = filename[0] ? filename : _("{standard input}");
    physical_input_line = 0;

    // 表示缓冲区中部分行中的字符数量
    partial_size = 0;
    return (buffer_start + BEFORE_SIZE);
}

/**
 * @Description: 解析文件
 */
char *
input_scrub_next_buffer (char **bufp)
{
    char *limit;		/*->just after last char of buffer.  */
    if (sb_index != (size_t) -1) {
        if (sb_index >= from_sb.len) {
        sb_kill (&from_sb);
        if (from_sb_expansion == expanding_macro) {
            // cond_finish_check (macro_nest);
#ifdef md_macro_end
	      /* Allow the target to clean up per-macro expansion
	         data.  */
	      md_macro_end ();
#endif
	    }
        --macro_nest;
        partial_where = NULL;
        partial_size = 0;
        // if (next_saved_file != NULL)
        //     *bufp = input_scrub_pop (next_saved_file);
        return partial_where;
    }

        partial_where = from_sb.ptr + from_sb.len;
        partial_size = 0;
        *bufp = from_sb.ptr + sb_index;
        sb_index = from_sb.len;
        return partial_where;
    }

    if (partial_size) {
        memmove (buffer_start + BEFORE_SIZE, partial_where, partial_size);
        memcpy (buffer_start + BEFORE_SIZE, save_source, AFTER_SIZE);
    }
    while (1) {
        char *p;
        char *start = buffer_start + BEFORE_SIZE + partial_size;
        *bufp = buffer_start + BEFORE_SIZE;
        limit = input_file_give_next_buffer (start);
        if (!limit) {
            if (!partial_size) {
                break;
            }
            as_warn (_("end of file not at end of a line; newline inserted"));
            p = buffer_start + BEFORE_SIZE + partial_size;
            *p++ = '\n';
            limit = p;
        } else {
            *limit = '\0';
            // 找到最后的新行
            for (p = limit - 1; *p != '\n' || TC_EOL_IN_INSN (p); --p)
                if (p < start)
                    return NULL;
            ++p;
        }
        partial_where = p;
        partial_size = limit - p;
        memcpy (save_source, partial_where, (int) AFTER_SIZE);
        memcpy (partial_where, AFTER_STRING, (int) AFTER_SIZE);
        return partial_where;        
    }   
    return partial_where;
}

void
bump_line_counters (void) {
  if (sb_index == (size_t) -1)
    ++physical_input_line;

  if (logical_input_line != -1u)
    ++logical_input_line;
}

void
input_scrub_close (void) {
  input_file_close ();
  physical_input_line = 0;
  logical_input_line = -1u;
}

int
seen_at_least_1_file (void) {
    return (physical_input_file != NULL);
}

void
input_scrub_end (void) {
    if (buffer_start) {
        free (buffer_start);
        buffer_start = 0;
        input_file_end ();
    }
}