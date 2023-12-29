/*
 * @Author: hxy
 * @Date: 2023-10-02 20:36:30
 * @LastEditTime: 2023-11-14 16:10:55
 * @Description: 请填写简介
 */

//
// Created by jjk on 2023/9/26.
//
#ifndef TC_ARM_H
#define TC_ARM_H

#include "as.h"
#include "obj-elf.h"
#define TC_ARM 1

extern const char EXP_CHARS[];
extern const char FLT_CHARS[];
#ifndef TARGET_BYTES_BIG_ENDIAN
#define TARGET_BYTES_BIG_ENDIAN 0
#endif

# define ARM_BI_ENDIAN
# define TARGET_FORMAT	elf32_arm_target_format ()


#define TARGET_ARCH 	bfd_arch_arm
#define TC_SYMFIELD_TYPE 	unsigned int

#define WORKING_DOT_WORD

#define TARGET_ARCH 	bfd_arch_arm

#define TC_START_LABEL(STR, NUL_CHAR, NEXT_CHAR)			\
  (NEXT_CHAR == ':' || (NEXT_CHAR == '/' && arm_data_in_code ()))
  
extern unsigned int arm_frag_max_var (struct frag *);
#define md_frag_max_var arm_frag_max_var

#define tc_symbol_chars arm_symbol_chars
extern const char arm_symbol_chars[];

#define tc_comment_chars arm_comment_chars
extern char arm_comment_chars[];

#define tc_line_separator_chars arm_line_separator_chars
#define tc_canonicalize_symbol_name(str) arm_canonicalize_symbol_name (str);

#define md_start_line_hook() arm_start_line_hook ()

/* 有条件地支持不带冒号的label  */
#define LABELS_WITHOUT_COLONS codecomposer_syntax

/* 使用双斜杠行注释 */
#define DOUBLESLASH_LINE_COMMENTS

#define OPTIONAL_REGISTER_PREFIX '%'

# define md_cons_align(nbytes)		mapping_state (MAP_DATA)

extern bool codecomposer_syntax;
// 划开不同行
extern char arm_line_separator_chars[];
#include "obstack.h"

void md_show_usage (FILE * fp);
int  md_parse_option (int c ,const char *arg);
extern char * arm_canonicalize_symbol_name (char *);
#define LOCAL_LABELS_FB    1

enum mstate
{
    MAP_UNDEFINED = 0, /* Must be zero, for seginfo in new sections.  */
    MAP_DATA,
    MAP_ARM,
    MAP_THUMB
};
struct arm_frag_type
{
  int thumb_mode;
#ifdef OBJ_ELF
  /* If there is a mapping symbol at offset 0 in this frag,
     it will be saved in FIRST_MAP.  If there are any mapping
     symbols in this frag, the last one will be saved in
     LAST_MAP.  */
  symbolS *first_map, *last_map;
#endif
};

/* State variables for predication block handling.  */
enum pred_state
{
    OUTSIDE_PRED_BLOCK, MANUAL_PRED_BLOCK, AUTOMATIC_PRED_BLOCK
};
enum pred_type {
    SCALAR_PRED, VECTOR_PRED
};
struct current_pred
{
    int mask;
    enum pred_state state;
    int cc;
    int block_length;
    char *insn;
    int state_handled;
    int warn_deprecated;
    int insn_cond;
    enum pred_type type;
};
# define GLOBAL_OFFSET_TABLE_NAME	"_GLOBAL_OFFSET_TABLE_"
struct arm_segment_info_type
{
    enum mstate mapstate;

    /* Bit N indicates that an R_ARM_NONE relocation has been output for
        __aeabi_unwind_cpp_prN already if set. This enables dependencies to be
        emitted only once per section, to save unnecessary bloat.  */
    unsigned int marked_pr_dependency;

    struct current_pred current_pred;
};

#define TC_FRAG_TYPE		struct arm_frag_type
#define TC_FRAG_INIT(fragp, max_bytes) arm_init_frag (fragp, max_bytes)
# define TC_SEGMENT_INFO_TYPE 		struct arm_segment_info_type
#define HANDLE_ALIGN(fragp)	arm_handle_align (fragp)

#define TC_FIX_TYPE int
#define TC_INIT_FIX_DATA(FIX) ((FIX)->tc_fix_data = 0)

#define ARM_GET_FLAG(s)   	(*symbol_get_tc (s))
#define ARM_SET_FLAG(s,v) 	(*symbol_get_tc (s) |= (v))
#define ARM_RESET_FLAG(s,v) 	(*symbol_get_tc (s) &= ~(v))

