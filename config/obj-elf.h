/*
 * @Author: hxy
 * @Date: 2023-10-06 11:14:08
 * @LastEditTime: 2023-11-14 16:53:18
 * @Description: 请填写简介
 */
#ifndef _OBJ_ELF_H
#define _OBJ_ELF_H
#define OBJ_ELF 1
#include "elf-bfd.h"
enum elf_visibility
{
  visibility_unchanged = 0,
  visibility_local,
  visibility_hidden,
  visibility_remove
};
struct elf_versioned_name_list
{
  char *name;
  struct elf_versioned_name_list *next;
};

struct elf_obj_sy
{
  /* Whether the symbol has been marked as local.  */
  unsigned int local : 1;

  /* Whether the symbol has been marked for rename with @@@.  */
  unsigned int rename : 1;

  /* Whether the symbol has a bad version name.  */
  unsigned int bad_version : 1;

  /* Whether visibility of the symbol should be changed.  */
  ENUM_BITFIELD (elf_visibility) visibility : 2;

  /* Use this to keep track of .size expressions that involve
     differences that we can't compute yet.  */
  expressionS *size;

  /* The list of names specified by the .symver directive.  */
  struct elf_versioned_name_list *versioned_name;

#ifdef ECOFF_DEBUGGING
  /* If we are generating ECOFF debugging information, we need some
     additional fields for each symbol.  */
  struct efdr *ecoff_file;
  struct localsym *ecoff_symbol;
  valueT ecoff_extern_size;
#endif
};
/*匹配节的新西 */
struct elf_section_match
{
  const char *   group_name;
  const char *   linked_to_symbol_name;   /*存储链接到的符号的名称*/
  unsigned int   section_id;  /*表示节的标识符*/
  unsigned int   sh_info;		/*节的信息字段  */
  bfd_vma        sh_flags;		/* 节的标志字段  */
  flagword       flags;
};
#define OBJ_SYMFIELD_TYPE struct elf_obj_sy
//与目标文件相关的伪指令表
extern void elf_pop_insert (void);
#ifndef obj_pop_insert
#define obj_pop_insert()	elf_pop_insert()
#endif

void elf_obj_symbol_new_hook (symbolS *);
#ifndef obj_symbol_new_hook
#define obj_symbol_new_hook	elf_obj_symbol_new_hook
#endif

#ifndef obj_begin
#define obj_begin() elf_begin ()
#endif

#ifndef obj_sec_sym_ok_for_reloc
#define obj_sec_sym_ok_for_reloc(SEC)	((SEC)->owner != 0)
#endif

#ifndef obj_frob_file_before_adjust
#define obj_frob_file_before_adjust  elf_frob_file_before_adjust
#endif

#ifndef obj_frob_file
#define obj_frob_file  elf_frob_file
#endif
extern void elf_frob_file (void);

/* should be conditional on address size! */
#define elf_symbol(asymbol) ((elf_symbol_type *) (&(asymbol)->the_bfd))

#ifndef S_SET_ALIGN
#define S_SET_ALIGN(S,V) \
  (elf_symbol (symbol_get_bfdsym (S))->internal_elf_sym.st_value = (V))
#endif

extern void elf_fixup_removed_symbol (symbolS **);
#ifndef obj_fixup_removed_symbol
#define obj_fixup_removed_symbol(sympp) elf_fixup_removed_symbol (sympp)
#endif

#ifndef obj_frob_file_after_relocs
#define obj_frob_file_after_relocs  elf_frob_file_after_relocs
#endif
extern void elf_frob_file_after_relocs (void);

/* should be conditional on address size! */
#define elf_symbol(asymbol) ((elf_symbol_type *) (&(asymbol)->the_bfd))

extern void elf_frob_file_before_adjust (void);

extern void elf_begin (void);

extern const pseudo_typeS md_pseudo_table[];
// 解析.common 伪指令
extern symbolS * elf_common_parse (int ignore ATTRIBUTE_UNUSED, symbolS *symbolP,
				   addressT size);
extern void elf_frob_symbol (symbolS *, int *);
extern void obj_elf_common (int);
#endif
