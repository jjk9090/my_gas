/*
 * @Author: hxy
 * @Date: 2023-10-04 23:47:14
 * @LastEditTime: 2023-11-06 17:25:06
 * @Description: 请填写简介
 */
#include "as.h"
#include "subsegs.h"
#include "obstack.h"
static int totalfrags;
extern fragS zero_address_frag;

#ifndef NOP_OPCODE
#define NOP_OPCODE 0x00
#endif

#define MAX_MEM_ALIGNMENT_BYTES    6
#define MAX_MEM_FOR_RS_ALIGN_CODE ((1 << MAX_MEM_ALIGNMENT_BYTES) - 1)

void frag_init () {
    zero_address_frag.fr_type = rs_fill;
    predefined_address_frag.fr_type = rs_fill;
    subsegs_begin ();
}

/**
 * @Description: 初始化frag
 */
fragS *
frag_alloc (struct obstack *ob) {
    fragS *ptr;
    int oalign;   
    (void) obstack_alloc (ob, 0); 
    // 对齐
    oalign = obstack_alignment_mask (ob);
    obstack_alignment_mask (ob) = 0;
    ptr = (fragS *) obstack_alloc (ob, SIZEOF_STRUCT_FRAG);
    obstack_alignment_mask (ob) = oalign;
    memset (ptr, 0, SIZEOF_STRUCT_FRAG);
    totalfrags++;
    return ptr;
}

static void
frag_alloc_check (const struct obstack *ob) {
    if (ob->chunk_size == 0) {
        as_bad (_("attempt to allocate data in absolute section"));
        subseg_set (text_section, 0);
    }
}

void
frag_wane (fragS *fragP)
{
  fragP->fr_type = rs_fill;
  fragP->fr_offset = 0;
  fragP->fr_var = 0;
}

/**
 * @Description: 从当前代码片段链表的内存池 获取一段连虚的内存空间
 * @param {size_t} nchars
 */
char *frag_more(size_t nchars) {
    char *retval;
    frag_alloc_check(&frchain_now->frch_obstack);
    frag_grow(nchars);
    retval = obstack_next_free (&frchain_now->frch_obstack);
    obstack_blank_fast (&frchain_now->frch_obstack, nchars);
    return retval;   
}

/**
 * @Description: 返回当前片段（fragment）在输出文件中的字节数
 */
addressT
frag_now_fix_octets (void) {
    // 绝对段返回绝对偏移量
    if (now_seg == absolute_section)
        return abs_section_offset;

    // 计算当前片段的起始位置
    // 和当前可用内存空间的差值来计算当前片段在输出文件中的字节数
    return ((char *) obstack_next_free (&frchain_now->frch_obstack)
        - frag_now->fr_literal);
}
/**
 * @Description: 调用此函数关闭一个已完成的片段，
 * 并启动一个新的(空的)碎片，和旧碎片在同一子段。
 * frchain_now保持不变，但frag_now更新。
 */
void
frag_new (size_t old_frags_var_max_size
	  /* Number of chars (already allocated on obstack frags) in
	     variable_length part of frag.  */) {
    fragS *former_last_fragP;
    frchainS *frchP;

    gas_assert (frchain_now->frch_last == frag_now);

    /* Fix up old frag's fr_fix.  */
    frag_now->fr_fix = frag_now_fix_octets ();
    gas_assert (frag_now->fr_fix >= old_frags_var_max_size
            || now_seg == absolute_section);
    frag_now->fr_fix -= old_frags_var_max_size;
    /* Make sure its type is valid.  */
    gas_assert (frag_now->fr_type != 0);

    /* This will align the obstack so the next struct we allocate on it
        will begin at a correct boundary.  */
    obstack_finish (&frchain_now->frch_obstack);
    frchP = frchain_now;
    know (frchP);
    former_last_fragP = frchP->frch_last;
    gas_assert (former_last_fragP != 0);
    gas_assert (former_last_fragP == frag_now);
    frag_now = frag_alloc (&frchP->frch_obstack);

    frag_now->fr_file = as_where (&frag_now->fr_line);

    /* Generally, frag_now->points to an address rounded up to next
        alignment.  However, characters will add to obstack frags
        IMMEDIATELY after the struct frag, even if they are not starting
        at an alignment address.  */
    former_last_fragP->fr_next = frag_now;
    frchP->frch_last = frag_now;

    #ifndef NO_LISTING
    {
        // extern struct list_info_struct *listing_tail;
        // frag_now->line = listing_tail;
    }
    #endif

    gas_assert (frchain_now->frch_last == frag_now);

    frag_now->fr_next = NULL;
}
/**
 * @Description: 尝试增加当前片段的空间
 */
