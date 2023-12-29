/*
 * @Author: hxy
 * @Date: 2023-10-06 21:51:46
 * @LastEditTime: 2023-11-05 09:54:17
 * @Description: 文件依赖
 */
#include "as.h"
#include "filenames.h"

static char * dep_file = NULL;
/* Current column in output file.  */
static int column = 0;
/* Number of columns allowable.  */
#define MAX_COLUMNS 72
struct dependency
{
  char * file;
  struct dependency * next;
};

/* All the files we depend on.  */
static struct dependency * dep_chain = NULL;
void
register_dependency (const char *filename) {
    struct dependency *dep;
    if (dep_file == NULL)
        return;
    if (dep_file == NULL)
        return;

    for (dep = dep_chain; dep != NULL; dep = dep->next){
        if (!filename_cmp (filename, dep->file))
            return;
    }

    dep = XNEW (struct dependency);
    dep->file = xstrdup (filename);
    dep->next = dep_chain;
    dep_chain = dep;
}

static int
quote_string_for_make (FILE *file, const char *src)
{
    const char *p = src;
    int i = 0;

    for (;;) {
        char c = *p++;

        switch (c) {
            case '\0':
            case ' ':
            case '\t':
            {
                /* GNU make uses a weird quoting scheme for white space.
                A space or tab preceded by 2N+1 backslashes represents
                N backslashes followed by space; a space or tab
                preceded by 2N backslashes represents N backslashes at
                the end of a file name; and backslashes in other
                contexts should not be doubled.  */
                const char *q;

                for (q = p - 1; src < q && q[-1] == '\\'; q--) {
                    if (file)
                        putc ('\\', file);
                    i++;
                }
            }
                if (!c)
                    return i;
                if (file)
                    putc ('\\', file);
                i++;
                goto ordinary_char;

            case '$':
                if (file)
                    putc (c, file);
                i++;
            /* Fall through.  */
            /* This can mishandle things like "$(" but there's no easy fix.  */
            default:
            ordinary_char:
                /* This can mishandle characters in the string "\0\n%*?[\\~";
                    exactly which chars are mishandled depends on the `make' version.
                    We know of no portable solution for this;
                    even GNU make 3.76.1 doesn't solve the problem entirely.
                    (Also, '\0' is mishandled due to our calling conventions.)  */
                if (file)
                    putc (c, file);
                i++;
                break;
        }
    }
}

/**
 * @Description: 添加一些输出到文件，跟踪列和做必要时进行包装
 */
static void
wrap_output (FILE *f, const char *string, int spacer)
{
    int len = quote_string_for_make (NULL, string);

    if (len == 0)
        return;

    if (column
        && (MAX_COLUMNS
        - 1 /* spacer */
        - 2 /* ` \'   */
        < column + len)) {
        fprintf (f, " \\\n ");
        column = 0;
        if (spacer == ' ')
        spacer = '\0';
    }

    if (spacer == ' ') {
        putc (spacer, f);
        ++column;
    }

    quote_string_for_make (f, string);
    column += len;

    if (spacer == ':') {
        putc (spacer, f);
        ++column;
    }
}
void
print_dependencies (void)
{
    FILE *f;
    struct dependency *dep;

    if (dep_file == NULL)
        return;

    f = fopen (dep_file, FOPEN_WT);
    if (f == NULL) {
        as_warn (_("can't open `%s' for writing"), dep_file);
        return;
    }

    column = 0;
    wrap_output (f, out_file_name, ':');
    for (dep = dep_chain; dep != NULL; dep = dep->next)
        wrap_output (f, dep->file, ' ');

    putc ('\n', f);

    if (fclose (f))
        as_warn (_("can't close `%s'"), dep_file);
}
