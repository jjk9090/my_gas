/*
 * @Author: hxy
 * @Date: 2023-10-02 21:12:15
 * @LastEditTime: 2023-11-09 11:55:46
 * @Description: 请填写简介
 */

#include "as.h"
#include "subsegs.h"
#include "sb.h"
#include "macro.h"
#include "output-file.h"
#ifndef TARGET_MACH
#define TARGET_MACH 0
#endif
bfd *stdoutput;
void
output_file_create (const char *name) {
    // 创建elf文件
    if (name[0] == '-' && name[1] == '\0') {
        as_fatal (_("can't open a bfd on stdout %s"), name);
    } else if (!(stdoutput = bfd_openw (name, TARGET_FORMAT))) {
        bfd_error_type err = bfd_get_error ();

        if (err == bfd_error_invalid_target)
            as_fatal (_("selected target format '%s' unknown"), TARGET_FORMAT);
        else
            as_fatal (_("can't create %s: %s"), name, bfd_errmsg (err));
    }
    bfd_set_format (stdoutput, bfd_object);
    bfd_set_arch_mach (stdoutput, TARGET_ARCH, TARGET_MACH);
    if (flag_traditional_format)
        stdoutput->flags |= BFD_TRADITIONAL_FORMAT;
}

static void
stash_frchain_obs (asection *sec)
{
    segment_info_type *info = seg_info (sec);
    if (info) {
        struct frchain *frchp;
        for (frchp = info->frchainP; frchp; frchp = frchp->frch_next)
            obstack_ptr_grow (&notes, &frchp->frch_obstack);
        info->frchainP = NULL;
    }
}

/**
 * @Description: 退出程序前执行的函数
 */
void
output_file_close (void) {
    bool res;
    bfd *obfd = stdoutput;
    struct obstack **obs;
    asection *sec;
    const char *filename;

    if (obfd == NULL)
        return;

    /* Prevent an infinite loop - if the close failed we will call as_fatal
        which will call xexit() which may call this function again...  */
    stdoutput = NULL;

    /* We can't free obstacks attached to the output bfd sections before
        closing the output bfd since data in those obstacks may need to
        be accessed, but we can't access anything in the output bfd after
        it is closed..  */
    for (sec = obfd->sections; sec; sec = sec->next)
        stash_frchain_obs (sec);
    stash_frchain_obs (reg_section);
    stash_frchain_obs (expr_section);
    stash_frchain_obs (bfd_abs_section_ptr);
    stash_frchain_obs (bfd_und_section_ptr);
    obstack_ptr_grow (&notes, NULL);
    obs = obstack_finish (&notes);

    /* Close the bfd.  */
    if (!flag_always_generate_output && had_errors ())
        res = bfd_close_all_done (obfd);
    else
        res = bfd_close (obfd);
    now_seg = NULL;
    now_subseg = 0;

    filename = out_file_name;
    out_file_name = NULL;
    if (!keep_it && filename)
        unlink_if_ordinary (filename);

#ifdef md_end
    md_end ();
#endif
#ifdef obj_end
    obj_end ();
#endif
    macro_end ();
    expr_end ();
    read_end ();
    symbol_end ();
    subsegs_end (obs);

    if (!res)
        as_fatal ("%s: %s", filename, bfd_errmsg (bfd_get_error ()));
}