void
frag_grow (size_t nchars) {
    // 当前代码片段没有足够的空间来容纳指定数量的字符
    if (obstack_room (&frchain_now->frch_obstack) < nchars) {
        size_t oldc;
        size_t newc;

        /* Try to allocate a bit more than needed right now.  But don't do
            this if we would waste too much memory.  Especially necessary
            for extremely big (like 2GB initialized) frags.  */
        if (nchars < 0x10000)
            newc = 2 * nchars;
        else
            newc = nchars + 0x10000;
        newc += SIZEOF_STRUCT_FRAG;

        /* Check for possible overflow.  */
        if (newc < nchars)
        as_fatal (ngettext ("can't extend frag %lu char",
                    "can't extend frag %lu chars",
                    (unsigned long) nchars),
            (unsigned long) nchars);

        /* Force to allocate at least NEWC bytes, but not less than the
            default.  */
        oldc = obstack_chunk_size (&frchain_now->frch_obstack);
        if (newc > oldc)
            obstack_chunk_size (&frchain_now->frch_obstack) = newc;

        while (obstack_room (&frchain_now->frch_obstack) < nchars) {
            /* Not enough room in this frag.  Close it and start a new one.
                This must be done in a loop because the created frag may not
                be big enough if the current obstack chunk is used.  */
            frag_wane (frag_now);
            frag_new (0);
        }

        /* Restore the old chunk size.  */
        obstack_chunk_size (&frchain_now->frch_obstack) = oldc;
    }
}

/* Close the current frag, setting its fields for a relaxable frag.  Start a
   new frag.  */

static void
frag_var_init (relax_stateT type, size_t max_chars, size_t var,
	       relax_substateT subtype, symbolS *symbol, offsetT offset,
               char *opcode) {
    frag_now->fr_var = var;
    frag_now->fr_type = type;
    frag_now->fr_subtype = subtype;
    frag_now->fr_symbol = symbol;
    frag_now->fr_offset = offset;
    frag_now->fr_opcode = opcode;
#ifdef USING_CGEN
    frag_now->fr_cgen.insn = 0;
    frag_now->fr_cgen.opindex = 0;
    frag_now->fr_cgen.opinfo = 0;
#endif
#ifdef TC_FRAG_INIT
    TC_FRAG_INIT (frag_now, max_chars);
#endif
    frag_now->fr_file = as_where (&frag_now->fr_line);

    frag_new (max_chars);
}
/**
 * @Description: 创建一个可伸缩的数据片段
 */
char *
frag_var (relax_stateT type, size_t max_chars, size_t var,
	  relax_substateT subtype, symbolS *symbol, offsetT offset,
	  char *opcode) {
    char *retval;

    frag_grow (max_chars);
    retval = obstack_next_free (&frchain_now->frch_obstack);
    obstack_blank_fast (&frchain_now->frch_obstack, max_chars);
    frag_var_init (type, max_chars, var, subtype, symbol, offset, opcode);
    return retval;
}

/**
 * @Description: 创建一个对齐的数据片段
 */
