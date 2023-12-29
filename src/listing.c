/*
 * @Author: hxy
 * @Date: 2023-10-06 21:04:51
 * @LastEditTime: 2023-10-06 21:55:14
 * @Description: 维护和生成汇编列表
 */

#include "as.h"
#include "filenames.h"
#include "safe-ctype.h"
#include "input-file.h"
#include "subsegs.h"
#include "bfdver.h"
#include <time.h>
#include <stdarg.h>
#ifndef NO_LISTING

typedef struct file_info_struct
{
    struct file_info_struct * next;
    char *                    filename;
    long                      pos;
    unsigned int              linenum;
    int                       at_end;
} file_info_type;

enum edict_enum
{
    EDICT_NONE,
    EDICT_SBTTL,
    EDICT_TITLE,
    EDICT_NOLIST,
    EDICT_LIST,
    EDICT_NOLIST_NEXT,
    EDICT_EJECT
};

struct list_message
{
    char *message;
    struct list_message *next;
};

struct list_info_struct
{
    /* Frag which this line of source is nearest to.  */
    fragS *frag;

    /* The actual line in the source file.  */
    unsigned int line;

    /* Pointer to the file info struct for the file which this line
        belongs to.  */
    file_info_type *file;

    /* The expanded text of any macro that may have been executing.  */
    char *line_contents;

    /* Next in list.  */
    struct list_info_struct *next;

    /* Pointer to the file info struct for the high level language
        source line that belongs here.  */
    file_info_type *hll_file;

    /* High level language source line.  */
    unsigned int hll_line;

    /* Pointers to linked list of messages associated with this line.  */
    struct list_message *messages, *last_message;

    enum edict_enum edict;
    char *edict_arg;

    /* Nonzero if this line is to be omitted because it contains
        debugging information.  This can become a flags field if we come
        up with more information to store here.  */
    int debugging;
};

struct list_info_struct *        listing_tail;
static file_info_type *          file_info_head;
typedef struct list_info_struct list_info_type;
static const char *fn;
static struct list_info_struct * head;

static void
new_frag (void)
{
  frag_wane (frag_now);
  frag_new (0);
}

static file_info_type *
file_info (const char *file_name)
{
    /* Find an entry with this file name.  */
    file_info_type *p = file_info_head;

    while (p != (file_info_type *) NULL) {
        if (filename_cmp (p->filename, file_name) == 0)
            return p;
        p = p->next;
    }

    /* Make new entry.  */
    p = XNEW (file_info_type);
    p->next = file_info_head;
    file_info_head = p;
    p->filename = xstrdup (file_name);
    p->pos = 0;
    p->linenum = 0;
    p->at_end = 0;

    return p;
}
/**
 * @Description: 在生成汇编代码的过程中将相关信息添加到列表中符
 */
void
listing_newline (char *ps)
{
    const char *file;
    unsigned int line;
    static unsigned int last_line = 0xffff;
    static const char *last_file = NULL;
    list_info_type *new_i = NULL;

    if (listing == 0)
        return;

    if (now_seg == absolute_section)
        return;

#ifdef OBJ_ELF
  /* In ELF, anything in a section beginning with .debug or .line is
     considered to be debugging information.  This includes the
     statement which switches us into the debugging section, which we
     can only set after we are already in the debugging section.  */
    if ((listing & LISTING_NODEBUG) != 0
        && listing_tail != NULL
        && ! listing_tail->debugging) {
        const char *segname;

        segname = segment_name (now_seg);
        if (startswith (segname, ".debug")
        || startswith (segname, ".line"))
            listing_tail->debugging = 1;
    }
#endif

  /* PR 21977 - use the physical file name not the logical one unless high
     level source files are being included in the listing.  */
    if (listing & LISTING_HLL)
        file = as_where (&line);
    else
        file = as_where_physical (&line);

    if (ps == NULL) {
        // 判断文件名和行号是否与上一次相同
        if (line == last_line
            && !(last_file && file && filename_cmp (file, last_file)))
	        return;

        new_i = XNEW (list_info_type);

        if (strcmp (file, _("{standard input}")) == 0
            && input_line_pointer != NULL) {
            char *copy, *src, *dest;
            int len;
            int seen_quote = 0;
            int seen_slash = 0;

            for (copy = input_line_pointer;
                *copy && (seen_quote
                    || is_end_of_line [(unsigned char) *copy] != 1);
                copy++) {
                if (seen_slash)
                    seen_slash = 0;
                else if (*copy == '\\')
                    seen_slash = 1;
                else if (*copy == '"')
                    seen_quote = !seen_quote;
            }

            len = copy - input_line_pointer + 1;

            copy = XNEWVEC (char, len);
            // 原来行
            src = input_line_pointer;
            // 新疆行
            dest = copy;

            while (--len) {
                unsigned char c = *src++;

                /* 筛选 剪切行中的无用字符*/
                if (!ISCNTRL (c))
                    *dest++ = c;
            }

            *dest = 0;
            // 将当前行的内容保存到line_contents字段中
            new_i->line_contents = copy;
        }
        else
	        new_i->line_contents = NULL;
    } else {
        new_i = XNEW (list_info_type);
        new_i->line_contents = ps;
    }

    last_line = line;
    last_file = file;

    new_frag ();

    if (listing_tail)
        listing_tail->next = new_i;
    else
        head = new_i;

    listing_tail = new_i;

    new_i->frag = frag_now;
    new_i->line = line;
    new_i->file = file_info (file);
    new_i->next = (list_info_type *) NULL;
    new_i->messages = NULL;
    new_i->last_message = NULL;
    new_i->edict = EDICT_NONE;
    new_i->hll_file = (file_info_type *) NULL;
    new_i->hll_line = 0;
    new_i->debugging = 0;

    new_frag ();

#ifdef OBJ_ELF
    /* In ELF, anything in a section beginning with .debug or .line is
        considered to be debugging information.  */
    if ((listing & LISTING_NODEBUG) != 0) {
        const char *segname;

        segname = segment_name (now_seg);
        if (startswith (segname, ".debug")
            || startswith (segname, ".line"))
            new_i->debugging = 1;
    }
#endif
}
void
listing_file (const char *name)
{
  fn = name;
}
#endif