#define ARM_FLAG_THUMB 		(1 << 0)	/* The symbol is a Thumb symbol rather than an Arm symbol.  */
#define ARM_FLAG_INTERWORK 	(1 << 1)	/* The symbol is attached to code that supports interworking.  */
#define THUMB_FLAG_FUNC		(1 << 2)	/* The symbol is attached to the start of a Thumb function.  */

#define md_cleanup() arm_cleanup ()

#define md_finish arm_md_finish
extern void arm_md_finish (void);

#define md_post_relax_hook		arm_md_post_relax ()
extern void arm_md_post_relax (void);

#define DIFF_EXPR_OK

#ifdef OBJ_ELF
/* Values passed to md_apply_fix don't include the symbol value.  */
# define MD_APPLY_SYM_VALUE(FIX) 		arm_apply_sym_value (FIX, this_segment)
#endif

bool arm_is_eabi (void);
static inline int
arm_min (int am_p1, int am_p2) {
    return am_p1 < am_p2 ? am_p1 : am_p2;
}

#define ARM_IS_THUMB(s)		(ARM_GET_FLAG (s) & ARM_FLAG_THUMB)

#define SUB_SEGMENT_ALIGN(SEG, FRCHAIN)				\
  ((!(FRCHAIN)->frch_next && subseg_text_p (SEG))		\
   ? arm_min (2, get_recorded_alignment (SEG)) : 0)

#define THUMB_IS_FUNC(s)					\
  ((s) != NULL							\
   && ((arm_is_eabi ()						\
	&& (ARM_IS_THUMB (s))					\
	&& (symbol_get_bfdsym (s)->flags & BSF_FUNCTION))	\
       || (ARM_GET_FLAG (s) & THUMB_FLAG_FUNC)))

#define TC_FORCE_RELOCATION_SUB_SAME(FIX, SEG)	\
  (GENERIC_FORCE_RELOCATION_SUB_SAME (FIX, SEG)	\
   || THUMB_IS_FUNC ((FIX)->fx_addsy))


#define TC_FORCE_RELOCATION_LOCAL(FIX)			\
  (GENERIC_FORCE_RELOCATION_LOCAL (FIX)			\
   || (FIX)->fx_r_type == BFD_RELOC_ARM_GOT32		\
   || (FIX)->fx_r_type == BFD_RELOC_32			\
   || ((FIX)->fx_addsy != NULL				\
       && S_IS_WEAK ((FIX)->fx_addsy)))

#define TC_FORCE_RELOCATION_ABS(FIX)			\
  (((FIX)->fx_pcrel					\
    && (FIX)->fx_r_type != BFD_RELOC_32			\
    && (FIX)->fx_r_type != BFD_RELOC_ARM_GOT32)		\
   || TC_FORCE_RELOCATION(FIX))

#define md_do_align(N, FILL, LEN, MAX, LABEL)					\
  if (FILL == NULL && (N) != 0 && ! need_pass_2 && subseg_text_p (now_seg))	\
    {										\
      arm_frag_align_code (N, MAX);						\
      goto LABEL;								\
    }
    
#define TC_ALIGN_ZERO_IS_DEFAULT 1
# define EXTERN_FORCE_RELOC 			1
#define MD_PCREL_FROM_SECTION(F,S) md_pcrel_from_section(F,S)

#define TC_FORCE_RELOCATION(FIX) arm_force_relocation (FIX)
# define obj_frob_symbol(sym, punt)	armelf_frob_symbol ((sym), & (punt))
#define obj_adjust_symtab() 		 arm_adjust_symtab ()
# define md_elf_section_change_hook()	arm_elf_change_section ()

#define tc_frob_label(S) arm_frob_label (S)

#define TC_CONS_FIX_NEW cons_fix_new_arm

const char *md_atof (int type, char * litP, int * sizeP);
extern void arm_start_line_hook(void);
extern void md_number_to_chars (char * buf, valueT val, int n);
extern bool arm_data_in_code (void);
void mapping_state (enum mstate);
extern const char * elf32_arm_target_format (void);
extern void arm_cleanup (void);
extern void arm_handle_align (struct frag *);
extern int arm_apply_sym_value (struct fix *, segT);
extern int arm_force_relocation (struct fix *);
extern void armelf_frob_symbol (symbolS *, int *);
extern void arm_adjust_symtab (void);
extern void md_begin(void);
extern symbolS *md_undefined_symbol (char * name ATTRIBUTE_UNUSED);
extern void md_assemble (char *str);
extern void arm_frag_align_code (int, int);
extern void cons_fix_new_arm (fragS *, int, int, expressionS *,
			      bfd_reloc_code_real_type);
extern void arm_elf_change_section (void);
extern void arm_frob_label (symbolS *);
#endif //BINUTILS_TC_ARM_H