void
frag_align (int alignment, int fill_character, int max)
{ 
    if (now_seg == absolute_section) {
        addressT new_off;
        addressT mask;

        mask = (~(addressT) 0) << alignment;
        new_off = (abs_section_offset + ~mask) & mask;
        if (max == 0 || new_off - abs_section_offset <= (addressT) max)
            // 进行适当的调整来对齐
            abs_section_offset = new_off;
    } else {
        char *p;
        // 创建一个数据片段
        p = frag_var (rs_align, 1, 1, (relax_substateT) max,
                (symbolS *) 0, (offsetT) alignment, (char *) 0);
        // 最后将第一个字节设置为填充字符
        *p = fill_character;
    }
}

addressT
frag_now_fix (void)
{
    /* Symbols whose section has SEC_ELF_OCTETS set,
     resolve to octets instead of target bytes.  */
    if (now_seg->flags & SEC_OCTETS)
        return frag_now_fix_octets ();
    else
        return frag_now_fix_octets () / OCTETS_PER_BYTE;
}


bool
frag_offset_fixed_p (const fragS *frag1, const fragS *frag2, offsetT *offset)
{
  const fragS *frag;
  offsetT off;

  /* Start with offset initialised to difference between the two frags.
     Prior to assigning frag addresses this will be zero.  */
  off = frag1->fr_address - frag2->fr_address;
  if (frag1 == frag2)
    {
      *offset = off;
      return true;
    }

  /* Maybe frag2 is after frag1.  */
  frag = frag1;
  while (frag->fr_type == rs_fill)
    {
      off += frag->fr_fix + frag->fr_offset * frag->fr_var;
      frag = frag->fr_next;
      if (frag == NULL)
	break;
      if (frag == frag2)
	{
	  *offset = off;
	  return true;
	}
    }

  /* Maybe frag1 is after frag2.  */
  off = frag1->fr_address - frag2->fr_address;
  frag = frag2;
  while (frag->fr_type == rs_fill)
    {
      off -= frag->fr_fix + frag->fr_offset * frag->fr_var;
      frag = frag->fr_next;
      if (frag == NULL)
	break;
      if (frag == frag1)
	{
	  *offset = off;
	  return true;
	}
    }

  return false;
}

bool
frag_gtoffset_p (valueT off2, const fragS *frag2,
		 valueT off1, const fragS *frag1, offsetT *offset)
{
  /* Insanity check.  */
  if (frag2 == frag1 || off1 > frag1->fr_fix)
    return false;

  /* If the first symbol offset is at the end of the first frag and
     the second symbol offset at the beginning of the second frag then
     it is possible they are at the same address.  Go looking for a
     non-zero fr_fix in any frag between these frags.  If found then
     we can say the O_gt result will be true.  If no such frag is
     found we assume that frag1 or any of the following frags might
     have a variable tail and thus the answer is unknown.  This isn't
     strictly true; some frags don't have a variable tail, but it
     doesn't seem worth optimizing for those cases.  */
  const fragS *frag = frag1;
  offsetT delta = off2 - off1;
  for (;;)
    {
      delta += frag->fr_fix;
      frag = frag->fr_next;
      if (frag == frag2)
	{
	  if (delta == 0)
	    return false;
	  break;
	}
      /* If we run off the end of the frag chain then we have a case
	 where frag2 is not after frag1, ie. an O_gt expression not
	 created for .loc view.  */
      if (frag == NULL)
	return false;
    }

  *offset = (off2 - off1 - delta) * OCTETS_PER_BYTE;
  return true;
}

/**
 * @Description: 插入nop指令对齐，数量和alignment的值一样
 */
void
frag_align_code (int alignment, int max) {
    char *p;

    p = frag_var (rs_align_code, MAX_MEM_FOR_RS_ALIGN_CODE, 1,
            (relax_substateT) max, (symbolS *) 0,
            (offsetT) alignment, (char *) 0);
    *p = NOP_OPCODE;
}
