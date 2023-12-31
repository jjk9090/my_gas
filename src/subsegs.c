/*
 * @Author: hxy
 * @Date: 2023-10-04 23:02:37
 * @LastEditTime: 2023-11-09 11:59:03
 * @Description: 请填写简介
 */
#include "as.h"

#include "subsegs.h"
#include "obstack.h"
static struct obstack frchains;
frchainS *frchain_now;
static fragS dummy_frag;
void
subsegs_begin (void) {
    // 初始化frchain
    obstack_begin (&frchains, chunksize);
#if __GNUC__ >= 2
    // 对齐方式
    obstack_alignment_mask (&frchains) = __alignof__ (frchainS) - 1;
#endif
    // 当前frchain
    frchain_now = NULL;		/* Warn new_subseg() that we are booting.  */
    frag_now = &dummy_frag;
}

static void
alloc_seginfo (segT seg) {
    segment_info_type *seginfo;

    seginfo = obstack_alloc (&notes, sizeof (*seginfo));
    memset (seginfo, 0, sizeof (*seginfo));
    seginfo->bfd_section = seg;
    bfd_set_section_userdata (seg, seginfo);
}

/**
 * @Description: 是否强制创建新的节
 */
segT
subseg_get (const char *segname, int force_new) {
    segT secptr;
    // 当前段的名字
    const char *now_seg_name = now_seg ? bfd_section_name (now_seg) : 0;
    if (!force_new && now_seg_name && (now_seg_name == segname || !strcmp (now_seg_name, segname))) {
        return now_seg;
    }
    if (!force_new)
        secptr = bfd_make_section_old_way (stdoutput, segname);
    else
        secptr = bfd_make_section_anyway (stdoutput, segname);
    if (!seg_info (secptr)) {
        secptr->output_section = secptr;
        alloc_seginfo(secptr);
    }
    return secptr;
}

/**
 * @Description: 更改所在的子段（subsegment）
 * 而不创建新的 FRAG
 */
void
subseg_change (segT seg, int subseg)
{
  now_seg = seg;
  now_subseg = subseg;

  if (!seg_info (seg))
    alloc_seginfo (seg);
}

/**
 * @Description: 设置子段，初始化frchain_now 和 frag_now
 * 设置当前正在处理的节和子节，并创建和插入子节段
 */
static void
subseg_set_rest (segT seg, subsegT subseg) {
    frchainS *frcP;		/* crawl frchain chain */
    frchainS **lastPP;		/* address of last pointer */
    frchainS *newP;		/* address of new frchain */
    segment_info_type *seginfo;

    if (frag_now && frchain_now)
        frchain_now->frch_frag_now = frag_now;
    gas_assert (frchain_now == 0
	  || frchain_now->frch_last == frag_now);

    subseg_change (seg, (int) subseg);

    seginfo = seg_info (seg);
    // 应该保留section符号
    if (bfd_keep_unused_section_symbols (stdoutput))
        seg->symbol->flags |= BSF_SECTION_SYM_USED;
    for (frcP = *(lastPP = &seginfo->frchainP);
         frcP != NULL;
         frcP = *(lastPP = &frcP->frch_next)) {
        if (frcP->frch_subseg >= subseg)
            break;
    }
    
    // 创建一个新的 frchainS 结构体，
    // 并将其插入到与指定子段对应的 frchainP 链表中的正确位置
    if (frcP == NULL || frcP->frch_subseg != subseg) {
        newP = (frchainS *) obstack_alloc (&frchains, sizeof (frchainS));
        newP->frch_subseg = subseg;
        newP->fix_root = NULL;
        newP->fix_tail = NULL;
        // 初始化堆栈
        obstack_begin (&newP->frch_obstack, chunksize);
    #if __GNUC__ >= 2
        obstack_alignment_mask (&newP->frch_obstack) = __alignof__ (fragS) - 1;
    #endif
        newP->frch_frag_now = frag_alloc (&newP->frch_obstack);
        newP->frch_frag_now->fr_type = rs_fill;
        newP->frch_cfi_data = NULL;

        newP->frch_root = newP->frch_last = newP->frch_frag_now;
        *lastPP = newP;
        newP->frch_next = frcP;
        frcP = newP;
    }
    frchain_now = frcP;
    frag_now = frcP->frch_frag_now;
    gas_assert (frchain_now->frch_last == frag_now);
}

segT
subseg_new (const char *segname, subsegT subseg) {
    segT secptr;
    secptr = subseg_get (segname, 0);
    subseg_set_rest (secptr, subseg);
    return secptr;
}

void
subseg_set (segT secptr, subsegT subseg)
{
    if (! (secptr == now_seg && subseg == now_subseg))
        subseg_set_rest (secptr, subseg);
    mri_common_symbol = NULL;
}


/**
 * @Description: 创建节符号
 */
symbolS *
section_symbol (segT sec)
{
    segment_info_type *seginfo = seg_info (sec);
    symbolS *s;

    if (seginfo == 0)
        abort ();
    if (seginfo->sym)
        return seginfo->sym;

#ifndef EMIT_SECTION_SYMBOLS
#define EMIT_SECTION_SYMBOLS 1
#endif

    if (! EMIT_SECTION_SYMBOLS || symbol_table_frozen) {
        /* Here we know it won't be going into the symbol table.  */
        s = symbol_create (sec->symbol->name, sec, &zero_address_frag, 0);
    }
    else {
        segT seg;
        s = symbol_find (sec->symbol->name);
        /* We have to make sure it is the right symbol when we
        have multiple sections with the same section name.  */
        if (s == NULL
        || ((seg = S_GET_SEGMENT (s)) != sec
            && seg != undefined_section))
            s = symbol_new (sec->symbol->name, sec, &zero_address_frag, 0);
        else if (seg == undefined_section) {
            S_SET_SEGMENT (s, sec);
            symbol_set_frag (s, &zero_address_frag);
        }
    }
    
    S_CLEAR_EXTERNAL (s);

    /* Use the BFD section symbol, if possible.  */
    if (obj_sec_sym_ok_for_reloc (sec))
        symbol_set_bfdsym (s, sec->symbol);
    else
        symbol_get_bfdsym (s)->flags |= BSF_SECTION_SYM;

    seginfo->sym = s;
    return s;
}

int
subseg_text_p (segT sec) {
    return (bfd_section_flags (sec) & SEC_CODE) != 0;
}

/**
 * @Description: 用于创建一个新的段（section）对象。
 * 与 subseg_new 函数不同的是，即使已经存在一个同名的段对象，
 * subseg_force_new 仍然会创建一个新的段对象
 */
segT
subseg_force_new (const char *segname, subsegT subseg) {
    segT secptr;

    secptr = subseg_get (segname, 1);
    subseg_set_rest (secptr, subseg);
    return secptr;
}

void
subsegs_end (struct obstack **obs) {
    for (; *obs; obs++)
        _obstack_free (*obs, NULL);
    _obstack_free (&frchains, NULL);
    bfd_set_section_userdata (bfd_abs_section_ptr, NULL);
    bfd_set_section_userdata (bfd_und_section_ptr, NULL);
}