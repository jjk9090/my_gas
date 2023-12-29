/*
 * @Author: hxy
 * @Date: 2023-10-09 19:38:40
 * @LastEditTime: 2023-10-28 19:29:05
 * @Description: 请填写简介
 */
#include "as.h"
#include "obstack.h"
struct obstack cond_obstack;
struct file_line
{
  const char *file;
  unsigned int line;
};

/* We push one of these structures for each .if, and pop it at the
   .endif.  */

struct conditional_frame
{
  /* The source file & line number of the "if".  */
  struct file_line if_file_line;
  /* The source file & line of the "else".  */
  struct file_line else_file_line;
  /* The previous conditional.  */
  struct conditional_frame *previous_cframe;
  /* Have we seen an else yet?  */
  int else_seen;
  /* Whether we are currently ignoring input.  */
  int ignoring;
  /* Whether a conditional at a higher level is ignoring input.
     Set also when a branch of an "if .. elseif .." tree has matched
     to prevent further matches.  */
  int dead_tree;
  /* Macro nesting level at which this conditional was created.  */
  int macro_nest;
};

static struct conditional_frame *current_cframe = NULL;

/**
 * @Description: 跳过输入的Input
 */
int
ignore_input (void)
{
    char *s;

    s = input_line_pointer;

    if (NO_PSEUDO_DOT || flag_m68k_mri) {
        if (s[-1] != '.')
            --s;
    }
    else {
        if (s[-1] != '.')
        return (current_cframe != NULL) && (current_cframe->ignoring);
    }

  /* We cannot ignore certain pseudo ops.  */
    switch (s[0]) {
        case 'i': case  'I':
            if (s[1] == 'f' || s[1] == 'F')
                return 0;
            break;
        case 'e': case 'E':
            if (!strncasecmp (s, "else", 4)
            || !strncasecmp (s, "endif", 5)
            || !strncasecmp (s, "endc", 4))
                return 0;
            break;
        case 'l': case 'L':
            if (!strncasecmp (s, "linefile", 8))
                return 0;
                break;
    }

    return (current_cframe != NULL) && (current_cframe->ignoring);
}

void
cond_finish_check (int nest) {
    if (current_cframe != NULL && current_cframe->macro_nest >= nest) {
        if (nest >= 0)
            as_bad (_("end of macro inside conditional"));
        else
            as_bad (_("end of file inside conditional"));

        as_bad_where (current_cframe->if_file_line.file,
                current_cframe->if_file_line.line,
                _("here is the start of the unterminated conditional"));
        if (current_cframe->else_seen)
            as_bad_where (current_cframe->else_file_line.file,
                current_cframe->else_file_line.line,
                _("here is the \"else\" of the unterminated conditional"));
        cond_exit_macro (nest);
    }
}
void
cond_exit_macro (int nest) {
    while (current_cframe != NULL && current_cframe->macro_nest >= nest) {
        struct conditional_frame *hold;

        hold = current_cframe;
        current_cframe = current_cframe->previous_cframe;
        obstack_free (&cond_obstack, hold);
    }
}

