#include "as.h"
#include "elf32-arm.h"
#include "tc-arm.h"
#include "obj-elf.h"
#include "te-linux.h"
#include "safe-ctype.h"
#include <limits.h>
#include <stdarg.h>
#include "subsegs.h"
#include "obstack.h"
#include "libiberty.h"
#include "opcode/arm.h"
#include "cpu-arm.h"
#ifdef OBJ_ELF
#include "elf/arm.h"
#include "dw2gencfi.h"
#endif

const char * md_shortopts = "m:k";
#define OPTION_EB (OPTION_MD_BASE + 0)
#define OPTION_EL (OPTION_MD_BASE + 1)
#define OPTION_FIX_V4BX (OPTION_MD_BASE + 2)
#define OPTION_FDPIC (OPTION_MD_BASE + 3)

#define streq(a, b)	      (strcmp (a, b) == 0)

const char EXP_CHARS[] = "eE";
const char FLT_CHARS[] = "rRsSfFdDxXeEpPHh";
static int pic_code	     = false;
static int uses_apcs_26	     = false;
static int uses_apcs_float   = false;
static int thumb_mode = 0;
const char arm_symbol_chars[] = "#[]{}";
char arm_comment_chars[] = "@";
const char line_comment_chars[] = "#";
extern char arm_line_separator_chars[];

static int label_is_thumb_function_name = false;

static int fix_v4bx	     = false;
bool codecomposer_syntax = false;
#define NUM_FLOAT_VALS 8
const char * fp_const[] =
{
  "0.0", "1.0", "2.0", "3.0", "4.0", "5.0", "0.5", "10.0", 0
};
LITTLENUM_TYPE fp_values[NUM_FLOAT_VALS][MAX_LITTLENUMS];

// 哈希表
static htab_t  arm_ops_hsh;
static htab_t  arm_cond_hsh;
static htab_t  arm_vcond_hsh;
static htab_t  arm_shift_hsh;
static htab_t  arm_psr_hsh;
static htab_t  arm_reg_hsh;
static htab_t arm_reloc_hsh;

#ifdef OBJ_ELF
/* Pre-defined "_GLOBAL_OFFSET_TABLE_"	*/
symbolS * GOT_symbol;
#endif
symbolS *  last_label_seen;
#ifndef FPU_DEFAULT
# ifdef TE_LINUX
#  define FPU_DEFAULT FPU_ARCH_FPA
# endif
#endif
static int arm_fdpic;
// arm体系架构
static const arm_feature_set arm_ext_v1 = ARM_FEATURE_CORE_LOW (ARM_EXT_V1);
static const arm_feature_set arm_ext_v2 = ARM_FEATURE_CORE_LOW (ARM_EXT_V2);
static const arm_feature_set arm_ext_v2s = ARM_FEATURE_CORE_LOW (ARM_EXT_V2S);
static const arm_feature_set arm_ext_v3 = ARM_FEATURE_CORE_LOW (ARM_EXT_V3);
static const arm_feature_set arm_ext_v3m = ARM_FEATURE_CORE_LOW (ARM_EXT_V3M);
static const arm_feature_set arm_ext_v4 = ARM_FEATURE_CORE_LOW (ARM_EXT_V4);
static const arm_feature_set arm_ext_v4t = ARM_FEATURE_CORE_LOW (ARM_EXT_V4T);
static const arm_feature_set arm_ext_v5 = ARM_FEATURE_CORE_LOW (ARM_EXT_V5);
static const arm_feature_set arm_ext_v4t_5 =
  ARM_FEATURE_CORE_LOW (ARM_EXT_V4T | ARM_EXT_V5);
static const arm_feature_set arm_ext_v5t = ARM_FEATURE_CORE_LOW (ARM_EXT_V5T);
static const arm_feature_set arm_ext_v5e = ARM_FEATURE_CORE_LOW (ARM_EXT_V5E);
static const arm_feature_set arm_ext_v5exp = ARM_FEATURE_CORE_LOW (ARM_EXT_V5ExP);
static const arm_feature_set arm_ext_v5j = ARM_FEATURE_CORE_LOW (ARM_EXT_V5J);
static const arm_feature_set arm_ext_v6 = ARM_FEATURE_CORE_LOW (ARM_EXT_V6);
static const arm_feature_set arm_ext_v6k = ARM_FEATURE_CORE_LOW (ARM_EXT_V6K);
static const arm_feature_set arm_ext_v6t2 = ARM_FEATURE_CORE_LOW (ARM_EXT_V6T2);
/* Only for compatability of hint instructions.  */
static const arm_feature_set arm_ext_v6k_v6t2 =
  ARM_FEATURE_CORE_LOW (ARM_EXT_V6K | ARM_EXT_V6T2);
static const arm_feature_set arm_ext_v6_notm =
  ARM_FEATURE_CORE_LOW (ARM_EXT_V6_NOTM);
static const arm_feature_set arm_ext_v6_dsp =
  ARM_FEATURE_CORE_LOW (ARM_EXT_V6_DSP);
static const arm_feature_set arm_ext_barrier =
  ARM_FEATURE_CORE_LOW (ARM_EXT_BARRIER);
static const arm_feature_set arm_ext_msr =
  ARM_FEATURE_CORE_LOW (ARM_EXT_THUMB_MSR);
static const arm_feature_set arm_ext_div = ARM_FEATURE_CORE_LOW (ARM_EXT_DIV);
static const arm_feature_set arm_ext_v7 = ARM_FEATURE_CORE_LOW (ARM_EXT_V7);
static const arm_feature_set arm_ext_v7a = ARM_FEATURE_CORE_LOW (ARM_EXT_V7A);
static const arm_feature_set arm_ext_v7r = ARM_FEATURE_CORE_LOW (ARM_EXT_V7R);
static const arm_feature_set arm_ext_v8r = ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8R);
#ifdef OBJ_ELF
static const arm_feature_set ATTRIBUTE_UNUSED arm_ext_v7m = ARM_FEATURE_CORE_LOW (ARM_EXT_V7M);
#endif
static const arm_feature_set arm_ext_v8 = ARM_FEATURE_CORE_LOW (ARM_EXT_V8);
static const arm_feature_set arm_ext_m =
  ARM_FEATURE_CORE (ARM_EXT_V6M | ARM_EXT_V7M,
		    ARM_EXT2_V8M | ARM_EXT2_V8M_MAIN);
static const arm_feature_set arm_ext_mp = ARM_FEATURE_CORE_LOW (ARM_EXT_MP);
static const arm_feature_set arm_ext_sec = ARM_FEATURE_CORE_LOW (ARM_EXT_SEC);
static const arm_feature_set arm_ext_os = ARM_FEATURE_CORE_LOW (ARM_EXT_OS);
static const arm_feature_set arm_ext_adiv = ARM_FEATURE_CORE_LOW (ARM_EXT_ADIV);
static const arm_feature_set arm_ext_virt = ARM_FEATURE_CORE_LOW (ARM_EXT_VIRT);
static const arm_feature_set arm_ext_pan = ARM_FEATURE_CORE_HIGH (ARM_EXT2_PAN);
static const arm_feature_set arm_ext_v8m = ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8M);
static const arm_feature_set arm_ext_v8m_main =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8M_MAIN);
static const arm_feature_set arm_ext_v8_1m_main =
ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_1M_MAIN);
/* Instructions in ARMv8-M only found in M profile architectures.  */
static const arm_feature_set arm_ext_v8m_m_only =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8M | ARM_EXT2_V8M_MAIN);
static const arm_feature_set arm_ext_v6t2_v8m =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_V6T2_V8M);
/* Instructions shared between ARMv8-A and ARMv8-M.  */
static const arm_feature_set arm_ext_atomics =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_ATOMICS);
#ifdef OBJ_ELF
/* DSP instructions Tag_DSP_extension refers to.  */
static const arm_feature_set arm_ext_dsp =
  ARM_FEATURE_CORE_LOW (ARM_EXT_V5E | ARM_EXT_V5ExP | ARM_EXT_V6_DSP);
#endif
static const arm_feature_set arm_ext_ras =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_RAS);
/* FP16 instructions.  */
static const arm_feature_set arm_ext_fp16 =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_INST);
static const arm_feature_set arm_ext_fp16_fml =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_FP16_FML);
static const arm_feature_set arm_ext_v8_2 =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_2A);
static const arm_feature_set arm_ext_v8_3 =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_V8_3A);
static const arm_feature_set arm_ext_sb =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_SB);
static const arm_feature_set arm_ext_predres =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_PREDRES);
static const arm_feature_set arm_ext_bf16 =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_BF16);
static const arm_feature_set arm_ext_i8mm =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_I8MM);
static const arm_feature_set arm_ext_crc =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_CRC);
static const arm_feature_set arm_ext_cde =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_CDE);
static const arm_feature_set arm_ext_cde0 =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_CDE0);
static const arm_feature_set arm_ext_cde1 =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_CDE1);
static const arm_feature_set arm_ext_cde2 =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_CDE2);
static const arm_feature_set arm_ext_cde3 =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_CDE3);
static const arm_feature_set arm_ext_cde4 =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_CDE4);
static const arm_feature_set arm_ext_cde5 =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_CDE5);
static const arm_feature_set arm_ext_cde6 =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_CDE6);
static const arm_feature_set arm_ext_cde7 =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_CDE7);

static const arm_feature_set arm_arch_any = ARM_ANY;
static const arm_feature_set fpu_any = FPU_ANY;
static const arm_feature_set arm_arch_full ATTRIBUTE_UNUSED = ARM_FEATURE (-1, -1, -1);
static const arm_feature_set arm_arch_t2 = ARM_ARCH_THUMB2;
static const arm_feature_set arm_arch_none = ARM_ARCH_NONE;

static const arm_feature_set arm_cext_iwmmxt2 =
  ARM_FEATURE_COPROC (ARM_CEXT_IWMMXT2);
static const arm_feature_set arm_cext_iwmmxt =
  ARM_FEATURE_COPROC (ARM_CEXT_IWMMXT);
static const arm_feature_set arm_cext_xscale =
  ARM_FEATURE_COPROC (ARM_CEXT_XSCALE);
static const arm_feature_set arm_cext_maverick =
  ARM_FEATURE_COPROC (ARM_CEXT_MAVERICK);
static const arm_feature_set fpu_fpa_ext_v1 =
  ARM_FEATURE_COPROC (FPU_FPA_EXT_V1);
static const arm_feature_set fpu_fpa_ext_v2 =
  ARM_FEATURE_COPROC (FPU_FPA_EXT_V2);
static const arm_feature_set fpu_vfp_ext_v1xd =
  ARM_FEATURE_COPROC (FPU_VFP_EXT_V1xD);
static const arm_feature_set fpu_vfp_ext_v1 =
  ARM_FEATURE_COPROC (FPU_VFP_EXT_V1);
static const arm_feature_set fpu_vfp_ext_v2 =
  ARM_FEATURE_COPROC (FPU_VFP_EXT_V2);
static const arm_feature_set fpu_vfp_ext_v3xd =
  ARM_FEATURE_COPROC (FPU_VFP_EXT_V3xD);
static const arm_feature_set fpu_vfp_ext_v3 =
  ARM_FEATURE_COPROC (FPU_VFP_EXT_V3);
static const arm_feature_set fpu_vfp_ext_d32 =
  ARM_FEATURE_COPROC (FPU_VFP_EXT_D32);
static const arm_feature_set fpu_neon_ext_v1 =
  ARM_FEATURE_COPROC (FPU_NEON_EXT_V1);
static const arm_feature_set fpu_vfp_v3_or_neon_ext =
  ARM_FEATURE_COPROC (FPU_NEON_EXT_V1 | FPU_VFP_EXT_V3);
static const arm_feature_set mve_ext =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE);
static const arm_feature_set mve_fp_ext =
  ARM_FEATURE_CORE_HIGH (ARM_EXT2_MVE_FP);
/* Note: This has more than one bit set, which means using it with
   mark_feature_used (which returns if *any* of the bits are set in the current
   cpu variant) can give surprising results.  */
static const arm_feature_set armv8m_fp =
  ARM_FEATURE_COPROC (FPU_VFP_V5_SP_D16);
#ifdef OBJ_ELF
static const arm_feature_set fpu_vfp_fp16 =
  ARM_FEATURE_COPROC (FPU_VFP_EXT_FP16);
static const arm_feature_set fpu_neon_ext_fma =
  ARM_FEATURE_COPROC (FPU_NEON_EXT_FMA);
#endif
static const arm_feature_set fpu_vfp_ext_fma =
  ARM_FEATURE_COPROC (FPU_VFP_EXT_FMA);
static const arm_feature_set fpu_vfp_ext_armv8 =
  ARM_FEATURE_COPROC (FPU_VFP_EXT_ARMV8);
static const arm_feature_set fpu_vfp_ext_armv8xd =
  ARM_FEATURE_COPROC (FPU_VFP_EXT_ARMV8xD);
static const arm_feature_set fpu_neon_ext_armv8 =
  ARM_FEATURE_COPROC (FPU_NEON_EXT_ARMV8);
static const arm_feature_set fpu_crypto_ext_armv8 =
  ARM_FEATURE_COPROC (FPU_CRYPTO_EXT_ARMV8);
static const arm_feature_set fpu_neon_ext_v8_1 =
  ARM_FEATURE_COPROC (FPU_NEON_EXT_RDMA);
static const arm_feature_set fpu_neon_ext_dotprod =
  ARM_FEATURE_COPROC (FPU_NEON_EXT_DOTPROD);
static const arm_feature_set pacbti_ext =
  ARM_FEATURE_CORE_HIGH_HIGH (ARM_EXT3_PACBTI);

// cpu
static int mfloat_abi_opt = -1;
static arm_feature_set cpu_variant;
static arm_feature_set selected_cpu = ARM_ARCH_NONE;
static arm_feature_set selected_fpu = FPU_NONE;
/* Architecture feature bits selected by the last -mcpu/-march or .cpu/.arch
   directive.  */
static arm_feature_set selected_arch = ARM_ARCH_NONE;
/* Extension feature bits selected by the last -mcpu/-march or .arch_extension
   directive.  */
static arm_feature_set selected_ext = ARM_ARCH_NONE;
/* Feature bits selected by the last .object_arch directive.  */
static arm_feature_set selected_object_arch = ARM_ARCH_NONE;

static const arm_feature_set fpu_default = FPU_DEFAULT;
static const arm_feature_set fpu_any_hard = FPU_ANY_HARD;
static const arm_feature_set fpu_arch_maverick = FPU_ARCH_MAVERICK;
static const arm_feature_set fpu_endian_pure = FPU_ARCH_ENDIAN_PURE;
static arm_feature_set arm_arch_used;
static arm_feature_set thumb_arch_used;
static arm_feature_set *mcpu_ext_opt = NULL;
static const arm_feature_set *march_cpu_opt = NULL;
static arm_feature_set *march_ext_opt = NULL;

static const struct arm_ext_table * selected_ctx_ext_table = NULL;
// 标识是否正在处理新的统一 ARM/Thumb 语法  不过这个汇编器大概旧版就行
static bool unified_syntax = false;

/* Warn on using deprecated features.  */
static int warn_on_deprecated = true;
static int warn_on_restrict_it = false;
/* bfd */
static int support_interwork = false;

static int attributes_set_explicitly[NUM_KNOWN_OBJ_ATTRIBUTES];

#define ARM_GET_FLAG(s)   	(*symbol_get_tc (s))
#define ARM_SET_FLAG(s,v) 	(*symbol_get_tc (s) |= (v))
#define ARM_RESET_FLAG(s,v) 	(*symbol_get_tc (s) &= ~(v))

#define ARM_FLAG_THUMB 		(1 << 0)	/* The symbol is a Thumb symbol rather than an Arm symbol.  */
#define ARM_FLAG_INTERWORK 	(1 << 1)	/* The symbol is attached to code that supports interworking.  */
#define THUMB_FLAG_FUNC		(1 << 2)	/* The symbol is attached to the start of a Thumb function.  */

#define ARM_IS_THUMB(s)		(ARM_GET_FLAG (s) & ARM_FLAG_THUMB)
#define ARM_IS_INTERWORK(s)	(ARM_GET_FLAG (s) & ARM_FLAG_INTERWORK)

#ifdef OBJ_ELF

static int in_pred_block (void);

/* For ELF objects THUMB_IS_FUNC is inferred from
   ARM_IS_THUMB and the function type.  */
#define THUMB_IS_FUNC(s)					\
  ((s) != NULL							\
   && ((arm_is_eabi ()						\
	&& (ARM_IS_THUMB (s))					\
	&& (symbol_get_bfdsym (s)->flags & BSF_FUNCTION))	\
       || (ARM_GET_FLAG (s) & THUMB_FLAG_FUNC)))

#define ARM_IS_FUNC(s)					\
  (((s) != NULL						\
    && arm_is_eabi ()					\
    && !(ARM_IS_THUMB (s))				\
    /* && !(THUMB_FLAG_FUNC & ARM_GET_FLAG (s)) \ */	\
    && (symbol_get_bfdsym (s)->flags & BSF_FUNCTION)))


#endif
#ifdef OBJ_ELF
const char *
elf32_arm_target_format (void)
{
  if (target_big_endian)
    return "elf32-bigarm";
      else
    return "elf32-littlearm";
#endif
}

struct option md_longopts[] =
{
#ifdef OPTION_EB
  {"EB", no_argument, NULL, OPTION_EB},
#endif
#ifdef OPTION_EL
  {"EL", no_argument, NULL, OPTION_EL},
#endif
  {"fix-v4bx", no_argument, NULL, OPTION_FIX_V4BX},
#ifdef OBJ_ELF
  {"fdpic", no_argument, NULL, OPTION_FDPIC},
#endif
  {NULL, no_argument, NULL, 0}
};

// ???
size_t md_longopts_size = sizeof (md_longopts);
// arm架构 option定义一组选项
struct arm_option_table
{
    const char *  option;		/* Option name to match.  */
    const char *  help;		/* Help information.  */
    int *         var;		/* Variable to change.	*/
    int	        value;		/* What to change it to.  */
    const char *  deprecated;	/* If non-null, print this message.  */
};

// arm架构 定义一组长选项（long options）的信息
struct arm_long_option_table
{
    const char *option;			/* Substring to match.	*/
    const char *help;			/* Help information.  */
    bool (*func) (const char *subopt);	/* 解析和处理与长选项关联的子选项.  */
    const char *deprecated;		/* If non-null, print this message.  */
};

struct arm_option_table arm_opts[] =  {
  /* data */
  {"k",	     N_("generate PIC code"),	   &pic_code,	 1, NULL},
  {"mthumb", N_("assemble Thumb code"),	   &thumb_mode,	 1, NULL},
};

struct asm_opcode
{
    /* Basic string to match.  */
    const char * template_name;

    /* Parameters to instruction.	 */
    unsigned int operands[8];

    /* Conditional tag - see opcode_lookup.  */
    unsigned int tag : 4;

    /* Basic instruction code.  */
    unsigned int avalue;

    /* Thumb-format instruction code.  */
    unsigned int tvalue;

    /* Which architecture variant provides this instruction.  */
    const arm_feature_set * avariant;
    const arm_feature_set * tvariant;

    /* Function to call to encode instruction in ARM format.  */
    void (* aencode) (void);

    /* Function to call to encode instruction in Thumb format.  */
    void (* tencode) (void);

    /* Indicates whether this instruction may be vector predicated.  */
    unsigned int mayBeVecPred : 1;
};

enum pred_instruction_type
{
    OUTSIDE_PRED_INSN,
    INSIDE_VPT_INSN,
    INSIDE_IT_INSN,
    INSIDE_IT_LAST_INSN,
    IF_INSIDE_IT_LAST_INSN, /* Either outside or inside;
                    if inside, should be the last one.  */
    NEUTRAL_IT_INSN,        /* This could be either inside or outside,
                    i.e. BKPT and NOP.  */
    IT_INSN,		   /* The IT insn has been parsed.  */
    VPT_INSN,		   /* The VPT/VPST insn has been parsed.  */
    MVE_OUTSIDE_PRED_INSN , /* Instruction to indicate a MVE instruction without
                    a predication code.  */
    MVE_UNPREDICABLE_INSN,  /* MVE instruction that is non-predicable.  */
};

/* 移位操作的类型 */
enum shift_kind
{
    SHIFT_LSL, SHIFT_LSR, SHIFT_ASR, SHIFT_ROR, SHIFT_RRX, SHIFT_UXTW
};

enum lit_type
{
    CONST_THUMB,
    CONST_ARM,
    CONST_VEC
};

enum neon_el_type
{
    NT_invtype,
    NT_untyped,
    NT_integer,
    NT_float,
    NT_poly,
    NT_signed,
    NT_bfloat,
    NT_unsigned
};

enum arm_reg_type
{
  REG_TYPE_RN,
  REG_TYPE_CP,
  REG_TYPE_CN,
  REG_TYPE_FN,
  REG_TYPE_VFS,
  REG_TYPE_VFD,
  REG_TYPE_NQ,
  REG_TYPE_VFSD,
  REG_TYPE_NDQ,
  REG_TYPE_NSD,
  REG_TYPE_NSDQ,
  REG_TYPE_VFC,
  REG_TYPE_MVF,
  REG_TYPE_MVD,
  REG_TYPE_MVFX,
  REG_TYPE_MVDX,
  REG_TYPE_MVAX,
  REG_TYPE_MQ,
  REG_TYPE_DSPSC,
  REG_TYPE_MMXWR,
  REG_TYPE_MMXWC,
  REG_TYPE_MMXWCG,
  REG_TYPE_XSCALE,
  REG_TYPE_RNB,
  REG_TYPE_ZR,
  REG_TYPE_PSEUDO
};

// 解析参数时需要的整数值
typedef enum
{
  PARSE_OPERAND_SUCCESS,
  PARSE_OPERAND_FAIL,
  PARSE_OPERAND_FAIL_NO_BACKTRACK
} parse_operand_result;

enum arm_float_abi
{
  ARM_FLOAT_ABI_HARD,
  ARM_FLOAT_ABI_SOFTFP,
  ARM_FLOAT_ABI_SOFT
};

typedef enum
{
  /* Varieties of non-ALU group relocation.  */

  GROUP_LDR,
  GROUP_LDRS,
  GROUP_LDC,
  GROUP_MVE
} group_reloc_type;
/* 文字池*/
#define MAX_LITERAL_POOL_SIZE 1024
typedef struct literal_pool
{
    expressionS	         literals [MAX_LITERAL_POOL_SIZE];
    unsigned int	         next_free_entry;
    unsigned int	         id;
    symbolS *	         symbol;
    segT		         section;
    subsegT	         sub_section;

    struct literal_pool *  next;
    unsigned int		 alignment;
} literal_pool;

/* Pointer to a linked list of literal pools.  */
literal_pool * list_of_pools = NULL;

#ifdef OBJ_ELF
#  define now_pred seg_info (now_seg)->tc_segment_info_data.current_pred
#else
static struct current_pred now_pred;
#endif
// cond条件
struct asm_cond
{
    const char *	 template_name;
    unsigned long  value;
};

static const struct asm_cond vconds[] =
{
    {"t", 0xf},
    {"e", 0x10}
};

struct asm_shift_name
{
    const char	  *name;
    enum shift_kind  kind;
};

enum parse_shift_mode
{
    NO_SHIFT_RESTRICT,		/* Any kind of shift is accepted.  */
    SHIFT_IMMEDIATE,		/* Shift operand must be an immediate.	*/
    SHIFT_LSL_OR_ASR_IMMEDIATE,	/* Shift must be LSL or ASR immediate.	*/
    SHIFT_ASR_IMMEDIATE,		/* Shift must be ASR immediate.	 */
    SHIFT_LSL_IMMEDIATE,		/* Shift must be LSL immediate.	 */
    SHIFT_UXTW_IMMEDIATE		/* Shift must be UXTW immediate.  */
};
/* Table of all shift-in-operand names.	 */
static const struct asm_shift_name shift_names [] =
{
    { "asl", SHIFT_LSL },	 { "ASL", SHIFT_LSL },
    { "lsl", SHIFT_LSL },	 { "LSL", SHIFT_LSL },
    { "lsr", SHIFT_LSR },	 { "LSR", SHIFT_LSR },
    { "asr", SHIFT_ASR },	 { "ASR", SHIFT_ASR },
    { "ror", SHIFT_ROR },	 { "ROR", SHIFT_ROR },
    { "rrx", SHIFT_RRX },	 { "RRX", SHIFT_RRX },
    { "uxtw", SHIFT_UXTW}, { "UXTW", SHIFT_UXTW}
};

/* The bit that distinguishes CPSR and SPSR.  */
#define SPSR_BIT   (1 << 22)

/* The individual PSR flag bits.  */
#define PSR_c	(1 << 16)
#define PSR_x	(1 << 17)
#define PSR_s	(1 << 18)
#define PSR_f	(1 << 19)

struct asm_psr
{
  const char *   template_name;
  unsigned long  field;
};

static char selected_cpu_name[20];
/* Table of all PSR suffixes.  Bare "CPSR" and "SPSR" are handled
   within psr_required_here.  */
static const struct asm_psr psrs[] =
{
    /* Backward compatibility notation.  Note that "all" is no longer
        truly all possible PSR bits.  */
    {"all",  PSR_c | PSR_f},
    {"flg",  PSR_f},
    {"ctl",  PSR_c},

    /* Individual flags.	*/
    {"f",	   PSR_f},
    {"c",	   PSR_c},
    {"x",	   PSR_x},
    {"s",	   PSR_s},

    /* Combinations of flags.  */
    {"fs",   PSR_f | PSR_s},
    {"fx",   PSR_f | PSR_x},
    {"fc",   PSR_f | PSR_c},
    {"sf",   PSR_s | PSR_f},
    {"sx",   PSR_s | PSR_x},
    {"sc",   PSR_s | PSR_c},
    {"xf",   PSR_x | PSR_f},
    {"xs",   PSR_x | PSR_s},
    {"xc",   PSR_x | PSR_c},
    {"cf",   PSR_c | PSR_f},
    {"cs",   PSR_c | PSR_s},
    {"cx",   PSR_c | PSR_x},
    {"fsx",  PSR_f | PSR_s | PSR_x},
    {"fsc",  PSR_f | PSR_s | PSR_c},
    {"fxs",  PSR_f | PSR_x | PSR_s},
    {"fxc",  PSR_f | PSR_x | PSR_c},
    {"fcs",  PSR_f | PSR_c | PSR_s},
    {"fcx",  PSR_f | PSR_c | PSR_x},
    {"sfx",  PSR_s | PSR_f | PSR_x},
    {"sfc",  PSR_s | PSR_f | PSR_c},
    {"sxf",  PSR_s | PSR_x | PSR_f},
    {"sxc",  PSR_s | PSR_x | PSR_c},
    {"scf",  PSR_s | PSR_c | PSR_f},
    {"scx",  PSR_s | PSR_c | PSR_x},
    {"xfs",  PSR_x | PSR_f | PSR_s},
    {"xfc",  PSR_x | PSR_f | PSR_c},
    {"xsf",  PSR_x | PSR_s | PSR_f},
    {"xsc",  PSR_x | PSR_s | PSR_c},
    {"xcf",  PSR_x | PSR_c | PSR_f},
    {"xcs",  PSR_x | PSR_c | PSR_s},
    {"cfs",  PSR_c | PSR_f | PSR_s},
    {"cfx",  PSR_c | PSR_f | PSR_x},
    {"csf",  PSR_c | PSR_s | PSR_f},
    {"csx",  PSR_c | PSR_s | PSR_x},
    {"cxf",  PSR_c | PSR_x | PSR_f},
    {"cxs",  PSR_c | PSR_x | PSR_s},
    {"fsxc", PSR_f | PSR_s | PSR_x | PSR_c},
    {"fscx", PSR_f | PSR_s | PSR_c | PSR_x},
    {"fxsc", PSR_f | PSR_x | PSR_s | PSR_c},
    {"fxcs", PSR_f | PSR_x | PSR_c | PSR_s},
    {"fcsx", PSR_f | PSR_c | PSR_s | PSR_x},
    {"fcxs", PSR_f | PSR_c | PSR_x | PSR_s},
    {"sfxc", PSR_s | PSR_f | PSR_x | PSR_c},
    {"sfcx", PSR_s | PSR_f | PSR_c | PSR_x},
    {"sxfc", PSR_s | PSR_x | PSR_f | PSR_c},
    {"sxcf", PSR_s | PSR_x | PSR_c | PSR_f},
    {"scfx", PSR_s | PSR_c | PSR_f | PSR_x},
    {"scxf", PSR_s | PSR_c | PSR_x | PSR_f},
    {"xfsc", PSR_x | PSR_f | PSR_s | PSR_c},
    {"xfcs", PSR_x | PSR_f | PSR_c | PSR_s},
    {"xsfc", PSR_x | PSR_s | PSR_f | PSR_c},
    {"xscf", PSR_x | PSR_s | PSR_c | PSR_f},
    {"xcfs", PSR_x | PSR_c | PSR_f | PSR_s},
    {"xcsf", PSR_x | PSR_c | PSR_s | PSR_f},
    {"cfsx", PSR_c | PSR_f | PSR_s | PSR_x},
    {"cfxs", PSR_c | PSR_f | PSR_x | PSR_s},
    {"csfx", PSR_c | PSR_s | PSR_f | PSR_x},
    {"csxf", PSR_c | PSR_s | PSR_x | PSR_f},
    {"cxfs", PSR_c | PSR_x | PSR_f | PSR_s},
    {"cxsf", PSR_c | PSR_x | PSR_s | PSR_f},
};
struct neon_type_el
{
  enum neon_el_type type;
  unsigned size;
};

#define NTA_HASTYPE  1
#define NTA_HASINDEX 2

struct neon_typed_alias
{
    unsigned char        defined;
    unsigned char        index;
    struct neon_type_el  eltype;
};

struct reg_entry
{
    const char *               name;
    unsigned int               number;
    unsigned char              type;
    unsigned char              builtin;
    struct neon_typed_alias *  neon;
};

const char * const reg_expected_msgs[] =
{
  [REG_TYPE_RN]	    = N_("ARM register expected"),
  [REG_TYPE_CP]	    = N_("bad or missing co-processor number"),
  [REG_TYPE_CN]	    = N_("co-processor register expected"),
  [REG_TYPE_FN]	    = N_("FPA register expected"),
  [REG_TYPE_VFS]    = N_("VFP single precision register expected"),
  [REG_TYPE_VFD]    = N_("VFP/Neon double precision register expected"),
  [REG_TYPE_NQ]	    = N_("Neon quad precision register expected"),
  [REG_TYPE_VFSD]   = N_("VFP single or double precision register expected"),
  [REG_TYPE_NDQ]    = N_("Neon double or quad precision register expected"),
  [REG_TYPE_NSD]    = N_("Neon single or double precision register expected"),
  [REG_TYPE_NSDQ]   = N_("VFP single, double or Neon quad precision register"
			 " expected"),
  [REG_TYPE_VFC]    = N_("VFP system register expected"),
  [REG_TYPE_MVF]    = N_("Maverick MVF register expected"),
  [REG_TYPE_MVD]    = N_("Maverick MVD register expected"),
  [REG_TYPE_MVFX]   = N_("Maverick MVFX register expected"),
  [REG_TYPE_MVDX]   = N_("Maverick MVDX register expected"),
  [REG_TYPE_MVAX]   = N_("Maverick MVAX register expected"),
  [REG_TYPE_DSPSC]  = N_("Maverick DSPSC register expected"),
  [REG_TYPE_MMXWR]  = N_("iWMMXt data register expected"),
  [REG_TYPE_MMXWC]  = N_("iWMMXt control register expected"),
  [REG_TYPE_MMXWCG] = N_("iWMMXt scalar register expected"),
  [REG_TYPE_XSCALE] = N_("XScale accumulator register expected"),
  [REG_TYPE_MQ]	    = N_("MVE vector register expected"),
  [REG_TYPE_RNB]    = "",
  [REG_TYPE_ZR]     = N_("ZR register expected"),
  [REG_TYPE_PSEUDO] = N_("Pseudo register expected"),
};

struct reloc_entry
{
  const char *              name;
  bfd_reloc_code_real_type  reloc;
};
#ifdef OBJ_ELF
# ifdef EABI_DEFAULT
static int meabi_flags = EABI_DEFAULT;
# else
static int meabi_flags = EF_ARM_EABI_UNKNOWN;
# endif
bool
arm_is_eabi (void)
{
  return (EF_ARM_EABI_VERSION (meabi_flags) >= EF_ARM_EABI_VER4);
}
#endif

#define MODE_RECORDED (1 << 4)

// 位定义
#define CONDS_BIT	 0x00100000
#define LOAD_BIT	 0x00100000

/* 操作数的最大数目*/
#define ARM_IT_MAX_OPERANDS 6
#define ARM_IT_MAX_RELOCS 3
/* Defines for various bits that we will want to toggle.  */
#define INST_IMMEDIATE	0x02000000
#define OFFSET_REG	0x02000000
#define HWOFFSET_IMM	0x00400000
#define SHIFT_BY_REG	0x00000010
#define PRE_INDEX	0x01000000
#define INDEX_UP	0x00800000
#define WRITE_BACK	0x00200000
#define LDM_TYPE_2_OR_3	0x00400000
#define CPSI_MMOD	0x00020000

#define LITERAL_MASK	0xf000f000
#define OPCODE_MASK	0xfe1fffff
#define V4_STR_BIT	0x00000020
#define VLDR_VMOV_SAME	0x0040f000

#define T2_SUBS_PC_LR	0xf3de8f00

#define DATA_OP_SHIFT	21
#define SBIT_SHIFT	20

#define T2_OPCODE_MASK	0xfe1fffff
#define T2_DATA_OP_SHIFT 21
#define T2_SBIT_SHIFT	 20

#define A_COND_MASK         0xf0000000
#define A_PUSH_POP_OP_MASK  0x0fff0000

/* Opcodes for pushing/poping registers to/from the stack.  */
#define A1_OPCODE_PUSH    0x092d0000
#define A2_OPCODE_PUSH    0x052d0004
#define A2_OPCODE_POP     0x049d0004


#define OPCODE_AND	0
#define OPCODE_EOR	1
#define OPCODE_SUB	2
#define OPCODE_RSB	3
#define OPCODE_ADD	4
#define OPCODE_ADC	5
#define OPCODE_SBC	6
#define OPCODE_RSC	7
#define OPCODE_TST	8
#define OPCODE_TEQ	9
#define OPCODE_CMP	10
#define OPCODE_CMN	11
#define OPCODE_ORR	12
#define OPCODE_MOV	13
#define OPCODE_BIC	14
#define OPCODE_MVN	15

#define T2_OPCODE_AND	0
#define T2_OPCODE_BIC	1
#define T2_OPCODE_ORR	2
#define T2_OPCODE_ORN	3
#define T2_OPCODE_EOR	4
#define T2_OPCODE_ADD	8
#define T2_OPCODE_ADC	10
#define T2_OPCODE_SBC	11
#define T2_OPCODE_SUB	13
#define T2_OPCODE_RSB	14

#define T_OPCODE_MUL 0x4340
#define T_OPCODE_TST 0x4200
#define T_OPCODE_CMN 0x42c0
#define T_OPCODE_NEG 0x4240
#define T_OPCODE_MVN 0x43c0

#define T_OPCODE_ADD_R3	0x1800
#define T_OPCODE_SUB_R3 0x1a00
#define T_OPCODE_ADD_HI 0x4400
#define T_OPCODE_ADD_ST 0xb000
#define T_OPCODE_SUB_ST 0xb080
#define T_OPCODE_ADD_SP 0xa800
#define T_OPCODE_ADD_PC 0xa000
#define T_OPCODE_ADD_I8 0x3000
#define T_OPCODE_SUB_I8 0x3800
#define T_OPCODE_ADD_I3 0x1c00
#define T_OPCODE_SUB_I3 0x1e00

#define T_OPCODE_ASR_R	0x4100
#define T_OPCODE_LSL_R	0x4080
#define T_OPCODE_LSR_R	0x40c0
#define T_OPCODE_ROR_R	0x41c0
#define T_OPCODE_ASR_I	0x1000
#define T_OPCODE_LSL_I	0x0000
#define T_OPCODE_LSR_I	0x0800

#define T_OPCODE_MOV_I8	0x2000
#define T_OPCODE_CMP_I8 0x2800
#define T_OPCODE_CMP_LR 0x4280
#define T_OPCODE_MOV_HR 0x4600
#define T_OPCODE_CMP_HR 0x4500

#define T_OPCODE_LDR_PC 0x4800
#define T_OPCODE_LDR_SP 0x9800
#define T_OPCODE_STR_SP 0x9000
#define T_OPCODE_LDR_IW 0x6800
#define T_OPCODE_STR_IW 0x6000
#define T_OPCODE_LDR_IH 0x8800
#define T_OPCODE_STR_IH 0x8000
#define T_OPCODE_LDR_IB 0x7800
#define T_OPCODE_STR_IB 0x7000
#define T_OPCODE_LDR_RW 0x5800
#define T_OPCODE_STR_RW 0x5000
#define T_OPCODE_LDR_RH 0x5a00
#define T_OPCODE_STR_RH 0x5200
#define T_OPCODE_LDR_RB 0x5c00
#define T_OPCODE_STR_RB 0x5400

#define T_OPCODE_PUSH	0xb400
#define T_OPCODE_POP	0xbc00

#define T_OPCODE_BRANCH 0xe000

#define THUMB_SIZE	2	/* Size of thumb instruction.  */
#define THUMB_PP_PC_LR 0x0100
#define THUMB_LOAD_BIT 0x0800
#define THUMB2_LOAD_BIT 0x00100000

#define FAIL	(-1)
#define SUCCESS (0)
/* 寄存器 的类型*/
#define REG_PC	15
#define REG_SP	13
#define REG_LR	14

/* ARM指令在目标文件中占用4bytes, Thumb指令占用2bytes */
#define INSN_SIZE	4

#define NEON_MAX_TYPE_ELS 5

#define MAX_MEM_ALIGNMENT_BYTES    6
#define MAX_MEM_FOR_RS_ALIGN_CODE ((1 << MAX_MEM_ALIGNMENT_BYTES) - 1)

struct neon_type
{
  struct neon_type_el el[NEON_MAX_TYPE_ELS];
  unsigned elems;
};
// 表示arm汇编指令的数据结构
struct arm_it {
  const char *	error;  // 指向错误消息的指针
  unsigned long instruction;
  unsigned int	size;
  unsigned int	size_req;
  unsigned int	cond;   // 指令的条件执行码
  /* "uncond_value" is set to the value in place of the conditional field in
     unconditional versions of the instruction, or -1u if nothing is
     appropriate.  */
  unsigned int	uncond_value;
  struct neon_type vectype;
  /* This does not indicate an actual NEON instruction, only that
     the mnemonic accepts neon-style type suffixes.  */
  int		is_neon;
  /* Set to the opcode if the instruction needs relaxation.
     Zero if the instruction is not relaxed.  */
  unsigned long	relax;
  struct
  {
    bfd_reloc_code_real_type type;
    expressionS		     exp;
    int			     pc_rel;
  } relocs[ARM_IT_MAX_RELOCS];  /*涉及到的所有重定位信息*/


  enum pred_instruction_type pred_insn_type;

  struct
  {
    unsigned reg;   /* 操作数中涉及的寄存器编号*/
    signed int imm;
    struct neon_type_el vectype;
    unsigned present	: 1;  /* 该操作数是否存在  */
    unsigned isreg	: 1;  /* Operand was a register.  */
    unsigned immisreg	: 2;  /* .imm field is a second register.
				 0: imm, 1: gpr, 2: MVE Q-register.  */
    unsigned isscalar   : 2;  /* Operand is a (SIMD) scalar:
				 0) not scalar,
				 1) Neon scalar,
				 2) MVE scalar.  */
    unsigned immisalign : 1;  /* Immediate is an alignment specifier.  */
    unsigned immisfloat : 1;  /* Immediate was parsed as a float.  */
    /* Note: we abuse "regisimm" to mean "is Neon register" in VMOV
       instructions. This allows us to disambiguate ARM <-> vector insns.  */
    unsigned regisimm   : 1;  /* 64-bit immediate, reg forms high 32 bits.  */
    unsigned isvec      : 1;  /* Is a single, double or quad VFP/Neon reg.  */
    unsigned isquad     : 1;  /* Operand is SIMD quad register.  */
    unsigned issingle   : 1;  /* Operand is VFP single-precision register.  */
    unsigned iszr	: 1;  /* Operand is ZR register.  */
    unsigned hasreloc	: 1;  /* Operand has relocation suffix.  */
    unsigned writeback	: 1;  /* Operand has trailing !  */
    unsigned preind	: 1;  /* Preindexed address.  */
    unsigned postind	: 1;  /* Postindexed address.  */
    unsigned negative	: 1;  /* Index register was negated.  */
    unsigned shifted	: 1;  /* Shift applied to operation.  */
    unsigned shift_kind : 3;  /* Shift operation (enum shift_kind).  */
  } operands[ARM_IT_MAX_OPERANDS];
};

enum reg_list_els {
    REGLIST_RN,
    REGLIST_PSEUDO,
    REGLIST_CLRM,
    REGLIST_VFP_S,
    REGLIST_VFP_S_VPR,
    REGLIST_VFP_D,
    REGLIST_VFP_D_VPR,
    REGLIST_NEON_D
};

static struct arm_it inst;

#define is_immediate_prefix(C) ((C) == '#' || (C) == '$')
#define skip_whitespace(str)  do { if (*(str) == ' ') ++(str); } while (0)
#define skip_past_comma(str) skip_past_char (str, ',')
static bool out_of_range_p (offsetT value, offsetT bits) {
    gas_assert (bits < (offsetT)(sizeof (value) * 8));
    return (value & ~((1 << bits)-1))
        && ((value & ~((1 << bits)-1)) != ~((1 << bits)-1));
}
int 
md_parse_option (int c ,const char *arg) {
    switch (c) {

    }
}

/**
 * @Description: 去掉字符串中c对应的字符
 */
static inline int
skip_past_char (char ** str, char c) {
    /* PR gas/14987: Allow for whitespace before the expected character.  */
    skip_whitespace (*str);

    if (**str == c) {
        (*str)++;
        return SUCCESS;
    }
    else
        return FAIL;
}


/**
 * @Description: 将n个字节的整数(in val)转换为适当的字节流
 */
void
md_number_to_chars (char * buf, valueT val, int n)
{
  if (target_big_endian)
    number_to_chars_bigendian (buf, val, n);
  else
    number_to_chars_littleendian (buf, val, n);
}

void
md_show_usage (FILE * fp) {
  struct arm_option_table *opt;
  struct arm_long_option_table *lopt;
fprintf (fp, _(" ARM-specific assembler options:\n"));
  for (opt = arm_opts; opt->option != NULL; opt++)
    if (opt->help != NULL)
      fprintf (fp, "  -%-23s%s\n", opt->option, _(opt->help));
#ifdef OPTION_EB
  fprintf (fp, _("\
  -EB                     assemble code for a big-endian cpu\n"));
#endif
#ifdef OPTION_EL
  fprintf (fp, _("\
  -EL                     assemble code for a little-endian cpu\n"));
#endif
}

static void
opcode_select (int width) {
    switch (width) {
        case 32:
            if (thumb_mode) {

            }
            break;
        default:
            as_bad (_("invalid instruction size selected (%d)"), width);
    }
}

static void
s_arm (int ignore ATTRIBUTE_UNUSED) {
    opcode_select (32);
    demand_empty_rest_of_line ();
}

/**
 * @Description: 解析到.syntax [unified|divided]
 */
static void
s_syntax (int unused ATTRIBUTE_UNUSED) {
    char *name, delim;

    delim = get_symbol_name (& name);

    if (!strcasecmp (name, "unified"))
        unified_syntax = true;
    else if (!strcasecmp (name, "divided"))
        unified_syntax = false;
    else {
        as_bad (_("unrecognized syntax mode \"%s\""), name);
        return;
    }
    (void) restore_line_pointer (delim);
    demand_empty_rest_of_line ();
}

static int
parse_reloc (char **str) {
    struct reloc_entry *r;
    char *p, *q;
    if (**str != '(')
        return BFD_RELOC_UNUSED;
}
/**
 * @Description: .word 数据伪指令
 */
static void
s_arm_elf_cons (int nbytes) {
    expressionS exp;

    if (is_it_end_of_statement()) {
        demand_empty_rest_of_line ();
        return;
    }
#ifdef md_cons_align
    md_cons_align (nbytes);
#endif   
    mapping_state (MAP_DATA);
    do {
        int reloc;
        char *base = input_line_pointer;

        expression(&exp);
        if (exp.X_op != O_symbol) {
           emit_expr (&exp, (unsigned int) nbytes);
        } else {
            char *before_reloc = input_line_pointer;
            reloc = parse_reloc(&input_line_pointer);
            if (reloc == BFD_RELOC_UNUSED)
                emit_expr (&exp, (unsigned int) nbytes);
        }
    } while (*input_line_pointer++ == ',');

    /* Put terminator back into stream.  */
    input_line_pointer --;
    demand_empty_rest_of_line ();
}

static void s_arm_arch (int);
static void s_arm_cpu (int);
static void s_arm_fpu (int);
// 该表描述了汇编程序中所有与机器相关的伪操作
const pseudo_typeS md_pseudo_table[] = {
    { "align",	   s_align_ptwo,  2 },
    { "arm",	   s_arm,	  0 },
    { "cpu",	   s_arm_cpu,	  0 },
    { "arch",	   s_arm_arch,	  0 },
    { "syntax",	   s_syntax,	  0 },
    { "fpu",	   s_arm_fpu,	  0 },
    { "word",	   s_arm_elf_cons, 4 },
    { 0, 0, 0 }
};
/**
 * @Description:  ARM 架构下 Thumb 模式下进行符号名规范化处理的
 */
char *
arm_canonicalize_symbol_name (char * name) {
  int len;

  if (thumb_mode && (len = strlen (name)) > 5
      && streq (name + len - 5, "/data"))
    *(name + len - 5) = 0;

  return name;
}
#define MIX_ARM_THUMB_OPERANDS(arm_operand, thumb_operand) \
	((arm_operand) | ((thumb_operand) << 16))

enum operand_parse_code
{
  OP_stop,	/* end of line */

  OP_RR,	/* ARM register */
  OP_RRnpc,	/* ARM register, not r15 */
  OP_RRnpcsp,	/* ARM register, neither r15 nor r13 (a.k.a. 'BadReg') */
  OP_RRnpcb,	/* ARM register, not r15, in square brackets */
  OP_RRnpctw,	/* ARM register, not r15 in Thumb-state or with writeback,
		   optional trailing ! */
  OP_RRw,	/* ARM register, not r15, optional trailing ! */
  OP_RCP,	/* Coprocessor number */
  OP_RCN,	/* Coprocessor register */
  OP_RF,	/* FPA register */
  OP_RVS,	/* VFP single precision register */
  OP_RVD,	/* VFP double precision register (0..15) */
  OP_RND,       /* Neon double precision register (0..31) */
  OP_RNDMQ,     /* Neon double precision (0..31) or MVE vector register.  */
  OP_RNDMQR,    /* Neon double precision (0..31), MVE vector or ARM register.
		 */
  OP_RNSDMQR,    /* Neon single or double precision, MVE vector or ARM register.
		 */
  OP_RNQ,	/* Neon quad precision register */
  OP_RNQMQ,	/* Neon quad or MVE vector register.  */
  OP_RVSD,	/* VFP single or double precision register */
  OP_RVSD_COND,	/* VFP single, double precision register or condition code.  */
  OP_RVSDMQ,	/* VFP single, double precision or MVE vector register.  */
  OP_RNSD,      /* Neon single or double precision register */
  OP_RNDQ,      /* Neon double or quad precision register */
  OP_RNDQMQ,     /* Neon double, quad or MVE vector register.  */
  OP_RNDQMQR,   /* Neon double, quad, MVE vector or ARM register.  */
  OP_RNSDQ,	/* Neon single, double or quad precision register */
  OP_RNSC,      /* Neon scalar D[X] */
  OP_RVC,	/* VFP control register */
  OP_RMF,	/* Maverick F register */
  OP_RMD,	/* Maverick D register */
  OP_RMFX,	/* Maverick FX register */
  OP_RMDX,	/* Maverick DX register */
  OP_RMAX,	/* Maverick AX register */
  OP_RMDS,	/* Maverick DSPSC register */
  OP_RIWR,	/* iWMMXt wR register */
  OP_RIWC,	/* iWMMXt wC register */
  OP_RIWG,	/* iWMMXt wCG register */
  OP_RXA,	/* XScale accumulator register */

  OP_RNSDMQ,	/* Neon single, double or MVE vector register */
  OP_RNSDQMQ,	/* Neon single, double or quad register or MVE vector register
		 */
  OP_RNSDQMQR,	/* Neon single, double or quad register, MVE vector register or
		   GPR (no SP/SP)  */
  OP_RMQ,	/* MVE vector register.  */
  OP_RMQRZ,	/* MVE vector or ARM register including ZR.  */
  OP_RMQRR,     /* MVE vector or ARM register.  */

  /* New operands for Armv8.1-M Mainline.  */
  OP_LR,	/* ARM LR register */
  OP_SP,	/* ARM SP register */
  OP_R12,
  OP_RRe,	/* ARM register, only even numbered.  */
  OP_RRo,	/* ARM register, only odd numbered, not r13 or r15.  */
  OP_RRnpcsp_I32, /* ARM register (no BadReg) or literal 1 .. 32 */
  OP_RR_ZR,	/* ARM register or ZR but no PC */

  OP_REGLST,	/* ARM register list */
  OP_CLRMLST,	/* CLRM register list */
  OP_VRSLST,	/* VFP single-precision register list */
  OP_VRDLST,	/* VFP double-precision register list */
  OP_VRSDLST,   /* VFP single or double-precision register list (& quad) */
  OP_NRDLST,    /* Neon double-precision register list (d0-d31, qN aliases) */
  OP_NSTRLST,   /* Neon element/structure list */
  OP_VRSDVLST,  /* VFP single or double-precision register list and VPR */
  OP_MSTRLST2,	/* MVE vector list with two elements.  */
  OP_MSTRLST4,	/* MVE vector list with four elements.  */

  OP_RNDQ_I0,   /* Neon D or Q reg, or immediate zero.  */
  OP_RVSD_I0,	/* VFP S or D reg, or immediate zero.  */
  OP_RSVD_FI0, /* VFP S or D reg, or floating point immediate zero.  */
  OP_RSVDMQ_FI0, /* VFP S, D, MVE vector register or floating point immediate
		    zero.  */
  OP_RR_RNSC,   /* ARM reg or Neon scalar.  */
  OP_RNSD_RNSC, /* Neon S or D reg, or Neon scalar.  */
  OP_RNSDQ_RNSC, /* Vector S, D or Q reg, or Neon scalar.  */
  OP_RNSDQ_RNSC_MQ, /* Vector S, D or Q reg, Neon scalar or MVE vector register.
		     */
  OP_RNSDQ_RNSC_MQ_RR, /* Vector S, D or Q reg, or MVE vector reg , or Neon
			  scalar, or ARM register.  */
  OP_RNDQ_RNSC, /* Neon D or Q reg, or Neon scalar.  */
  OP_RNDQ_RNSC_RR, /* Neon D or Q reg, Neon scalar, or ARM register.  */
  OP_RNDQMQ_RNSC_RR, /* Neon D or Q reg, Neon scalar, MVE vector or ARM
			register.  */
  OP_RNDQMQ_RNSC, /* Neon D, Q or MVE vector reg, or Neon scalar.  */
  OP_RND_RNSC,  /* Neon D reg, or Neon scalar.  */
  OP_VMOV,      /* Neon VMOV operands.  */
  OP_RNDQ_Ibig,	/* Neon D or Q reg, or big immediate for logic and VMVN.  */
  /* Neon D, Q or MVE vector register, or big immediate for logic and VMVN.  */
  OP_RNDQMQ_Ibig,
  OP_RNDQ_I63b, /* Neon D or Q reg, or immediate for shift.  */
  OP_RNDQMQ_I63b_RR, /* Neon D or Q reg, immediate for shift, MVE vector or
			ARM register.  */
  OP_RIWR_I32z, /* iWMMXt wR register, or immediate 0 .. 32 for iWMMXt2.  */
  OP_VLDR,	/* VLDR operand.  */

  OP_I0,        /* immediate zero */
  OP_I7,	/* immediate value 0 .. 7 */
  OP_I15,	/*		   0 .. 15 */
  OP_I16,	/*		   1 .. 16 */
  OP_I16z,      /*                 0 .. 16 */
  OP_I31,	/*		   0 .. 31 */
  OP_I31w,	/*		   0 .. 31, optional trailing ! */
  OP_I32,	/*		   1 .. 32 */
  OP_I32z,	/*		   0 .. 32 */
  OP_I48_I64,	/*		   48 or 64 */
  OP_I63,	/*		   0 .. 63 */
  OP_I63s,	/*		 -64 .. 63 */
  OP_I64,	/*		   1 .. 64 */
  OP_I64z,	/*		   0 .. 64 */
  OP_I127,	/*		   0 .. 127 */
  OP_I255,	/*		   0 .. 255 */
  OP_I511,	/*		   0 .. 511 */
  OP_I4095,	/*		   0 .. 4095 */
  OP_I8191,	/*		   0 .. 8191 */
  OP_I4b,	/* immediate, prefix optional, 1 .. 4 */
  OP_I7b,	/*			       0 .. 7 */
  OP_I15b,	/*			       0 .. 15 */
  OP_I31b,	/*			       0 .. 31 */

  OP_SH,	/* shifter operand */
  OP_SHG,	/* shifter operand with possible group relocation */
  OP_ADDR,	/* Memory address expression (any mode) */
  OP_ADDRMVE,	/* Memory address expression for MVE's VSTR/VLDR.  */
  OP_ADDRGLDR,	/* Mem addr expr (any mode) with possible LDR group reloc */
  OP_ADDRGLDRS, /* Mem addr expr (any mode) with possible LDRS group reloc */
  OP_ADDRGLDC,  /* Mem addr expr (any mode) with possible LDC group reloc */
  OP_EXP,	/* arbitrary expression */
  OP_EXPi,	/* same, with optional immediate prefix */
  OP_EXPr,	/* same, with optional relocation suffix */
  OP_EXPs,	/* same, with optional non-first operand relocation suffix */
  OP_HALF,	/* 0 .. 65535 or low/high reloc.  */
  OP_IROT1,	/* VCADD rotate immediate: 90, 270.  */
  OP_IROT2,	/* VCMLA rotate immediate: 0, 90, 180, 270.  */

  OP_CPSF,	/* CPS flags */
  OP_ENDI,	/* Endianness specifier */
  OP_wPSR,	/* CPSR/SPSR/APSR mask for msr (writing).  */
  OP_rPSR,	/* CPSR/SPSR/APSR mask for msr (reading).  */
  OP_COND,	/* conditional code */
  OP_TB,	/* Table branch.  */

  OP_APSR_RR,   /* ARM register or "APSR_nzcv".  */

  OP_RRnpc_I0,	/* ARM register or literal 0 */
  OP_RR_EXr,	/* ARM register or expression with opt. reloc stuff. */
  OP_RR_EXi,	/* ARM register or expression with imm prefix */
  OP_RF_IF,	/* FPA register or immediate */
  OP_RIWR_RIWC, /* iWMMXt R or C reg */
  OP_RIWC_RIWG, /* iWMMXt wC or wCG reg */

  /* Optional operands.	 */
  OP_oI7b,	 /* immediate, prefix optional, 0 .. 7 */
  OP_oI31b,	 /*				0 .. 31 */
  OP_oI32b,      /*                             1 .. 32 */
  OP_oI32z,      /*                             0 .. 32 */
  OP_oIffffb,	 /*				0 .. 65535 */
  OP_oI255c,	 /*	  curly-brace enclosed, 0 .. 255 */

  OP_oRR,	 /* ARM register */
  OP_oLR,	 /* ARM LR register */
  OP_oRRnpc,	 /* ARM register, not the PC */
  OP_oRRnpcsp,	 /* ARM register, neither the PC nor the SP (a.k.a. BadReg) */
  OP_oRRw,	 /* ARM register, not r15, optional trailing ! */
  OP_oRND,       /* Optional Neon double precision register */
  OP_oRNQ,       /* Optional Neon quad precision register */
  OP_oRNDQMQ,     /* Optional Neon double, quad or MVE vector register.  */
  OP_oRNDQ,      /* Optional Neon double or quad precision register */
  OP_oRNSDQ,	 /* Optional single, double or quad precision vector register */
  OP_oRNSDQMQ,	 /* Optional single, double or quad register or MVE vector
		    register.  */
  OP_oRNSDMQ,	 /* Optional single, double register or MVE vector
		    register.  */
  OP_oSHll,	 /* LSL immediate */
  OP_oSHar,	 /* ASR immediate */
  OP_oSHllar,	 /* LSL or ASR immediate */
  OP_oROR,	 /* ROR 0/8/16/24 */
  OP_oBARRIER_I15, /* Option argument for a barrier instruction.  */

  OP_oRMQRZ,	/* optional MVE vector or ARM register including ZR.  */

  /* Some pre-defined mixed (ARM/THUMB) operands.  */
  OP_RR_npcsp		= MIX_ARM_THUMB_OPERANDS (OP_RR, OP_RRnpcsp),
  OP_RRnpc_npcsp	= MIX_ARM_THUMB_OPERANDS (OP_RRnpc, OP_RRnpcsp),
  OP_oRRnpc_npcsp	= MIX_ARM_THUMB_OPERANDS (OP_oRRnpc, OP_oRRnpcsp),

  OP_FIRST_OPTIONAL = OP_oI7b
};

/* Tag values used in struct asm_opcode's tag field.  */
enum opcode_tag
{
  OT_unconditional,	/* Instruction cannot be conditionalized.
			   The ARM condition field is still 0xE.  */
  OT_unconditionalF,	/* Instruction cannot be conditionalized
			   and carries 0xF in its ARM condition field.  */
  OT_csuffix,		/* Instruction takes a conditional suffix.  */
  OT_csuffixF,		/* Some forms of the instruction take a scalar
			   conditional suffix, others place 0xF where the
			   condition field would be, others take a vector
			   conditional suffix.  */
  OT_cinfix3,		/* Instruction takes a conditional infix,
			   beginning at character index 3.  (In
			   unified mode, it becomes a suffix.)  */
  OT_cinfix3_deprecated, /* The same as OT_cinfix3.  This is used for
			    tsts, cmps, cmns, and teqs. */
  OT_cinfix3_legacy,	/* Legacy instruction takes a conditional infix at
			   character index 3, even in unified mode.  Used for
			   legacy instructions where suffix and infix forms
			   may be ambiguous.  */
  OT_csuf_or_in3,	/* Instruction takes either a conditional
			   suffix or an infix at character index 3.  */
  OT_odd_infix_unc,	/* This is the unconditional variant of an
			   instruction that takes a conditional infix
			   at an unusual position.  In unified mode,
			   this variant will accept a suffix.  */
  OT_odd_infix_0	/* Values greater than or equal to OT_odd_infix_0
			   are the conditional variants of instructions that
			   take conditional infixes in unusual positions.
			   The infix appears at character index
			   (tag - OT_odd_infix_0).  These are not accepted
			   in unified mode.  */
};
#define T16_32_TAB				\
  X(_adc,   4140, eb400000),			\
  X(_adcs,  4140, eb500000),			\
  X(_add,   1c00, eb000000),			\
  X(_adds,  1c00, eb100000),			\
  X(_addi,  0000, f1000000),			\
  X(_addis, 0000, f1100000),			\
  X(_add_pc,000f, f20f0000),			\
  X(_add_sp,000d, f10d0000),			\
  X(_adr,   000f, f20f0000),			\
  X(_and,   4000, ea000000),			\
  X(_ands,  4000, ea100000),			\
  X(_asr,   1000, fa40f000),			\
  X(_asrs,  1000, fa50f000),			\
  X(_aut,   0000, f3af802d),			\
  X(_autg,   0000, fb500f00),			\
  X(_b,     e000, f000b000),			\
  X(_bcond, d000, f0008000),			\
  X(_bf,    0000, f040e001),			\
  X(_bfcsel,0000, f000e001),			\
  X(_bfx,   0000, f060e001),			\
  X(_bfl,   0000, f000c001),			\
  X(_bflx,  0000, f070e001),			\
  X(_bic,   4380, ea200000),			\
  X(_bics,  4380, ea300000),			\
  X(_bxaut, 0000, fb500f10),			\
  X(_cinc,  0000, ea509000),			\
  X(_cinv,  0000, ea50a000),			\
  X(_cmn,   42c0, eb100f00),			\
  X(_cmp,   2800, ebb00f00),			\
  X(_cneg,  0000, ea50b000),			\
  X(_cpsie, b660, f3af8400),			\
  X(_cpsid, b670, f3af8600),			\
  X(_cpy,   4600, ea4f0000),			\
  X(_csel,  0000, ea508000),			\
  X(_cset,  0000, ea5f900f),			\
  X(_csetm, 0000, ea5fa00f),			\
  X(_csinc, 0000, ea509000),			\
  X(_csinv, 0000, ea50a000),			\
  X(_csneg, 0000, ea50b000),			\
  X(_dec_sp,80dd, f1ad0d00),			\
  X(_dls,   0000, f040e001),			\
  X(_dlstp, 0000, f000e001),			\
  X(_eor,   4040, ea800000),			\
  X(_eors,  4040, ea900000),			\
  X(_inc_sp,00dd, f10d0d00),			\
  X(_lctp,  0000, f00fe001),			\
  X(_ldmia, c800, e8900000),			\
  X(_ldr,   6800, f8500000),			\
  X(_ldrb,  7800, f8100000),			\
  X(_ldrh,  8800, f8300000),			\
  X(_ldrsb, 5600, f9100000),			\
  X(_ldrsh, 5e00, f9300000),			\
  X(_ldr_pc,4800, f85f0000),			\
  X(_ldr_pc2,4800, f85f0000),			\
  X(_ldr_sp,9800, f85d0000),			\
  X(_le,    0000, f00fc001),			\
  X(_letp,  0000, f01fc001),			\
  X(_lsl,   0000, fa00f000),			\
  X(_lsls,  0000, fa10f000),			\
  X(_lsr,   0800, fa20f000),			\
  X(_lsrs,  0800, fa30f000),			\
  X(_mov,   2000, ea4f0000),			\
  X(_movs,  2000, ea5f0000),			\
  X(_mul,   4340, fb00f000),                     \
  X(_muls,  4340, ffffffff), /* no 32b muls */	\
  X(_mvn,   43c0, ea6f0000),			\
  X(_mvns,  43c0, ea7f0000),			\
  X(_neg,   4240, f1c00000), /* rsb #0 */	\
  X(_negs,  4240, f1d00000), /* rsbs #0 */	\
  X(_orr,   4300, ea400000),			\
  X(_orrs,  4300, ea500000),			\
  X(_pac,   0000, f3af801d),			\
  X(_pacbti, 0000, f3af800d),			\
  X(_pacg,  0000, fb60f000),			\
  X(_pop,   bc00, e8bd0000), /* ldmia sp!,... */	\
  X(_push,  b400, e92d0000), /* stmdb sp!,... */	\
  X(_rev,   ba00, fa90f080),			\
  X(_rev16, ba40, fa90f090),			\
  X(_revsh, bac0, fa90f0b0),			\
  X(_ror,   41c0, fa60f000),			\
  X(_rors,  41c0, fa70f000),			\
  X(_sbc,   4180, eb600000),			\
  X(_sbcs,  4180, eb700000),			\
  X(_stmia, c000, e8800000),			\
  X(_str,   6000, f8400000),			\
  X(_strb,  7000, f8000000),			\
  X(_strh,  8000, f8200000),			\
  X(_str_sp,9000, f84d0000),			\
  X(_sub,   1e00, eba00000),			\
  X(_subs,  1e00, ebb00000),			\
  X(_subi,  8000, f1a00000),			\
  X(_subis, 8000, f1b00000),			\
  X(_sxtb,  b240, fa4ff080),			\
  X(_sxth,  b200, fa0ff080),			\
  X(_tst,   4200, ea100f00),			\
  X(_uxtb,  b2c0, fa5ff080),			\
  X(_uxth,  b280, fa1ff080),			\
  X(_nop,   bf00, f3af8000),			\
  X(_yield, bf10, f3af8001),			\
  X(_wfe,   bf20, f3af8002),			\
  X(_wfi,   bf30, f3af8003),			\
  X(_wls,   0000, f040c001),			\
  X(_wlstp, 0000, f000c001),			\
  X(_sev,   bf40, f3af8004),                    \
  X(_sevl,  bf50, f3af8005),			\
  X(_udf,   de00, f7f0a000)

#define X(a,b,c) T_MNEM##a
enum t16_32_codes { T16_32_OFFSET = 0xF7FF, T16_32_TAB };
#undef X

#define X(a,b,c) 0x##b
static const unsigned short thumb_op16[] = { T16_32_TAB };
#define THUMB_OP16(n) (thumb_op16[(n) - (T16_32_OFFSET + 1)])
#undef X

#define X(a,b,c) 0x##c
static const unsigned int thumb_op32[] = { T16_32_TAB };
#define THUMB_OP32(n)        (thumb_op32[(n) - (T16_32_OFFSET + 1)])
#define THUMB_SETS_FLAGS(n)  (THUMB_OP32 (n) & 0x00100000)
#undef X
#undef T16_32_TAB

#define OPS0()		  { OP_stop, }
#define OPS1(a)		  { OP_##a, }
#define OPS2(a,b)	  { OP_##a,OP_##b, }
#define OPS3(a,b,c)	  { OP_##a,OP_##b,OP_##c, }
#define OPS4(a,b,c,d)	  { OP_##a,OP_##b,OP_##c,OP_##d, }
#define OPS5(a,b,c,d,e)	  { OP_##a,OP_##b,OP_##c,OP_##d,OP_##e, }
#define OPS6(a,b,c,d,e,f) { OP_##a,OP_##b,OP_##c,OP_##d,OP_##e,OP_##f, }

#define OPS_1(a)	   { a, }
#define OPS_2(a,b)	   { a,b, }
#define OPS_3(a,b,c)	   { a,b,c, }
#define OPS_4(a,b,c,d)	   { a,b,c,d, }
#define OPS_5(a,b,c,d,e)   { a,b,c,d,e, }
#define OPS_6(a,b,c,d,e,f) { a,b,c,d,e,f, }

// error 宏
#define BAD_SYNTAX	_("syntax error")
#define BAD_ARGS	_("bad arguments to instruction")
#define BAD_SP          _("r13 not allowed here")
#define BAD_PC		_("r15 not allowed here")
#define BAD_ODD		_("Odd register not allowed here")
#define BAD_EVEN	_("Even register not allowed here")
#define BAD_COND	_("instruction cannot be conditional")
#define BAD_OVERLAP	_("registers may not be the same")
#define BAD_HIREG	_("lo register required")
#define BAD_THUMB32	_("instruction not supported in Thumb16 mode")
#define BAD_ADDR_MODE   _("instruction does not accept this addressing mode")
#define BAD_BRANCH	_("branch must be last instruction in IT block")
#define BAD_BRANCH_OFF	_("branch out of range or not a multiple of 2")
#define BAD_NO_VPT	_("instruction not allowed in VPT block")
#define BAD_NOT_IT	_("instruction not allowed in IT block")
#define BAD_NOT_VPT	_("instruction missing MVE vector predication code")
#define BAD_FPU		_("selected FPU does not support instruction")
#define BAD_OUT_IT 	_("thumb conditional instruction should be in IT block")
#define BAD_OUT_VPT	\
	_("vector predicated instruction should be in VPT/VPST block")
#define BAD_IT_COND	_("incorrect condition in IT block")
#define BAD_VPT_COND	_("incorrect condition in VPT/VPST block")
#define BAD_IT_IT 	_("IT falling in the range of a previous IT block")
#define MISSING_FNSTART	_("missing .fnstart before unwinding directive")
#define BAD_PC_ADDRESSING \
	_("cannot use register index with PC-relative addressing")
#define BAD_PC_WRITEBACK \
	_("cannot use writeback with PC-relative addressing")
#define BAD_RANGE	_("branch out of range")
#define BAD_FP16	_("selected processor does not support fp16 instruction")
#define BAD_BF16	_("selected processor does not support bf16 instruction")
#define BAD_CDE	_("selected processor does not support cde instruction")
#define BAD_CDE_COPROC	_("coprocessor for insn is not enabled for cde")
#define UNPRED_REG(R)	_("using " R " results in unpredictable behaviour")
#define THUMB1_RELOC_ONLY  _("relocation valid in thumb1 code only")
#define MVE_NOT_IT	_("Warning: instruction is UNPREDICTABLE in an IT " \
			  "block")
#define MVE_NOT_VPT	_("Warning: instruction is UNPREDICTABLE in a VPT " \
			  "block")
#define MVE_BAD_PC	_("Warning: instruction is UNPREDICTABLE with PC" \
			  " operand")
#define MVE_BAD_SP	_("Warning: instruction is UNPREDICTABLE with SP" \
			  " operand")
#define BAD_SIMD_TYPE	_("bad type in SIMD instruction")
#define BAD_MVE_AUTO	\
  _("GAS auto-detection mode and -march=all is deprecated for MVE, please" \
    " use a valid -march or -mcpu option.")
#define BAD_MVE_SRCDEST	_("Warning: 32-bit element size and same destination "\
			  "and source operands makes instruction UNPREDICTABLE")
#define BAD_EL_TYPE	_("bad element type for instruction")
#define MVE_BAD_QREG	_("MVE vector register Q[0..7] expected")
#define BAD_PACBTI	_("selected processor does not support PACBTI extention")


#define FAIL	(-1)
#define SUCCESS (0)

#define COND_ALWAYS 0xE

/**
 * @Description: 处理IT块和VPT
 */
static int
handle_pred_state (void) {
    now_pred.state_handled = 1;
    now_pred.insn_cond = false;
    switch (now_pred.state) {
        case OUTSIDE_PRED_BLOCK:
            switch (inst.pred_insn_type) {
                case OUTSIDE_PRED_INSN:
                    if (inst.cond > COND_ALWAYS) {
                        /* Case 17:  Outside a pred block, with a VPT code: syntax error.
                        */
                        inst.error = BAD_SYNTAX;
                        return FAIL;
                    }
                    /* Case 18: Outside a pred block, with no code: OK!  */
                    break;
            }
    }
    return SUCCESS;
}
// 检查约束条件并抛出错误信息
#define constraint(expr, err)			\
  do						\
    {						\
      if (expr)					\
	{					\
	  inst.error = err;			\
	  return;				\
	}					\
    }						\
  while (0)
/* 检测并拒绝使用特定的寄存器  */
#define reject_bad_reg(reg)					\
  do								\
   if (reg == REG_PC)						\
     {								\
       inst.error = BAD_PC;					\
       return;							\
     }								\
   else if (reg == REG_SP					\
	    && !ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v8))	\
     {								\
       inst.error = BAD_SP;					\
       return;							\
     }								\
  while (0)

// 处理Thumb-2指令集中的IT指令
#define set_pred_insn_type(type)			\
  do						\
    {						\
      inst.pred_insn_type = type;			\
      if (handle_pred_state () == FAIL)		\
	return;					\
    }						\
  while (0)

#define set_pred_insn_type_last()				\
  do							\
    {							\
      if (inst.cond == COND_ALWAYS)			\
	set_pred_insn_type (IF_INSIDE_IT_LAST_INSN);	\
      else						\
	set_pred_insn_type (INSIDE_IT_LAST_INSN);		\
    }							\
  while (0)

#ifdef OBJ_ELF
#  define now_pred seg_info (now_seg)->tc_segment_info_data.current_pred
#else
static struct current_pred now_pred;
#endif
// static int
// in_pred_block (void)
// {
//   if (!now_pred.state_handled)
//     handle_pred_state ();

//   return now_pred.state != OUTSIDE_PRED_BLOCK;
// }
#define rotate_left(v, n) (v << (n & 31) | v >> ((32 - n) & 31))

/* neon 工具函数 */
/* Write immediate bits [7:0] to the following locations:

//   |28/24|23     19|18 16|15                    4|3     0|
//   |  a  |x x x x x|b c d|x x x x x x x x x x x x|e f g h|

//   This function is used by VMOV/VMVN/VORR/VBIC.  */
// static void
// neon_write_immbits (unsigned immbits)
// {
//     inst.instruction |= immbits & 0xf;
//     inst.instruction |= ((immbits >> 4) & 0x7) << 16;
//     inst.instruction |= ((immbits >> 7) & 0x1) << (thumb_mode ? 28 : 24);
// }

// /* Returns 1 if a number has "quarter-precision" float format
//    0baBbbbbbc defgh000 00000000 00000000.  */
static int
is_quarter_float (unsigned imm)
{
    int bs = (imm & 0x20000000) ? 0x3e000000 : 0x40000000;
    return (imm & 0x7ffff) == 0 && ((imm & 0x7e000000) ^ bs) == 0;
}

/* Directives: Literal pools.  */
static literal_pool *
find_literal_pool (void)
{
    literal_pool * pool;

    for (pool = list_of_pools; pool != NULL; pool = pool->next) {
        if (pool->section == now_seg
        && pool->sub_section == now_subseg)
        break;
    }

    return pool;
}

static literal_pool *
find_or_make_literal_pool (void)
{
  /* Next literal pool ID number.  */
  static unsigned int latest_pool_num = 1;
  literal_pool *      pool;

  pool = find_literal_pool ();

  if (pool == NULL)
    {
      /* Create a new pool.  */
      pool = XNEW (literal_pool);
      if (! pool)
	return NULL;

      pool->next_free_entry = 0;
      pool->section	    = now_seg;
      pool->sub_section	    = now_subseg;
      pool->next	    = list_of_pools;
      pool->symbol	    = NULL;
      pool->alignment	    = 2;

      /* Add it to the list.  */
      list_of_pools = pool;
    }

  /* New pools, and emptied pools, will have a NULL symbol.  */
  if (pool->symbol == NULL)
    {
      pool->symbol = symbol_create (FAKE_LABEL_NAME, undefined_section,
				    &zero_address_frag, 0);
      pool->id = latest_pool_num ++;
    }

  /* Done.  */
  return pool;
}
/**
 * @Description: 将全局变量inst中的字面量添加到相应的字面量池中
 */
static int
add_to_lit_pool (unsigned int nbytes)
{
#define PADDING_SLOT 0x1
#define LIT_ENTRY_SIZE_MASK 0xFF
    literal_pool * pool;
    unsigned int entry, pool_size = 0;
    bool padding_slot_p = false;
    unsigned imm1 = 0;
    unsigned imm2 = 0;

    if (nbytes == 8) {
        imm1 = inst.operands[1].imm;
        imm2 = (inst.operands[1].regisimm ? inst.operands[1].reg
            : inst.relocs[0].exp.X_unsigned ? 0
            : (int64_t) inst.operands[1].imm >> 32);
        if (target_big_endian) {
            imm1 = imm2;
            imm2 = inst.operands[1].imm;
        }
    }

    pool = find_or_make_literal_pool ();

    /* Check if this literal value is already in the pool.  */
    for (entry = 0; entry < pool->next_free_entry; entry ++) {
        if (nbytes == 4) {
            if ((pool->literals[entry].X_op == inst.relocs[0].exp.X_op)
                && (inst.relocs[0].exp.X_op == O_constant)
                && (pool->literals[entry].X_add_number
                == inst.relocs[0].exp.X_add_number)
                && (pool->literals[entry].X_md == nbytes)
                && (pool->literals[entry].X_unsigned
                == inst.relocs[0].exp.X_unsigned))
                break;

            if ((pool->literals[entry].X_op == inst.relocs[0].exp.X_op)
                && (inst.relocs[0].exp.X_op == O_symbol)
                && (pool->literals[entry].X_add_number
                == inst.relocs[0].exp.X_add_number)
                && (pool->literals[entry].X_add_symbol
                == inst.relocs[0].exp.X_add_symbol)
                && (pool->literals[entry].X_op_symbol
                == inst.relocs[0].exp.X_op_symbol)
                && (pool->literals[entry].X_md == nbytes))
                break;
        } else if ((nbytes == 8)
            && !(pool_size & 0x7)
            && ((entry + 1) != pool->next_free_entry)
            && (pool->literals[entry].X_op == O_constant)
            && (pool->literals[entry].X_add_number == (offsetT) imm1)
            && (pool->literals[entry].X_unsigned
            == inst.relocs[0].exp.X_unsigned)
            && (pool->literals[entry + 1].X_op == O_constant)
            && (pool->literals[entry + 1].X_add_number == (offsetT) imm2)
            && (pool->literals[entry + 1].X_unsigned
            == inst.relocs[0].exp.X_unsigned))
            break;

        padding_slot_p = ((pool->literals[entry].X_md >> 8) == PADDING_SLOT);
        if (padding_slot_p && (nbytes == 4))
            break;

        pool_size += 4;
    }

    /* Do we need to create a new entry?	*/
    if (entry == pool->next_free_entry) {
        if (entry >= MAX_LITERAL_POOL_SIZE) {
            inst.error = _("literal pool overflow");
            return FAIL;
        }

        if (nbytes == 8) {
        /* For 8-byte entries, we align to an 8-byte boundary,
            and split it into two 4-byte entries, because on 32-bit
            host, 8-byte constants are treated as big num, thus
            saved in "generic_bignum" which will be overwritten
            by later assignments.

            We also need to make sure there is enough space for
            the split.

            We also check to make sure the literal operand is a
            constant number.  */
            if (!(inst.relocs[0].exp.X_op == O_constant
                || inst.relocs[0].exp.X_op == O_big)) {
                inst.error = _("invalid type for literal pool");
                return FAIL;
            } else if (pool_size & 0x7) {
                if ((entry + 2) >= MAX_LITERAL_POOL_SIZE) {
                    inst.error = _("literal pool overflow");
                    return FAIL;
                }

                pool->literals[entry] = inst.relocs[0].exp;
                pool->literals[entry].X_op = O_constant;
                pool->literals[entry].X_add_number = 0;
                pool->literals[entry++].X_md = (PADDING_SLOT << 8) | 4;
                pool->next_free_entry += 1;
                pool_size += 4;
            } else if ((entry + 1) >= MAX_LITERAL_POOL_SIZE) {
                inst.error = _("literal pool overflow");
                return FAIL;
            }

            pool->literals[entry] = inst.relocs[0].exp;
            pool->literals[entry].X_op = O_constant;
            pool->literals[entry].X_add_number = imm1;
            pool->literals[entry].X_unsigned = inst.relocs[0].exp.X_unsigned;
            pool->literals[entry++].X_md = 4;
            pool->literals[entry] = inst.relocs[0].exp;
            pool->literals[entry].X_op = O_constant;
            pool->literals[entry].X_add_number = imm2;
            pool->literals[entry].X_unsigned = inst.relocs[0].exp.X_unsigned;
            pool->literals[entry].X_md = 4;
            pool->alignment = 3;
            pool->next_free_entry += 1;
        } else {
            pool->literals[entry] = inst.relocs[0].exp;
            pool->literals[entry].X_md = 4;
        }

        // #ifdef OBJ_ELF
        //     /* PR ld/12974: Record the location of the first source line to reference
        //     this entry in the literal pool.  If it turns out during linking that the
        //     symbol does not exist we will be able to give an accurate line number for
        //     the (first use of the) missing reference.  */
        //     if (debug_type == DEBUG_DWARF2)
        //     dwarf2_where (pool->locs + entry);
        // #endif
        pool->next_free_entry += 1;
    } else if (padding_slot_p) {
        pool->literals[entry] = inst.relocs[0].exp;
        pool->literals[entry].X_md = nbytes;
    }

    inst.relocs[0].exp.X_op	      = O_symbol;
    inst.relocs[0].exp.X_add_number = pool_size;
    inst.relocs[0].exp.X_add_symbol = pool->symbol;

    return SUCCESS;
}


// /* Compress quarter-float representation to 0b...000 abcdefgh.  */
// static unsigned
// neon_qfloat_bits (unsigned imm)
// {
//     return ((imm >> 19) & 0x7f) | ((imm >> 24) & 0x80);
// }

// /* Look up and encode a simple mnemonic, for use as a helper function for the
//    Neon-style VFP syntax.  This avoids duplication of bits of the insns table,
//    etc.  It is assumed that operand parsing has already been done, and that the
//    operands are in the form expected by the given opcode (this isn't necessarily
//    the same as the form in which they were parsed, hence some massaging must
//    take place before this function is called).
//    Checks current arch version against that in the looked-up opcode.  */
// static void
// do_vfp_nsyn_opcode (const char *opname)
// {
//     const struct asm_opcode *opcode;

//     opcode = (const struct asm_opcode *) str_hash_find (arm_ops_hsh, opname);

//     if (!opcode)
//         abort ();

//     constraint (!ARM_CPU_HAS_FEATURE (cpu_variant,
//             thumb_mode ? *opcode->tvariant : *opcode->avariant),
//             _(BAD_FPU));

//     inst.is_neon = 1;

//     if (thumb_mode) {
//         inst.instruction = opcode->tvalue;
//         opcode->tencode ();
//     }
//     else {
//         inst.instruction = (inst.cond << 28) | opcode->avalue;
//         opcode->aencode ();
//     }
// }

// /* Returns TRUE if double precision value V may be cast
//    to single precision without loss of accuracy.  */
// static bool
// is_double_a_single (uint64_t v)
// {
//     int exp = (v >> 52) & 0x7FF;
//     uint64_t mantissa = v & 0xFFFFFFFFFFFFFULL;

//     return ((exp == 0 || exp == 0x7FF
//         || (exp >= 1023 - 126 && exp <= 1023 + 127))
//         && (mantissa & 0x1FFFFFFFL) == 0);
// }

// /* Returns a double precision value casted to single precision
//    (ignoring the least significant bits in exponent and mantissa).  */
// static int
// double_to_single (uint64_t v)
// {
//     unsigned int sign = (v >> 63) & 1;
//     int exp = (v >> 52) & 0x7FF;
//     uint64_t mantissa = v & 0xFFFFFFFFFFFFFULL;

//     if (exp == 0x7FF)
//         exp = 0xFF;
//     else {
//         exp = exp - 1023 + 127;
//         if (exp >= 0xFF) {
//             /* Infinity.  */
//             exp = 0x7F;
//             mantissa = 0;
//         }
//         else if (exp < 0) {
//             /* No denormalized numbers.  */
//             exp = 0;
//             mantissa = 0;
//         }
//     }
//     mantissa >>= 29;
//     return (sign << 31) | (exp << 23) | mantissa;
// }

// /* Invert low-order SIZE bits of XHI:XLO.  */
// static void
// neon_invert_size (unsigned *xlo, unsigned *xhi, int size)
// {
//     unsigned immlo = xlo ? *xlo : 0;
//     unsigned immhi = xhi ? *xhi : 0;

//     switch (size) {
//         case 8:
//             immlo = (~immlo) & 0xff;
//             break;

//         case 16:
//             immlo = (~immlo) & 0xffff;
//             break;

//         case 64:
//             immhi = (~immhi) & 0xffffffff;
//         /* fall through.  */

//         case 32:
//             immlo = (~immlo) & 0xffffffff;
//             break;

//         default:
//             abort ();
//     }

//     if (xlo)
//         *xlo = immlo;

//     if (xhi)
//         *xhi = immhi;
// }


/**
 * @Description: 移位操作编码到指令码
 */
static void
encode_arm_shift (int i) {
    // .imm是寄存器
    if (inst.operands[i].immisreg) {
        int op_index;
        for (op_index = 0; op_index <= i; ++op_index) {
            if (inst.operands[op_index].present && inst.operands[op_index].isreg
	            && inst.operands[op_index].reg == REG_PC) {
                as_warn (UNPRED_REG ("r15"));
            }
        }
        if (inst.operands[i].imm == REG_PC)
	        as_warn (UNPRED_REG ("r15"));
    }
    
    if (inst.operands[i].shift_kind == SHIFT_RRX)
        // 在指令码中将移位类型设置为指定的移位操作类型
        inst.instruction |= SHIFT_ROR << 5;
    else {
        inst.instruction |= inst.operands[i].shift_kind << 5;
        if (inst.operands[i].immisreg) {
            inst.instruction |= SHIFT_BY_REG;
            // 在指令码中将移位操作数设置为一个寄存器
            inst.instruction |= inst.operands[i].imm << 8;
        } else {
            // 需要进行相应的重定位操作
            inst.relocs[0].type = BFD_RELOC_ARM_SHIFT_IMM;
        }
    }
}

/**
 * @Description: 寄存器编号等信息存储到指令码中的特定位置
 */
static void
encode_arm_shifter_operand (int i) {
    if (inst.operands[i].isreg) {
        // 是寄存器 按位或
        inst.instruction |= inst.operands[i].reg;
        encode_arm_shift (i);
    } else {
        inst.instruction |= INST_IMMEDIATE;
        // 不需要重定位
        if (inst.relocs[0].type != BFD_RELOC_ARM_IMMEDIATE) {
            // 将立即数值存储到 inst.instruction
            inst.instruction |= inst.operands[i].imm;
        }
    }
}

/**
 * @Description: 将ARM指令中算术运算相关的操作编码到指令码中
 */
static void
do_arit (void) {
    constraint (inst.relocs[0].type >= BFD_RELOC_ARM_THUMB_ALU_ABS_G0_NC
	      && inst.relocs[0].type <= BFD_RELOC_ARM_THUMB_ALU_ABS_G3_NC ,
	      THUMB1_RELOC_ONLY);
    if (!inst.operands[1].present) {
        inst.operands[1].reg = inst.operands[0].reg;
    }
    inst.instruction |= inst.operands[0].reg << 12;
    inst.instruction |= inst.operands[1].reg << 16;
    // ADD R0, R1, R2, LSL #3
    encode_arm_shifter_operand (2);
}

/**
 * @Description: 处理 Thumb 汇编中带有三个操作数的算术指令
 */
static void
do_t_arit3c (void) {
    int Rd, Rs, Rn;
    Rd = inst.operands[0].reg;
    Rs = (inst.operands[1].present
        ? inst.operands[1].reg    /* Rd, Rs, foo */
        : inst.operands[0].reg);
    Rn = inst.operands[2].reg;
    reject_bad_reg (Rd);
    reject_bad_reg (Rs);
    if (inst.operands[2].isreg)
        reject_bad_reg (Rn);
    if (unified_syntax) {

    } else {
        constraint (THUMB_SETS_FLAGS (inst.instruction), BAD_THUMB32);
        constraint (!inst.operands[2].isreg || inst.operands[2].shifted,
            _("unshifted register required"));
        constraint (Rd > 7 || Rs > 7 || Rn > 7, BAD_HIREG);
        inst.instruction = THUMB_OP16 (inst.instruction);
        inst.instruction |= Rd;
        if (Rd == Rs)
	        inst.instruction |= Rn << 3;
        else if (Rd == Rn)
	        inst.instruction |= Rs << 3;
        else
	        constraint (1, _("dest must overlap one source register"));
    }
}

static void
do_t_add_sub (void) {
    int Rd, Rs, Rn;
    Rd = inst.operands[0].reg;
    Rs = (inst.operands[1].present
        ? inst.operands[1].reg    /* Rd, Rs, foo */
        : inst.operands[0].reg);  /* Rd, foo -> Rd, Rd, foo */
    if (Rd == REG_PC)
        set_pred_insn_type_last ();
    if (unified_syntax) {

    } else {
        constraint (inst.instruction == T_MNEM_adds
		  || inst.instruction == T_MNEM_subs,
		  BAD_THUMB32);
        /* Rd, Rs, #imm */
        if (!inst.operands[2].isreg) {
            constraint ((Rd > 7 && (Rd != REG_SP || Rs != REG_SP))
		      || (Rs > 7 && Rs != REG_SP && Rs != REG_PC),
		      BAD_HIREG);
            inst.instruction = (inst.instruction == T_MNEM_add
			      ? 0x0000 : 0x8000);
            inst.instruction |= (Rd << 4) | Rs;
            inst.relocs[0].type = BFD_RELOC_ARM_THUMB_ADD;
            return;
        }
        Rn = inst.operands[2].reg;
        constraint (inst.operands[2].shifted, _("unshifted register required"));

      /* We now have Rd, Rs, and Rn set to registers.  */
        if (Rd > 7 || Rs > 7 || Rn > 7) {
            /* Can't do this for SUB.	 */
            constraint (inst.instruction == T_MNEM_sub, BAD_HIREG);
            inst.instruction = T_OPCODE_ADD_HI;
            inst.instruction |= (Rd & 8) << 4;
            inst.instruction |= (Rd & 7);
            if (Rs == Rd)
                inst.instruction |= Rn << 3;
            else if (Rn == Rd)
                inst.instruction |= Rs << 3;
            else
                constraint (1, _("dest must overlap one source register"));
        } else {
            inst.instruction = (inst.instruction == T_MNEM_add
                        ? T_OPCODE_ADD_R3 : T_OPCODE_SUB_R3);
            inst.instruction |= Rd | (Rs << 3) | (Rn << 6);
        }
    }
}

/**
 * @Description: 解析 Thumb 指令中的 cpy 操作
 */
static void
do_t_cpy (void)
{
    if (inst.size_req == 4) {
        inst.instruction = THUMB_OP32 (T_MNEM_mov);
        inst.instruction |= inst.operands[0].reg << 8;
        inst.instruction |= inst.operands[1].reg;
    } else {
      inst.instruction |= (inst.operands[0].reg & 0x8) << 4;
      inst.instruction |= (inst.operands[0].reg & 0x7);
      inst.instruction |= inst.operands[1].reg << 3;
    }
}

/**
 * @Description: cmp指令
 */
static void
do_cmp (void)
{
  inst.instruction |= inst.operands[0].reg << 16;
  encode_arm_shifter_operand (1);
}

static void
do_t_mov_cmp (void)
{
    unsigned Rn, Rm;

    Rn = inst.operands[0].reg;
    Rm = inst.operands[1].reg;

    if (Rn == REG_PC)
        set_pred_insn_type_last ();

    
    inst.instruction = THUMB_OP16 (inst.instruction);

    /* PR 10443: Do not silently ignore shifted operands.  */
    constraint (inst.operands[1].shifted,
            _("shifts in CMP/MOV instructions are only supported in unified syntax"));

    if (inst.operands[1].isreg){
        if (Rn < 8 && Rm < 8) {
            /* A move of two lowregs is encoded as ADD Rd, Rs, #0
                since a MOV instruction produces unpredictable results.  */
            if (inst.instruction == T_OPCODE_MOV_I8)
                inst.instruction = T_OPCODE_ADD_I3;
            else
                inst.instruction = T_OPCODE_CMP_LR;

            inst.instruction |= Rn;
            inst.instruction |= Rm << 3;
        } else {
            if (inst.instruction == T_OPCODE_MOV_I8)
                inst.instruction = T_OPCODE_MOV_HR;
            else
                inst.instruction = T_OPCODE_CMP_HR;
            do_t_cpy ();
        }
    } else {
        constraint (Rn > 7,
            _("only lo regs allowed with immediate"));
        inst.instruction |= Rn << 8;
        inst.relocs[0].type = BFD_RELOC_ARM_THUMB_IMM;
    }
}

/**
 * @Description: mov指令编码进inst
 */
static void
do_mov (void)
{
    constraint (inst.relocs[0].type >= BFD_RELOC_ARM_THUMB_ALU_ABS_G0_NC
            && inst.relocs[0].type <= BFD_RELOC_ARM_THUMB_ALU_ABS_G3_NC ,
            THUMB1_RELOC_ONLY);
    inst.instruction |= inst.operands[0].reg << 12;
    encode_arm_shifter_operand (1);
}

static void
encode_thumb32_shifted_operand (int i) {
    unsigned int value = inst.relocs[0].exp.X_add_number;
    unsigned int shift = inst.operands[i].shift_kind;

    constraint (inst.operands[i].immisreg,
            _("shift by register not allowed in thumb mode"));
    inst.instruction |= inst.operands[i].reg;
    if (shift == SHIFT_RRX)
        inst.instruction |= SHIFT_ROR << 4;
    else {
        constraint (inst.relocs[0].exp.X_op != O_constant,
            _("expression too complex"));

        constraint (value > 32
            || (value == 32 && (shift == SHIFT_LSL
                        || shift == SHIFT_ROR)),
            _("shift expression is too large"));

        if (value == 0)
            shift = SHIFT_LSL;
        else if (value == 32)
            value = 0;

        inst.instruction |= shift << 4;
        inst.instruction |= (value & 0x1c) << 10;
        inst.instruction |= (value & 0x03) << 6;
    }
}

// rsb操作码
static void
do_t_rsb (void) {
    unsigned Rd, Rs;

    Rd = inst.operands[0].reg;
    Rs = (inst.operands[1].present
        ? inst.operands[1].reg    /* Rd, Rs, foo */
        : inst.operands[0].reg);  /* Rd, foo -> Rd, Rd, foo */

    reject_bad_reg (Rd);
    reject_bad_reg (Rs);
    if (inst.operands[2].isreg)
        reject_bad_reg (inst.operands[2].reg);

    inst.instruction |= Rd << 8;
    inst.instruction |= Rs << 16;
    if (!inst.operands[2].isreg) {
        bool narrow;

        if ((inst.instruction & 0x00100000) != 0)
            narrow = !in_pred_block ();
        else
            narrow = in_pred_block ();

        if (Rd > 7 || Rs > 7)
            narrow = false;

        if (inst.size_req == 4 || !unified_syntax)
            narrow = false;

        if (inst.relocs[0].exp.X_op != O_constant
            || inst.relocs[0].exp.X_add_number != 0)
            narrow = false;

        /* Turn rsb #0 into 16-bit neg.  We should probably do this via
        relaxation, but it doesn't seem worth the hassle.  */
        if (narrow) {
            inst.relocs[0].type = BFD_RELOC_UNUSED;
            inst.instruction = THUMB_OP16 (T_MNEM_negs);
            inst.instruction |= Rs << 3;
            inst.instruction |= Rd;
        } else {
            inst.instruction = (inst.instruction & 0xe1ffffff) | 0x10000000;
            inst.relocs[0].type = BFD_RELOC_ARM_T32_IMMEDIATE;
        }
    } else
        encode_thumb32_shifted_operand (2);
}

static bool in_my_get_expression = false;
/* Third argument to my_get_expression.	 */
#define GE_NO_PREFIX 0
#define GE_IMM_PREFIX 1
#define GE_OPT_PREFIX 2
/* This is a bit of a hack. Use an optional prefix, and also allow big (64-bit)
   immediates, as can be used in Neon VMVN and VMOV immediate instructions.  */
#define GE_OPT_PREFIX_BIG 3

static bool
walk_no_bignums (symbolS * sp)
{
    if (symbol_get_value_expression (sp)->X_op == O_big)
        return true;

    if (symbol_get_value_expression (sp)->X_add_symbol) {
        return (walk_no_bignums (symbol_get_value_expression (sp)->X_add_symbol)
            || (symbol_get_value_expression (sp)->X_op_symbol
            && walk_no_bignums (symbol_get_value_expression (sp)->X_op_symbol)));
    }

    return false;
}

static int
my_get_expression (expressionS * ep, char ** str, int prefix_mode) {
    char * save_in;
    // 处理#0
    /* In unified syntax, all prefixes are optional.  */
    if (unified_syntax)
        prefix_mode = (prefix_mode == GE_OPT_PREFIX_BIG) ? prefix_mode
            : GE_OPT_PREFIX;
    switch (prefix_mode) {
        case GE_NO_PREFIX: break;
        case GE_IMM_PREFIX:
            if (!is_immediate_prefix (**str)) {
                inst.error = _("immediate expression requires a # prefix");
                return FAIL;
            }
            (*str)++;
            break;
        case GE_OPT_PREFIX:
        case GE_OPT_PREFIX_BIG:
            if (is_immediate_prefix (**str))
                (*str)++;
            break;
        default:
            abort ();
    }

    memset (ep, 0, sizeof (expressionS));

    save_in = input_line_pointer;
    input_line_pointer = *str;
    in_my_get_expression = true;
    expression (ep);
    in_my_get_expression = false;

    if (ep->X_op == O_illegal || ep->X_op == O_absent) {
        /* We found a bad or missing expression in md_operand().  */
        *str = input_line_pointer;
        input_line_pointer = save_in;
        if (inst.error == NULL)
            inst.error = (ep->X_op == O_absent
                    ? _("missing expression") :_("bad expression"));
        return 1;
    }

    /* Get rid of any bignums now, so that we don't generate an error for which
        we can't establish a line number later on.	 Big numbers are never valid
        in instructions, which is where this routine is always called.  */
    if (prefix_mode != GE_OPT_PREFIX_BIG
      && (ep->X_op == O_big
	  || (ep->X_add_symbol
	      && (walk_no_bignums (ep->X_add_symbol)
		  || (ep->X_op_symbol
		      && walk_no_bignums (ep->X_op_symbol)))))) {
        inst.error = _("invalid constant");
        *str = input_line_pointer;
        input_line_pointer = save_in;
        return 1;
    }
    // ep->X_add_number存放了立即数
    *str = input_line_pointer;
    input_line_pointer = save_in;
    return SUCCESS;
}

static int
parse_fpa_immediate (char ** str)
{
  LITTLENUM_TYPE words[MAX_LITTLENUMS];
  char *	 save_in;
  expressionS	 exp;
  int		 i;
  int		 j;

  /* First try and match exact strings, this is to guarantee
     that some formats will work even for cross assembly.  */

  for (i = 0; fp_const[i]; i++)
    {
      if (strncmp (*str, fp_const[i], strlen (fp_const[i])) == 0)
	{
	  char *start = *str;

	  *str += strlen (fp_const[i]);
	  if (is_end_of_line[(unsigned char) **str])
	    return i + 8;
	  *str = start;
	}
    }

  /* Just because we didn't get a match doesn't mean that the constant
     isn't valid, just that it is in a format that we don't
     automatically recognize.  Try parsing it with the standard
     expression routines.  */

  memset (words, 0, MAX_LITTLENUMS * sizeof (LITTLENUM_TYPE));

  /* Look for a raw floating point number.  */
  if ((save_in = atof_ieee (*str, 'x', words)) != NULL
      && is_end_of_line[(unsigned char) *save_in])
    {
      for (i = 0; i < NUM_FLOAT_VALS; i++)
	{
	  for (j = 0; j < MAX_LITTLENUMS; j++)
	    {
	      if (words[j] != fp_values[i][j])
		break;
	    }

	  if (j == MAX_LITTLENUMS)
	    {
	      *str = save_in;
	      return i + 8;
	    }
	}
    }

  /* Try and parse a more complex expression, this will probably fail
     unless the code uses a floating point prefix (eg "0f").  */
  save_in = input_line_pointer;
  input_line_pointer = *str;
  if (expression (&exp) == absolute_section
      && exp.X_op == O_big
      && exp.X_add_number < 0)
    {
      /* FIXME: 5 = X_PRECISION, should be #define'd where we can use it.
	 Ditto for 15.	*/
#define X_PRECISION 5
#define E_PRECISION 15L
    //   if (gen_to_words (words, X_PRECISION, E_PRECISION) == 0)
	// {
	//   for (i = 0; i < NUM_FLOAT_VALS; i++)
	//     {
	//       for (j = 0; j < MAX_LITTLENUMS; j++)
	// 	{
	// 	  if (words[j] != fp_values[i][j])
	// 	    break;
	// 	}

	//       if (j == MAX_LITTLENUMS)
	// 	{
	// 	  *str = input_line_pointer;
	// 	  input_line_pointer = save_in;
	// 	  return i + 8;
	// 	}
	//     }
	// }
    
    }

  *str = input_line_pointer;
  input_line_pointer = save_in;
  inst.error = _("invalid FPA immediate expression");
  return FAIL;
}

/**
 * @Description: mul指令
 */
static void
do_mul (void)
{
    constraint (inst.operands[2].reg == REG_PC, BAD_PC);

    if (!inst.operands[2].present)
        inst.operands[2].reg = inst.operands[0].reg;
    inst.instruction |= inst.operands[0].reg << 16;
    inst.instruction |= inst.operands[1].reg;
    inst.instruction |= inst.operands[2].reg << 8;

    if (inst.operands[0].reg == inst.operands[1].reg
        && !ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v6))
        as_tsktsk (_("Rd and Rm should be different in mul"));
}

static void
do_t_mul (void)
{
    bool narrow;
    unsigned Rd, Rn, Rm;

    if (!inst.operands[2].present)
        inst.operands[2].reg = inst.operands[0].reg;

    Rd = inst.operands[0].reg;
    Rn = inst.operands[1].reg;
    Rm = inst.operands[2].reg;

    if (unified_syntax) {
        
    } else{
        constraint (inst.instruction == T_MNEM_muls, BAD_THUMB32);
        constraint (Rn > 7 || Rm > 7,
            BAD_HIREG);
        narrow = true;
    }

    if (narrow) {
        /* 16-bit MULS/Conditional MUL.  */
        inst.instruction = THUMB_OP16 (inst.instruction);
        inst.instruction |= Rd;

        if (Rd == Rn)
            inst.instruction |= Rm << 3;
        else if (Rd == Rm)
            inst.instruction |= Rn << 3;
        else
            constraint (1, _("dest must overlap one source register"));
    } else {
        constraint (inst.instruction != T_MNEM_mul,
            _("Thumb-2 MUL must not set flags"));
        /* 32-bit MUL.  */
        inst.instruction = THUMB_OP32 (inst.instruction);
        inst.instruction |= Rd << 8;
        inst.instruction |= Rn << 16;
        inst.instruction |= Rm << 0;

        reject_bad_reg (Rd);
        reject_bad_reg (Rn);
        reject_bad_reg (Rm);
    }
}

/**
 * @Description: 编码b指令
 */
static void
encode_branch (int default_reloc) {
    if (inst.operands[0].hasreloc) {
        constraint (inst.operands[0].imm != BFD_RELOC_ARM_PLT32
            && inst.operands[0].imm != BFD_RELOC_ARM_TLS_CALL,
            _("the only valid suffixes here are '(plt)' and '(tlscall)'"));
        inst.relocs[0].type = inst.operands[0].imm == BFD_RELOC_ARM_PLT32
        ? BFD_RELOC_ARM_PLT32
        : thumb_mode ? BFD_RELOC_ARM_THM_TLS_CALL : BFD_RELOC_ARM_TLS_CALL;
    }
    else
        inst.relocs[0].type = (bfd_reloc_code_real_type) default_reloc;
    inst.relocs[0].pc_rel = 1;
}

static void
do_t_branch (void)
{
    int opcode;
    int cond;
    bfd_reloc_code_real_type reloc;

    cond = inst.cond;
    set_pred_insn_type (IF_INSIDE_IT_LAST_INSN);

    // if (in_pred_block ()){
    //     /* Conditional branches inside IT blocks are encoded as unconditional
    //     branches.  */
    //     cond = COND_ALWAYS;
    // }
    // else
    //     cond = inst.cond;

    if (cond != COND_ALWAYS)
        opcode = T_MNEM_bcond;
    else
        opcode = inst.instruction;

    if (unified_syntax
        && (inst.size_req == 4
        || (inst.size_req != 2
            && (inst.operands[0].hasreloc
            || inst.relocs[0].exp.X_op == O_constant)))){
        inst.instruction = THUMB_OP32(opcode);
        if (cond == COND_ALWAYS)
            reloc = BFD_RELOC_THUMB_PCREL_BRANCH25;
        else {
            constraint (!ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v6t2),
                    _("selected architecture does not support "
                    "wide conditional branch instruction"));

            gas_assert (cond != 0xF);
            inst.instruction |= cond << 22;
            reloc = BFD_RELOC_THUMB_PCREL_BRANCH20;
        }
    } else {
        inst.instruction = THUMB_OP16(opcode);
        if (cond == COND_ALWAYS)
            reloc = BFD_RELOC_THUMB_PCREL_BRANCH12;
        else {
            inst.instruction |= cond << 8;
            reloc = BFD_RELOC_THUMB_PCREL_BRANCH9;
        }
        /* Allow section relaxation.  */
        if (unified_syntax && inst.size_req != 2)
            inst.relax = opcode;
    }
    inst.relocs[0].type = reloc;
    inst.relocs[0].pc_rel = 1;
}

/**
 * @Description: b 指令
 */
static void
do_branch (void)
{
#ifdef OBJ_ELF
  if (EF_ARM_EABI_VERSION (meabi_flags) >= EF_ARM_EABI_VER4)
    encode_branch (BFD_RELOC_ARM_PCREL_JUMP);
  else
#endif
    encode_branch (BFD_RELOC_ARM_PCREL_BRANCH);
}

/**
 * @Description: bl指令
 */
static void
do_bl (void)
{
#ifdef OBJ_ELF
    if (EF_ARM_EABI_VERSION (meabi_flags) >= EF_ARM_EABI_VER4) {
        if (inst.cond == COND_ALWAYS)
            encode_branch (BFD_RELOC_ARM_PCREL_CALL);
        else
            encode_branch (BFD_RELOC_ARM_PCREL_JUMP);
    }
    else
#endif
        encode_branch (BFD_RELOC_ARM_PCREL_BRANCH);
}

static void
do_t_branch23 (void)
{
  set_pred_insn_type_last ();
  encode_branch (BFD_RELOC_THUMB_PCREL_BRANCH23);

  /* md_apply_fix blows up with 'bl foo(PLT)' where foo is defined in
     this file.  We used to simply ignore the PLT reloc type here --
     the branch encoding is now needed to deal with TLSCALL relocs.
     So if we see a PLT reloc now, put it back to how it used to be to
     keep the preexisting behaviour.  */
  if (inst.relocs[0].type == BFD_RELOC_ARM_PLT32)
    inst.relocs[0].type = BFD_RELOC_THUMB_PCREL_BRANCH23;
}

/**
 * @Description: 移位操作
 */
static void
do_shift (void)
{
    unsigned int Rm = (inst.operands[1].present
                ? inst.operands[1].reg
                : inst.operands[0].reg);

    inst.instruction |= inst.operands[0].reg << 12;
    inst.instruction |= Rm;
    if (inst.operands[2].isreg) { /* Rd, {Rm,} Rs */
        inst.instruction |= inst.operands[2].reg << 8;
        inst.instruction |= SHIFT_BY_REG;
        /* PR 12854: Error on extraneous shifts.  */
        constraint (inst.operands[2].shifted,
            _("extraneous shift as part of operand to shift insn"));
    }
    else
        inst.relocs[0].type = BFD_RELOC_ARM_SHIFT_IMM;
}

static void
do_t_shift (void)
{
    if (!inst.operands[1].present)
        inst.operands[1].reg = inst.operands[0].reg;

    if (unified_syntax){
    } else {
        constraint (inst.operands[0].reg > 7
            || inst.operands[1].reg > 7, BAD_HIREG);
        constraint (THUMB_SETS_FLAGS (inst.instruction), BAD_THUMB32);

        if (inst.operands[2].isreg) {  /* Rd, {Rs,} Rn */
            constraint (inst.operands[2].reg > 7, BAD_HIREG);
            constraint (inst.operands[0].reg != inst.operands[1].reg,
                    _("source1 and dest must be same register"));

            switch (inst.instruction) {
                case T_MNEM_asr: inst.instruction = T_OPCODE_ASR_R; break;
                case T_MNEM_lsl: inst.instruction = T_OPCODE_LSL_R; break;
                case T_MNEM_lsr: inst.instruction = T_OPCODE_LSR_R; break;
                case T_MNEM_ror: inst.instruction = T_OPCODE_ROR_R; break;
                default: abort ();
            }

            inst.instruction |= inst.operands[0].reg;
            inst.instruction |= inst.operands[2].reg << 3;

            /* PR 12854: Error on extraneous shifts.  */
            constraint (inst.operands[2].shifted,
                    _("extraneous shift as part of operand to shift insn"));
        } else {
            switch (inst.instruction) {
                case T_MNEM_asr: inst.instruction = T_OPCODE_ASR_I; break;
                case T_MNEM_lsl: inst.instruction = T_OPCODE_LSL_I; break;
                case T_MNEM_lsr: inst.instruction = T_OPCODE_LSR_I; break;
                case T_MNEM_ror: inst.error = _("ror #imm not supported"); return;
                default: abort ();
            }
            inst.relocs[0].type = BFD_RELOC_ARM_THUMB_SHIFT;
            inst.instruction |= inst.operands[0].reg;
            inst.instruction |= inst.operands[1].reg << 3;
        }
    }
}

/**
 * @Description: 将一个无符号整数编码为Thumb32指令的立即数形式
 */
static unsigned int
encode_thumb32_immediate (unsigned int val)
{
    unsigned int a, i;

    if (val <= 0xff)
        return val;

    for (i = 1; i <= 24; i++) {
        a = val >> i;
        if ((val & ~(0xffU << i)) == 0)
        return ((val >> i) & 0x7f) | ((32 - i) << 7);
    }

    a = val & 0xff;
    if (val == ((a << 16) | a))
        return 0x100 | a;
    if (val == ((a << 24) | (a << 16) | (a << 8) | a))
        return 0x300 | a;

    a = val & 0xff00;
    if (val == ((a << 16) | a))
        return 0x200 | (a >> 8);

    return FAIL;
}

static unsigned int
thumb32_negate_data_op (valueT *instruction, unsigned int value)
{
    unsigned int op, new_inst;
    unsigned int rd;
    unsigned int negated, inverted;

    negated = encode_thumb32_immediate (-value);
    inverted = encode_thumb32_immediate (~value);

    rd = (*instruction >> 8) & 0xf;
    op = (*instruction >> T2_DATA_OP_SHIFT) & 0xf;
    switch (op) {
        /* ADD <-> SUB.  Includes CMP <-> CMN.  */
        case T2_OPCODE_SUB:
            new_inst = T2_OPCODE_ADD;
            value = negated;
            break;

        case T2_OPCODE_ADD:
            new_inst = T2_OPCODE_SUB;
            value = negated;
            break;

        /* ORR <-> ORN.  Includes MOV <-> MVN.  */
        case T2_OPCODE_ORR:
            new_inst = T2_OPCODE_ORN;
            value = inverted;
            break;

        case T2_OPCODE_ORN:
            new_inst = T2_OPCODE_ORR;
            value = inverted;
            break;

        /* AND <-> BIC.  TST has no inverted equivalent.  */
        case T2_OPCODE_AND:
            new_inst = T2_OPCODE_BIC;
            if (rd == 15)
                value = FAIL;
            else
                value = inverted;
            break;

        case T2_OPCODE_BIC:
            new_inst = T2_OPCODE_AND;
            value = inverted;
            break;

        /* ADC <-> SBC  */
        case T2_OPCODE_ADC:
            new_inst = T2_OPCODE_SBC;
            value = inverted;
            break;

        case T2_OPCODE_SBC:
            new_inst = T2_OPCODE_ADC;
            value = inverted;
            break;

        /* We cannot do anything.	 */
        default:
            return FAIL;
    }

    if (value == (unsigned int)FAIL)
        return FAIL;

    *instruction &= T2_OPCODE_MASK;
    *instruction |= new_inst << T2_DATA_OP_SHIFT;
    return value;
}


/**
 * @Description: 将一个无符号整数编码为ARM指令的立即数形式
 */
static unsigned int
encode_arm_immediate (unsigned int val)
{
    unsigned int a, i;

    if (val <= 0xff)
        return val;

    for (i = 2; i < 32; i += 2)
        if ((a = rotate_left (val, i)) <= 0xff)
        return a | (i << 7); /* 12-bit pack: [shift-cnt,const].  */

    return FAIL;
}

static unsigned int
validate_immediate_twopart (unsigned int   val,
			    unsigned int * highpart) {
    unsigned int a;
    unsigned int i;

    for (i = 0; i < 32; i += 2)
        if (((a = rotate_left (val, i)) & 0xff) != 0) {
            if (a & 0xff00) {
                if (a & ~ 0xffff)
                continue;
                * highpart = (a  >> 8) | ((i + 24) << 7);
            }
            else if (a & 0xff0000) {
                if (a & 0xff000000)
                continue;
                * highpart = (a >> 16) | ((i + 16) << 7);
            }
            else {
                gas_assert (a & 0xff000000);
                * highpart = (a >> 24) | ((i + 8) << 7);
            }

            return (a & 0xff) | (i << 7);
        }

    return FAIL;
}

static int
validate_offset_imm (unsigned int val, int hwse) {
    if ((hwse && val > 255) || val > 4095)
        return FAIL;
    return val;
}
/**
 * @Description: 描述了一个"=expr"加载伪操作
 */
static bool
move_or_literal_pool (int i, enum lit_type t, bool mode_3) {
    unsigned long tbit;
    bool thumb_p = (t == CONST_THUMB);
    bool arm_p   = (t == CONST_ARM);
    if (thumb_p)
        tbit = (inst.instruction > 0xffff) ? THUMB2_LOAD_BIT : THUMB_LOAD_BIT;
    else
        tbit = LOAD_BIT;
    if ((inst.instruction & tbit) == 0) {
        inst.error = _("invalid pseudo operation");
        return true;
    }
    if (inst.relocs[0].exp.X_op != O_constant
      && inst.relocs[0].exp.X_op != O_symbol
      && inst.relocs[0].exp.X_op != O_big) {
        inst.error = _("constant expression expected");
        return true;
    }
    if (inst.relocs[0].exp.X_op == O_constant
      || inst.relocs[0].exp.X_op == O_big) {
        uint64_t v;
        if (inst.relocs[0].exp.X_op == O_big) {
            LITTLENUM_TYPE w[X_PRECISION];
            LITTLENUM_TYPE * l;

            if (inst.relocs[0].exp.X_add_number == -1){
                gen_to_words (w, X_PRECISION, E_PRECISION);
                l = w;
                /* FIXME: Should we check words w[2..5] ?  */
            } else
	            l = generic_bignum;

            v = l[3] & LITTLENUM_MASK;
            v <<= LITTLENUM_NUMBER_OF_BITS;
            v |= l[2] & LITTLENUM_MASK;
            v <<= LITTLENUM_NUMBER_OF_BITS;
            v |= l[1] & LITTLENUM_MASK;
            v <<= LITTLENUM_NUMBER_OF_BITS;
            v |= l[0] & LITTLENUM_MASK;
	    } else
	        v = inst.relocs[0].exp.X_add_number;

        if (!inst.operands[i].issingle) {
            if (thumb_p) {
            /* LDR should not use lead in a flag-setting instruction being
            chosen so we do not check whether movs can be used.  */

                if ((ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v6t2)
                || ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v6t2_v8m))
                && inst.operands[i].reg != 13
                && inst.operands[i].reg != 15) {
                    /* Check if on thumb2 it can be done with a mov.w, mvn or
                        movw instruction.  */
                    unsigned int newimm;
                    bool isNegated = false;

                    newimm = encode_thumb32_immediate (v);
                    if (newimm == (unsigned int) FAIL) {
                        newimm = encode_thumb32_immediate (~v);
                        isNegated = true;
                    }

                    /* The number can be loaded with a mov.w or mvn
                        instruction.  */
                    if (newimm != (unsigned int) FAIL
                        && ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v6t2)) {
                        inst.instruction = (0xf04f0000  /*  MOV.W.  */
                                | (inst.operands[i].reg << 8));
                        /* Change to MOVN.  */
                        inst.instruction |= (isNegated ? 0x200000 : 0);
                        inst.instruction |= (newimm & 0x800) << 15;
                        inst.instruction |= (newimm & 0x700) << 4;
                        inst.instruction |= (newimm & 0x0ff);
                        return true;
                    }
                    /* The number can be loaded with a movw instruction.  */
                    else if ((v & ~0xFFFF) == 0
                        && ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v6t2_v8m)) {
                        int imm = v & 0xFFFF;

                        inst.instruction = 0xf2400000;  /* MOVW.  */
                        inst.instruction |= (inst.operands[i].reg << 8);
                        inst.instruction |= (imm & 0xf000) << 4;
                        inst.instruction |= (imm & 0x0800) << 15;
                        inst.instruction |= (imm & 0x0700) << 4;
                        inst.instruction |= (imm & 0x00ff);
                        /*  In case this replacement is being done on Armv8-M
                        Baseline we need to make sure to disable the
                        instruction size check, as otherwise GAS will reject
                        the use of this T32 instruction.  */
                        inst.size_req = 0;
                        return true;
                    }
                }
            }
            else if (arm_p) {
                int value = encode_arm_immediate (v);

                if (value != FAIL) {
                    /* This can be done with a mov instruction.  */
                    inst.instruction &= LITERAL_MASK;
                    inst.instruction |= INST_IMMEDIATE | (OPCODE_MOV << DATA_OP_SHIFT);
                    inst.instruction |= value & 0xfff;
                    return true;
                }

                value = encode_arm_immediate (~ v);
                if (value != FAIL) {
                    /* This can be done with a mvn instruction.  */
                    inst.instruction &= LITERAL_MASK;
                    inst.instruction |= INST_IMMEDIATE | (OPCODE_MVN << DATA_OP_SHIFT);
                    inst.instruction |= value & 0xfff;
                    return true;
                }
            } 
        }
    }

    if (add_to_lit_pool ((!inst.operands[i].isvec
			|| inst.operands[i].issingle) ? 4 : 8) == FAIL)
        return true;

    inst.operands[1].reg = REG_PC;
    inst.operands[1].isreg = 1;
    inst.operands[1].preind = 1;
    inst.relocs[0].pc_rel = 1;
    inst.relocs[0].type = (thumb_p
                ? BFD_RELOC_ARM_THUMB_OFFSET
                : (mode_3
                ? BFD_RELOC_ARM_HWLITERAL
                : BFD_RELOC_ARM_LITERAL));
    return false;
}
/**
 * @Description: 在列表中只有一个寄存器-返回它的寄存器号。否则返回-1
 */
static int
only_one_reg_in_list (int range)
{
  int i = ffs (range) - 1;
  return (i > 15 || range != (1 << i)) ? -1 : i;
}

/**
 * @Description: 帮助push/pop and ldm/stm
 */
static void
encode_thumb2_multi (bool do_io, int base, unsigned mask,
		     bool writeback) {
    bool load, store;

    gas_assert (base != -1 || !do_io);
    load = do_io && ((inst.instruction & (1 << 20)) != 0);
    store = do_io && !load;

    if (mask & (1 << 13))
        inst.error =  _("SP not allowed in register list");

    if (do_io && (mask & (1 << base)) != 0
        && writeback)
        inst.error = _("having the base register in the register list when "
            "using write back is UNPREDICTABLE");

    if (load) {
        if (mask & (1 << 15)) {
            if (mask & (1 << 14))
                inst.error = _("LR and PC should not both be in register list");
            else
                set_pred_insn_type_last ();
        }
    }
    else if (store) {
        if (mask & (1 << 15))
        inst.error = _("PC not allowed in register list");
    }

    if (do_io && ((mask & (mask - 1)) == 0)) {
        /* Single register transfers implemented as str/ldr.  */
        if (writeback) {
            if (inst.instruction & (1 << 23))
                inst.instruction = 0x00000b04; /* ia! -> [base], #4 */
            else
                inst.instruction = 0x00000d04; /* db! -> [base, #-4]! */
        }
        else {
            if (inst.instruction & (1 << 23))
                inst.instruction = 0x00800000; /* ia -> [base] */
            else
                inst.instruction = 0x00000c04; /* db -> [base, #-4] */
        }

        inst.instruction |= 0xf8400000;
        if (load)
            inst.instruction |= 0x00100000;

        mask = ffs (mask) - 1;
        mask <<= 12;
    } else if (writeback)
        inst.instruction |= WRITE_BACK;

    inst.instruction |= mask;
    if (do_io)
        inst.instruction |= base << 16;
}
/**
 * @Description: 将堆栈操作编码到指令码中
 */
static void
encode_ldmstm(int from_push_pop_mnem)
{
    int base_reg = inst.operands[0].reg;
    int range = inst.operands[1].imm;
    int one_reg;

    inst.instruction |= base_reg << 16;
    inst.instruction |= range;

    if (inst.operands[1].writeback)
        inst.instruction |= LDM_TYPE_2_OR_3;

    if (inst.operands[0].writeback) {
        inst.instruction |= WRITE_BACK;
        /* Check for unpredictable uses of writeback.  */
        if (inst.instruction & LOAD_BIT) {
        /* Not allowed in LDM type 2.	 */
            if ((inst.instruction & LDM_TYPE_2_OR_3)
                && ((range & (1 << REG_PC)) == 0))
                as_warn (_("writeback of base register is UNPREDICTABLE"));
            /* Only allowed if base reg not in list for other types.  */
            else if (range & (1 << base_reg))
                as_warn (_("writeback of base register when in register list is UNPREDICTABLE"));
        } else {/* STM.  */
            /* Not allowed for type 2.  */
            if (inst.instruction & LDM_TYPE_2_OR_3)
                as_warn (_("writeback of base register is UNPREDICTABLE"));
            /* Only allowed if base reg not in list, or first in list.  */
            else if ((range & (1 << base_reg))
                && (range & ((1 << base_reg) - 1)))
                as_warn (_("if writeback register is in list, it must be the lowest reg in the list"));
        }
    }

    /* If PUSH/POP has only one register, then use the A2 encoding.  */
    one_reg = only_one_reg_in_list (range);
    if (from_push_pop_mnem && one_reg >= 0){
        int is_push = (inst.instruction & A_PUSH_POP_OP_MASK) == A1_OPCODE_PUSH;

        if (is_push && one_reg == 13 /* SP */)
        /* PR 22483: The A2 encoding cannot be used when
        pushing the stack pointer as this is UNPREDICTABLE.  */
            return;

        inst.instruction &= A_COND_MASK;
        inst.instruction |= is_push ? A2_OPCODE_PUSH : A2_OPCODE_POP;
        inst.instruction |= one_reg << 12;
    }
}

/* Subroutine of encode_arm_addr_mode_2 and encode_arm_addr_mode_3.  */
static void
encode_arm_addr_mode_common (int i, bool is_t)
{
    /* PR 14260:
        Generate an error if the operand is not a register.  */
    constraint (!inst.operands[i].isreg,
            _("Instruction does not support =N addresses"));

    inst.instruction |= inst.operands[i].reg << 16;

    if (inst.operands[i].preind) {
        if (is_t) {
            inst.error = _("instruction does not accept preindexed addressing");
            return;
        }
        inst.instruction |= PRE_INDEX;
        if (inst.operands[i].writeback)
            inst.instruction |= WRITE_BACK;
    }
    else if (inst.operands[i].postind) {
        gas_assert (inst.operands[i].writeback);
        if (is_t)
            inst.instruction |= WRITE_BACK;
    }
    else { /* unindexed - only for coprocessor */
        inst.error = _("instruction does not accept unindexed addressing");
        return;
    }

    if (((inst.instruction & WRITE_BACK) || !(inst.instruction & PRE_INDEX))
        && (((inst.instruction & 0x000f0000) >> 16)
        == ((inst.instruction & 0x0000f000) >> 12)))
        as_warn ((inst.instruction & LOAD_BIT)
            ? _("destination register same as write-back base")
            : _("source register same as write-back base"));
}
/**
 * @Description: 将解析得到的ARM指令中的操作数编码为ARM格式的模式2加载或存储指令
 */
static void
encode_arm_addr_mode_2 (int i, bool is_t) {
    const bool is_pc = (inst.operands[i].reg == REG_PC);
    encode_arm_addr_mode_common (i, is_t);
    if (inst.operands[i].immisreg) {
        constraint ((inst.operands[i].imm == REG_PC
            || (is_pc && inst.operands[i].writeback)),
            BAD_PC_ADDRESSING);
        inst.instruction |= INST_IMMEDIATE;  /* yes, this is backwards */
        inst.instruction |= inst.operands[i].imm;
        if (!inst.operands[i].negative)
        inst.instruction |= INDEX_UP;
        if (inst.operands[i].shifted) {
            if (inst.operands[i].shift_kind == SHIFT_RRX)
                inst.instruction |= SHIFT_ROR << 5;
            else {
                inst.instruction |= inst.operands[i].shift_kind << 5;
                inst.relocs[0].type = BFD_RELOC_ARM_SHIFT_IMM;
            }
        }
    }
    else { /* immediate offset in inst.relocs[0] */
        if (is_pc && !inst.relocs[0].pc_rel) {
            const bool is_load = ((inst.instruction & LOAD_BIT) != 0);

            /* If is_t is TRUE, it's called from do_ldstt.  ldrt/strt
                cannot use PC in addressing.
                PC cannot be used in writeback addressing, either.  */
            constraint ((is_t || inst.operands[i].writeback),
                    BAD_PC_ADDRESSING);

            /* Use of PC in str is deprecated for ARMv7.  */
            if (warn_on_deprecated
                && !is_load
                && ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v7))
                as_tsktsk (_("use of PC in this instruction is deprecated"));
        }

      if (inst.relocs[0].type == BFD_RELOC_UNUSED) {
        /* Prefer + for zero encoded value.  */
        if (!inst.operands[i].negative)
            inst.instruction |= INDEX_UP;
        inst.relocs[0].type = BFD_RELOC_ARM_OFFSET_IMM;
        }
    }
}
static void
do_t_push_pop (void)
{
    unsigned mask;

    constraint (inst.operands[0].writeback,
            _("push/pop do not support {reglist}^"));
    constraint (inst.relocs[0].type != BFD_RELOC_UNUSED,
            _("expression too complex"));

    mask = inst.operands[0].imm;
    if (inst.size_req != 4 && (mask & ~0xff) == 0)
        inst.instruction = THUMB_OP16 (inst.instruction) | mask;
    else if (inst.size_req != 4
        && (mask & ~0xff) == (1U << (inst.instruction == T_MNEM_push
                        ? REG_LR : REG_PC))) {
        inst.instruction = THUMB_OP16 (inst.instruction);
        inst.instruction |= THUMB_PP_PC_LR;
        inst.instruction |= mask & 0xff;
    }
    else if (unified_syntax) {
        inst.instruction = THUMB_OP32 (inst.instruction);
        encode_thumb2_multi (true /* do_io */, 13, mask, true);
    }
    else {
        inst.error = _("invalid register list to push/pop instruction");
        return;
    }
}

/**
 * @Description: push,pop操作
 */
static void
do_push_pop (void)
{
    constraint (inst.operands[0].writeback,
            _("push/pop do not support {reglist}^"));
    inst.operands[1] = inst.operands[0];
    memset (&inst.operands[0], 0, sizeof inst.operands[0]);
    inst.operands[0].isreg = 1;
    inst.operands[0].writeback = 1;
    inst.operands[0].reg = REG_SP;
    encode_ldmstm (/*from_push_pop_mnem=*/true);
}

/**
 * @Description: ldm相关指令
 */
static void
do_ldmstm (void)
{
  encode_ldmstm (/*from_push_pop_mnem=*/false);
}

static void
do_t_ldmstm (void)
{
    /* This really doesn't seem worth it.  */
    constraint (inst.relocs[0].type != BFD_RELOC_UNUSED,
            _("expression too complex"));
    constraint (inst.operands[1].writeback,
            _("Thumb load/store multiple does not support {reglist}^"));

    if (unified_syntax) {
        
    } else {
        constraint (inst.operands[0].reg > 7
            || (inst.operands[1].imm & ~0xff), BAD_HIREG);
        constraint (inst.instruction != T_MNEM_ldmia
            && inst.instruction != T_MNEM_stmia,
            _("Thumb-2 instruction only valid in unified syntax"));
        if (inst.instruction == T_MNEM_stmia) {
            if (!inst.operands[0].writeback)
                as_warn (_("this instruction will write back the base register"));
            if ((inst.operands[1].imm & (1 << inst.operands[0].reg))
                && (inst.operands[1].imm & ((1 << inst.operands[0].reg) - 1)))
                as_warn (_("value stored for r%d is UNKNOWN"),
                    inst.operands[0].reg);
        }
        else {
            if (!inst.operands[0].writeback
                && !(inst.operands[1].imm & (1 << inst.operands[0].reg)))
                as_warn (_("this instruction will write back the base register"));
            else if (inst.operands[0].writeback
                && (inst.operands[1].imm & (1 << inst.operands[0].reg)))
                as_warn (_("this instruction will not write back the base register"));
        }

        inst.instruction = THUMB_OP16 (inst.instruction);
        inst.instruction |= inst.operands[0].reg << 8;
        inst.instruction |= inst.operands[1].imm;
    }
}

/* 检查ARM and thumb 汇编指令中的"ldr pc, #imm"指令是否对齐到4字节边界  */
static void
check_ldr_r15_aligned (void)
{
    constraint (!(inst.operands[1].immisreg)
            && (inst.operands[0].reg == REG_PC
            && inst.operands[1].reg == REG_PC
            && (inst.relocs[0].exp.X_add_number & 0x3)),
            _("ldr to register 15 must be 4-byte aligned"));
}
/**
 * @Description: ldr等涉及内存指令
 */
static void
do_ldst (void)
{
    inst.instruction |= inst.operands[0].reg << 12;
    if (!inst.operands[1].isreg)
        if (move_or_literal_pool (0, CONST_ARM, /*mode_3=*/false))
            return;
    encode_arm_addr_mode_2 (1, /*is_t=*/false);
    check_ldr_r15_aligned ();
}
static void
do_t_ldst (void)
{
    unsigned long opcode;
    int Rn;

    if (inst.operands[0].isreg
        && !inst.operands[0].preind
        && inst.operands[0].reg == REG_PC)
        set_pred_insn_type_last ();

    opcode = inst.instruction;

    constraint (inst.operands[0].reg > 7, BAD_HIREG);

    if (inst.instruction == T_MNEM_ldrsh || inst.instruction == T_MNEM_ldrsb) {
        /* Only [Rn,Rm] is acceptable.  */
        constraint (inst.operands[1].reg > 7 || inst.operands[1].imm > 7, BAD_HIREG);
        constraint (!inst.operands[1].isreg || !inst.operands[1].immisreg
            || inst.operands[1].postind || inst.operands[1].shifted
            || inst.operands[1].negative,
            _("Thumb does not support this addressing mode"));
        inst.instruction = THUMB_OP16 (inst.instruction);
        goto op16;
    }

    inst.instruction = THUMB_OP16 (inst.instruction);
    if (!inst.operands[1].isreg)
        if (move_or_literal_pool (0, CONST_THUMB, /*mode_3=*/false))
            return;

    constraint (!inst.operands[1].preind
	      || inst.operands[1].shifted
	      || inst.operands[1].writeback,
	      _("Thumb does not support this addressing mode"));
    if (inst.operands[1].reg == REG_PC || inst.operands[1].reg == REG_SP) {
        constraint (inst.instruction & 0x0600,
            _("byte or halfword not valid for base register"));
        constraint (inst.operands[1].reg == REG_PC
            && !(inst.instruction & THUMB_LOAD_BIT),
            _("r15 based store not allowed"));
        constraint (inst.operands[1].immisreg,
            _("invalid base register for register offset"));

        if (inst.operands[1].reg == REG_PC)
            inst.instruction = T_OPCODE_LDR_PC;
        else if (inst.instruction & THUMB_LOAD_BIT)
            inst.instruction = T_OPCODE_LDR_SP;
        else
            inst.instruction = T_OPCODE_STR_SP;

        inst.instruction |= inst.operands[0].reg << 8;
        inst.relocs[0].type = BFD_RELOC_ARM_THUMB_OFFSET;
        return;
    }

    constraint (inst.operands[1].reg > 7, BAD_HIREG);
    if (!inst.operands[1].immisreg) {
        /* Immediate offset.  */
        inst.instruction |= inst.operands[0].reg;
        inst.instruction |= inst.operands[1].reg << 3;
        inst.relocs[0].type = BFD_RELOC_ARM_THUMB_OFFSET;
        return;
    }

    /* Register offset.  */
    constraint (inst.operands[1].imm > 7, BAD_HIREG);
    constraint (inst.operands[1].negative,
            _("Thumb does not support this addressing mode"));

    op16:
    switch (inst.instruction){
        case T_OPCODE_STR_IW: inst.instruction = T_OPCODE_STR_RW; break;
        case T_OPCODE_STR_IH: inst.instruction = T_OPCODE_STR_RH; break;
        case T_OPCODE_STR_IB: inst.instruction = T_OPCODE_STR_RB; break;
        case T_OPCODE_LDR_IW: inst.instruction = T_OPCODE_LDR_RW; break;
        case T_OPCODE_LDR_IH: inst.instruction = T_OPCODE_LDR_RH; break;
        case T_OPCODE_LDR_IB: inst.instruction = T_OPCODE_LDR_RB; break;
        case 0x5600 /* ldrsb */:
        case 0x5e00 /* ldrsh */: break;
        default: abort ();
    }

    inst.instruction |= inst.operands[0].reg;
    inst.instruction |= inst.operands[1].reg << 3;
    inst.instruction |= inst.operands[1].imm << 6;
}

/* 将ARM地址模式编码为ARM格式的模式3加载或存储指令的函数 */
static void
encode_arm_addr_mode_3 (int i, bool is_t)
{
    if (inst.operands[i].immisreg && inst.operands[i].shifted) {
        inst.error = _("instruction does not accept scaled register index");
        return;
    }

    encode_arm_addr_mode_common (i, is_t);

    if (inst.operands[i].immisreg) {
        constraint ((inst.operands[i].imm == REG_PC
            || (is_t && inst.operands[i].reg == REG_PC)),
            BAD_PC_ADDRESSING);
        constraint (inst.operands[i].reg == REG_PC && inst.operands[i].writeback,
            BAD_PC_WRITEBACK);
        inst.instruction |= inst.operands[i].imm;
        if (!inst.operands[i].negative)
            inst.instruction |= INDEX_UP;
    } else { /* immediate offset in inst.relocs[0] */
        constraint ((inst.operands[i].reg == REG_PC && !inst.relocs[0].pc_rel
            && inst.operands[i].writeback),
            BAD_PC_WRITEBACK);
        inst.instruction |= HWOFFSET_IMM;
      if (inst.relocs[0].type == BFD_RELOC_UNUSED) {
        /* Prefer + for zero encoded value.  */
        if (!inst.operands[i].negative)
            inst.instruction |= INDEX_UP;

            inst.relocs[0].type = BFD_RELOC_ARM_OFFSET_IMM8;
        }
    }
}


/**
 * @Description: ldrh
 */
static void
do_ldstv4 (void)
{
    constraint (inst.operands[0].reg == REG_PC, BAD_PC);
    inst.instruction |= inst.operands[0].reg << 12;
    if (!inst.operands[1].isreg)
        if (move_or_literal_pool (0, CONST_ARM, /*mode_3=*/true))
            return;
    encode_arm_addr_mode_3 (1, /*is_t=*/false);
}

/**
 * @Description: bx指令
 */
static void
do_bx (void)
{
    bool want_reloc;

    if (inst.operands[0].reg == REG_PC)
        as_tsktsk (_("use of r15 in bx in ARM mode is not really useful"));

    inst.instruction |= inst.operands[0].reg;
    /* Output R_ARM_V4BX relocations if is an EABI object that looks like
        it is for ARMv4t or earlier.  */
    want_reloc = !ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v5);
    if (!ARM_FEATURE_ZERO (selected_object_arch)
        && !ARM_CPU_HAS_FEATURE (selected_object_arch, arm_ext_v5))
        want_reloc = true;

    #ifdef OBJ_ELF
    if (EF_ARM_EABI_VERSION (meabi_flags) < EF_ARM_EABI_VER4)
    #endif
        want_reloc = false;

    if (want_reloc)
        inst.relocs[0].type = BFD_RELOC_ARM_V4BX;
}

static void
do_t_bx (void)
{
    set_pred_insn_type_last ();
    inst.instruction |= inst.operands[0].reg << 3;
    /* ??? FIXME: Should add a hacky reloc here if reg is REG_PC.	 The reloc
        should cause the alignment to be checked once it is known.	 This is
        because BX PC only works if the instruction is word aligned.  */
}

/**
 * @Description: swi指令
 */
static void
do_swi (void)
{
    inst.relocs[0].type = BFD_RELOC_ARM_SWI;
    inst.relocs[0].pc_rel = 0;
}
static void
do_t_swi (void)
{
    inst.relocs[0].type = BFD_RELOC_ARM_SWI;
}

static void
do_nop (void) {
    if (inst.operands[0].present
        || ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v6k)) {
        /* Architectural NOP hints are CPSR sets with no bits selected.  */
        inst.instruction &= 0xf0000000;
        inst.instruction |= 0x0320f000;
        if (inst.operands[0].present)
            inst.instruction |= inst.operands[0].imm;
    }
}

static void
do_t_nop (void) {
    set_pred_insn_type (NEUTRAL_IT_INSN);

    if (unified_syntax) {
        if (inst.size_req == 4 || inst.operands[0].imm > 15) {
            inst.instruction = THUMB_OP32 (inst.instruction);
            inst.instruction |= inst.operands[0].imm;
        } else {
            /* PR9722: Check for Thumb2 availability before
                generating a thumb2 nop instruction.  */
            if (ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v6t2)) {
                inst.instruction = THUMB_OP16 (inst.instruction);
                inst.instruction |= inst.operands[0].imm << 4;
            } else
                inst.instruction = 0x46c0;
        }
    } else {
        constraint (inst.operands[0].present,
            _("Thumb does not support NOP with hints"));
        inst.instruction = 0x46c0;
    }
}

static void
do_t_mvn_tst (void) {
    unsigned Rn, Rm;

    Rn = inst.operands[0].reg;
    Rm = inst.operands[1].reg;

    if (inst.instruction == T_MNEM_cmp
        || inst.instruction == T_MNEM_cmn)
        constraint (Rn == REG_PC, BAD_PC);
    else
        reject_bad_reg (Rn);
    reject_bad_reg (Rm);

    constraint (inst.instruction > 0xffff
		  || inst.instruction == T_MNEM_mvns, BAD_THUMB32);
    constraint (!inst.operands[1].isreg || inst.operands[1].shifted,
        _("unshifted register required"));
    constraint (Rn > 7 || Rm > 7,
        BAD_HIREG);

    inst.instruction = THUMB_OP16 (inst.instruction);
    inst.instruction |= Rn;
    inst.instruction |= Rm << 3;
}

// 遇到smull
do_mull (void) {
    inst.instruction |= inst.operands[0].reg << 12;
    inst.instruction |= inst.operands[1].reg << 16;
    inst.instruction |= inst.operands[2].reg;
    inst.instruction |= inst.operands[3].reg << 8;

    /* rdhi and rdlo must be different.  */
    if (inst.operands[0].reg == inst.operands[1].reg)
        as_tsktsk (_("rdhi and rdlo must be different"));

    /* rdhi, rdlo and rm must all be different before armv6.  */
    if ((inst.operands[0].reg == inst.operands[2].reg
        || inst.operands[1].reg == inst.operands[2].reg)
        && !ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v6))
        as_tsktsk (_("rdhi, rdlo and rm must all be different"));
}

static void
do_t_mull (void) {
    unsigned RdLo, RdHi, Rn, Rm;

    RdLo = inst.operands[0].reg;
    RdHi = inst.operands[1].reg;
    Rn = inst.operands[2].reg;
    Rm = inst.operands[3].reg;

    reject_bad_reg (RdLo);
    reject_bad_reg (RdHi);
    reject_bad_reg (Rn);
    reject_bad_reg (Rm);

    inst.instruction |= RdLo << 12;
    inst.instruction |= RdHi << 8;
    inst.instruction |= Rn << 16;
    inst.instruction |= Rm;

    if (RdLo == RdHi)
        as_tsktsk (_("rdhi and rdlo must be different"));
}

// insns相关宏
#define TxCE(mnem, op, top, nops, ops, ae, te) \
  { mnem, OPS##nops ops, OT_csuffix, 0x##op, top, ARM_VARIANT, \
    THUMB_VARIANT, do_##ae, do_##te, 0 }
#define TxC3(mnem, op, top, nops, ops, ae, te) \
  { mnem, OPS##nops ops, OT_cinfix3, 0x##op, top, ARM_VARIANT, \
    THUMB_VARIANT, do_##ae, do_##te, 0 }
#define TxC3w(mnem, op, top, nops, ops, ae, te) \
  { mnem, OPS##nops ops, OT_cinfix3_deprecated, 0x##op, top, ARM_VARIANT, \
    THUMB_VARIANT, do_##ae, do_##te, 0 }

#define TCE(mnem, aop, top, nops, ops, ae, te) \
      TxCE (mnem, aop, 0x##top, nops, ops, ae, te)
#define tCE(mnem, aop, top, nops, ops, ae, te) \
      TxCE (mnem, aop, T_MNEM##top, nops, ops, ae, te)
#define tC3(mnem, aop, top, nops, ops, ae, te) \
      TxC3 (mnem, aop, T_MNEM##top, nops, ops, ae, te)
#define tC3w(mnem, aop, top, nops, ops, ae, te) \
      TxC3w (mnem, aop, T_MNEM##top, nops, ops, ae, te)
#define TC3(mnem, aop, top, nops, ops, ae, te) \
      TxC3 (mnem, aop, 0x##top, nops, ops, ae, te)
#define C3(mnem, op, nops, ops, ae)	\
  { #mnem, OPS##nops ops, OT_cinfix3, 0x##op, 0x0, ARM_VARIANT, 0, do_##ae, NULL, 0 }

// insns链存放指令
static const struct asm_opcode insns[] = {
#define ARM_VARIANT    & arm_ext_v1 /* Core ARM Instructions.  */
#define THUMB_VARIANT  & arm_ext_v4t 
    tCE("and",	0000000, _and,     3, (RR, oRR, SH), arit, t_arit3c),
    tC3("ands",	0100000, _ands,	   3, (RR, oRR, SH), arit, t_arit3c),
    tCE("sub",	0400000, _sub,	   3, (RR, oRR, SH), arit, t_add_sub),
    tC3("subs",	0500000, _subs,	   3, (RR, oRR, SH), arit, t_add_sub),
    tCE("add",	0800000, _add,	   3, (RR, oRR, SHG), arit, t_add_sub),
    tC3("adds",	0900000, _adds,	   3, (RR, oRR, SHG), arit, t_add_sub),
    tCE("cmp",	1500000, _cmp,	   2, (RR, SH),      cmp,  t_mov_cmp),
    tC3w("cmps",	1500000, _cmp,	   2, (RR, SH),      cmp,  t_mov_cmp),
    tCE("cmn",	1700000, _cmn,	   2, (RR, SH),      cmp,  t_mvn_tst),

    tCE("mov",	1a00000, _mov,	   2, (RR, SH),      mov,  t_mov_cmp),
    tC3("movs",	1b00000, _movs,	   2, (RR, SHG),     mov,  t_mov_cmp),
    tCE("mvn",	1e00000, _mvn,	   2, (RR, SH),      mov,  t_mvn_tst),

    tCE("ldr",	4100000, _ldr,	   2, (RR, ADDRGLDR),ldst, t_ldst),
    tC3("ldrb",	4500000, _ldrb,	   2, (RRnpc_npcsp, ADDRGLDR),ldst, t_ldst),
    tCE("str",	4000000, _str,	   _2, (MIX_ARM_THUMB_OPERANDS (OP_RR,
								OP_RRnpc),
					OP_ADDRGLDR),ldst, t_ldst),
    tC3("strb",	4400000, _strb,	   2, (RRnpc_npcsp, ADDRGLDR),ldst, t_ldst),

    tCE("b",	a000000, _b,	   1, (EXPr),	     branch, t_branch),
    TCE("bl",	b000000, f000f800, 1, (EXPr),	     bl, t_branch23),
    
    tCE("nop",	1a00000, _nop,	   1, (oI255c),	     nop,  t_nop),

    /* 移位操作 */
    tCE("lsl",	1a00000, _lsl,	   3, (RR, oRR, SH), shift, t_shift),
    tC3("lsls",	1b00000, _lsls,	   3, (RR, oRR, SH), shift, t_shift),
    tCE("lsr",	1a00020, _lsr,	   3, (RR, oRR, SH), shift, t_shift),
    tC3("lsrs",	1b00020, _lsrs,	   3, (RR, oRR, SH), shift, t_shift),
    tCE("asr",	1a00040, _asr,	   3, (RR, oRR, SH), shift, t_shift),
    tC3("asrs",      1b00040, _asrs,     3, (RR, oRR, SH), shift, t_shift),
    tCE("ror",	1a00060, _ror,	   3, (RR, oRR, SH), shift, t_shift),
    tC3("rors",	1b00060, _rors,	   3, (RR, oRR, SH), shift, t_shift),

    tCE("stm",	8800000, _stmia,    2, (RRw, REGLST), ldmstm, t_ldmstm),
    tC3("stmia",	8800000, _stmia,    2, (RRw, REGLST), ldmstm, t_ldmstm),
    tC3("stmea",	8800000, _stmia,    2, (RRw, REGLST), ldmstm, t_ldmstm),
    tCE("ldm",	8900000, _ldmia,    2, (RRw, REGLST), ldmstm, t_ldmstm),
    tC3("ldmia",	8900000, _ldmia,    2, (RRw, REGLST), ldmstm, t_ldmstm),
    tC3("ldmfd",	8900000, _ldmia,    2, (RRw, REGLST), ldmstm, t_ldmstm),

    tCE("push",	92d0000, _push,     1, (REGLST),	     push_pop, t_push_pop),
    tCE("pop",	8bd0000, _pop,	   1, (REGLST),	     push_pop, t_push_pop),

    TCE("rsb",	0600000, ebc00000, 3, (RR, oRR, SH), arit, t_rsb),
    #undef THUMB_VARIANT
    #define THUMB_VARIANT  & arm_ext_os
    TCE("swi",	f000000, df00,     1, (EXPi),        swi, t_swi),
    
    #undef  THUMB_VARIANT
    #define THUMB_VARIANT  & arm_ext_v6t2
    TC3("stmdb",	9000000, e9000000, 2, (RRw, REGLST), ldmstm, t_ldmstm),
    TC3("stmfd",     9000000, e9000000, 2, (RRw, REGLST), ldmstm, t_ldmstm),

    TC3("ldmdb",	9100000, e9100000, 2, (RRw, REGLST), ldmstm, t_ldmstm),
    TC3("ldmea",	9100000, e9100000, 2, (RRw, REGLST), ldmstm, t_ldmstm),

    C3(stmib,	9800000,	   2, (RRw, REGLST), ldmstm),
    C3(stmfa,	9800000,	   2, (RRw, REGLST), ldmstm),
    C3(stmda,	8000000,	   2, (RRw, REGLST), ldmstm),
    C3(stmed,	8000000,	   2, (RRw, REGLST), ldmstm),
    C3(ldmib,	9900000,	   2, (RRw, REGLST), ldmstm),
    C3(ldmed,	9900000,	   2, (RRw, REGLST), ldmstm),
    C3(ldmda,	8100000,	   2, (RRw, REGLST), ldmstm),
    C3(ldmfa,	8100000,	   2, (RRw, REGLST), ldmstm),
    // tCE("ldr",	4100000, _ldr,	   2, (RR, ADDRGLDR),ldst, t_ldst),
    // tC3("ldrb",	4500000, _ldrb,	   2, (RRnpc_npcsp, ADDRGLDR),ldst, t_ldst),
    #undef  ARM_VARIANT
    #define ARM_VARIANT    & arm_ext_v2	/* ARM 2 - multiplies.	*/
    #undef  THUMB_VARIANT
    #define THUMB_VARIANT  & arm_ext_v4t
    tCE("mul",	0000090, _mul,	   3, (RRnpc, RRnpc, oRR), mul, t_mul),
    tC3("muls",	0100090, _muls,	   3, (RRnpc, RRnpc, oRR), mul, t_mul),

    #undef  ARM_VARIANT
    #define ARM_VARIANT    & arm_ext_v4	/* ARM Architecture 4.	*/
    #undef  THUMB_VARIANT
    #define THUMB_VARIANT  & arm_ext_v4t
    TCE("smull",	0c00090, fb800000, 4, (RRnpc, RRnpc, RRnpc, RRnpc), mull, t_mull),

    tC3("ldrh",	01000b0, _ldrh,     2, (RRnpc_npcsp, ADDRGLDRS), ldstv4, t_ldst),
    tC3("strh",	00000b0, _strh,     2, (RRnpc_npcsp, ADDRGLDRS), ldstv4, t_ldst),
    #undef  ARM_VARIANT
    #define ARM_VARIANT  & arm_ext_v4t_5

    /* ARM Architecture 4T.  */
    /* Note: bx (and blx) are required on V5, even if the processor does
        not support Thumb.	 */
    TCE("bx",	12fff10, 4700, 1, (RR),	bx, t_bx),
};
#undef ARM_VARIANT
#undef THUMB_VARIANT
#undef TCE

/* Table of all conditional affixes.  */
static const struct asm_cond conds[] =
{
  {"eq", 0x0},
  {"ne", 0x1},
  {"cs", 0x2}, {"hs", 0x2},
  {"cc", 0x3}, {"ul", 0x3}, {"lo", 0x3},
  {"mi", 0x4},
  {"pl", 0x5},
  {"vs", 0x6},
  {"vc", 0x7},
  {"hi", 0x8},
  {"ls", 0x9},
  {"ge", 0xa},
  {"lt", 0xb},
  {"gt", 0xc},
  {"le", 0xd},
  {"al", 0xe}
};

// 寄存器

#define REGDEF(s,n,t) { #s, n, REG_TYPE_##t, true, 0 }
#define REGNUM(p,n,t) REGDEF(p##n, n, t)
#define REGNUM2(p,n,t) REGDEF(p##n, 2 * n, t)
#define REGSET(p,t) \
  REGNUM(p, 0,t), REGNUM(p, 1,t), REGNUM(p, 2,t), REGNUM(p, 3,t), \
  REGNUM(p, 4,t), REGNUM(p, 5,t), REGNUM(p, 6,t), REGNUM(p, 7,t), \
  REGNUM(p, 8,t), REGNUM(p, 9,t), REGNUM(p,10,t), REGNUM(p,11,t), \
  REGNUM(p,12,t), REGNUM(p,13,t), REGNUM(p,14,t), REGNUM(p,15,t)
#define REGSETH(p,t) \
  REGNUM(p,16,t), REGNUM(p,17,t), REGNUM(p,18,t), REGNUM(p,19,t), \
  REGNUM(p,20,t), REGNUM(p,21,t), REGNUM(p,22,t), REGNUM(p,23,t), \
  REGNUM(p,24,t), REGNUM(p,25,t), REGNUM(p,26,t), REGNUM(p,27,t), \
  REGNUM(p,28,t), REGNUM(p,29,t), REGNUM(p,30,t), REGNUM(p,31,t)
#define REGSET2(p,t) \
  REGNUM2(p, 0,t), REGNUM2(p, 1,t), REGNUM2(p, 2,t), REGNUM2(p, 3,t), \
  REGNUM2(p, 4,t), REGNUM2(p, 5,t), REGNUM2(p, 6,t), REGNUM2(p, 7,t), \
  REGNUM2(p, 8,t), REGNUM2(p, 9,t), REGNUM2(p,10,t), REGNUM2(p,11,t), \
  REGNUM2(p,12,t), REGNUM2(p,13,t), REGNUM2(p,14,t), REGNUM2(p,15,t)
#define SPLRBANK(base,bank,t) \
  REGDEF(lr_##bank, 768|((base+0)<<16), t), \
  REGDEF(sp_##bank, 768|((base+1)<<16), t), \
  REGDEF(spsr_##bank, 768|(base<<16)|SPSR_BIT, t), \
  REGDEF(LR_##bank, 768|((base+0)<<16), t), \
  REGDEF(SP_##bank, 768|((base+1)<<16), t), \
  REGDEF(SPSR_##bank, 768|(base<<16)|SPSR_BIT, t)

#define ARM_MERGE_FEATURE_SETS(TARG,F1,F2)		\
  do							\
    {							\
      (TARG).core[0] = (F1).core[0] | (F2).core[0];	\
      (TARG).core[1] = (F1).core[1] | (F2).core[1];	\
      (TARG).core[2] = (F1).core[2] | (F2).core[2];	\
      (TARG).coproc = (F1).coproc | (F2).coproc;	\
    }							\
  while (0)
static const struct reg_entry reg_names[] =
{
  /* ARM integer registers.  */
  REGSET(r, RN), REGSET(R, RN),

  /* ATPCS synonyms.  */
  REGDEF(a1,0,RN), REGDEF(a2,1,RN), REGDEF(a3, 2,RN), REGDEF(a4, 3,RN),
  REGDEF(v1,4,RN), REGDEF(v2,5,RN), REGDEF(v3, 6,RN), REGDEF(v4, 7,RN),
  REGDEF(v5,8,RN), REGDEF(v6,9,RN), REGDEF(v7,10,RN), REGDEF(v8,11,RN),

  REGDEF(A1,0,RN), REGDEF(A2,1,RN), REGDEF(A3, 2,RN), REGDEF(A4, 3,RN),
  REGDEF(V1,4,RN), REGDEF(V2,5,RN), REGDEF(V3, 6,RN), REGDEF(V4, 7,RN),
  REGDEF(V5,8,RN), REGDEF(V6,9,RN), REGDEF(V7,10,RN), REGDEF(V8,11,RN),

  /* Well-known aliases.  */
  REGDEF(wr, 7,RN), REGDEF(sb, 9,RN), REGDEF(sl,10,RN), REGDEF(fp,11,RN),
  REGDEF(ip,12,RN), REGDEF(sp,13,RN), REGDEF(lr,14,RN), REGDEF(pc,15,RN),

  REGDEF(WR, 7,RN), REGDEF(SB, 9,RN), REGDEF(SL,10,RN), REGDEF(FP,11,RN),
  REGDEF(IP,12,RN), REGDEF(SP,13,RN), REGDEF(LR,14,RN), REGDEF(PC,15,RN),

  /* Defining the new Zero register from ARMv8.1-M.  */
  REGDEF(zr,15,ZR),
  REGDEF(ZR,15,ZR),

  /* Coprocessor numbers.  */
  REGSET(p, CP), REGSET(P, CP),

  /* Coprocessor register numbers.  The "cr" variants are for backward
     compatibility.  */
  REGSET(c,  CN), REGSET(C, CN),
  REGSET(cr, CN), REGSET(CR, CN),

  /* ARM banked registers.  */
  REGDEF(R8_usr,512|(0<<16),RNB), REGDEF(r8_usr,512|(0<<16),RNB),
  REGDEF(R9_usr,512|(1<<16),RNB), REGDEF(r9_usr,512|(1<<16),RNB),
  REGDEF(R10_usr,512|(2<<16),RNB), REGDEF(r10_usr,512|(2<<16),RNB),
  REGDEF(R11_usr,512|(3<<16),RNB), REGDEF(r11_usr,512|(3<<16),RNB),
  REGDEF(R12_usr,512|(4<<16),RNB), REGDEF(r12_usr,512|(4<<16),RNB),
  REGDEF(SP_usr,512|(5<<16),RNB), REGDEF(sp_usr,512|(5<<16),RNB),
  REGDEF(LR_usr,512|(6<<16),RNB), REGDEF(lr_usr,512|(6<<16),RNB),

  REGDEF(R8_fiq,512|(8<<16),RNB), REGDEF(r8_fiq,512|(8<<16),RNB),
  REGDEF(R9_fiq,512|(9<<16),RNB), REGDEF(r9_fiq,512|(9<<16),RNB),
  REGDEF(R10_fiq,512|(10<<16),RNB), REGDEF(r10_fiq,512|(10<<16),RNB),
  REGDEF(R11_fiq,512|(11<<16),RNB), REGDEF(r11_fiq,512|(11<<16),RNB),
  REGDEF(R12_fiq,512|(12<<16),RNB), REGDEF(r12_fiq,512|(12<<16),RNB),
  REGDEF(SP_fiq,512|(13<<16),RNB), REGDEF(sp_fiq,512|(13<<16),RNB),
  REGDEF(LR_fiq,512|(14<<16),RNB), REGDEF(lr_fiq,512|(14<<16),RNB),
  REGDEF(SPSR_fiq,512|(14<<16)|SPSR_BIT,RNB), REGDEF(spsr_fiq,512|(14<<16)|SPSR_BIT,RNB),

  SPLRBANK(0,IRQ,RNB), SPLRBANK(0,irq,RNB),
  SPLRBANK(2,SVC,RNB), SPLRBANK(2,svc,RNB),
  SPLRBANK(4,ABT,RNB), SPLRBANK(4,abt,RNB),
  SPLRBANK(6,UND,RNB), SPLRBANK(6,und,RNB),
  SPLRBANK(12,MON,RNB), SPLRBANK(12,mon,RNB),
  REGDEF(elr_hyp,768|(14<<16),RNB), REGDEF(ELR_hyp,768|(14<<16),RNB),
  REGDEF(sp_hyp,768|(15<<16),RNB), REGDEF(SP_hyp,768|(15<<16),RNB),
  REGDEF(spsr_hyp,768|(14<<16)|SPSR_BIT,RNB),
  REGDEF(SPSR_hyp,768|(14<<16)|SPSR_BIT,RNB),

  /* FPA registers.  */
  REGNUM(f,0,FN), REGNUM(f,1,FN), REGNUM(f,2,FN), REGNUM(f,3,FN),
  REGNUM(f,4,FN), REGNUM(f,5,FN), REGNUM(f,6,FN), REGNUM(f,7, FN),

  REGNUM(F,0,FN), REGNUM(F,1,FN), REGNUM(F,2,FN), REGNUM(F,3,FN),
  REGNUM(F,4,FN), REGNUM(F,5,FN), REGNUM(F,6,FN), REGNUM(F,7, FN),

  /* VFP SP registers.	*/
  REGSET(s,VFS),  REGSET(S,VFS),
  REGSETH(s,VFS), REGSETH(S,VFS),

  /* VFP DP Registers.	*/
  REGSET(d,VFD),  REGSET(D,VFD),
  /* Extra Neon DP registers.  */
  REGSETH(d,VFD), REGSETH(D,VFD),

  /* Neon QP registers.  */
  REGSET2(q,NQ),  REGSET2(Q,NQ),

  /* VFP control registers.  */
  REGDEF(fpsid,0,VFC), REGDEF(fpscr,1,VFC), REGDEF(fpexc,8,VFC),
  REGDEF(FPSID,0,VFC), REGDEF(FPSCR,1,VFC), REGDEF(FPEXC,8,VFC),
  REGDEF(fpinst,9,VFC), REGDEF(fpinst2,10,VFC),
  REGDEF(FPINST,9,VFC), REGDEF(FPINST2,10,VFC),
  REGDEF(mvfr0,7,VFC), REGDEF(mvfr1,6,VFC),
  REGDEF(MVFR0,7,VFC), REGDEF(MVFR1,6,VFC),
  REGDEF(mvfr2,5,VFC), REGDEF(MVFR2,5,VFC),
  REGDEF(fpscr_nzcvqc,2,VFC), REGDEF(FPSCR_nzcvqc,2,VFC),
  REGDEF(vpr,12,VFC), REGDEF(VPR,12,VFC),
  REGDEF(fpcxt_ns,14,VFC), REGDEF(FPCXT_NS,14,VFC),
  REGDEF(fpcxt_s,15,VFC), REGDEF(FPCXT_S,15,VFC),
  REGDEF(fpcxtns,14,VFC), REGDEF(FPCXTNS,14,VFC),
  REGDEF(fpcxts,15,VFC), REGDEF(FPCXTS,15,VFC),

  /* Maverick DSP coprocessor registers.  */
  REGSET(mvf,MVF),  REGSET(mvd,MVD),  REGSET(mvfx,MVFX),  REGSET(mvdx,MVDX),
  REGSET(MVF,MVF),  REGSET(MVD,MVD),  REGSET(MVFX,MVFX),  REGSET(MVDX,MVDX),

  REGNUM(mvax,0,MVAX), REGNUM(mvax,1,MVAX),
  REGNUM(mvax,2,MVAX), REGNUM(mvax,3,MVAX),
  REGDEF(dspsc,0,DSPSC),

  REGNUM(MVAX,0,MVAX), REGNUM(MVAX,1,MVAX),
  REGNUM(MVAX,2,MVAX), REGNUM(MVAX,3,MVAX),
  REGDEF(DSPSC,0,DSPSC),

  /* iWMMXt data registers - p0, c0-15.	 */
  REGSET(wr,MMXWR), REGSET(wR,MMXWR), REGSET(WR, MMXWR),

  /* iWMMXt control registers - p1, c0-3.  */
  REGDEF(wcid,	0,MMXWC),  REGDEF(wCID,	 0,MMXWC),  REGDEF(WCID,  0,MMXWC),
  REGDEF(wcon,	1,MMXWC),  REGDEF(wCon,	 1,MMXWC),  REGDEF(WCON,  1,MMXWC),
  REGDEF(wcssf, 2,MMXWC),  REGDEF(wCSSF, 2,MMXWC),  REGDEF(WCSSF, 2,MMXWC),
  REGDEF(wcasf, 3,MMXWC),  REGDEF(wCASF, 3,MMXWC),  REGDEF(WCASF, 3,MMXWC),

  /* iWMMXt scalar (constant/offset) registers - p1, c8-11.  */
  REGDEF(wcgr0, 8,MMXWCG),  REGDEF(wCGR0, 8,MMXWCG),  REGDEF(WCGR0, 8,MMXWCG),
  REGDEF(wcgr1, 9,MMXWCG),  REGDEF(wCGR1, 9,MMXWCG),  REGDEF(WCGR1, 9,MMXWCG),
  REGDEF(wcgr2,10,MMXWCG),  REGDEF(wCGR2,10,MMXWCG),  REGDEF(WCGR2,10,MMXWCG),
  REGDEF(wcgr3,11,MMXWCG),  REGDEF(wCGR3,11,MMXWCG),  REGDEF(WCGR3,11,MMXWCG),

  /* XScale accumulator registers.  */
  REGNUM(acc,0,XSCALE), REGNUM(ACC,0,XSCALE),

  /* DWARF ABI defines RA_AUTH_CODE to 143.  */
  REGDEF(ra_auth_code,143,PSEUDO),
};
#undef REGDEF
#undef REGNUM
#undef REGSET

#ifdef OBJ_ELF
static struct reloc_entry reloc_names[] =
{
  { "got",     BFD_RELOC_ARM_GOT32   },	 { "GOT",     BFD_RELOC_ARM_GOT32   },
  { "gotoff",  BFD_RELOC_ARM_GOTOFF  },	 { "GOTOFF",  BFD_RELOC_ARM_GOTOFF  },
  { "plt",     BFD_RELOC_ARM_PLT32   },	 { "PLT",     BFD_RELOC_ARM_PLT32   },
  { "target1", BFD_RELOC_ARM_TARGET1 },	 { "TARGET1", BFD_RELOC_ARM_TARGET1 },
  { "target2", BFD_RELOC_ARM_TARGET2 },	 { "TARGET2", BFD_RELOC_ARM_TARGET2 },
  { "sbrel",   BFD_RELOC_ARM_SBREL32 },	 { "SBREL",   BFD_RELOC_ARM_SBREL32 },
  { "tlsgd",   BFD_RELOC_ARM_TLS_GD32},  { "TLSGD",   BFD_RELOC_ARM_TLS_GD32},
  { "tlsldm",  BFD_RELOC_ARM_TLS_LDM32}, { "TLSLDM",  BFD_RELOC_ARM_TLS_LDM32},
  { "tlsldo",  BFD_RELOC_ARM_TLS_LDO32}, { "TLSLDO",  BFD_RELOC_ARM_TLS_LDO32},
  { "gottpoff",BFD_RELOC_ARM_TLS_IE32},  { "GOTTPOFF",BFD_RELOC_ARM_TLS_IE32},
  { "tpoff",   BFD_RELOC_ARM_TLS_LE32},  { "TPOFF",   BFD_RELOC_ARM_TLS_LE32},
  { "got_prel", BFD_RELOC_ARM_GOT_PREL}, { "GOT_PREL", BFD_RELOC_ARM_GOT_PREL},
  { "tlsdesc", BFD_RELOC_ARM_TLS_GOTDESC},
	{ "TLSDESC", BFD_RELOC_ARM_TLS_GOTDESC},
  { "tlscall", BFD_RELOC_ARM_TLS_CALL},
	{ "TLSCALL", BFD_RELOC_ARM_TLS_CALL},
  { "tlsdescseq", BFD_RELOC_ARM_TLS_DESCSEQ},
	{ "TLSDESCSEQ", BFD_RELOC_ARM_TLS_DESCSEQ},
  { "gotfuncdesc", BFD_RELOC_ARM_GOTFUNCDESC },
	{ "GOTFUNCDESC", BFD_RELOC_ARM_GOTFUNCDESC },
  { "gotofffuncdesc", BFD_RELOC_ARM_GOTOFFFUNCDESC },
	{ "GOTOFFFUNCDESC", BFD_RELOC_ARM_GOTOFFFUNCDESC },
  { "funcdesc", BFD_RELOC_ARM_FUNCDESC },
	{ "FUNCDESC", BFD_RELOC_ARM_FUNCDESC },
   { "tlsgd_fdpic", BFD_RELOC_ARM_TLS_GD32_FDPIC },      { "TLSGD_FDPIC", BFD_RELOC_ARM_TLS_GD32_FDPIC },
   { "tlsldm_fdpic", BFD_RELOC_ARM_TLS_LDM32_FDPIC },    { "TLSLDM_FDPIC", BFD_RELOC_ARM_TLS_LDM32_FDPIC },
   { "gottpoff_fdpic", BFD_RELOC_ARM_TLS_IE32_FDPIC },   { "GOTTPOFF_FDIC", BFD_RELOC_ARM_TLS_IE32_FDPIC },
};
#endif

#define TRANSITION(from, to) (mapstate == (from) && state == (to))
#define ARM_SET_THUMB(s,t)      ((t) ? ARM_SET_FLAG (s, ARM_FLAG_THUMB)     : ARM_RESET_FLAG (s, ARM_FLAG_THUMB))
#define ARM_SET_INTERWORK(s,t)  ((t) ? ARM_SET_FLAG (s, ARM_FLAG_INTERWORK) : ARM_RESET_FLAG (s, ARM_FLAG_INTERWORK))
#define THUMB_SET_FUNC(s,t)     ((t) ? ARM_SET_FLAG (s, THUMB_FLAG_FUNC)    : ARM_RESET_FLAG (s, THUMB_FLAG_FUNC))

/**
 * @Description: 为转换到STATE创建一个新的映射符号
 */
static void
make_mapping_symbol (enum mstate state, valueT value, fragS *frag)
{
    symbolS * symbolP;
    const char * symname;
    int type;

    switch (state) {
        case MAP_DATA:
            symname = "$d";
            type = BSF_NO_FLAGS;
            break;
        case MAP_ARM:
            symname = "$a";
            type = BSF_NO_FLAGS;
            break;
        case MAP_THUMB:
            symname = "$t";
            type = BSF_NO_FLAGS;
            break;
        default:
            abort ();
    }

    symbolP = symbol_new (symname, now_seg, frag, value);
    symbol_get_bfdsym (symbolP)->flags |= type | BSF_LOCAL;

    switch (state) {
        case MAP_ARM:
            THUMB_SET_FUNC (symbolP, 0);
            ARM_SET_THUMB (symbolP, 0);
            ARM_SET_INTERWORK (symbolP, support_interwork);
            break;

        case MAP_THUMB:
            THUMB_SET_FUNC (symbolP, 1);
            ARM_SET_THUMB (symbolP, 1);
            ARM_SET_INTERWORK (symbolP, support_interwork);
            break;

        case MAP_DATA:
        default:
            break;
    }

    /* Save the mapping symbols for future reference.  Also check that
        we do not place two mapping symbols at the same offset within a
        frag.  We'll handle overlap between frags in
        check_mapping_symbols.

        If .fill or other data filling directive generates zero sized data,
        the mapping symbol for the following code will have the same value
        as the one generated for the data filling directive.  In this case,
        we replace the old symbol with the new one at the same address.  */
    if (value == 0) {
        if (frag->tc_frag_data.first_map != NULL) {
            know (S_GET_VALUE (frag->tc_frag_data.first_map) == 0);
            symbol_remove (frag->tc_frag_data.first_map, &symbol_rootP, &symbol_lastP);
        }
        frag->tc_frag_data.first_map = symbolP;
    }
    if (frag->tc_frag_data.last_map != NULL) {
        know (S_GET_VALUE (frag->tc_frag_data.last_map) <= S_GET_VALUE (symbolP));
        if (S_GET_VALUE (frag->tc_frag_data.last_map) == S_GET_VALUE (symbolP))
        symbol_remove (frag->tc_frag_data.last_map, &symbol_rootP, &symbol_lastP);
    }
    frag->tc_frag_data.last_map = symbolP;
}

/**
 * @Description: 根据指定的映射状态和已分配的空间大小，
 * 添加或更新映射符号的位置和属性
 */
static void
mapping_state_2 (enum mstate state, int max_chars) {
    enum mstate mapstate = seg_info (now_seg)->tc_segment_info_data.mapstate;

    if (!SEG_NORMAL (now_seg))
        return;

    if (mapstate == state)
        /* The mapping symbol has already been emitted.
        There is nothing else to do.  */
        return;

    if (TRANSITION (MAP_UNDEFINED, MAP_ARM)
        || TRANSITION (MAP_UNDEFINED, MAP_THUMB)) {
        struct frag * const frag_first = seg_info (now_seg)->frchainP->frch_root;
        const int add_symbol = (frag_now != frag_first) || (frag_now_fix () > 0);

        if (add_symbol)
            make_mapping_symbol (MAP_DATA, (valueT) 0, frag_first);
    }

    seg_info (now_seg)->tc_segment_info_data.mapstate = state;
    make_mapping_symbol (state, (valueT) frag_now_fix () - max_chars, frag_now);
}

void
arm_init_frag (fragS * fragP, int max_chars)
{
    bool frag_thumb_mode;

    /* If the current ARM vs THUMB mode has not already
        been recorded into this frag then do so now.  */
    if ((fragP->tc_frag_data.thumb_mode & MODE_RECORDED) == 0)
        fragP->tc_frag_data.thumb_mode = thumb_mode | MODE_RECORDED;

    /* PR 21809: Do not set a mapping state for debug sections
        - it just confuses other tools.  */
    if (bfd_section_flags (now_seg) & SEC_DEBUGGING)
        return;

    frag_thumb_mode = fragP->tc_frag_data.thumb_mode ^ MODE_RECORDED;

    /* Record a mapping symbol for alignment frags.  We will delete this
        later if the alignment ends up empty.  */
    switch (fragP->fr_type) {
        case rs_align:
        case rs_align_test:
        case rs_fill:
            mapping_state_2 (MAP_DATA, max_chars);
            break;
        case rs_align_code:
            mapping_state_2 (frag_thumb_mode ? MAP_THUMB : MAP_ARM, max_chars);
            break;
        default:
            break;
    }
}

/**
 * @Description: 处理映射状态
 */
void
mapping_state (enum mstate state) {
    enum mstate mapstate = seg_info (now_seg)->tc_segment_info_data.mapstate;
    if (mapstate == state)
        /* The mapping symbol has already been emitted.
        There is nothing else to do.  */
        return;
    if (state == MAP_ARM || state == MAP_THUMB) 
        record_alignment (now_seg, state == MAP_ARM ? 2 : 1);
    if (TRANSITION (MAP_UNDEFINED, MAP_DATA))
        /* This case will be evaluated later.  */
        return;

    mapping_state_2 (state, 0);
}

/**
 * @Description: 设置一些常量浮点数的值
 */
static void
set_constant_flonums (void)
{
    int i;

    for (i = 0; i < NUM_FLOAT_VALS; i++)
        if (atof_ieee ((char *) fp_const[i], 'x', fp_values[i]) == NULL)
            abort ();
}

void
md_begin (void) {
    unsigned mach;
    unsigned int i;
    // 操作码
    arm_ops_hsh = str_htab_create ();
    // // 条件码
    arm_cond_hsh = str_htab_create ();
    arm_vcond_hsh = str_htab_create ();
    arm_shift_hsh = str_htab_create ();
    arm_psr_hsh = str_htab_create ();
    arm_reg_hsh = str_htab_create ();
    arm_reloc_hsh = str_htab_create ();
    for (i = 0; i < sizeof (insns) / sizeof (struct asm_opcode); i++)
        if (str_hash_find (arm_ops_hsh, insns[i].template_name) == NULL)
            str_hash_insert (arm_ops_hsh, insns[i].template_name, insns + i, 0);
    for (i = 0; i < sizeof (conds) / sizeof (struct asm_cond); i++)
        str_hash_insert (arm_cond_hsh, conds[i].template_name, conds + i, 0);
    for (i = 0; i < sizeof (vconds) / sizeof (struct asm_cond); i++)
        str_hash_insert (arm_vcond_hsh, vconds[i].template_name, vconds + i, 0);
    for (i = 0; i < sizeof (shift_names) / sizeof (struct asm_shift_name); i++)
        str_hash_insert (arm_shift_hsh, shift_names[i].name, shift_names + i, 0);
    for (i = 0; i < sizeof (reg_names) / sizeof (struct reg_entry); i++)
        str_hash_insert (arm_reg_hsh, reg_names[i].name, reg_names + i, 0);
#ifdef OBJ_ELF
    for (i = 0; i < ARRAY_SIZE (reloc_names); i++) {
        struct reloc_entry * entry = reloc_names + i;

        if (arm_is_eabi() && entry->reloc == BFD_RELOC_ARM_PLT32)
            /* This makes encode_branch() use the EABI versions of this relocation.  */
            entry->reloc = BFD_RELOC_UNUSED;

        str_hash_insert (arm_reloc_hsh, entry->name, entry, 0);
    }
#endif
    set_constant_flonums ();
    ARM_MERGE_FEATURE_SETS (selected_cpu, selected_arch, selected_ext);
    selected_fpu = fpu_default;
    if (ARM_FEATURE_ZERO (selected_fpu)) {
        // if (!no_cpu_selected ())
        //     selected_fpu = fpu_default;
        // else
        //     selected_fpu = fpu_arch_fpa;
    }
#ifdef CPU_DEFAULT
#else
  /*  Autodection of feature mode: allow all features in cpu_variant but leave
      selected_cpu unset.  It will be set in aeabi_set_public_attributes ()
      after all instruction have been processed and we can decide what CPU
      should be selected.  */
    if (ARM_FEATURE_ZERO (selected_arch))
        ARM_MERGE_FEATURE_SETS (cpu_variant, arm_arch_any, selected_fpu);
    else
        ARM_MERGE_FEATURE_SETS (cpu_variant, selected_cpu, selected_fpu);
#endif
    arm_arch_used = thumb_arch_used = arm_arch_none;
#if defined OBJ_COFF || defined OBJ_ELF 
    {
        unsigned int flags = 0;

        flags = meabi_flags;

        switch (meabi_flags) {
            case EF_ARM_EABI_UNKNOWN:
                if (uses_apcs_26)      flags |= F_APCS26;
                if (support_interwork) flags |= F_INTERWORK;
                if (uses_apcs_float)   flags |= F_APCS_FLOAT;
                if (pic_code)	       flags |= F_PIC;
                if (!ARM_CPU_HAS_FEATURE (cpu_variant, fpu_any_hard))
                    flags |= F_SOFT_FLOAT; 
                switch (mfloat_abi_opt) {
                    case ARM_FLOAT_ABI_SOFT:
                    case ARM_FLOAT_ABI_SOFTFP:
                        flags |= F_SOFT_FLOAT;
                        break;

                    case ARM_FLOAT_ABI_HARD:
                        if (flags & F_SOFT_FLOAT)
                            as_bad (_("hard-float conflicts with specified fpu"));
                        break;
                }

                    /* Using pure-endian doubles (even if soft-float).	*/
                if (ARM_CPU_HAS_FEATURE (cpu_variant, fpu_endian_pure))
                    flags |= F_VFP_FLOAT;

                if (ARM_CPU_HAS_FEATURE (cpu_variant, fpu_arch_maverick))
                    flags |= EF_ARM_MAVERICK_FLOAT;
                break;
            case EF_ARM_EABI_VER4:
            case EF_ARM_EABI_VER5:
                break;
            default:
	            abort ();
            
        }
        bfd_set_private_flags (stdoutput, flags);
    }
#endif
     /* 根据CPU的特性（features）确定目标机器类型 */
    if (ARM_CPU_HAS_FEATURE (cpu_variant, arm_cext_iwmmxt2))
        mach = bfd_mach_arm_iWMMXt2;
    else if (ARM_CPU_HAS_FEATURE (cpu_variant, arm_cext_iwmmxt))
        mach = bfd_mach_arm_iWMMXt;
    else if (ARM_CPU_HAS_FEATURE (cpu_variant, arm_cext_xscale))
        mach = bfd_mach_arm_XScale;
    else if (ARM_CPU_HAS_FEATURE (cpu_variant, arm_cext_maverick))
        mach = bfd_mach_arm_ep9312;
    else if (ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v5e))
        mach = bfd_mach_arm_5TE;
    else if (ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v5)) {
        if (ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v4t))
            mach = bfd_mach_arm_5T;
        else
            mach = bfd_mach_arm_5;
    }
    else if (ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v4)) {
        if (ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v4t))
            mach = bfd_mach_arm_4T;
        else
            mach = bfd_mach_arm_4;
    }
    else if (ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v3m))
        mach = bfd_mach_arm_3M;
    else if (ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v3))
        mach = bfd_mach_arm_3;
    else if (ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v2s))
        mach = bfd_mach_arm_2a;
    else if (ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v2))
        mach = bfd_mach_arm_2;
    else
        mach = bfd_mach_arm_unknown;

    bfd_set_arch_mach (stdoutput, TARGET_ARCH, mach);
}

/**
 * @Description: 记录上一个被扫描到的标签的地址
 */
void
arm_start_line_hook (void)
{
    last_label_seen = NULL;
}

bool
arm_data_in_code (void)
{
    printf("arm_data_in_code\n");
    if (thumb_mode && startswith (input_line_pointer + 1, "data:")) {
        *input_line_pointer = '/';
        input_line_pointer += 5;
        *input_line_pointer = 0;
        return true;
    }

    return false;
}

/**
 * @Description: arm解析float
 */
const char *
md_atof (int type, char * litP, int * sizeP) {
    int prec;
    LITTLENUM_TYPE words[MAX_LITTLENUMS];
    char *t;
    int i;
    // type:'f'
    switch (type) {
        case 'H':
        case 'h':
        /* bfloat16, despite not being part of the IEEE specification, can also
        be handled by atof_ieee().  */
        case 'b':
            prec = 1;
            break;

        case 'f':
        case 'F':
        case 's':
        case 'S':
            prec = 2;
            break;

        case 'd':
        case 'D':
        case 'r':
        case 'R':
            prec = 4;
            break;

        case 'x':
        case 'X':
            prec = 5;
            break;

        case 'p':
        case 'P':
            prec = 5;
            break;

        default:
            *sizeP = 0;
            return _("Unrecognized or unsupported floating point constant");
    }
    // t是去掉要解析的浮点数剩下的字符串
    t = atof_ieee (input_line_pointer, type, words);
    if (t)
        // input_line_pointer也去掉数值
        input_line_pointer = t;
    *sizeP = prec * sizeof (LITTLENUM_TYPE);

    for (i = 0; i < prec; i += 2) {
        md_number_to_chars (litP, (valueT) words[i + 1],
                    sizeof (LITTLENUM_TYPE));
        md_number_to_chars (litP + sizeof (LITTLENUM_TYPE),
                    (valueT) words[i], sizeof (LITTLENUM_TYPE));
        litP += 2 * sizeof (LITTLENUM_TYPE);
    }

    return NULL;
}

symbolS *
md_undefined_symbol (char * name ATTRIBUTE_UNUSED) {
#ifdef OBJ_ELF
    if (name[0] == '_' && name[1] == 'G'
      && streq (name, GLOBAL_OFFSET_TABLE_NAME)) {
        if (!GOT_symbol) {
            if (symbol_find (name))
                as_bad (_("GOT already in the symbol table"));

            GOT_symbol = symbol_new (name, undefined_section,
                    &zero_address_frag, 0);
        }

      return GOT_symbol;
    }
#endif

    return NULL;   
}

/**
 * @Description: 对于操作码进行查找和分析 
 */
static const struct asm_opcode *
opcode_lookup (char **str) {
    char *end, *base;
    char *affix;
    // 操作码
    const struct asm_opcode *opcode;
    // 条件码
    const struct asm_cond *cond;
    char save[2];
    // arm常用助记符以' '空格结束
    for (base = end = *str; *end != '\0'; end++)
        if (*end == ' ' || *end == '.')
            break;
    if (end == base)
        return NULL;
    *str = end;
    /* 查找助记符 */
    opcode = (const struct asm_opcode *) str_hash_find_n (arm_ops_hsh, base,
                                end - base);
    cond = NULL;
    /* step U:直接能在哈希表中找到，目前一般处理的都属于这一类 */
    if (opcode) {
        // 无条件执行的指令
        if (opcode->tag < OT_odd_infix_0) {
            inst.cond = COND_ALWAYS;
            return opcode;
        }
        if (warn_on_deprecated && unified_syntax)
            as_tsktsk (_("conditional infixes are deprecated in unified syntax"));
        affix = base + (opcode->tag - OT_odd_infix_0);
        cond = (const struct asm_cond *) str_hash_find_n (arm_cond_hsh, affix, 2);
        gas_assert (cond);

        inst.cond = cond->value;
        return opcode;
    }   

    if (!opcode || !cond) {
        if (end - base < 3)
            return NULL;

        /* Look for suffixed mnemonic.  */
        affix = end - 2;
        cond = (const struct asm_cond *) str_hash_find_n (arm_cond_hsh, affix, 2);
        opcode = (const struct asm_opcode *) str_hash_find_n (arm_ops_hsh, base,
							    affix - base);
    }
    if (opcode && cond) {
        switch (opcode->tag) {
            case OT_cinfix3_legacy:
            /* Ignore conditional suffixes matched on infix only mnemonics.  */
            break;

            case OT_cinfix3:
            case OT_cinfix3_deprecated:
            case OT_odd_infix_unc:
            if (!unified_syntax)
                return NULL;
            case OT_csuffix:
            case OT_csuffixF:
            case OT_csuf_or_in3:
                inst.cond = cond->value;
                return opcode;
            case OT_unconditionalF:
                if (thumb_mode)
                    inst.cond = cond->value;
                else {
                    /* Delayed diagnostic.  */
                    inst.error = BAD_COND;
                    inst.cond = COND_ALWAYS;
                }
                return opcode;
        }
    }
}

static void
first_error (const char *err) {
    if (!inst.error)
        inst.error = err;
}

static struct reg_entry *
arm_reg_parse_multi (char **ccp) {
    char *start = *ccp;
    char *p;
    struct reg_entry *reg;

    skip_whitespace (start);
#ifdef OPTIONAL_REGISTER_PREFIX
    if (*start == OPTIONAL_REGISTER_PREFIX)
        start++;
#endif
    p = start;
    if (!ISALPHA (*p) || !is_name_beginner (*p))
        return NULL;
    do
        p++;
    while (ISALPHA (*p) || ISDIGIT (*p) || *p == '_');
    // 寻找寄存器
    reg = (struct reg_entry *) str_hash_find_n (arm_reg_hsh, start, p - start);
    if (!reg)
        return NULL;
    // 把原来的str去掉寄存器
    *ccp = p;
    return reg;
}

/**
 * @Description: 断ARM体系结构中的寄存器是否符合特定的替代语法规则
 */
static int
arm_reg_alt_syntax (char **ccp, char *start, struct reg_entry *reg,
		    enum arm_reg_type type) {
    /* Alternative syntaxes are accepted for a few register classes.  */
    switch (type) {
        case REG_TYPE_MVF:
        case REG_TYPE_MVD:
        case REG_TYPE_MVFX:
        case REG_TYPE_MVDX:
        /* Generic coprocessor register names are allowed for these.  */
        if (reg && reg->type == REG_TYPE_CN)
            return reg->number;
        break;

        case REG_TYPE_CP:
        /* For backward compatibility, a bare number is valid here.  */
        {
            unsigned long processor = strtoul (start, ccp, 10);
            if (*ccp != start && processor <= 15)
                return processor;
        }
        /* Fall through.  */

        case REG_TYPE_MMXWC:
            /* WC includes WCG.  ??? I'm not sure this is true for all
            instructions that take WC registers.  */
            if (reg && reg->type == REG_TYPE_MMXWCG)
            return reg->number;
            break;

        default:
            break;
    }

    return FAIL;
}

/**
 * @Description: 对arm架构的寄存器解析  与arm_reg_parse_multi不同的是
 * 会检查寄存器的类型
 */
static int
arm_reg_parse (char **ccp, enum arm_reg_type type) {
    char *start = *ccp;
    // 得到寄存器
    struct reg_entry *reg = arm_reg_parse_multi (ccp);
    int ret;
    if (reg && reg->type == type)
        return reg->number;

    if (reg && reg->type == type)
        return reg->number;

    // 是否符合reg语法
    if ((ret = arm_reg_alt_syntax (ccp, start, reg, type)) != FAIL)
        return ret;

    *ccp = start;
    return FAIL;
}

static int
parse_typed_reg_or_scalar (char **ccp, enum arm_reg_type type,
			   enum arm_reg_type *rtype,
			   struct neon_typed_alias *typeinfo) {
    char *str = *ccp;
    // 找到str中对应的寄存器
    struct reg_entry *reg = arm_reg_parse_multi (&str); 
    struct neon_typed_alias atype;
    struct neon_type_el parsetype;    
    atype.defined = 0;
    atype.index = -1;
    atype.eltype.type = NT_invtype;
    atype.eltype.size = -1;
    if (typeinfo)
        *typeinfo = atype;

    if (rtype)
        *rtype = type;

    *ccp = str;

    return reg->number;
}

/**
 * @Description: OP_RR 分析操作数时遇到arm 寄存器
 */
static int
arm_typed_reg_parse (char **ccp, enum arm_reg_type type,
		     enum arm_reg_type *rtype, struct neon_type_el *vectype) {
    struct neon_typed_alias atype;
    char *str = *ccp;
    int reg = parse_typed_reg_or_scalar (&str, type, rtype, &atype);
    if (reg == FAIL)
        return FAIL;

    /* Do not allow regname(... to parse as a register.  */
    if (*str == '(')
        return FAIL;

    /* Do not allow a scalar (reg+index) to parse as a register.  */
    if ((atype.defined & NTA_HASINDEX) != 0) {
        first_error (_("register operand expected, but got scalar"));
        return FAIL;
    }

    if (vectype)
        *vectype = atype.eltype;

    *ccp = str;

    return reg;
}

static unsigned
parse_qfloat_immediate (char **ccp, int *immed) {
    char *str = *ccp;
    char *fpnum;
    LITTLENUM_TYPE words[MAX_LITTLENUMS];
    int found_fpchar = 0;

    skip_past_char (&str, '#');

    /* We must not accidentally parse an integer as a floating-point number. Make
        sure that the value we parse is not an integer by checking for special
        characters '.' or 'e'.
        FIXME: This is a horrible hack, but doing better is tricky because type
        information isn't in a very usable state at parse time.  */
    fpnum = str;
    skip_whitespace (fpnum);

    if (startswith (fpnum, "0x"))
        return FAIL;
    else {
        for (; *fpnum != '\0' && *fpnum != ' ' && *fpnum != '\n'; fpnum++)
            if (*fpnum == '.' || *fpnum == 'e' || *fpnum == 'E') {
                found_fpchar = 1;
                break;
            }

            if (!found_fpchar)
                return FAIL;
    }

    if ((str = atof_ieee (str, 's', words)) != NULL) {
        unsigned fpword = 0;
        int i;

        /* Our FP word must be 32 bits (single-precision FP).  */
        for (i = 0; i < 32 / LITTLENUM_NUMBER_OF_BITS; i++) {
            fpword <<= LITTLENUM_NUMBER_OF_BITS;
            fpword |= words[i];
        }

        if (is_quarter_float (fpword) || (fpword & 0x7fffffff) == 0)
            *immed = fpword;
        else
            return FAIL;

        *ccp = str;

        return SUCCESS;
    }

    return FAIL;
}

/**
 * @Description: mov指令
 */
static int
parse_neon_mov (char **str, int *which_operand) {

}

/**
 * @Description: 对于左移右移
 */
static int
parse_shift (char **str, int i, enum parse_shift_mode mode) {
    const struct asm_shift_name *shift_name;
    enum shift_kind shift;
    char *s = *str;
    char *p = s;
    int reg;

    for (p = *str; ISALPHA (*p); p++)
        ;

    if (p == *str) {
        inst.error = _("shift expression expected");
        return FAIL;
    }

    shift_name
        = (const struct asm_shift_name *) str_hash_find_n (arm_shift_hsh, *str,
                                p - *str);

    if (shift_name == NULL) {
        inst.error = _("shift expression expected");
        return FAIL;
    }

    shift = shift_name->kind;

    switch (mode) {
        case NO_SHIFT_RESTRICT:
        case SHIFT_IMMEDIATE:
            if (shift == SHIFT_UXTW) {
                inst.error = _("'UXTW' not allowed here");
                return FAIL;
            }
            break;

        case SHIFT_LSL_OR_ASR_IMMEDIATE:
            if (shift != SHIFT_LSL && shift != SHIFT_ASR) {
                inst.error = _("'LSL' or 'ASR' required");
                return FAIL;
            }
            break;

        case SHIFT_LSL_IMMEDIATE:
            if (shift != SHIFT_LSL) {
                inst.error = _("'LSL' required");
                return FAIL;
            }
            break;

        case SHIFT_ASR_IMMEDIATE:
            if (shift != SHIFT_ASR) {
                inst.error = _("'ASR' required");
                return FAIL;
            }
            break;
        case SHIFT_UXTW_IMMEDIATE:
            if (shift != SHIFT_UXTW) {
                inst.error = _("'UXTW' required");
                return FAIL;
            }
            break;

        default: abort ();
    }

    if (shift != SHIFT_RRX) {
        /* Whitespace can appear here if the next thing is a bare digit.	*/
        skip_whitespace (p);

        if (mode == NO_SHIFT_RESTRICT
        && (reg = arm_reg_parse (&p, REG_TYPE_RN)) != FAIL) {
            inst.operands[i].imm = reg;
            inst.operands[i].immisreg = 1;
        }
        else if (my_get_expression (&inst.relocs[0].exp, &p, GE_IMM_PREFIX))
        return FAIL;
    }
    inst.operands[i].shift_kind = shift;
    inst.operands[i].shifted = 1;
    *str = p;
    return SUCCESS;
}

/**
 * @Description: OP_SH
 */
static int
parse_shifter_operand (char **str, int i) {
    int value;
    expressionS exp;

    if ((value = arm_reg_parse (str, REG_TYPE_RN)) != FAIL) {
        inst.operands[i].reg = value;
        inst.operands[i].isreg = 1;

        /* parse_shift will override this if appropriate */
        inst.relocs[0].exp.X_op = O_constant;
        inst.relocs[0].exp.X_add_number = 0;

        if (skip_past_comma (str) == FAIL)
            return SUCCESS;

        /* Shift operation on register.  */
        return parse_shift (str, i, NO_SHIFT_RESTRICT);
    }

    // 处理#0,...
    // inst中的relocs[0]中的exp存放了汇编代码中的立即数
    if (my_get_expression (&inst.relocs[0].exp, str, GE_IMM_PREFIX))
        return FAIL;
    if (skip_past_comma (str) == SUCCESS) {
        /* #x, y -- ie explicit rotation by Y.  */
        if (my_get_expression (&exp, str, GE_NO_PREFIX))
            return FAIL;

        if (exp.X_op != O_constant || inst.relocs[0].exp.X_op != O_constant) {
            inst.error = _("constant expression expected");
            return FAIL;
        }

        value = exp.X_add_number;
        if (value < 0 || value > 30 || value % 2 != 0) {
            inst.error = _("invalid rotation");
            return FAIL;
        }
        if (inst.relocs[0].exp.X_add_number < 0
        || inst.relocs[0].exp.X_add_number > 255) {
            inst.error = _("invalid constant");
            return FAIL;
        }

        /* Encode as specified.  */
        inst.operands[i].imm = inst.relocs[0].exp.X_add_number | value << 7;
        return SUCCESS;
    }

    inst.relocs[0].type = BFD_RELOC_ARM_IMMEDIATE;
    inst.relocs[0].pc_rel = 0;
    return SUCCESS;
}

static int
parse_big_immediate (char **str, int i, expressionS *in_exp,
		     bool allow_symbol_p) {
    expressionS exp;
    expressionS *exp_p = in_exp ? in_exp : &exp;
    char *ptr = *str;
    if (exp_p->X_op == O_constant) {
        inst.operands[i].imm = exp_p->X_add_number & 0xffffffff;
        /* If we're on a 64-bit host, then a 64-bit number can be returned using
        O_constant.  We have to be careful not to break compilation for
        32-bit X_add_number, though.  */
        if ((exp_p->X_add_number & ~(offsetT)(0xffffffffU)) != 0) {
            /* X >> 32 is illegal if sizeof (exp_p->X_add_number) == 4.  */
            inst.operands[i].reg = (((exp_p->X_add_number >> 16) >> 16)
                        & 0xffffffff);
            inst.operands[i].regisimm = 1;
        }
    } else if (exp_p->X_op == O_big
	   && LITTLENUM_NUMBER_OF_BITS * exp_p->X_add_number > 32) {
        unsigned parts = 32 / LITTLENUM_NUMBER_OF_BITS, j, idx = 0;

        /* Bignums have their least significant bits in
        generic_bignum[0]. Make sure we put 32 bits in imm and
        32 bits in reg,  in a (hopefully) portable way.  */
        gas_assert (parts != 0);
        if (LITTLENUM_NUMBER_OF_BITS * exp_p->X_add_number > 64) {
            LITTLENUM_TYPE m = -1;

            if (generic_bignum[parts * 2] != 0
                && generic_bignum[parts * 2] != m)
                return FAIL;

            for (j = parts * 2 + 1; j < (unsigned) exp_p->X_add_number; j++)
                if (generic_bignum[j] != generic_bignum[j-1])
            return FAIL;
        }
        inst.operands[i].imm = 0;
        for (j = 0; j < parts; j++, idx++)
            inst.operands[i].imm |= ((unsigned) generic_bignum[idx]
                        << (LITTLENUM_NUMBER_OF_BITS * j));
            inst.operands[i].reg = 0;
        for (j = 0; j < parts; j++, idx++)
            inst.operands[i].reg |= ((unsigned) generic_bignum[idx]
                        << (LITTLENUM_NUMBER_OF_BITS * j));
        inst.operands[i].regisimm = 1;

    } else 
        return FAIL;
    *str = ptr;

    return SUCCESS;
}

/**
 * @Description: 对汇编代码除去操作码的操作数代码进行解析
 */
static parse_operand_result
parse_address_main (char **str,int i,int group_relocations,group_reloc_type group_type) {
    char *p = *str;
    int reg;
    // 如str r0,[sp]   则去掉[
    if (skip_past_char (&p, '[') == FAIL) {
        // 再说....
        if (group_type == GROUP_MVE
            && (reg = arm_reg_parse (&p, REG_TYPE_RN)) != FAIL) {
            /* [r0-r15] expected as argument but receiving r0-r15 without
                    [] brackets.  */
            inst.error = BAD_SYNTAX;
            return PARSE_OPERAND_FAIL;
        } else if (skip_past_char(&p,'=') == FAIL) {
            /* Bare address - translate to PC-relative offset.  */
            inst.relocs[0].pc_rel = 1;
            inst.operands[i].reg = REG_PC;
            inst.operands[i].isreg = 1;
            inst.operands[i].preind = 1;

            if (my_get_expression (&inst.relocs[0].exp, &p, GE_OPT_PREFIX_BIG))
                return PARSE_OPERAND_FAIL;
        } else if (parse_big_immediate (&p, i, &inst.relocs[0].exp,
				    /*allow_symbol_p=*/true))
            return PARSE_OPERAND_FAIL;

        *str = p;
        return PARSE_OPERAND_SUCCESS;
    }
    /* PR gas/14887: Allow for whitespace after the opening bracket.  */
    skip_whitespace (p);
    if (group_type == GROUP_MVE) {
        enum arm_reg_type rtype = REG_TYPE_MQ;
        struct neon_type_el et;
        if ((reg = arm_typed_reg_parse (&p, rtype, &rtype, &et)) != FAIL) {
            inst.operands[i].isquad = 1;
        }
        else if ((reg = arm_reg_parse (&p, REG_TYPE_RN)) == FAIL) {
            inst.error = BAD_ADDR_MODE;
            return PARSE_OPERAND_FAIL;
        }
    } else if ((reg = arm_reg_parse (&p, REG_TYPE_RN)) == FAIL) {
        if (group_type == GROUP_MVE)
            inst.error = BAD_ADDR_MODE;
        else
            inst.error = _(reg_expected_msgs[REG_TYPE_RN]);
        return PARSE_OPERAND_FAIL;
    }
    inst.operands[i].reg = reg;
    inst.operands[i].isreg = 1;

    if (skip_past_comma (&p) == SUCCESS) {
        inst.operands[i].preind = 1;
        if (*p == '+') p++;
        else if (*p == '-') p++, inst.operands[i].negative = 1;
        enum arm_reg_type rtype = REG_TYPE_MQ;
        struct neon_type_el et;
        if ((reg = arm_reg_parse (&p, REG_TYPE_RN)) != FAIL) {
            inst.operands[i].imm = reg;
            inst.operands[i].immisreg = 1;

            if (skip_past_comma (&p) == SUCCESS)
                if (parse_shift (&p, i, SHIFT_IMMEDIATE) == FAIL)
                    return PARSE_OPERAND_FAIL;
        } else if (skip_past_char (&p, ':') == SUCCESS) {
            // parse_operand_result result = parse_neon_alignment (&p, i);

            // if (result != PARSE_OPERAND_SUCCESS)
            //     return result;
        } else {
            if (inst.operands[i].negative) {
                inst.operands[i].negative = 0;
                p--;
            }
            if (group_relocations
                && ((*p == '#' && *(p + 1) == ':') || *p == ':')) {
                    
            } else {
                char *q = p;
                if (my_get_expression (&inst.relocs[0].exp, &p, GE_IMM_PREFIX))
                    return PARSE_OPERAND_FAIL;
                if (inst.relocs[0].exp.X_op == O_constant
                    && inst.relocs[0].exp.X_add_number == 0) {
                    skip_whitespace (q);
                    if (*q == '#') {
                        q++;
                        skip_whitespace (q);
                    }
                    if (*q == '-')
                        inst.operands[i].negative = 1;
                }
            }          
        }      
    }
    if (skip_past_char (&p, ']') == FAIL) {
        inst.error = _("']' expected");
        return PARSE_OPERAND_FAIL;
    }

    if (skip_past_char (&p, '!') == SUCCESS)
        inst.operands[i].writeback = 1;
    else if (skip_past_comma (&p) == SUCCESS){
        if (skip_past_char (&p, '{') == SUCCESS) {

        } else {
            inst.operands[i].postind = 1;
            inst.operands[i].writeback = 1;
            if (inst.operands[i].preind) {
                inst.error = _("cannot combine pre- and post-indexing");
                return PARSE_OPERAND_FAIL;
            }
            if (*p == '+') p++;
            else if (*p == '-') p++, inst.operands[i].negative = 1;

            enum arm_reg_type rtype = REG_TYPE_MQ;
            struct neon_type_el et;

            if ((reg = arm_reg_parse (&p, REG_TYPE_RN)) != FAIL) {

            } else {
                char *q = p;

                if (inst.operands[i].negative) {
                    inst.operands[i].negative = 0;
                    p--;
                }
                if (my_get_expression (&inst.relocs[0].exp, &p, GE_IMM_PREFIX))
                    return PARSE_OPERAND_FAIL;
            }
        }
    }
    if (inst.operands[i].preind == 0 && inst.operands[i].postind == 0) {
        inst.operands[i].preind = 1;
        inst.relocs[0].exp.X_op = O_constant;
        inst.relocs[0].exp.X_add_number = 0;
    }
    *str = p;
    return PARSE_OPERAND_SUCCESS;
}

/**
 * @Description: str r0, [sp]
 */
static parse_operand_result
parse_address_group_reloc (char **str, int i, group_reloc_type type) {
    return parse_address_main (str, i, 1, type);
}

/**
 * @Description: 解析arm汇编中的操作数形式例如
 * #<immediate>
 * #<immediate>, <rotate>
 * <Rm>
 * <Rm>, <shift>
 */
static parse_operand_result
parse_shifter_operand_group_reloc (char **str, int i) {
    return parse_shifter_operand (str, i) == SUCCESS
	   ? PARSE_OPERAND_SUCCESS : PARSE_OPERAND_FAIL;
}

/* Parse an ARM register list.  Returns the bitmask, or FAIL.  */
static long
parse_reg_list (char ** strp, enum reg_list_els etype) {
    char *str = *strp;
    long range = 0;
    int another_range;

    gas_assert (etype == REGLIST_RN || etype == REGLIST_CLRM
            || etype == REGLIST_PSEUDO);
    do {
        skip_whitespace (str);

        another_range = 0;
        if (*str == '{') {
            int in_range = 0;
            int cur_reg = -1;
            str++;

            do {
                int reg;
                const char apsr_str[] = "apsr";
                int apsr_str_len = strlen (apsr_str);
                enum arm_reg_type rt;

                if (etype == REGLIST_RN || etype == REGLIST_CLRM)
                    rt = REG_TYPE_RN;
                else
                    rt = REG_TYPE_PSEUDO;
                reg = arm_reg_parse (&str, rt);

                if (etype == REGLIST_CLRM) {

                } else {
                    if (reg == FAIL) {
                        first_error (_(reg_expected_msgs[REGLIST_RN]));
                        return FAIL;
                    }
                }

                if (in_range) {

                }
                if (range & (1 << reg))
                    as_tsktsk (_("Warning: duplicated register (r%d) in register list"),
                        reg);
                else if (reg <= cur_reg)
                    as_tsktsk (_("Warning: register range not in ascending order"));

                range |= 1 << reg;
                cur_reg = reg;
            } while (skip_past_comma (&str) != FAIL
                || (in_range = 1, *str++ == '-'));

            str--;
            if (skip_past_char (&str, '}') == FAIL) {
                first_error (_("missing `}'"));
                return FAIL;
            }
        }
    } while (another_range);

    *strp = str;
    return range;
}

static int
parse_immediate (char **str, int *val, int min, int max,
		 bool prefix_opt) {
    expressionS exp;

    my_get_expression (&exp, str, prefix_opt ? GE_OPT_PREFIX : GE_IMM_PREFIX);
    if (exp.X_op != O_constant) {
        inst.error = _("constant expression required");
        return FAIL;
    }

    if (exp.X_add_number < min || exp.X_add_number > max) {
        inst.error = _("immediate value out of range");
        return FAIL;
    }

    *val = exp.X_add_number;
    return SUCCESS;
}
/**
 * @Description: 分析操作数   操作数解析器
 */
static int
parse_operands (char *str, const unsigned int *pattern, bool thumb) {
    unsigned const int *upat = pattern;
    char *backtrack_pos = 0;
    const char *backtrack_error = 0;
    int i, val = 0, backtrack_index = 0;
    enum arm_reg_type rtype;
    parse_operand_result result;
    unsigned int op_parse_code;
    bool partial_match;
#define po_char_or_fail(chr)			\
  do						\
    {						\
      if (skip_past_char (&str, chr) == FAIL)	\
	goto bad_args;				\
    }						\
  while (0)

#define po_reg_or_fail(regtype)					\
  do								\
    {								\
      val = arm_typed_reg_parse (& str, regtype, & rtype,	\
				 & inst.operands[i].vectype);	\
      if (val == FAIL)						\
	{							\
	  first_error (_(reg_expected_msgs[regtype]));		\
	  goto failure;						\
	}							\
      inst.operands[i].reg = val;				\
      inst.operands[i].isreg = 1;				\
      inst.operands[i].isquad = (rtype == REG_TYPE_NQ);		\
      inst.operands[i].issingle = (rtype == REG_TYPE_VFS);	\
      inst.operands[i].isvec = (rtype == REG_TYPE_VFS		\
			     || rtype == REG_TYPE_VFD		\
			     || rtype == REG_TYPE_NQ);		\
      inst.operands[i].iszr = (rtype == REG_TYPE_ZR);		\
    }								\
  while (0)

#define po_misc_or_fail(expr)			\
  do						\
    {						\
      if (expr)					\
	goto failure;				\
    }						\
  while (0)

#define po_misc_or_fail_no_backtrack(expr)		\
  do							\
    {							\
      result = expr;					\
      if (result == PARSE_OPERAND_FAIL_NO_BACKTRACK)	\
	backtrack_pos = 0;				\
      if (result != PARSE_OPERAND_SUCCESS)		\
	goto failure;					\
    }							\
  while (0)

#define po_imm_or_fail(min, max, popt)				\
  do								\
    {								\
      if (parse_immediate (&str, &val, min, max, popt) == FAIL)	\
	goto failure;						\
      inst.operands[i].imm = val;				\
    }								\
  while (0)

    skip_whitespace (str);
    for (i = 0; upat[i] != OP_stop; i++) {
        op_parse_code = upat[i];
        if (op_parse_code >= 1<<16)
            op_parse_code = thumb ? (op_parse_code >> 16)
                    : (op_parse_code & ((1<<16)-1));
        
        if (op_parse_code >= OP_FIRST_OPTIONAL) {
            /* Remember where we are in case we need to backtrack. nop  */
            backtrack_pos = str;
            backtrack_error = inst.error;
            backtrack_index = i;
        }
        if (i > 0 && (i > 1 || inst.operands[0].present))
            po_char_or_fail (',');
        switch (op_parse_code) {
            case OP_oRRnpc:
            case OP_oRRnpcsp:
            case OP_RRnpc:
            case OP_RRnpcsp:
            case OP_oRR:
            case OP_RRe:
            case OP_RRo:
            case OP_LR:
            case OP_oLR:
            case OP_SP:
            case OP_R12:
            // mov str
            case OP_RR:    po_reg_or_fail (REG_TYPE_RN);	  break;
            case OP_oRNSDQ:
            case OP_RNSDQ: po_reg_or_fail (REG_TYPE_NSDQ);    break;
            case OP_VMOV:
                /* WARNING: parse_neon_mov can move the operand counter, i. If we're
                    not careful then bad things might happen.  */
                // po_misc_or_fail (parse_neon_mov (&str, &i) == FAIL);
                break;
            // #0
            case OP_SH:
                po_misc_or_fail (parse_shifter_operand (&str, i));
                break;
            // str
            case OP_ADDRGLDR:
                po_misc_or_fail_no_backtrack (
                    // 解析地址，可能会进行重定位
                    parse_address_group_reloc (&str, i, GROUP_LDR));
                break;
            case OP_SHG:
                po_misc_or_fail_no_backtrack (
                    parse_shifter_operand_group_reloc (&str, i));
                break;
            // push 操作符
            case OP_REGLST:
                val = parse_reg_list (&str, REGLIST_RN);
                if (*str == '^') {
                    inst.operands[i].writeback = 1;
                    str++;
                }
                break;
            // 114 bl func
            case OP_EXPr:	EXPr:
                po_misc_or_fail (my_get_expression (&inst.relocs[0].exp, &str,
                                    GE_NO_PREFIX));
                if (inst.relocs[0].exp.X_op == O_symbol) {
                    val = parse_reloc (&str);
                    if (val == -1) {
                        inst.error = _("unrecognized relocation suffix");
                        goto failure;
                    } else if (val != BFD_RELOC_UNUSED) {
                        inst.operands[i].imm = val;
                        inst.operands[i].hasreloc = 1;
                    }
                }
                break;
            case OP_RRnpctw:
            case OP_RRw:
            case OP_oRRw:
                po_reg_or_fail (REG_TYPE_RN);
                if (skip_past_char (&str, '!') == SUCCESS)
                    inst.operands[i].writeback = 1;
                break;
            case OP_oI255c:
                po_char_or_fail ('{');
                po_imm_or_fail (0, 255, true);
                po_char_or_fail ('}');
                break;
            default:
                as_fatal (_("unhandled operand code %d"), op_parse_code);
        }

        switch (op_parse_code) {
            case OP_oRRnpc:
            case OP_RRnpc:
            case OP_RRnpcb:
            case OP_RRw:
            case OP_oRRw:
            case OP_RRnpc_I0:
                if (inst.operands[i].isreg && inst.operands[i].reg == REG_PC)
                    inst.error = BAD_PC;
                break;
                
            case OP_CPSF:
            case OP_ENDI:
            case OP_oROR:
            case OP_wPSR:
            case OP_rPSR:
            case OP_COND:
            case OP_oBARRIER_I15:
            case OP_REGLST:
            case OP_CLRMLST:
            case OP_VRSLST:
            case OP_VRDLST:
            case OP_VRSDLST:
            case OP_VRSDVLST:
            case OP_NRDLST:
            case OP_NSTRLST:
            case OP_MSTRLST2:
            case OP_MSTRLST4:
                if (val == FAIL)
                    goto failure;
                inst.operands[i].imm = val;
                break;
        }
        inst.operands[i].present = 1;
        continue;

        bad_args:
            inst.error = BAD_ARGS;
        failure:
            if (!backtrack_pos) {
                /* The parse routine should already have set inst.error, but set a
                    default here just in case.  */
                if (!inst.error)
                    inst.error = BAD_SYNTAX;
                return FAIL;
            }
            /* Do not backtrack over a trailing optional argument that
            absorbed some text.  We will only fail again, with the
            'garbage following instruction' error message, which is
            probably less helpful than the current one.  */
            if (backtrack_index == i && backtrack_pos != str
                && upat[i+1] == OP_stop) {
                if (!inst.error)
                    inst.error = BAD_SYNTAX;
                return FAIL;
            }
            str = backtrack_pos;
            inst.error = backtrack_error;
            inst.operands[backtrack_index].present = 0;
            i = backtrack_index;
            backtrack_pos = 0;
    }
    if (*str != '\0' && !inst.error)
        inst.error = _("garbage following instruction");
    return inst.error ? FAIL : SUCCESS;
}

static void
it_fsm_pre_encode (void) {
    if (inst.cond != COND_ALWAYS)
        inst.pred_insn_type =  INSIDE_IT_INSN;
    else
        // 该指令不在IT块内部
        inst.pred_insn_type = OUTSIDE_PRED_INSN;

    now_pred.state_handled = 0;
}

static void
it_fsm_post_encode (void)
{
    int is_last;

    if (!now_pred.state_handled)
        handle_pred_state ();

    if (now_pred.insn_cond
        && warn_on_restrict_it
        && !now_pred.warn_deprecated
        && warn_on_deprecated
        && (ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v8)
            || ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v8r))
        && !ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_m)) {
        if (inst.instruction >= 0x10000) {
            as_tsktsk (_("IT blocks containing 32-bit Thumb instructions are "
                    "performance deprecated in ARMv8-A and ARMv8-R"));
            now_pred.warn_deprecated = true;
        } 

        if (now_pred.block_length > 1) {
            as_tsktsk (_("IT blocks containing more than one conditional "
                    "instruction are performance deprecated in ARMv8-A and "
                    "ARMv8-R"));
            now_pred.warn_deprecated = true;
        }
    }

    is_last = (now_pred.mask == 0x10);
    if (is_last) {
        now_pred.state = OUTSIDE_PRED_BLOCK;
        now_pred.mask = 0;
    }
}


static void
fix_new_arm (fragS *	   frag,
	     int	   where,
	     short int	   size,
	     expressionS * exp,
	     int	   pc_rel,
	     int	   reloc) {
    fixS *	   new_fix;

    switch (exp->X_op) {
        case O_constant:
            if (pc_rel) {
                /* Create an absolute valued symbol, so we have something to
                    refer to in the object file.  Unfortunately for us, gas's
                    generic expression parsing will already have folded out
                    any use of .set foo/.type foo %function that may have
                    been used to set type information of the target location,
                    that's being specified symbolically.  We have to presume
                    the user knows what they are doing.  */
                char name[16 + 8];
                symbolS *symbol;

                sprintf (name, "*ABS*0x%lx", (unsigned long)exp->X_add_number);

                symbol = symbol_find_or_make (name);
                S_SET_SEGMENT (symbol, absolute_section);
                symbol_set_frag (symbol, &zero_address_frag);
                S_SET_VALUE (symbol, exp->X_add_number);
                exp->X_op = O_symbol;
                exp->X_add_symbol = symbol;
                exp->X_add_number = 0;
            }
        /* FALLTHROUGH */
        case O_symbol:
        case O_add:
        case O_subtract:
            new_fix = fix_new_exp (frag, where, size, exp, pc_rel,
                        (enum bfd_reloc_code_real) reloc);
            break;

        default:
            new_fix = (fixS *) fix_new (frag, where, size, make_expr_symbol (exp), 0,
                        pc_rel, (enum bfd_reloc_code_real) reloc);
            break;
    }

    /* Mark whether the fix is to a THUMB instruction, or an ARM
        instruction.  */
    new_fix->tc_fix_data = thumb_mode;
}


/**
 * @Description: 输出指令序列
 */
static void
output_inst (const char * str) {
     char * to = NULL;

    if (inst.error) {
        as_bad ("%s -- `%s'", inst.error, str);
        return;
    }
    // if (inst.relax) {
    //     // output_relax_insn ();
    //     return;
    // }
    if (inst.size == 0)
        return;

    to = frag_more (inst.size);
    frag_now->tc_frag_data.thumb_mode = thumb_mode | MODE_RECORDED;
    if (inst.size > INSN_SIZE) {
        gas_assert (inst.size == (2 * INSN_SIZE));
        md_number_to_chars (to, inst.instruction, INSN_SIZE);
        md_number_to_chars (to + INSN_SIZE, inst.instruction, INSN_SIZE);
    }
    else
        // 将给定的指令编码转换为字符形式
        md_number_to_chars (to, inst.instruction, inst.size);
    int r;
    for (r = 0; r < ARM_IT_MAX_RELOCS; r++) {
        if (inst.relocs[r].type != BFD_RELOC_UNUSED)
            fix_new_arm (frag_now, to - frag_now->fr_literal,
                    inst.size, & inst.relocs[r].exp, inst.relocs[r].pc_rel,
                    inst.relocs[r].type);
    }
}

/**
 * @Description: 处理arm汇编语言
 */
void
md_assemble (char *str) {
    char *p = str;
    // op 操作码
    const struct asm_opcode *opcode;
    if (last_label_seen != NULL) {
        // 将last_label_seen symbol的frag设置为上一个frag：frag_now
        symbol_set_frag (last_label_seen, frag_now);
        S_SET_VALUE (last_label_seen, (valueT) frag_now_fix ());
        S_SET_SEGMENT (last_label_seen, now_seg);
    }
    memset (&inst, '\0', sizeof (inst));
    int r;
    // 重定位类型
    for (r = 0; r < ARM_IT_MAX_RELOCS; r++)
        inst.relocs[r].type = BFD_RELOC_UNUSED;
    // 查找操作码
    opcode = opcode_lookup (&p);

   /* 定无条件指令的条件字段的值，以指示无条件执行 */
    inst.uncond_value = (opcode->tag == OT_csuffixF) ?0xf: -1u;  
    // 默认为arm模式
    if (ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v1)) {
        bool is_bx;
        is_bx = (opcode->aencode == do_bx);
        /* 判断架构是否支持这条指令 */
        if (!(is_bx && fix_v4bx)
            && !(opcode->avariant &&
            ARM_CPU_HAS_FEATURE (cpu_variant, *opcode->avariant))) {
            as_bad (_("selected processor does not support `%s' in ARM mode"), str);
            return;
        }
        if (inst.size_req) {
            as_bad (_("width suffixes are invalid in ARM mode -- `%s'"), str);
            return;
        }
        inst.instruction = opcode->avalue;
        if (opcode->tag == OT_unconditionalF)
            inst.instruction |= 0xFU << 28;
        else
            inst.instruction |= inst.cond << 28;
        inst.size = INSN_SIZE;
        // 处理操作数 p为该行汇编代码（除了操作符的
        if (!parse_operands (p, opcode->operands, /*thumb=*/false)) {
            it_fsm_pre_encode ();
            // // 执行mov指令的操作
            opcode->aencode ();
            it_fsm_post_encode ();
        }
         /* Arm mode bx is marked as both v4T and v5 because it's still required
        on a hypothetical non-thumb v5 core.  */
        if (is_bx)
            ARM_MERGE_FEATURE_SETS (arm_arch_used, arm_arch_used, arm_ext_v4t);
        else
            ARM_MERGE_FEATURE_SETS (arm_arch_used, arm_arch_used,
                    *opcode->avariant);
        if (!inst.error) {
            mapping_state (MAP_ARM);
        }
    }
    output_inst (str);
}

/**
 * @Description: 请填写简介
 */
static void
check_pred_blocks_finished (void) {
#ifdef OBJ_ELF
    asection *sect;

    for (sect = stdoutput->sections; sect != NULL; sect = sect->next)
        if (seg_info (sect)->tc_segment_info_data.current_pred.state
        == MANUAL_PRED_BLOCK) {
        if (now_pred.type == SCALAR_PRED)
            as_warn (_("section '%s' finished with an open IT block."),
                sect->name);
        else
            as_warn (_("section '%s' finished with an open VPT/VPST block."),
                sect->name);
    }
#else
  if (now_pred.state == MANUAL_PRED_BLOCK)
    {
      if (now_pred.type == SCALAR_PRED)
       as_warn (_("file finished with an open IT block."));
      else
	as_warn (_("file finished with an open VPT/VPST block."));
    }
#endif
}

/**
 * @Description: 切换节时需要发出一个新的映射符号
 */
void
arm_elf_change_section (void)
{
    /* Link an unlinked unwind index table section to the .text section.	*/
    if (elf_section_type (now_seg) == SHT_ARM_EXIDX
        && elf_linked_to_section (now_seg) == NULL)
        elf_linked_to_section (now_seg) = text_section;
}

void
arm_cleanup (void) {
    literal_pool * pool;
    /* Ensure that all the predication blocks are properly closed.  */
    check_pred_blocks_finished ();
    for (pool = list_of_pools; pool; pool = pool->next) {
        /* Put it at the end of the relevant section.  */
        subseg_set (pool->section, pool->sub_section);
#ifdef OBJ_ELF
        arm_elf_change_section ();
#endif
        // s_ltorg (0);
    }
}

void
arm_md_finish (void) {
    if (EF_ARM_EABI_VERSION (meabi_flags) < EF_ARM_EABI_VER4)
        return;

    //   aeabi_set_public_attributes ();
    printf("忘记设置aeabi_set_public_attributes ()\n");
}

static void
insert_data_mapping_symbol (enum mstate state,
			    valueT value, fragS *frag, offsetT bytes) {
    /* If there was already a mapping symbol, remove it.  */
    if (frag->tc_frag_data.last_map != NULL
        && S_GET_VALUE (frag->tc_frag_data.last_map) == frag->fr_address + value) {
        symbolS *symp = frag->tc_frag_data.last_map;

        if (value == 0) {
            know (frag->tc_frag_data.first_map == symp);
            frag->tc_frag_data.first_map = NULL;
        }
        frag->tc_frag_data.last_map = NULL;
        symbol_remove (symp, &symbol_rootP, &symbol_lastP);
    }

    make_mapping_symbol (MAP_DATA, value, frag);
    make_mapping_symbol (state, value + bytes, frag);
}

void
arm_handle_align (fragS * fragP)
{
  static unsigned char const arm_noop[2][2][4] =
    {
      {  /* ARMv1 */
	{0x00, 0x00, 0xa0, 0xe1},  /* LE */
	{0xe1, 0xa0, 0x00, 0x00},  /* BE */
      },
      {  /* ARMv6k */
	{0x00, 0xf0, 0x20, 0xe3},  /* LE */
	{0xe3, 0x20, 0xf0, 0x00},  /* BE */
      },
    };
  static unsigned char const thumb_noop[2][2][2] =
    {
      {  /* Thumb-1 */
	{0xc0, 0x46},  /* LE */
	{0x46, 0xc0},  /* BE */
      },
      {  /* Thumb-2 */
	{0x00, 0xbf},  /* LE */
	{0xbf, 0x00}   /* BE */
      }
    };
  static unsigned char const wide_thumb_noop[2][4] =
    {  /* Wide Thumb-2 */
      {0xaf, 0xf3, 0x00, 0x80},  /* LE */
      {0xf3, 0xaf, 0x80, 0x00},  /* BE */
    };

  unsigned bytes, fix, noop_size;
  char * p;
  const unsigned char * noop;
  const unsigned char *narrow_noop = NULL;
#ifdef OBJ_ELF
  enum mstate state;
#endif

  if (fragP->fr_type != rs_align_code)
    return;

  bytes = fragP->fr_next->fr_address - fragP->fr_address - fragP->fr_fix;
  p = fragP->fr_literal + fragP->fr_fix;
  fix = 0;

  if (bytes > MAX_MEM_FOR_RS_ALIGN_CODE)
    bytes &= MAX_MEM_FOR_RS_ALIGN_CODE;

  gas_assert ((fragP->tc_frag_data.thumb_mode & MODE_RECORDED) != 0);

  if (fragP->tc_frag_data.thumb_mode & (~ MODE_RECORDED))
    {
      if (ARM_CPU_HAS_FEATURE (selected_cpu_name[0]
			       ? selected_cpu : arm_arch_none, arm_ext_v6t2))
	{
	  narrow_noop = thumb_noop[1][target_big_endian];
	  noop = wide_thumb_noop[target_big_endian];
	}
      else
	noop = thumb_noop[0][target_big_endian];
      noop_size = 2;
#ifdef OBJ_ELF
      state = MAP_THUMB;
#endif
    }
  else
    {
      noop = arm_noop[ARM_CPU_HAS_FEATURE (selected_cpu_name[0]
					   ? selected_cpu : arm_arch_none,
					   arm_ext_v6k) != 0]
		     [target_big_endian];
      noop_size = 4;
#ifdef OBJ_ELF
      state = MAP_ARM;
#endif
    }

  fragP->fr_var = noop_size;

  if (bytes & (noop_size - 1))
    {
      fix = bytes & (noop_size - 1);
#ifdef OBJ_ELF
      insert_data_mapping_symbol (state, fragP->fr_fix, fragP, fix);
#endif
      memset (p, 0, fix);
      p += fix;
      bytes -= fix;
    }

  if (narrow_noop)
    {
      if (bytes & noop_size)
	{
	  /* Insert a narrow noop.  */
	  memcpy (p, narrow_noop, noop_size);
	  p += noop_size;
	  bytes -= noop_size;
	  fix += noop_size;
	}

      /* Use wide noops for the remainder */
      noop_size = 4;
    }

  while (bytes >= noop_size)
    {
      memcpy (p, noop, noop_size);
      p += noop_size;
      bytes -= noop_size;
      fix += noop_size;
    }

  fragP->fr_fix += fix;
}
typedef struct {
    int val;
    arm_feature_set flags;
} cpu_arch_ver_table;

static const cpu_arch_ver_table cpu_arch_ver[] =
{
    {TAG_CPU_ARCH_PRE_V4,     ARM_ARCH_V1},
    {TAG_CPU_ARCH_PRE_V4,     ARM_ARCH_V2},
    {TAG_CPU_ARCH_PRE_V4,     ARM_ARCH_V2S},
    {TAG_CPU_ARCH_PRE_V4,     ARM_ARCH_V3},
    {TAG_CPU_ARCH_PRE_V4,     ARM_ARCH_V3M},
    {TAG_CPU_ARCH_V4,	      ARM_ARCH_V4xM},
    {TAG_CPU_ARCH_V4,	      ARM_ARCH_V4},
    {TAG_CPU_ARCH_V4T,	      ARM_ARCH_V4TxM},
    {TAG_CPU_ARCH_V4T,	      ARM_ARCH_V4T},
    {TAG_CPU_ARCH_V5T,	      ARM_ARCH_V5xM},
    {TAG_CPU_ARCH_V5T,	      ARM_ARCH_V5},
    {TAG_CPU_ARCH_V5T,	      ARM_ARCH_V5TxM},
    {TAG_CPU_ARCH_V5T,	      ARM_ARCH_V5T},
    {TAG_CPU_ARCH_V5TE,	      ARM_ARCH_V5TExP},
    {TAG_CPU_ARCH_V5TE,	      ARM_ARCH_V5TE},
    {TAG_CPU_ARCH_V5TEJ,      ARM_ARCH_V5TEJ},
    {TAG_CPU_ARCH_V6,	      ARM_ARCH_V6},
    {TAG_CPU_ARCH_V6KZ,	      ARM_ARCH_V6Z},
    {TAG_CPU_ARCH_V6KZ,	      ARM_ARCH_V6KZ},
    {TAG_CPU_ARCH_V6K,	      ARM_ARCH_V6K},
    {TAG_CPU_ARCH_V6T2,	      ARM_ARCH_V6T2},
    {TAG_CPU_ARCH_V6T2,	      ARM_ARCH_V6KT2},
    {TAG_CPU_ARCH_V6T2,	      ARM_ARCH_V6ZT2},
    {TAG_CPU_ARCH_V6T2,	      ARM_ARCH_V6KZT2},

    /* When assembling a file with only ARMv6-M or ARMv6S-M instruction, GNU as
       always selected build attributes to match those of ARMv6-M
       (resp. ARMv6S-M).  However, due to these architectures being a strict
       subset of ARMv7-M in terms of instructions available, ARMv7-M attributes
       would be selected when fully respecting chronology of architectures.
       It is thus necessary to make a special case of ARMv6-M and ARMv6S-M and
       move them before ARMv7 architectures.  */
    {TAG_CPU_ARCH_V6_M,	      ARM_ARCH_V6M},
    {TAG_CPU_ARCH_V6S_M,      ARM_ARCH_V6SM},

    {TAG_CPU_ARCH_V7,	      ARM_ARCH_V7},
    {TAG_CPU_ARCH_V7,	      ARM_ARCH_V7A},
    {TAG_CPU_ARCH_V7,	      ARM_ARCH_V7R},
    {TAG_CPU_ARCH_V7,	      ARM_ARCH_V7M},
    {TAG_CPU_ARCH_V7,	      ARM_ARCH_V7VE},
    {TAG_CPU_ARCH_V7E_M,      ARM_ARCH_V7EM},
    {TAG_CPU_ARCH_V8,	      ARM_ARCH_V8A},
    {TAG_CPU_ARCH_V8,	      ARM_ARCH_V8_1A},
    {TAG_CPU_ARCH_V8,	      ARM_ARCH_V8_2A},
    {TAG_CPU_ARCH_V8,	      ARM_ARCH_V8_3A},
    {TAG_CPU_ARCH_V8M_BASE,   ARM_ARCH_V8M_BASE},
    {TAG_CPU_ARCH_V8M_MAIN,   ARM_ARCH_V8M_MAIN},
    {TAG_CPU_ARCH_V8R,	      ARM_ARCH_V8R},
    {TAG_CPU_ARCH_V8,	      ARM_ARCH_V8_4A},
    {TAG_CPU_ARCH_V8,	      ARM_ARCH_V8_5A},
    {TAG_CPU_ARCH_V8_1M_MAIN, ARM_ARCH_V8_1M_MAIN},
    {TAG_CPU_ARCH_V8,	    ARM_ARCH_V8_6A},
    {TAG_CPU_ARCH_V8,	    ARM_ARCH_V8_7A},
    {TAG_CPU_ARCH_V8,	    ARM_ARCH_V8_8A},
    {TAG_CPU_ARCH_V9,	    ARM_ARCH_V9A},
    {TAG_CPU_ARCH_V9,	    ARM_ARCH_V9_1A},
    {TAG_CPU_ARCH_V9,	    ARM_ARCH_V9_2A},
    {TAG_CPU_ARCH_V9,	    ARM_ARCH_V9_3A},
    {-1,		    ARM_ARCH_NONE}
};

valueT
md_section_align (segT	 segment ATTRIBUTE_UNUSED,
		  valueT size) {
    return size;
}

static bool
no_cpu_selected (void) {
    return ARM_FEATURE_EQUAL (selected_cpu, arm_arch_none);
}


static int
get_aeabi_cpu_arch_from_fset (const arm_feature_set *arch_ext_fset,
			      const arm_feature_set *ext_fset,
			      char *profile, int exact_match) {
    arm_feature_set arch_fset;
    const cpu_arch_ver_table *p_ver, *p_ver_ret = NULL;
    if (ARM_FEATURE_EQUAL (*arch_ext_fset, arm_arch_any)) {
        /* Force revisiting of decision for each new architecture.  */
        gas_assert (MAX_TAG_CPU_ARCH <= TAG_CPU_ARCH_V9);
        *profile = 'A';
        return TAG_CPU_ARCH_V9;
    }
    ARM_CLEAR_FEATURE (arch_fset, *arch_ext_fset, *ext_fset);
    for (p_ver = cpu_arch_ver; p_ver->val != -1; p_ver++) {
        arm_feature_set known_arch_fset;
        ARM_CLEAR_FEATURE (known_arch_fset, p_ver->flags, fpu_any);
        if (exact_match) {
            if (ARM_FEATURE_EQUAL (*arch_ext_fset, known_arch_fset)) {
                p_ver_ret = p_ver;
                goto found;
            } else if (p_ver_ret == NULL
                       && ARM_FEATURE_EQUAL (arch_fset, known_arch_fset))
                p_ver_ret = p_ver;
        } else {
            if (ARM_FSET_CPU_SUBSET (arch_fset, known_arch_fset)) {
                arm_feature_set added_fset;

                /* Compute features added by this architecture over the one
               recorded in p_ver_ret.  */
                if (p_ver_ret != NULL)
                    ARM_CLEAR_FEATURE (added_fset, known_arch_fset,
                                       p_ver_ret->flags);
                /* First architecture that match incl. with extensions, or the
               only difference in features over the recorded match is
               features that were optional and are now mandatory.  */
                if (p_ver_ret == NULL
                    || ARM_FSET_CPU_SUBSET (added_fset, arch_fset)) {
                    p_ver_ret = p_ver;
                    goto found;
                }
            } else if (p_ver_ret == NULL) {
                arm_feature_set needed_ext_fset;

                ARM_CLEAR_FEATURE (needed_ext_fset, arch_fset, known_arch_fset);
            }
        }
    }
    if (p_ver_ret == NULL)
        return -1;
    found:
    
    /* Tag_CPU_arch_profile.  */
    if (!ARM_CPU_HAS_FEATURE (p_ver_ret->flags, arm_ext_v8r)
        && (ARM_CPU_HAS_FEATURE (p_ver_ret->flags, arm_ext_v7a)
            || ARM_CPU_HAS_FEATURE (p_ver_ret->flags, arm_ext_v8)
            || (ARM_CPU_HAS_FEATURE (p_ver_ret->flags, arm_ext_atomics)
                && !ARM_CPU_HAS_FEATURE (p_ver_ret->flags, arm_ext_v8m_m_only))))
        *profile = 'A';
    else if (ARM_CPU_HAS_FEATURE (p_ver_ret->flags, arm_ext_v7r)
        || ARM_CPU_HAS_FEATURE (p_ver_ret->flags, arm_ext_v8r))
        *profile = 'R';
    else if (ARM_CPU_HAS_FEATURE (p_ver_ret->flags, arm_ext_m))
        *profile = 'M';
    else
        *profile = '\0';
    return p_ver_ret->val;
}

static void
aeabi_set_attribute_int (int tag, int value) {
  if (tag < 1
      || tag >= NUM_KNOWN_OBJ_ATTRIBUTES
      || !attributes_set_explicitly[tag])
    if (!bfd_elf_add_proc_attr_int (stdoutput, tag, value))
      as_fatal (_("error adding attribute: %s"),
		bfd_errmsg (bfd_get_error ()));
}

static void
aeabi_set_public_attributes (void) {
    char profile = '\0';
    int arch = -1;
    int virt_sec = 0;
    int fp16_optional = 0;
    int skip_exact_match = 0;
    arm_feature_set flags, flags_arch, flags_ext;
    if (no_cpu_selected ()) {
        ARM_MERGE_FEATURE_SETS (flags, arm_arch_used, thumb_arch_used);

        if (ARM_CPU_HAS_FEATURE (arm_arch_used, arm_arch_any))
            ARM_MERGE_FEATURE_SETS (flags, flags, arm_ext_v1);

        if (ARM_CPU_HAS_FEATURE (thumb_arch_used, arm_arch_any))
            ARM_MERGE_FEATURE_SETS (flags, flags, arm_ext_v4t);

        /* Code run during relaxation relies on selected_cpu being set.  */
        ARM_CLEAR_FEATURE (flags_arch, flags, fpu_any);
        flags_ext = arm_arch_none;
        ARM_CLEAR_FEATURE (selected_arch, flags_arch, flags_ext);
        selected_ext = flags_ext;
        selected_cpu = flags;
    } else {
        ARM_MERGE_FEATURE_SETS (flags_arch, selected_arch, selected_ext);
        ARM_CLEAR_FEATURE (flags_arch, flags_arch, fpu_any);
        flags_ext = selected_ext;
        flags = selected_cpu;
    }
    /* Allow the user to override the reported architecture.  */
    if (!ARM_FEATURE_ZERO (selected_object_arch)) {
        ARM_CLEAR_FEATURE (flags_arch, selected_object_arch, fpu_any);
        flags_ext = arm_arch_none;
    }
    else
        skip_exact_match = ARM_FEATURE_EQUAL (selected_cpu, arm_arch_any);
    if (!skip_exact_match)
        arch = get_aeabi_cpu_arch_from_fset (&flags_arch, &flags_ext, &profile, 1);
    if (arch == -1)
        arch = get_aeabi_cpu_arch_from_fset (&flags_arch, &flags_ext, &profile, 0);
    if (arch == -1)
        as_bad (_("no architecture contains all the instructions used\n"));
    if (selected_cpu_name[0]) {
        printf("select apu name\n");
    }
    /* Tag_CPU_arch.  */
    aeabi_set_attribute_int (Tag_CPU_arch, arch);
    if (profile != '\0')
        aeabi_set_attribute_int (Tag_CPU_arch_profile, profile);

    /* Tag_DSP_extension.  */
    if (ARM_CPU_HAS_FEATURE (selected_ext, arm_ext_dsp))
        aeabi_set_attribute_int (Tag_DSP_extension, 1);
    ARM_CLEAR_FEATURE (flags_arch, flags, fpu_any);
    /* Tag_ARM_ISA_use.  */
    if (ARM_CPU_HAS_FEATURE (flags, arm_ext_v1)
        || ARM_FEATURE_ZERO (flags_arch))
        aeabi_set_attribute_int (Tag_ARM_ISA_use, 1);

}

void
arm_md_post_relax (void) {
    aeabi_set_public_attributes ();
    XDELETE (mcpu_ext_opt);
    mcpu_ext_opt = NULL;
    XDELETE (march_ext_opt);
    march_ext_opt = NULL;
}

int
arm_apply_sym_value (struct fix * fixP, segT this_seg) {
    if (fixP->fx_addsy
        && ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v5t)
        /* PR 17444: If the local symbol is in a different section then a reloc
        will always be generated for it, so applying the symbol value now
        will result in a double offset being stored in the relocation.  */
        && (S_GET_SEGMENT (fixP->fx_addsy) == this_seg)
        && !S_FORCE_RELOC (fixP->fx_addsy, true)) {
        switch (fixP->fx_r_type) {
            case BFD_RELOC_ARM_PCREL_BLX:
            case BFD_RELOC_THUMB_PCREL_BRANCH23:
                if (ARM_IS_FUNC (fixP->fx_addsy))
                    return 1;
                break;

            case BFD_RELOC_ARM_PCREL_CALL:
            case BFD_RELOC_THUMB_PCREL_BLX:
                if (THUMB_IS_FUNC (fixP->fx_addsy))
                    return 1;
                break;

            default:
                break;
        }

    }
    return 0;
}

/* Generic function to determine whether a fixup requires a relocation.  */
int
generic_force_reloc (fixS *fix) {
    if (fix->fx_r_type == BFD_RELOC_VTABLE_INHERIT
        || fix->fx_r_type == BFD_RELOC_VTABLE_ENTRY)
        return 1;

    if (fix->fx_addsy == NULL)
        return 0;

    return S_FORCE_RELOC (fix->fx_addsy, fix->fx_subsy == NULL);
}

int
arm_force_relocation (struct fix * fixp)
{
#if defined (OBJ_COFF) && defined (TE_PE)
  if (fixp->fx_r_type == BFD_RELOC_RVA)
    return 1;
#endif

  /* In case we have a call or a branch to a function in ARM ISA mode from
     a thumb function or vice-versa force the relocation. These relocations
     are cleared off for some cores that might have blx and simple transformations
     are possible.  */

#ifdef OBJ_ELF
    switch (fixp->fx_r_type) {
        case BFD_RELOC_ARM_PCREL_JUMP:
        case BFD_RELOC_ARM_PCREL_CALL:
        case BFD_RELOC_THUMB_PCREL_BLX:
            if (THUMB_IS_FUNC (fixp->fx_addsy))
                return 1;
            break;

        case BFD_RELOC_ARM_PCREL_BLX:
        case BFD_RELOC_THUMB_PCREL_BRANCH25:
        case BFD_RELOC_THUMB_PCREL_BRANCH20:
        case BFD_RELOC_THUMB_PCREL_BRANCH23:
            if (ARM_IS_FUNC (fixp->fx_addsy))
                return 1;
            break;

        default:
            break;
    }
#endif

    /* Resolve these relocations even if the symbol is extern or weak.
        Technically this is probably wrong due to symbol preemption.
        In practice these relocations do not have enough range to be useful
        at dynamic link time, and some code (e.g. in the Linux kernel)
        expects these references to be resolved.  */
    if (fixp->fx_r_type == BFD_RELOC_ARM_IMMEDIATE
        || fixp->fx_r_type == BFD_RELOC_ARM_OFFSET_IMM
        || fixp->fx_r_type == BFD_RELOC_ARM_OFFSET_IMM8
        || fixp->fx_r_type == BFD_RELOC_ARM_ADRL_IMMEDIATE
        || fixp->fx_r_type == BFD_RELOC_ARM_CP_OFF_IMM
        || fixp->fx_r_type == BFD_RELOC_ARM_CP_OFF_IMM_S2
        || fixp->fx_r_type == BFD_RELOC_ARM_THUMB_OFFSET
        || fixp->fx_r_type == BFD_RELOC_THUMB_PCREL_BRANCH12
        || fixp->fx_r_type == BFD_RELOC_ARM_T32_ADD_IMM
        || fixp->fx_r_type == BFD_RELOC_ARM_T32_IMMEDIATE
        || fixp->fx_r_type == BFD_RELOC_ARM_T32_IMM12
        || fixp->fx_r_type == BFD_RELOC_ARM_T32_OFFSET_IMM
        || fixp->fx_r_type == BFD_RELOC_ARM_T32_ADD_PC12
        || fixp->fx_r_type == BFD_RELOC_ARM_T32_CP_OFF_IMM
        || fixp->fx_r_type == BFD_RELOC_ARM_T32_CP_OFF_IMM_S2)
        return 0;

        /* Always leave these relocations for the linker.  */
    if ((fixp->fx_r_type >= BFD_RELOC_ARM_ALU_PC_G0_NC
        && fixp->fx_r_type <= BFD_RELOC_ARM_LDC_SB_G2)
        || fixp->fx_r_type == BFD_RELOC_ARM_LDR_PC_G0)
        return 1;

    /* Always generate relocations against function symbols.  */
    if (fixp->fx_r_type == BFD_RELOC_32
        && fixp->fx_addsy
        && (symbol_get_bfdsym (fixp->fx_addsy)->flags & BSF_FUNCTION))
        return 1;

    return generic_force_reloc (fixp);
}

long
md_pcrel_from_section (fixS * fixP, segT seg) {
    offsetT base = fixP->fx_where + fixP->fx_frag->fr_address;

    /* If this is pc-relative and we are going to emit a relocation
        then we just want to put out any pipeline compensation that the linker
        will need.  Otherwise we want to use the calculated base.
        For WinCE we skip the bias for externals as well, since this
        is how the MS ARM-CE assembler behaves and we want to be compatible.  */
    if (fixP->fx_pcrel
        && ((fixP->fx_addsy && S_GET_SEGMENT (fixP->fx_addsy) != seg)
        || (arm_force_relocation (fixP)
#ifdef TE_WINCE
	      && !S_IS_EXTERNAL (fixP->fx_addsy)
#endif
	      )))
        base = 0;


    switch (fixP->fx_r_type) {
        /* PC relative addressing on the Thumb is slightly odd as the
        bottom two bits of the PC are forced to zero for the
        calculation.  This happens *after* application of the
        pipeline offset.  However, Thumb adrl already adjusts for
        this, so we need not do it again.  */
        case BFD_RELOC_ARM_THUMB_ADD:
        return base & ~3;

        case BFD_RELOC_ARM_THUMB_OFFSET:
        case BFD_RELOC_ARM_T32_OFFSET_IMM:
        case BFD_RELOC_ARM_T32_ADD_PC12:
        case BFD_RELOC_ARM_T32_CP_OFF_IMM:
        return (base + 4) & ~3;

        /* Thumb branches are simply offset by +4.  */
        case BFD_RELOC_THUMB_PCREL_BRANCH5:
        case BFD_RELOC_THUMB_PCREL_BRANCH7:
        case BFD_RELOC_THUMB_PCREL_BRANCH9:
        case BFD_RELOC_THUMB_PCREL_BRANCH12:
        case BFD_RELOC_THUMB_PCREL_BRANCH20:
        case BFD_RELOC_THUMB_PCREL_BRANCH25:
        case BFD_RELOC_THUMB_PCREL_BFCSEL:
        case BFD_RELOC_ARM_THUMB_BF17:
        case BFD_RELOC_ARM_THUMB_BF19:
        case BFD_RELOC_ARM_THUMB_BF13:
        case BFD_RELOC_ARM_THUMB_LOOP12:
        return base + 4;

        case BFD_RELOC_THUMB_PCREL_BRANCH23:
        if (fixP->fx_addsy
        && (S_GET_SEGMENT (fixP->fx_addsy) == seg)
        && !S_FORCE_RELOC (fixP->fx_addsy, true)
        && ARM_IS_FUNC (fixP->fx_addsy)
        && ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v5t))
        base = fixP->fx_where + fixP->fx_frag->fr_address;
        return base + 4;

        /* BLX is like branches above, but forces the low two bits of PC to
        zero.  */
        case BFD_RELOC_THUMB_PCREL_BLX:
        if (fixP->fx_addsy
        && (S_GET_SEGMENT (fixP->fx_addsy) == seg)
        && !S_FORCE_RELOC (fixP->fx_addsy, true)
        && THUMB_IS_FUNC (fixP->fx_addsy)
        && ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v5t))
        base = fixP->fx_where + fixP->fx_frag->fr_address;
        return (base + 4) & ~3;

        /* ARM mode branches are offset by +8.  However, the Windows CE
        loader expects the relocation not to take this into account.  */
        case BFD_RELOC_ARM_PCREL_BLX:
        if (fixP->fx_addsy
            && (S_GET_SEGMENT (fixP->fx_addsy) == seg)
            && !S_FORCE_RELOC (fixP->fx_addsy, true)
            && ARM_IS_FUNC (fixP->fx_addsy)
            && ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v5t))
        base = fixP->fx_where + fixP->fx_frag->fr_address;
        return base + 8;

        case BFD_RELOC_ARM_PCREL_CALL:
        if (fixP->fx_addsy
        && (S_GET_SEGMENT (fixP->fx_addsy) == seg)
        && !S_FORCE_RELOC (fixP->fx_addsy, true)
        && THUMB_IS_FUNC (fixP->fx_addsy)
        && ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v5t))
        base = fixP->fx_where + fixP->fx_frag->fr_address;
        return base + 8;

        case BFD_RELOC_ARM_PCREL_BRANCH:
        case BFD_RELOC_ARM_PCREL_JUMP:
        case BFD_RELOC_ARM_PLT32:

            return base + 8;


        /* ARM mode loads relative to PC are also offset by +8.  Unlike
        branches, the Windows CE loader *does* expect the relocation
        to take this into account.  */
        case BFD_RELOC_ARM_OFFSET_IMM:
        case BFD_RELOC_ARM_OFFSET_IMM8:
        case BFD_RELOC_ARM_HWLITERAL:
        case BFD_RELOC_ARM_LITERAL:
        case BFD_RELOC_ARM_CP_OFF_IMM:
            return base + 8;


        /* Other PC-relative relocations are un-offset.  */
        default:
            return base;
    }
}

static valueT
md_chars_to_number (char * buf, int n)
{
    valueT result = 0;
    unsigned char * where = (unsigned char *) buf;

    if (target_big_endian) {
        while (n--) {
            result <<= 8;
            result |= (*where++ & 255);
        }
    } else {
        while (n--) {
            result <<= 8;
            result |= (where[n] & 255);
        }
    }

    return result;
}

static unsigned long
get_thumb32_insn (char * buf) {
    unsigned long insn;
    insn = md_chars_to_number (buf, THUMB_SIZE) << 16;
    insn |= md_chars_to_number (buf + THUMB_SIZE, THUMB_SIZE);

    return insn;
}

static void
put_thumb32_insn (char * buf, unsigned long insn) {
    md_number_to_chars (buf, insn >> 16, THUMB_SIZE);
    md_number_to_chars (buf + THUMB_SIZE, insn, THUMB_SIZE);
}

static int
v8_1_branch_value_check (int val, int nbits, int is_signed) {
    gas_assert (nbits > 0 && nbits <= 32);
    if (is_signed) {
        int cmp = (1 << (nbits - 1));
        if ((val < -cmp) || (val >= cmp) || (val & 0x01))
            return FAIL;
    }
    else {
        if ((val <= 0) || (val >= (1 << nbits)) || (val & 0x1))
            return FAIL;
    }
    return SUCCESS;
}

static int
negate_data_op (unsigned long * instruction,
		unsigned long	value) {
    int op, new_inst;
    unsigned long negated, inverted;

    negated = encode_arm_immediate (-value);
    inverted = encode_arm_immediate (~value);

    op = (*instruction >> DATA_OP_SHIFT) & 0xf;
    switch (op) {
        /* First negates.	 */
        case OPCODE_SUB:		 /* ADD <-> SUB	 */
            new_inst = OPCODE_ADD;
            value = negated;
            break;

        case OPCODE_ADD:
            new_inst = OPCODE_SUB;
            value = negated;
            break;

        case OPCODE_CMP:		 /* CMP <-> CMN	 */
            new_inst = OPCODE_CMN;
            value = negated;
            break;

        case OPCODE_CMN:
            new_inst = OPCODE_CMP;
            value = negated;
            break;

        /* Now Inverted ops.  */
        case OPCODE_MOV:		 /* MOV <-> MVN	 */
            new_inst = OPCODE_MVN;
            value = inverted;
            break;

        case OPCODE_MVN:
            new_inst = OPCODE_MOV;
            value = inverted;
            break;

        case OPCODE_AND:		 /* AND <-> BIC	 */
            new_inst = OPCODE_BIC;
            value = inverted;
            break;

        case OPCODE_BIC:
            new_inst = OPCODE_AND;
            value = inverted;
            break;

        case OPCODE_ADC:		  /* ADC <-> SBC  */
            new_inst = OPCODE_SBC;
            value = inverted;
            break;

        case OPCODE_SBC:
            new_inst = OPCODE_ADC;
            value = inverted;
            break;

        /* We cannot do anything.	 */
        default:
            return FAIL;
    }

    if (value == (unsigned) FAIL)
        return FAIL;

    *instruction &= OPCODE_MASK;
    *instruction |= new_inst << DATA_OP_SHIFT;
    return value;
}

static void
encode_thumb2_b_bl_offset (char * buf, offsetT value) {
#define T2I1I2MASK  ((1 << 13) | (1 << 11))
    offsetT newval;
    offsetT newval2;
    addressT S, I1, I2, lo, hi;

    S = (value >> 24) & 0x01;
    I1 = (value >> 23) & 0x01;
    I2 = (value >> 22) & 0x01;
    hi = (value >> 12) & 0x3ff;
    lo = (value >> 1) & 0x7ff;
    newval   = md_chars_to_number (buf, THUMB_SIZE);
    newval2  = md_chars_to_number (buf + THUMB_SIZE, THUMB_SIZE);
    newval  |= (S << 10) | hi;
    newval2 &=  ~T2I1I2MASK;
    newval2 |= (((I1 ^ S) << 13) | ((I2 ^ S) << 11) | lo) ^ T2I1I2MASK;
    md_number_to_chars (buf, newval, THUMB_SIZE);
    md_number_to_chars (buf + THUMB_SIZE, newval2, THUMB_SIZE);
}

void
md_apply_fix (fixS *	fixP,
	       valueT * valP,
	       segT	seg)
{
  valueT	 value = * valP;
  valueT	 newval;
  unsigned int	 newimm;
  unsigned long	 temp;
  int		 sign;
  char *	 buf = fixP->fx_where + fixP->fx_frag->fr_literal;

  gas_assert (fixP->fx_r_type <= BFD_RELOC_UNUSED);

  /* Note whether this will delete the relocation.  */

  if (fixP->fx_addsy == 0 && !fixP->fx_pcrel)
    fixP->fx_done = 1;

  /* On a 64-bit host, silently truncate 'value' to 32 bits for
     consistency with the behaviour on 32-bit hosts.  Remember value
     for emit_reloc.  */
  value &= 0xffffffff;
  value ^= 0x80000000;
  value -= 0x80000000;

  *valP = value;
  fixP->fx_addnumber = value;

  /* Same treatment for fixP->fx_offset.  */
  fixP->fx_offset &= 0xffffffff;
  fixP->fx_offset ^= 0x80000000;
  fixP->fx_offset -= 0x80000000;

  switch (fixP->fx_r_type)
    {
    case BFD_RELOC_NONE:
      /* This will need to go in the object file.  */
      fixP->fx_done = 0;
      break;

    case BFD_RELOC_ARM_IMMEDIATE:
      /* We claim that this fixup has been processed here,
	 even if in fact we generate an error because we do
	 not have a reloc for it, so tc_gen_reloc will reject it.  */
      fixP->fx_done = 1;

      if (fixP->fx_addsy)
	{
	  const char *msg = 0;

	  if (! S_IS_DEFINED (fixP->fx_addsy))
	    msg = _("undefined symbol %s used as an immediate value");
	  else if (S_GET_SEGMENT (fixP->fx_addsy) != seg)
	    msg = _("symbol %s is in a different section");
	  else if (S_IS_WEAK (fixP->fx_addsy))
	    msg = _("symbol %s is weak and may be overridden later");

	  if (msg)
	    {
	      as_bad_where (fixP->fx_file, fixP->fx_line,
			    msg, S_GET_NAME (fixP->fx_addsy));
	      break;
	    }
	}

      temp = md_chars_to_number (buf, INSN_SIZE);

      /* If the offset is negative, we should use encoding A2 for ADR.  */
      if ((temp & 0xfff0000) == 0x28f0000 && (offsetT) value < 0)
	newimm = negate_data_op (&temp, value);
      else
	{
	  newimm = encode_arm_immediate (value);

	  /* If the instruction will fail, see if we can fix things up by
	     changing the opcode.  */
	  if (newimm == (unsigned int) FAIL)
	    newimm = negate_data_op (&temp, value);
	  /* MOV accepts both ARM modified immediate (A1 encoding) and
	     UINT16 (A2 encoding) when possible, MOVW only accepts UINT16.
	     When disassembling, MOV is preferred when there is no encoding
	     overlap.  */
	  if (newimm == (unsigned int) FAIL
	      && ((temp >> DATA_OP_SHIFT) & 0xf) == OPCODE_MOV
	      && ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v6t2)
	      && !((temp >> SBIT_SHIFT) & 0x1)
	      && value <= 0xffff)
	    {
	      /* Clear bits[23:20] to change encoding from A1 to A2.  */
	      temp &= 0xff0fffff;
	      /* Encoding high 4bits imm.  Code below will encode the remaining
		 low 12bits.  */
	      temp |= (value & 0x0000f000) << 4;
	      newimm = value & 0x00000fff;
	    }
	}

      if (newimm == (unsigned int) FAIL)
	{
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			_("invalid constant (%lx) after fixup"),
			(unsigned long) value);
	  break;
	}

      newimm |= (temp & 0xfffff000);
      md_number_to_chars (buf, (valueT) newimm, INSN_SIZE);
      break;

    case BFD_RELOC_ARM_ADRL_IMMEDIATE:
      {
	unsigned int highpart = 0;
	unsigned int newinsn  = 0xe1a00000; /* nop.  */

	if (fixP->fx_addsy)
	  {
	    const char *msg = 0;

	    if (! S_IS_DEFINED (fixP->fx_addsy))
	      msg = _("undefined symbol %s used as an immediate value");
	    else if (S_GET_SEGMENT (fixP->fx_addsy) != seg)
	      msg = _("symbol %s is in a different section");
	    else if (S_IS_WEAK (fixP->fx_addsy))
	      msg = _("symbol %s is weak and may be overridden later");

	    if (msg)
	      {
		as_bad_where (fixP->fx_file, fixP->fx_line,
			      msg, S_GET_NAME (fixP->fx_addsy));
		break;
	      }
	  }

	newimm = encode_arm_immediate (value);
	temp = md_chars_to_number (buf, INSN_SIZE);

	/* If the instruction will fail, see if we can fix things up by
	   changing the opcode.	 */
	if (newimm == (unsigned int) FAIL
	    && (newimm = negate_data_op (& temp, value)) == (unsigned int) FAIL)
	  {
	    /* No ?  OK - try using two ADD instructions to generate
	       the value.  */
	    newimm = validate_immediate_twopart (value, & highpart);

	    /* Yes - then make sure that the second instruction is
	       also an add.  */
	    if (newimm != (unsigned int) FAIL)
	      newinsn = temp;
	    /* Still No ?  Try using a negated value.  */
	    else if ((newimm = validate_immediate_twopart (- value, & highpart)) != (unsigned int) FAIL)
	      temp = newinsn = (temp & OPCODE_MASK) | OPCODE_SUB << DATA_OP_SHIFT;
	    /* Otherwise - give up.  */
	    else
	      {
		as_bad_where (fixP->fx_file, fixP->fx_line,
			      _("unable to compute ADRL instructions for PC offset of 0x%lx"),
			      (long) value);
		break;
	      }

	    /* Replace the first operand in the 2nd instruction (which
	       is the PC) with the destination register.  We have
	       already added in the PC in the first instruction and we
	       do not want to do it again.  */
	    newinsn &= ~ 0xf0000;
	    newinsn |= ((newinsn & 0x0f000) << 4);
	  }

	newimm |= (temp & 0xfffff000);
	md_number_to_chars (buf, (valueT) newimm, INSN_SIZE);

	highpart |= (newinsn & 0xfffff000);
	md_number_to_chars (buf + INSN_SIZE, (valueT) highpart, INSN_SIZE);
      }
      break;

    case BFD_RELOC_ARM_OFFSET_IMM:
      if (!fixP->fx_done && seg->use_rela_p)
	value = 0;
      /* Fall through.  */

    case BFD_RELOC_ARM_LITERAL:
      sign = (offsetT) value > 0;

      if ((offsetT) value < 0)
	value = - value;

      if (validate_offset_imm (value, 0) == FAIL)
	{
	  if (fixP->fx_r_type == BFD_RELOC_ARM_LITERAL)
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("invalid literal constant: pool needs to be closer"));
	  else
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("bad immediate value for offset (%ld)"),
			  (long) value);
	  break;
	}

      newval = md_chars_to_number (buf, INSN_SIZE);
      if (value == 0)
	newval &= 0xfffff000;
      else
	{
	  newval &= 0xff7ff000;
	  newval |= value | (sign ? INDEX_UP : 0);
	}
      md_number_to_chars (buf, newval, INSN_SIZE);
      break;

    case BFD_RELOC_ARM_OFFSET_IMM8:
    case BFD_RELOC_ARM_HWLITERAL:
      sign = (offsetT) value > 0;

      if ((offsetT) value < 0)
	value = - value;

      if (validate_offset_imm (value, 1) == FAIL)
	{
	  if (fixP->fx_r_type == BFD_RELOC_ARM_HWLITERAL)
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("invalid literal constant: pool needs to be closer"));
	  else
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("bad immediate value for 8-bit offset (%ld)"),
			  (long) value);
	  break;
	}

      newval = md_chars_to_number (buf, INSN_SIZE);
      if (value == 0)
	newval &= 0xfffff0f0;
      else
	{
	  newval &= 0xff7ff0f0;
	  newval |= ((value >> 4) << 8) | (value & 0xf) | (sign ? INDEX_UP : 0);
	}
      md_number_to_chars (buf, newval, INSN_SIZE);
      break;

    case BFD_RELOC_ARM_T32_OFFSET_U8:
      if (value > 1020 || value % 4 != 0)
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("bad immediate value for offset (%ld)"), (long) value);
      value /= 4;

      newval = md_chars_to_number (buf+2, THUMB_SIZE);
      newval |= value;
      md_number_to_chars (buf+2, newval, THUMB_SIZE);
      break;

    case BFD_RELOC_ARM_T32_OFFSET_IMM:
      /* This is a complicated relocation used for all varieties of Thumb32
	 load/store instruction with immediate offset:

	 1110 100P u1WL NNNN XXXX YYYY iiii iiii - +/-(U) pre/post(P) 8-bit,
						   *4, optional writeback(W)
						   (doubleword load/store)

	 1111 100S uTTL 1111 XXXX iiii iiii iiii - +/-(U) 12-bit PC-rel
	 1111 100S 0TTL NNNN XXXX 1Pu1 iiii iiii - +/-(U) pre/post(P) 8-bit
	 1111 100S 0TTL NNNN XXXX 1110 iiii iiii - positive 8-bit (T instruction)
	 1111 100S 1TTL NNNN XXXX iiii iiii iiii - positive 12-bit
	 1111 100S 0TTL NNNN XXXX 1100 iiii iiii - negative 8-bit

	 Uppercase letters indicate bits that are already encoded at
	 this point.  Lowercase letters are our problem.  For the
	 second block of instructions, the secondary opcode nybble
	 (bits 8..11) is present, and bit 23 is zero, even if this is
	 a PC-relative operation.  */
      newval = md_chars_to_number (buf, THUMB_SIZE);
      newval <<= 16;
      newval |= md_chars_to_number (buf+THUMB_SIZE, THUMB_SIZE);

      if ((newval & 0xf0000000) == 0xe0000000)
	{
	  /* Doubleword load/store: 8-bit offset, scaled by 4.  */
	  if ((offsetT) value >= 0)
	    newval |= (1 << 23);
	  else
	    value = -value;
	  if (value % 4 != 0)
	    {
	      as_bad_where (fixP->fx_file, fixP->fx_line,
			    _("offset not a multiple of 4"));
	      break;
	    }
	  value /= 4;
	  if (value > 0xff)
	    {
	      as_bad_where (fixP->fx_file, fixP->fx_line,
			    _("offset out of range"));
	      break;
	    }
	  newval &= ~0xff;
	}
      else if ((newval & 0x000f0000) == 0x000f0000)
	{
	  /* PC-relative, 12-bit offset.  */
	  if ((offsetT) value >= 0)
	    newval |= (1 << 23);
	  else
	    value = -value;
	  if (value > 0xfff)
	    {
	      as_bad_where (fixP->fx_file, fixP->fx_line,
			    _("offset out of range"));
	      break;
	    }
	  newval &= ~0xfff;
	}
      else if ((newval & 0x00000100) == 0x00000100)
	{
	  /* Writeback: 8-bit, +/- offset.  */
	  if ((offsetT) value >= 0)
	    newval |= (1 << 9);
	  else
	    value = -value;
	  if (value > 0xff)
	    {
	      as_bad_where (fixP->fx_file, fixP->fx_line,
			    _("offset out of range"));
	      break;
	    }
	  newval &= ~0xff;
	}
      else if ((newval & 0x00000f00) == 0x00000e00)
	{
	  /* T-instruction: positive 8-bit offset.  */
	  if (value > 0xff)
	    {
	      as_bad_where (fixP->fx_file, fixP->fx_line,
			    _("offset out of range"));
	      break;
	    }
	  newval &= ~0xff;
	  newval |= value;
	}
      else
	{
	  /* Positive 12-bit or negative 8-bit offset.  */
	  unsigned int limit;
	  if ((offsetT) value >= 0)
	    {
	      newval |= (1 << 23);
	      limit = 0xfff;
	    }
	  else
	    {
	      value = -value;
	      limit = 0xff;
	    }
	  if (value > limit)
	    {
	      as_bad_where (fixP->fx_file, fixP->fx_line,
			    _("offset out of range"));
	      break;
	    }
	  newval &= ~limit;
	}

      newval |= value;
      md_number_to_chars (buf, (newval >> 16) & 0xffff, THUMB_SIZE);
      md_number_to_chars (buf + THUMB_SIZE, newval & 0xffff, THUMB_SIZE);
      break;

    case BFD_RELOC_ARM_SHIFT_IMM:
      newval = md_chars_to_number (buf, INSN_SIZE);
      if (value > 32
	  || (value == 32
	      && (((newval & 0x60) == 0) || (newval & 0x60) == 0x60)))
	{
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			_("shift expression is too large"));
	  break;
	}

      if (value == 0)
	/* Shifts of zero must be done as lsl.	*/
	newval &= ~0x60;
      else if (value == 32)
	value = 0;
      newval &= 0xfffff07f;
      newval |= (value & 0x1f) << 7;
      md_number_to_chars (buf, newval, INSN_SIZE);
      break;

    case BFD_RELOC_ARM_T32_IMMEDIATE:
    case BFD_RELOC_ARM_T32_ADD_IMM:
    case BFD_RELOC_ARM_T32_IMM12:
    case BFD_RELOC_ARM_T32_ADD_PC12:
      /* We claim that this fixup has been processed here,
	 even if in fact we generate an error because we do
	 not have a reloc for it, so tc_gen_reloc will reject it.  */
      fixP->fx_done = 1;

      if (fixP->fx_addsy
	  && ! S_IS_DEFINED (fixP->fx_addsy))
	{
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			_("undefined symbol %s used as an immediate value"),
			S_GET_NAME (fixP->fx_addsy));
	  break;
	}

      newval = md_chars_to_number (buf, THUMB_SIZE);
      newval <<= 16;
      newval |= md_chars_to_number (buf+2, THUMB_SIZE);

      newimm = FAIL;
      if ((fixP->fx_r_type == BFD_RELOC_ARM_T32_IMMEDIATE
	   /* ARMv8-M Baseline MOV will reach here, but it doesn't support
	      Thumb2 modified immediate encoding (T2).  */
	   && ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v6t2))
	  || fixP->fx_r_type == BFD_RELOC_ARM_T32_ADD_IMM)
	{
	  newimm = encode_thumb32_immediate (value);
	  if (newimm == (unsigned int) FAIL)
	    newimm = thumb32_negate_data_op (&newval, value);
	}
      if (newimm == (unsigned int) FAIL)
	{
	  if (fixP->fx_r_type != BFD_RELOC_ARM_T32_IMMEDIATE)
	    {
	      /* Turn add/sum into addw/subw.  */
	      if (fixP->fx_r_type == BFD_RELOC_ARM_T32_ADD_IMM)
		newval = (newval & 0xfeffffff) | 0x02000000;
	      /* No flat 12-bit imm encoding for addsw/subsw.  */
	      if ((newval & 0x00100000) == 0)
		{
		  /* 12 bit immediate for addw/subw.  */
		  if ((offsetT) value < 0)
		    {
		      value = -value;
		      newval ^= 0x00a00000;
		    }
		  if (value > 0xfff)
		    newimm = (unsigned int) FAIL;
		  else
		    newimm = value;
		}
	    }
	  else
	    {
	      /* MOV accepts both Thumb2 modified immediate (T2 encoding) and
		 UINT16 (T3 encoding), MOVW only accepts UINT16.  When
		 disassembling, MOV is preferred when there is no encoding
		 overlap.  */
	      if (((newval >> T2_DATA_OP_SHIFT) & 0xf) == T2_OPCODE_ORR
		  /* NOTE: MOV uses the ORR opcode in Thumb 2 mode
		     but with the Rn field [19:16] set to 1111.  */
		  && (((newval >> 16) & 0xf) == 0xf)
		  && ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v6t2_v8m)
		  && !((newval >> T2_SBIT_SHIFT) & 0x1)
		  && value <= 0xffff)
		{
		  /* Toggle bit[25] to change encoding from T2 to T3.  */
		  newval ^= 1 << 25;
		  /* Clear bits[19:16].  */
		  newval &= 0xfff0ffff;
		  /* Encoding high 4bits imm.  Code below will encode the
		     remaining low 12bits.  */
		  newval |= (value & 0x0000f000) << 4;
		  newimm = value & 0x00000fff;
		}
	    }
	}

      if (newimm == (unsigned int)FAIL)
	{
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			_("invalid constant (%lx) after fixup"),
			(unsigned long) value);
	  break;
	}

      newval |= (newimm & 0x800) << 15;
      newval |= (newimm & 0x700) << 4;
      newval |= (newimm & 0x0ff);

      md_number_to_chars (buf,   (valueT) ((newval >> 16) & 0xffff), THUMB_SIZE);
      md_number_to_chars (buf+2, (valueT) (newval & 0xffff), THUMB_SIZE);
      break;

    case BFD_RELOC_ARM_SMC:
      if (value > 0xf)
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("invalid smc expression"));

      newval = md_chars_to_number (buf, INSN_SIZE);
      newval |= (value & 0xf);
      md_number_to_chars (buf, newval, INSN_SIZE);
      break;

    case BFD_RELOC_ARM_HVC:
      if (value > 0xffff)
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("invalid hvc expression"));
      newval = md_chars_to_number (buf, INSN_SIZE);
      newval |= (value & 0xf) | ((value & 0xfff0) << 4);
      md_number_to_chars (buf, newval, INSN_SIZE);
      break;

    case BFD_RELOC_ARM_SWI:
      if (fixP->tc_fix_data != 0)
	{
	  if (value > 0xff)
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("invalid swi expression"));
	  newval = md_chars_to_number (buf, THUMB_SIZE);
	  newval |= value;
	  md_number_to_chars (buf, newval, THUMB_SIZE);
	}
      else
	{
	  if (value > 0x00ffffff)
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("invalid swi expression"));
	  newval = md_chars_to_number (buf, INSN_SIZE);
	  newval |= value;
	  md_number_to_chars (buf, newval, INSN_SIZE);
	}
      break;

    case BFD_RELOC_ARM_MULTI:
      if (value > 0xffff)
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("invalid expression in load/store multiple"));
      newval = value | md_chars_to_number (buf, INSN_SIZE);
      md_number_to_chars (buf, newval, INSN_SIZE);
      break;

#ifdef OBJ_ELF
    case BFD_RELOC_ARM_PCREL_CALL:

      if (ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v5t)
	  && fixP->fx_addsy
	  && !S_FORCE_RELOC (fixP->fx_addsy, true)
	  && (S_GET_SEGMENT (fixP->fx_addsy) == seg)
	  && THUMB_IS_FUNC (fixP->fx_addsy))
	/* Flip the bl to blx. This is a simple flip
	   bit here because we generate PCREL_CALL for
	   unconditional bls.  */
	{
	  newval = md_chars_to_number (buf, INSN_SIZE);
	  newval = newval | 0x10000000;
	  md_number_to_chars (buf, newval, INSN_SIZE);
	  temp = 1;
	  fixP->fx_done = 1;
	}
      else
	temp = 3;
      goto arm_branch_common;

    case BFD_RELOC_ARM_PCREL_JUMP:
      if (ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v5t)
	  && fixP->fx_addsy
	  && !S_FORCE_RELOC (fixP->fx_addsy, true)
	  && (S_GET_SEGMENT (fixP->fx_addsy) == seg)
	  && THUMB_IS_FUNC (fixP->fx_addsy))
	{
	  /* This would map to a bl<cond>, b<cond>,
	     b<always> to a Thumb function. We
	     need to force a relocation for this particular
	     case.  */
	  newval = md_chars_to_number (buf, INSN_SIZE);
	  fixP->fx_done = 0;
	}
      /* Fall through.  */

    case BFD_RELOC_ARM_PLT32:
#endif
    case BFD_RELOC_ARM_PCREL_BRANCH:
      temp = 3;
      goto arm_branch_common;

    case BFD_RELOC_ARM_PCREL_BLX:

      temp = 1;
      if (ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v5t)
	  && fixP->fx_addsy
	  && !S_FORCE_RELOC (fixP->fx_addsy, true)
	  && (S_GET_SEGMENT (fixP->fx_addsy) == seg)
	  && ARM_IS_FUNC (fixP->fx_addsy))
	{
	  /* Flip the blx to a bl and warn.  */
	  const char *name = S_GET_NAME (fixP->fx_addsy);
	  newval = 0xeb000000;
	  as_warn_where (fixP->fx_file, fixP->fx_line,
			 _("blx to '%s' an ARM ISA state function changed to bl"),
			  name);
	  md_number_to_chars (buf, newval, INSN_SIZE);
	  temp = 3;
	  fixP->fx_done = 1;
	}

#ifdef OBJ_ELF
       if (EF_ARM_EABI_VERSION (meabi_flags) >= EF_ARM_EABI_VER4)
	 fixP->fx_r_type = BFD_RELOC_ARM_PCREL_CALL;
#endif

    arm_branch_common:
      /* We are going to store value (shifted right by two) in the
	 instruction, in a 24 bit, signed field.  Bits 26 through 32 either
	 all clear or all set and bit 0 must be clear.  For B/BL bit 1 must
	 also be clear.  */
      if (value & temp)
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("misaligned branch destination"));
      if ((value & 0xfe000000) != 0
	  && (value & 0xfe000000) != 0xfe000000)
	as_bad_where (fixP->fx_file, fixP->fx_line, BAD_RANGE);

      if (fixP->fx_done || !seg->use_rela_p)
	{
	  newval = md_chars_to_number (buf, INSN_SIZE);
	  newval |= (value >> 2) & 0x00ffffff;
	  /* Set the H bit on BLX instructions.  */
	  if (temp == 1)
	    {
	      if (value & 2)
		newval |= 0x01000000;
	      else
		newval &= ~0x01000000;
	    }
	  md_number_to_chars (buf, newval, INSN_SIZE);
	}
      break;

    case BFD_RELOC_THUMB_PCREL_BRANCH7: /* CBZ */
      /* CBZ can only branch forward.  */

      /* Attempts to use CBZ to branch to the next instruction
	 (which, strictly speaking, are prohibited) will be turned into
	 no-ops.

	 FIXME: It may be better to remove the instruction completely and
	 perform relaxation.  */
      if ((offsetT) value == -2)
	{
	  newval = md_chars_to_number (buf, THUMB_SIZE);
	  newval = 0xbf00; /* NOP encoding T1 */
	  md_number_to_chars (buf, newval, THUMB_SIZE);
	}
      else
	{
	  if (value & ~0x7e)
	    as_bad_where (fixP->fx_file, fixP->fx_line, BAD_RANGE);

	  if (fixP->fx_done || !seg->use_rela_p)
	    {
	      newval = md_chars_to_number (buf, THUMB_SIZE);
	      newval |= ((value & 0x3e) << 2) | ((value & 0x40) << 3);
	      md_number_to_chars (buf, newval, THUMB_SIZE);
	    }
	}
      break;

    case BFD_RELOC_THUMB_PCREL_BRANCH9: /* Conditional branch.	*/
      if (out_of_range_p (value, 8))
	as_bad_where (fixP->fx_file, fixP->fx_line, BAD_RANGE);

      if (fixP->fx_done || !seg->use_rela_p)
	{
	  newval = md_chars_to_number (buf, THUMB_SIZE);
	  newval |= (value & 0x1ff) >> 1;
	  md_number_to_chars (buf, newval, THUMB_SIZE);
	}
      break;

    case BFD_RELOC_THUMB_PCREL_BRANCH12: /* Unconditional branch.  */
      if (out_of_range_p (value, 11))
	as_bad_where (fixP->fx_file, fixP->fx_line, BAD_RANGE);

      if (fixP->fx_done || !seg->use_rela_p)
	{
	  newval = md_chars_to_number (buf, THUMB_SIZE);
	  newval |= (value & 0xfff) >> 1;
	  md_number_to_chars (buf, newval, THUMB_SIZE);
	}
      break;

    /* This relocation is misnamed, it should be BRANCH21.  */
    case BFD_RELOC_THUMB_PCREL_BRANCH20:
      if (fixP->fx_addsy
	  && (S_GET_SEGMENT (fixP->fx_addsy) == seg)
	  && !S_FORCE_RELOC (fixP->fx_addsy, true)
	  && ARM_IS_FUNC (fixP->fx_addsy)
	  && ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v5t))
	{
	  /* Force a relocation for a branch 20 bits wide.  */
	  fixP->fx_done = 0;
	}
      if (out_of_range_p (value, 20))
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("conditional branch out of range"));

      if (fixP->fx_done || !seg->use_rela_p)
	{
	  offsetT newval2;
	  addressT S, J1, J2, lo, hi;

	  S  = (value & 0x00100000) >> 20;
	  J2 = (value & 0x00080000) >> 19;
	  J1 = (value & 0x00040000) >> 18;
	  hi = (value & 0x0003f000) >> 12;
	  lo = (value & 0x00000ffe) >> 1;

	  newval   = md_chars_to_number (buf, THUMB_SIZE);
	  newval2  = md_chars_to_number (buf + THUMB_SIZE, THUMB_SIZE);
	  newval  |= (S << 10) | hi;
	  newval2 |= (J1 << 13) | (J2 << 11) | lo;
	  md_number_to_chars (buf, newval, THUMB_SIZE);
	  md_number_to_chars (buf + THUMB_SIZE, newval2, THUMB_SIZE);
	}
      break;

    case BFD_RELOC_THUMB_PCREL_BLX:
      /* If there is a blx from a thumb state function to
	 another thumb function flip this to a bl and warn
	 about it.  */

      if (fixP->fx_addsy
	  && !S_FORCE_RELOC (fixP->fx_addsy, true)
	  && (S_GET_SEGMENT (fixP->fx_addsy) == seg)
	  && THUMB_IS_FUNC (fixP->fx_addsy))
	{
	  const char *name = S_GET_NAME (fixP->fx_addsy);
	  as_warn_where (fixP->fx_file, fixP->fx_line,
			 _("blx to Thumb func '%s' from Thumb ISA state changed to bl"),
			 name);
	  newval = md_chars_to_number (buf + THUMB_SIZE, THUMB_SIZE);
	  newval = newval | 0x1000;
	  md_number_to_chars (buf+THUMB_SIZE, newval, THUMB_SIZE);
	  fixP->fx_r_type = BFD_RELOC_THUMB_PCREL_BRANCH23;
	  fixP->fx_done = 1;
	}


      goto thumb_bl_common;

    case BFD_RELOC_THUMB_PCREL_BRANCH23:
      /* A bl from Thumb state ISA to an internal ARM state function
	 is converted to a blx.  */
      if (fixP->fx_addsy
	  && (S_GET_SEGMENT (fixP->fx_addsy) == seg)
	  && !S_FORCE_RELOC (fixP->fx_addsy, true)
	  && ARM_IS_FUNC (fixP->fx_addsy)
	  && ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v5t))
	{
	  newval = md_chars_to_number (buf + THUMB_SIZE, THUMB_SIZE);
	  newval = newval & ~0x1000;
	  md_number_to_chars (buf+THUMB_SIZE, newval, THUMB_SIZE);
	  fixP->fx_r_type = BFD_RELOC_THUMB_PCREL_BLX;
	  fixP->fx_done = 1;
	}

    thumb_bl_common:

      if (fixP->fx_r_type == BFD_RELOC_THUMB_PCREL_BLX)
	/* For a BLX instruction, make sure that the relocation is rounded up
	   to a word boundary.  This follows the semantics of the instruction
	   which specifies that bit 1 of the target address will come from bit
	   1 of the base address.  */
	value = (value + 3) & ~ 3;

#ifdef OBJ_ELF
       if (EF_ARM_EABI_VERSION (meabi_flags) >= EF_ARM_EABI_VER4
	   && fixP->fx_r_type == BFD_RELOC_THUMB_PCREL_BLX)
	 fixP->fx_r_type = BFD_RELOC_THUMB_PCREL_BRANCH23;
#endif

      if (out_of_range_p (value, 22))
	{
	  if (!(ARM_CPU_HAS_FEATURE (cpu_variant, arm_ext_v6t2)))
	    as_bad_where (fixP->fx_file, fixP->fx_line, BAD_RANGE);
	  else if (out_of_range_p (value, 24))
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("Thumb2 branch out of range"));
	}

      if (fixP->fx_done || !seg->use_rela_p)
	encode_thumb2_b_bl_offset (buf, value);

      break;

    case BFD_RELOC_THUMB_PCREL_BRANCH25:
      if (out_of_range_p (value, 24))
	as_bad_where (fixP->fx_file, fixP->fx_line, BAD_RANGE);

      if (fixP->fx_done || !seg->use_rela_p)
	  encode_thumb2_b_bl_offset (buf, value);

      break;

    case BFD_RELOC_8:
      if (fixP->fx_done || !seg->use_rela_p)
	*buf = value;
      break;

    case BFD_RELOC_16:
      if (fixP->fx_done || !seg->use_rela_p)
	md_number_to_chars (buf, value, 2);
      break;

#ifdef OBJ_ELF
    case BFD_RELOC_ARM_TLS_CALL:
    case BFD_RELOC_ARM_THM_TLS_CALL:
    case BFD_RELOC_ARM_TLS_DESCSEQ:
    case BFD_RELOC_ARM_THM_TLS_DESCSEQ:
    case BFD_RELOC_ARM_TLS_GOTDESC:
    case BFD_RELOC_ARM_TLS_GD32:
    case BFD_RELOC_ARM_TLS_LE32:
    case BFD_RELOC_ARM_TLS_IE32:
    case BFD_RELOC_ARM_TLS_LDM32:
    case BFD_RELOC_ARM_TLS_LDO32:
      S_SET_THREAD_LOCAL (fixP->fx_addsy);
      break;

      /* Same handling as above, but with the arm_fdpic guard.  */
    case BFD_RELOC_ARM_TLS_GD32_FDPIC:
    case BFD_RELOC_ARM_TLS_IE32_FDPIC:
    case BFD_RELOC_ARM_TLS_LDM32_FDPIC:
      if (arm_fdpic)
	{
	  S_SET_THREAD_LOCAL (fixP->fx_addsy);
	}
      else
	{
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			_("Relocation supported only in FDPIC mode"));
	}
      break;

    case BFD_RELOC_ARM_GOT32:
    case BFD_RELOC_ARM_GOTOFF:
      break;

    case BFD_RELOC_ARM_GOT_PREL:
      if (fixP->fx_done || !seg->use_rela_p)
	md_number_to_chars (buf, value, 4);
      break;

    case BFD_RELOC_ARM_TARGET2:
      /* TARGET2 is not partial-inplace, so we need to write the
	 addend here for REL targets, because it won't be written out
	 during reloc processing later.  */
      if (fixP->fx_done || !seg->use_rela_p)
	md_number_to_chars (buf, fixP->fx_offset, 4);
      break;

      /* Relocations for FDPIC.  */
    case BFD_RELOC_ARM_GOTFUNCDESC:
    case BFD_RELOC_ARM_GOTOFFFUNCDESC:
    case BFD_RELOC_ARM_FUNCDESC:
      if (arm_fdpic)
	{
	  if (fixP->fx_done || !seg->use_rela_p)
	    md_number_to_chars (buf, 0, 4);
	}
      else
	{
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			_("Relocation supported only in FDPIC mode"));
      }
      break;
#endif

    case BFD_RELOC_RVA:
    case BFD_RELOC_32:
    case BFD_RELOC_ARM_TARGET1:
    case BFD_RELOC_ARM_ROSEGREL32:
    case BFD_RELOC_ARM_SBREL32:
    case BFD_RELOC_32_PCREL:
#ifdef TE_PE
    case BFD_RELOC_32_SECREL:
#endif
      if (fixP->fx_done || !seg->use_rela_p)
#ifdef TE_WINCE
	/* For WinCE we only do this for pcrel fixups.  */
	if (fixP->fx_done || fixP->fx_pcrel)
#endif
	  md_number_to_chars (buf, value, 4);
      break;

#ifdef OBJ_ELF
    case BFD_RELOC_ARM_PREL31:
      if (fixP->fx_done || !seg->use_rela_p)
	{
	  newval = md_chars_to_number (buf, 4) & 0x80000000;
	  if ((value ^ (value >> 1)) & 0x40000000)
	    {
	      as_bad_where (fixP->fx_file, fixP->fx_line,
			    _("rel31 relocation overflow"));
	    }
	  newval |= value & 0x7fffffff;
	  md_number_to_chars (buf, newval, 4);
	}
      break;
#endif

    case BFD_RELOC_ARM_CP_OFF_IMM:
    case BFD_RELOC_ARM_T32_CP_OFF_IMM:
    case BFD_RELOC_ARM_T32_VLDR_VSTR_OFF_IMM:
      if (fixP->fx_r_type == BFD_RELOC_ARM_CP_OFF_IMM)
	newval = md_chars_to_number (buf, INSN_SIZE);
      else
	newval = get_thumb32_insn (buf);
      if ((newval & 0x0f200f00) == 0x0d000900)
	{
	  /* This is a fp16 vstr/vldr.  The immediate offset in the mnemonic
	     has permitted values that are multiples of 2, in the range -510
	     to 510.  */
	  if (value + 510 > 510 + 510 || (value & 1))
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("co-processor offset out of range"));
	}
      else if ((newval & 0xfe001f80) == 0xec000f80)
	{
	  if (value + 511 > 512 + 511 || (value & 3))
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("co-processor offset out of range"));
	}
      else if (value + 1023 > 1023 + 1023 || (value & 3))
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("co-processor offset out of range"));
    cp_off_common:
      sign = (offsetT) value > 0;
      if ((offsetT) value < 0)
	value = -value;
      if (fixP->fx_r_type == BFD_RELOC_ARM_CP_OFF_IMM
	  || fixP->fx_r_type == BFD_RELOC_ARM_CP_OFF_IMM_S2)
	newval = md_chars_to_number (buf, INSN_SIZE);
      else
	newval = get_thumb32_insn (buf);
      if (value == 0)
	{
	  if (fixP->fx_r_type == BFD_RELOC_ARM_T32_VLDR_VSTR_OFF_IMM)
	    newval &= 0xffffff80;
	  else
	    newval &= 0xffffff00;
	}
      else
	{
	  if (fixP->fx_r_type == BFD_RELOC_ARM_T32_VLDR_VSTR_OFF_IMM)
	    newval &= 0xff7fff80;
	  else
	    newval &= 0xff7fff00;
	  if ((newval & 0x0f200f00) == 0x0d000900)
	    {
	      /* This is a fp16 vstr/vldr.

		 It requires the immediate offset in the instruction is shifted
		 left by 1 to be a half-word offset.

		 Here, left shift by 1 first, and later right shift by 2
		 should get the right offset.  */
	      value <<= 1;
	    }
	  newval |= (value >> 2) | (sign ? INDEX_UP : 0);
	}
      if (fixP->fx_r_type == BFD_RELOC_ARM_CP_OFF_IMM
	  || fixP->fx_r_type == BFD_RELOC_ARM_CP_OFF_IMM_S2)
	md_number_to_chars (buf, newval, INSN_SIZE);
      else
	put_thumb32_insn (buf, newval);
      break;

    case BFD_RELOC_ARM_CP_OFF_IMM_S2:
    case BFD_RELOC_ARM_T32_CP_OFF_IMM_S2:
      if (value + 255 > 255 + 255)
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("co-processor offset out of range"));
      value *= 4;
      goto cp_off_common;

    case BFD_RELOC_ARM_THUMB_OFFSET:
      newval = md_chars_to_number (buf, THUMB_SIZE);
      /* Exactly what ranges, and where the offset is inserted depends
	 on the type of instruction, we can establish this from the
	 top 4 bits.  */
      switch (newval >> 12)
	{
	case 4: /* PC load.  */
	  /* Thumb PC loads are somewhat odd, bit 1 of the PC is
	     forced to zero for these loads; md_pcrel_from has already
	     compensated for this.  */
	  if (value & 3)
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("invalid offset, target not word aligned (0x%08lX)"),
			  (((unsigned long) fixP->fx_frag->fr_address
			    + (unsigned long) fixP->fx_where) & ~3)
			  + (unsigned long) value);
	  else if (get_recorded_alignment (seg) < 2)
	    as_warn_where (fixP->fx_file, fixP->fx_line,
			   _("section does not have enough alignment to ensure safe PC-relative loads"));

	  if (value & ~0x3fc)
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("invalid offset, value too big (0x%08lX)"),
			  (long) value);

	  newval |= value >> 2;
	  break;

	case 9: /* SP load/store.  */
	  if (value & ~0x3fc)
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("invalid offset, value too big (0x%08lX)"),
			  (long) value);
	  newval |= value >> 2;
	  break;

	case 6: /* Word load/store.  */
	  if (value & ~0x7c)
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("invalid offset, value too big (0x%08lX)"),
			  (long) value);
	  newval |= value << 4; /* 6 - 2.  */
	  break;

	case 7: /* Byte load/store.  */
	  if (value & ~0x1f)
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("invalid offset, value too big (0x%08lX)"),
			  (long) value);
	  newval |= value << 6;
	  break;

	case 8: /* Halfword load/store.	 */
	  if (value & ~0x3e)
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("invalid offset, value too big (0x%08lX)"),
			  (long) value);
	  newval |= value << 5; /* 6 - 1.  */
	  break;

	default:
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			"Unable to process relocation for thumb opcode: %lx",
			(unsigned long) newval);
	  break;
	}
      md_number_to_chars (buf, newval, THUMB_SIZE);
      break;

    case BFD_RELOC_ARM_THUMB_ADD:
      /* This is a complicated relocation, since we use it for all of
	 the following immediate relocations:

	    3bit ADD/SUB
	    8bit ADD/SUB
	    9bit ADD/SUB SP word-aligned
	   10bit ADD PC/SP word-aligned

	 The type of instruction being processed is encoded in the
	 instruction field:

	   0x8000  SUB
	   0x00F0  Rd
	   0x000F  Rs
      */
      newval = md_chars_to_number (buf, THUMB_SIZE);
      {
	int rd = (newval >> 4) & 0xf;
	int rs = newval & 0xf;
	int subtract = !!(newval & 0x8000);

	/* Check for HI regs, only very restricted cases allowed:
	   Adjusting SP, and using PC or SP to get an address.	*/
	if ((rd > 7 && (rd != REG_SP || rs != REG_SP))
	    || (rs > 7 && rs != REG_SP && rs != REG_PC))
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			_("invalid Hi register with immediate"));

	/* If value is negative, choose the opposite instruction.  */
	if ((offsetT) value < 0)
	  {
	    value = -value;
	    subtract = !subtract;
	    if ((offsetT) value < 0)
	      as_bad_where (fixP->fx_file, fixP->fx_line,
			    _("immediate value out of range"));
	  }

	if (rd == REG_SP)
	  {
 	    if (value & ~0x1fc)
	      as_bad_where (fixP->fx_file, fixP->fx_line,
			    _("invalid immediate for stack address calculation"));
	    newval = subtract ? T_OPCODE_SUB_ST : T_OPCODE_ADD_ST;
	    newval |= value >> 2;
	  }
	else if (rs == REG_PC || rs == REG_SP)
	  {
	    /* PR gas/18541.  If the addition is for a defined symbol
	       within range of an ADR instruction then accept it.  */
	    if (subtract
		&& value == 4
		&& fixP->fx_addsy != NULL)
	      {
		subtract = 0;

		if (! S_IS_DEFINED (fixP->fx_addsy)
		    || S_GET_SEGMENT (fixP->fx_addsy) != seg
		    || S_IS_WEAK (fixP->fx_addsy))
		  {
		    as_bad_where (fixP->fx_file, fixP->fx_line,
				  _("address calculation needs a strongly defined nearby symbol"));
		  }
		else
		  {
		    offsetT v = fixP->fx_where + fixP->fx_frag->fr_address;

		    /* Round up to the next 4-byte boundary.  */
		    if (v & 3)
		      v = (v + 3) & ~ 3;
		    else
		      v += 4;
		    v = S_GET_VALUE (fixP->fx_addsy) - v;

		    if (v & ~0x3fc)
		      {
			as_bad_where (fixP->fx_file, fixP->fx_line,
				      _("symbol too far away"));
		      }
		    else
		      {
			fixP->fx_done = 1;
			value = v;
		      }
		  }
	      }

	    if (subtract || value & ~0x3fc)
	      as_bad_where (fixP->fx_file, fixP->fx_line,
			    _("invalid immediate for address calculation (value = 0x%08lX)"),
			    (unsigned long) (subtract ? - value : value));
	    newval = (rs == REG_PC ? T_OPCODE_ADD_PC : T_OPCODE_ADD_SP);
	    newval |= rd << 8;
	    newval |= value >> 2;
	  }
	else if (rs == rd)
	  {
	    if (value & ~0xff)
	      as_bad_where (fixP->fx_file, fixP->fx_line,
			    _("immediate value out of range"));
	    newval = subtract ? T_OPCODE_SUB_I8 : T_OPCODE_ADD_I8;
	    newval |= (rd << 8) | value;
	  }
	else
	  {
	    if (value & ~0x7)
	      as_bad_where (fixP->fx_file, fixP->fx_line,
			    _("immediate value out of range"));
	    newval = subtract ? T_OPCODE_SUB_I3 : T_OPCODE_ADD_I3;
	    newval |= rd | (rs << 3) | (value << 6);
	  }
      }
      md_number_to_chars (buf, newval, THUMB_SIZE);
      break;

    case BFD_RELOC_ARM_THUMB_IMM:
      newval = md_chars_to_number (buf, THUMB_SIZE);
      if (value > 255)
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("invalid immediate: %ld is out of range"),
		      (long) value);
      newval |= value;
      md_number_to_chars (buf, newval, THUMB_SIZE);
      break;

    case BFD_RELOC_ARM_THUMB_SHIFT:
      /* 5bit shift value (0..32).  LSL cannot take 32.	 */
      newval = md_chars_to_number (buf, THUMB_SIZE) & 0xf83f;
      temp = newval & 0xf800;
      if (value > 32 || (value == 32 && temp == T_OPCODE_LSL_I))
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("invalid shift value: %ld"), (long) value);
      /* Shifts of zero must be encoded as LSL.	 */
      if (value == 0)
	newval = (newval & 0x003f) | T_OPCODE_LSL_I;
      /* Shifts of 32 are encoded as zero.  */
      else if (value == 32)
	value = 0;
      newval |= value << 6;
      md_number_to_chars (buf, newval, THUMB_SIZE);
      break;

    case BFD_RELOC_VTABLE_INHERIT:
    case BFD_RELOC_VTABLE_ENTRY:
      fixP->fx_done = 0;
      return;

    case BFD_RELOC_ARM_MOVW:
    case BFD_RELOC_ARM_MOVT:
    case BFD_RELOC_ARM_THUMB_MOVW:
    case BFD_RELOC_ARM_THUMB_MOVT:
      if (fixP->fx_done || !seg->use_rela_p)
	{
	  /* REL format relocations are limited to a 16-bit addend.  */
	  if (!fixP->fx_done)
	    {
	      if (value + 0x8000 > 0x7fff + 0x8000)
		  as_bad_where (fixP->fx_file, fixP->fx_line,
				_("offset out of range"));
	    }
	  else if (fixP->fx_r_type == BFD_RELOC_ARM_MOVT
		   || fixP->fx_r_type == BFD_RELOC_ARM_THUMB_MOVT)
	    {
	      value >>= 16;
	    }

	  if (fixP->fx_r_type == BFD_RELOC_ARM_THUMB_MOVW
	      || fixP->fx_r_type == BFD_RELOC_ARM_THUMB_MOVT)
	    {
	      newval = get_thumb32_insn (buf);
	      newval &= 0xfbf08f00;
	      newval |= (value & 0xf000) << 4;
	      newval |= (value & 0x0800) << 15;
	      newval |= (value & 0x0700) << 4;
	      newval |= (value & 0x00ff);
	      put_thumb32_insn (buf, newval);
	    }
	  else
	    {
	      newval = md_chars_to_number (buf, 4);
	      newval &= 0xfff0f000;
	      newval |= value & 0x0fff;
	      newval |= (value & 0xf000) << 4;
	      md_number_to_chars (buf, newval, 4);
	    }
	}
      return;

   case BFD_RELOC_ARM_THUMB_ALU_ABS_G0_NC:
   case BFD_RELOC_ARM_THUMB_ALU_ABS_G1_NC:
   case BFD_RELOC_ARM_THUMB_ALU_ABS_G2_NC:
   case BFD_RELOC_ARM_THUMB_ALU_ABS_G3_NC:
      gas_assert (!fixP->fx_done);
      {
	bfd_vma insn;
	bool is_mov;
	bfd_vma encoded_addend = value;

	/* Check that addend can be encoded in instruction.  */
	if (!seg->use_rela_p && value > 255)
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			_("the offset 0x%08lX is not representable"),
			(unsigned long) encoded_addend);

	/* Extract the instruction.  */
	insn = md_chars_to_number (buf, THUMB_SIZE);
	is_mov = (insn & 0xf800) == 0x2000;

	/* Encode insn.  */
	if (is_mov)
	  {
	    if (!seg->use_rela_p)
	      insn |= encoded_addend;
	  }
	else
	  {
	    int rd, rs;

	    /* Extract the instruction.  */
	     /* Encoding is the following
		0x8000  SUB
		0x00F0  Rd
		0x000F  Rs
	     */
	     /* The following conditions must be true :
		- ADD
		- Rd == Rs
		- Rd <= 7
	     */
	    rd = (insn >> 4) & 0xf;
	    rs = insn & 0xf;
	    if ((insn & 0x8000) || (rd != rs) || rd > 7)
	      as_bad_where (fixP->fx_file, fixP->fx_line,
			_("Unable to process relocation for thumb opcode: %lx"),
			(unsigned long) insn);

	    /* Encode as ADD immediate8 thumb 1 code.  */
	    insn = 0x3000 | (rd << 8);

	    /* Place the encoded addend into the first 8 bits of the
	       instruction.  */
	    if (!seg->use_rela_p)
	      insn |= encoded_addend;
	  }

	/* Update the instruction.  */
	md_number_to_chars (buf, insn, THUMB_SIZE);
      }
      break;

   case BFD_RELOC_ARM_ALU_PC_G0_NC:
   case BFD_RELOC_ARM_ALU_PC_G0:
   case BFD_RELOC_ARM_ALU_PC_G1_NC:
   case BFD_RELOC_ARM_ALU_PC_G1:
   case BFD_RELOC_ARM_ALU_PC_G2:
   case BFD_RELOC_ARM_ALU_SB_G0_NC:
   case BFD_RELOC_ARM_ALU_SB_G0:
   case BFD_RELOC_ARM_ALU_SB_G1_NC:
   case BFD_RELOC_ARM_ALU_SB_G1:
   case BFD_RELOC_ARM_ALU_SB_G2:
     gas_assert (!fixP->fx_done);
     if (!seg->use_rela_p)
       {
	 bfd_vma insn;
	 bfd_vma encoded_addend;
	 bfd_vma addend_abs = llabs ((offsetT) value);

	 /* Check that the absolute value of the addend can be
	    expressed as an 8-bit constant plus a rotation.  */
	 encoded_addend = encode_arm_immediate (addend_abs);
	 if (encoded_addend == (unsigned int) FAIL)
	   as_bad_where (fixP->fx_file, fixP->fx_line,
			 _("the offset 0x%08lX is not representable"),
			 (unsigned long) addend_abs);

	 /* Extract the instruction.  */
	 insn = md_chars_to_number (buf, INSN_SIZE);

	 /* If the addend is positive, use an ADD instruction.
	    Otherwise use a SUB.  Take care not to destroy the S bit.  */
	 insn &= 0xff1fffff;
	 if ((offsetT) value < 0)
	   insn |= 1 << 22;
	 else
	   insn |= 1 << 23;

	 /* Place the encoded addend into the first 12 bits of the
	    instruction.  */
	 insn &= 0xfffff000;
	 insn |= encoded_addend;

	 /* Update the instruction.  */
	 md_number_to_chars (buf, insn, INSN_SIZE);
       }
     break;

    case BFD_RELOC_ARM_LDR_PC_G0:
    case BFD_RELOC_ARM_LDR_PC_G1:
    case BFD_RELOC_ARM_LDR_PC_G2:
    case BFD_RELOC_ARM_LDR_SB_G0:
    case BFD_RELOC_ARM_LDR_SB_G1:
    case BFD_RELOC_ARM_LDR_SB_G2:
      gas_assert (!fixP->fx_done);
      if (!seg->use_rela_p)
	{
	  bfd_vma insn;
	  bfd_vma addend_abs = llabs ((offsetT) value);

	  /* Check that the absolute value of the addend can be
	     encoded in 12 bits.  */
	  if (addend_abs >= 0x1000)
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("bad offset 0x%08lX (only 12 bits available for the magnitude)"),
			  (unsigned long) addend_abs);

	  /* Extract the instruction.  */
	  insn = md_chars_to_number (buf, INSN_SIZE);

	  /* If the addend is negative, clear bit 23 of the instruction.
	     Otherwise set it.  */
	  if ((offsetT) value < 0)
	    insn &= ~(1 << 23);
	  else
	    insn |= 1 << 23;

	  /* Place the absolute value of the addend into the first 12 bits
	     of the instruction.  */
	  insn &= 0xfffff000;
	  insn |= addend_abs;

	  /* Update the instruction.  */
	  md_number_to_chars (buf, insn, INSN_SIZE);
	}
      break;

    case BFD_RELOC_ARM_LDRS_PC_G0:
    case BFD_RELOC_ARM_LDRS_PC_G1:
    case BFD_RELOC_ARM_LDRS_PC_G2:
    case BFD_RELOC_ARM_LDRS_SB_G0:
    case BFD_RELOC_ARM_LDRS_SB_G1:
    case BFD_RELOC_ARM_LDRS_SB_G2:
      gas_assert (!fixP->fx_done);
      if (!seg->use_rela_p)
	{
	  bfd_vma insn;
	  bfd_vma addend_abs = llabs ((offsetT) value);

	  /* Check that the absolute value of the addend can be
	     encoded in 8 bits.  */
	  if (addend_abs >= 0x100)
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("bad offset 0x%08lX (only 8 bits available for the magnitude)"),
			  (unsigned long) addend_abs);

	  /* Extract the instruction.  */
	  insn = md_chars_to_number (buf, INSN_SIZE);

	  /* If the addend is negative, clear bit 23 of the instruction.
	     Otherwise set it.  */
	  if ((offsetT) value < 0)
	    insn &= ~(1 << 23);
	  else
	    insn |= 1 << 23;

	  /* Place the first four bits of the absolute value of the addend
	     into the first 4 bits of the instruction, and the remaining
	     four into bits 8 .. 11.  */
	  insn &= 0xfffff0f0;
	  insn |= (addend_abs & 0xf) | ((addend_abs & 0xf0) << 4);

	  /* Update the instruction.  */
	  md_number_to_chars (buf, insn, INSN_SIZE);
	}
      break;

    case BFD_RELOC_ARM_LDC_PC_G0:
    case BFD_RELOC_ARM_LDC_PC_G1:
    case BFD_RELOC_ARM_LDC_PC_G2:
    case BFD_RELOC_ARM_LDC_SB_G0:
    case BFD_RELOC_ARM_LDC_SB_G1:
    case BFD_RELOC_ARM_LDC_SB_G2:
      gas_assert (!fixP->fx_done);
      if (!seg->use_rela_p)
	{
	  bfd_vma insn;
	  bfd_vma addend_abs = llabs ((offsetT) value);

	  /* Check that the absolute value of the addend is a multiple of
	     four and, when divided by four, fits in 8 bits.  */
	  if (addend_abs & 0x3)
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("bad offset 0x%08lX (must be word-aligned)"),
			  (unsigned long) addend_abs);

	  if ((addend_abs >> 2) > 0xff)
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("bad offset 0x%08lX (must be an 8-bit number of words)"),
			  (unsigned long) addend_abs);

	  /* Extract the instruction.  */
	  insn = md_chars_to_number (buf, INSN_SIZE);

	  /* If the addend is negative, clear bit 23 of the instruction.
	     Otherwise set it.  */
	  if ((offsetT) value < 0)
	    insn &= ~(1 << 23);
	  else
	    insn |= 1 << 23;

	  /* Place the addend (divided by four) into the first eight
	     bits of the instruction.  */
	  insn &= 0xfffffff0;
	  insn |= addend_abs >> 2;

	  /* Update the instruction.  */
	  md_number_to_chars (buf, insn, INSN_SIZE);
	}
      break;

    case BFD_RELOC_THUMB_PCREL_BRANCH5:
      if (fixP->fx_addsy
	  && (S_GET_SEGMENT (fixP->fx_addsy) == seg)
	  && !S_FORCE_RELOC (fixP->fx_addsy, true)
	  && ARM_IS_FUNC (fixP->fx_addsy)
	  && ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v8_1m_main))
	{
	  /* Force a relocation for a branch 5 bits wide.  */
	  fixP->fx_done = 0;
	}
      if (v8_1_branch_value_check (value, 5, false) == FAIL)
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      BAD_BRANCH_OFF);

      if (fixP->fx_done || !seg->use_rela_p)
	{
	  addressT boff = value >> 1;

	  newval  = md_chars_to_number (buf, THUMB_SIZE);
	  newval |= (boff << 7);
	  md_number_to_chars (buf, newval, THUMB_SIZE);
	}
      break;

    case BFD_RELOC_THUMB_PCREL_BFCSEL:
      if (fixP->fx_addsy
	  && (S_GET_SEGMENT (fixP->fx_addsy) == seg)
	  && !S_FORCE_RELOC (fixP->fx_addsy, true)
	  && ARM_IS_FUNC (fixP->fx_addsy)
	  && ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v8_1m_main))
	{
	  fixP->fx_done = 0;
	}
      if ((value & ~0x7f) && ((value & ~0x3f) != (valueT) ~0x3f))
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("branch out of range"));

      if (fixP->fx_done || !seg->use_rela_p)
	{
	  newval  = md_chars_to_number (buf, THUMB_SIZE);

	  addressT boff = ((newval & 0x0780) >> 7) << 1;
	  addressT diff = value - boff;

	  if (diff == 4)
	    {
	      newval |= 1 << 1; /* T bit.  */
	    }
	  else if (diff != 2)
	    {
	      as_bad_where (fixP->fx_file, fixP->fx_line,
			    _("out of range label-relative fixup value"));
	    }
	  md_number_to_chars (buf, newval, THUMB_SIZE);
	}
      break;

    case BFD_RELOC_ARM_THUMB_BF17:
      if (fixP->fx_addsy
	  && (S_GET_SEGMENT (fixP->fx_addsy) == seg)
	  && !S_FORCE_RELOC (fixP->fx_addsy, true)
	  && ARM_IS_FUNC (fixP->fx_addsy)
	  && ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v8_1m_main))
	{
	  /* Force a relocation for a branch 17 bits wide.  */
	  fixP->fx_done = 0;
	}

      if (v8_1_branch_value_check (value, 17, true) == FAIL)
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      BAD_BRANCH_OFF);

      if (fixP->fx_done || !seg->use_rela_p)
	{
	  offsetT newval2;
	  addressT immA, immB, immC;

	  immA = (value & 0x0001f000) >> 12;
	  immB = (value & 0x00000ffc) >> 2;
	  immC = (value & 0x00000002) >> 1;

	  newval   = md_chars_to_number (buf, THUMB_SIZE);
	  newval2  = md_chars_to_number (buf + THUMB_SIZE, THUMB_SIZE);
	  newval  |= immA;
	  newval2 |= (immC << 11) | (immB << 1);
	  md_number_to_chars (buf, newval, THUMB_SIZE);
	  md_number_to_chars (buf + THUMB_SIZE, newval2, THUMB_SIZE);
	}
      break;

    case BFD_RELOC_ARM_THUMB_BF19:
      if (fixP->fx_addsy
	  && (S_GET_SEGMENT (fixP->fx_addsy) == seg)
	  && !S_FORCE_RELOC (fixP->fx_addsy, true)
	  && ARM_IS_FUNC (fixP->fx_addsy)
	  && ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v8_1m_main))
	{
	  /* Force a relocation for a branch 19 bits wide.  */
	  fixP->fx_done = 0;
	}

      if (v8_1_branch_value_check (value, 19, true) == FAIL)
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      BAD_BRANCH_OFF);

      if (fixP->fx_done || !seg->use_rela_p)
	{
	  offsetT newval2;
	  addressT immA, immB, immC;

	  immA = (value & 0x0007f000) >> 12;
	  immB = (value & 0x00000ffc) >> 2;
	  immC = (value & 0x00000002) >> 1;

	  newval   = md_chars_to_number (buf, THUMB_SIZE);
	  newval2  = md_chars_to_number (buf + THUMB_SIZE, THUMB_SIZE);
	  newval  |= immA;
	  newval2 |= (immC << 11) | (immB << 1);
	  md_number_to_chars (buf, newval, THUMB_SIZE);
	  md_number_to_chars (buf + THUMB_SIZE, newval2, THUMB_SIZE);
	}
      break;

    case BFD_RELOC_ARM_THUMB_BF13:
      if (fixP->fx_addsy
	  && (S_GET_SEGMENT (fixP->fx_addsy) == seg)
	  && !S_FORCE_RELOC (fixP->fx_addsy, true)
	  && ARM_IS_FUNC (fixP->fx_addsy)
	  && ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v8_1m_main))
	{
	  /* Force a relocation for a branch 13 bits wide.  */
	  fixP->fx_done = 0;
	}

      if (v8_1_branch_value_check (value, 13, true) == FAIL)
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      BAD_BRANCH_OFF);

      if (fixP->fx_done || !seg->use_rela_p)
	{
	  offsetT newval2;
	  addressT immA, immB, immC;

	  immA = (value & 0x00001000) >> 12;
	  immB = (value & 0x00000ffc) >> 2;
	  immC = (value & 0x00000002) >> 1;

	  newval   = md_chars_to_number (buf, THUMB_SIZE);
	  newval2  = md_chars_to_number (buf + THUMB_SIZE, THUMB_SIZE);
	  newval  |= immA;
	  newval2 |= (immC << 11) | (immB << 1);
	  md_number_to_chars (buf, newval, THUMB_SIZE);
	  md_number_to_chars (buf + THUMB_SIZE, newval2, THUMB_SIZE);
	}
      break;

    case BFD_RELOC_ARM_THUMB_LOOP12:
      if (fixP->fx_addsy
	  && (S_GET_SEGMENT (fixP->fx_addsy) == seg)
	  && !S_FORCE_RELOC (fixP->fx_addsy, true)
	  && ARM_IS_FUNC (fixP->fx_addsy)
	  && ARM_CPU_HAS_FEATURE (selected_cpu, arm_ext_v8_1m_main))
	{
	  /* Force a relocation for a branch 12 bits wide.  */
	  fixP->fx_done = 0;
	}

      bfd_vma insn = get_thumb32_insn (buf);
      /* le lr, <label>, le <label> or letp lr, <label> */
      if (((insn & 0xffffffff) == 0xf00fc001)
	  || ((insn & 0xffffffff) == 0xf02fc001)
	  || ((insn & 0xffffffff) == 0xf01fc001))
	value = -value;

      if (v8_1_branch_value_check (value, 12, false) == FAIL)
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      BAD_BRANCH_OFF);
      if (fixP->fx_done || !seg->use_rela_p)
	{
	  addressT imml, immh;

	  immh = (value & 0x00000ffc) >> 2;
	  imml = (value & 0x00000002) >> 1;

	  newval  = md_chars_to_number (buf + THUMB_SIZE, THUMB_SIZE);
	  newval |= (imml << 11) | (immh << 1);
	  md_number_to_chars (buf + THUMB_SIZE, newval, THUMB_SIZE);
	}
      break;

    case BFD_RELOC_ARM_V4BX:
      /* This will need to go in the object file.  */
      fixP->fx_done = 0;
      break;

    case BFD_RELOC_UNUSED:
    default:
      as_bad_where (fixP->fx_file, fixP->fx_line,
		    _("bad relocation fixup type (%d)"), fixP->fx_r_type);
    }
}

void
armelf_frob_symbol (symbolS * symp,
		    int *     puntp) {
    elf_frob_symbol (symp, puntp);
}

static void
check_mapping_symbols (bfd *abfd ATTRIBUTE_UNUSED, asection *sec,
		       void *dummy ATTRIBUTE_UNUSED) {
    segment_info_type *seginfo = seg_info (sec);
    fragS *fragp;
    if (seginfo == NULL || seginfo->frchainP == NULL)
        return;

    for (fragp = seginfo->frchainP->frch_root;
        fragp != NULL;
        fragp = fragp->fr_next) {
        symbolS *sym = fragp->tc_frag_data.last_map;
        fragS *next = fragp->fr_next;
        if (sym == NULL || next == NULL)
            continue;

        if (S_GET_VALUE (sym) < next->fr_address)
            /* Not at the end of this frag.  */
            continue;
        know (S_GET_VALUE (sym) == next->fr_address);
        do {
            if (next->tc_frag_data.first_map != NULL) {
                /* Next frag starts with a mapping symbol.  Discard this
                one.  */
                symbol_remove (sym, &symbol_rootP, &symbol_lastP);
                break;
            }

            if (next->fr_next == NULL) {
                /* This mapping symbol is at the end of the section.  Discard
                it.  */
                know (next->fr_fix == 0 && next->fr_var == 0);
                symbol_remove (sym, &symbol_rootP, &symbol_lastP);
                break;
            }

            if (next->fr_address != next->fr_next->fr_address)
                break;

            next = next->fr_next;
        }  while (next != NULL);
    }
}

/**
 * @Description: 调整符号表
 */
void
arm_adjust_symtab (void) {
    symbolS * sym;
    char	    bind;
    for (sym = symbol_rootP; sym != NULL; sym = symbol_next (sym)) {
        if (ARM_IS_THUMB (sym)) {
            elf_symbol_type * elf_sym;

            elf_sym = elf_symbol (symbol_get_bfdsym (sym));
            bind = ELF_ST_BIND (elf_sym->internal_elf_sym.st_info);

            if (! bfd_is_arm_special_symbol_name (elf_sym->symbol.name,
                BFD_ARM_SPECIAL_SYM_TYPE_ANY)) {
                /* If it's a .thumb_func, declare it as so,
                otherwise tag label as .code 16.  */
                if (THUMB_IS_FUNC (sym))
                ARM_SET_SYM_BRANCH_TYPE (elf_sym->internal_elf_sym.st_target_internal,
                                ST_BRANCH_TO_THUMB);
                else if (EF_ARM_EABI_VERSION (meabi_flags) < EF_ARM_EABI_VER4)
                    elf_sym->internal_elf_sym.st_info =
                ELF_ST_INFO (bind, STT_ARM_16BIT);
            }
        }
    }
    /* Remove any overlapping mapping symbols generated by alignment frags.  */
    bfd_map_over_sections (stdoutput, check_mapping_symbols, (char *) 0);
    /* Now do generic ELF adjustments.  */
    elf_adjust_symtab ();
}


void
arm_frag_align_code (int n, int max) {
    char * p;
    p = frag_var (rs_align_code,
            MAX_MEM_FOR_RS_ALIGN_CODE,
            1,
            (relax_substateT) max,
            (symbolS *) NULL,
            (offsetT) n,
            (char *) NULL);
    *p = 0;
}

struct arm_cpu_option_table {
    const char *           name;
    size_t                 name_len;
    const arm_feature_set	 value;
    const arm_feature_set	 ext;
    /* For some CPUs we assume an FPU unless the user explicitly sets
        -mfpu=...	*/
    const arm_feature_set	 default_fpu;
    /* The canonical name of the CPU, or NULL to use NAME converted to upper
        case.  */
    const char *           canonical_name;
};

/* This list should, at a minimum, contain all the cpu names
   recognized by GCC.  */
#define ARM_CPU_OPT(N, CN, V, E, DF) { N, sizeof (N) - 1, V, E, DF, CN }

static const struct arm_cpu_option_table arm_cpus[] = {
    ARM_CPU_OPT ("arm1",		  NULL,		       ARM_ARCH_V1,
        ARM_ARCH_NONE,
        FPU_ARCH_FPA),
    ARM_CPU_OPT ("arm7tdmi",	  NULL,		       ARM_ARCH_V4T,
        ARM_ARCH_NONE,
        FPU_ARCH_FPA),
    { NULL, 0, ARM_ARCH_NONE, ARM_ARCH_NONE, ARM_ARCH_NONE, NULL }
};
/**
 * @Description: 解析.cpu
 */
static void
s_arm_cpu (int ignored ATTRIBUTE_UNUSED) {
    const struct arm_cpu_option_table *opt;
    char *name;
    char saved_char;

    name = input_line_pointer;
    input_line_pointer = find_end_of_line (input_line_pointer, flag_m68k_mri);
    saved_char = *input_line_pointer;
    *input_line_pointer = 0;

    /* Skip the first "all" entry.  */
    for (opt = arm_cpus + 1; opt->name != NULL; opt++) 
        if (streq (opt->name, name)) {
            selected_arch = opt->value;
            selected_ext = opt->ext;
            ARM_MERGE_FEATURE_SETS (selected_cpu, selected_arch, selected_ext);
            if (opt->canonical_name)
                strcpy (selected_cpu_name, opt->canonical_name);
            else {
                int i;
                for (i = 0; opt->name[i]; i++)
                    selected_cpu_name[i] = TOUPPER (opt->name[i]);

                selected_cpu_name[i] = 0;
            }
            ARM_MERGE_FEATURE_SETS (cpu_variant, selected_cpu, selected_fpu);

            *input_line_pointer = saved_char;
            demand_empty_rest_of_line ();
            return;
        }
    as_bad (_("unknown cpu `%s'"), name);
    *input_line_pointer = saved_char;
}

struct arm_option_fpu_value_table {
    const char *           name;
    const arm_feature_set  value;
};
static const struct arm_option_fpu_value_table arm_fpus[] = {
    {"softvfp",		FPU_ARCH_VFP},
    {NULL,		ARM_ARCH_NONE}
};
static void
s_arm_fpu (int ignored ATTRIBUTE_UNUSED) {
    const struct arm_option_fpu_value_table *opt;
    char saved_char;
    char *name;

    name = input_line_pointer;
    input_line_pointer = find_end_of_line (input_line_pointer, flag_m68k_mri);
    saved_char = *input_line_pointer;
    *input_line_pointer = 0;
    for (opt = arm_fpus; opt->name != NULL; opt++)
        if (streq (opt->name, name)) {
            selected_fpu = opt->value;
            ARM_CLEAR_FEATURE (selected_cpu, selected_cpu, fpu_any);
#ifndef CPU_DEFAULT
        if (no_cpu_selected ())
            ARM_MERGE_FEATURE_SETS (cpu_variant, arm_arch_any, selected_fpu);
        else
#endif
            ARM_MERGE_FEATURE_SETS (cpu_variant, selected_cpu, selected_fpu);
            *input_line_pointer = saved_char;
            return;
        }
    as_bad (_("unknown floating point format `%s'\n"), name);
    *input_line_pointer = saved_char;
    ignore_rest_of_line ();
}

struct arm_ext_table {
    const char *		  name;
    size_t		  name_len;
    const arm_feature_set	  merge;
    const arm_feature_set	  clear;
};

struct arm_arch_option_table {
    const char *			name;
    size_t			name_len;
    const arm_feature_set		value;
    const arm_feature_set		default_fpu;
    const struct arm_ext_table *	ext_table;
};

#define ARM_ARCH_OPT(N, V, DF) { N, sizeof (N) - 1, V, DF, NULL }
#define ARM_ARCH_OPT2(N, V, DF, ext) \
  { N, sizeof (N) - 1, V, DF, ext##_ext_table }
static const struct arm_arch_option_table arm_archs[] = {
    ARM_ARCH_OPT ("armv4t",	  ARM_ARCH_V4T,		FPU_ARCH_FPA),
    { NULL, 0, ARM_ARCH_NONE, ARM_ARCH_NONE, NULL }
};
static void
s_arm_arch (int ignored ATTRIBUTE_UNUSED) {
    const struct arm_arch_option_table *opt;
    char saved_char;
    char *name;

    name = input_line_pointer;
    input_line_pointer = find_end_of_line (input_line_pointer, flag_m68k_mri);
    saved_char = *input_line_pointer;
    *input_line_pointer = 0;

    /* Skip the first "all" entry.  */
    for (opt = arm_archs + 1; opt->name != NULL; opt++)
        if (streq (opt->name, name)) {
            selected_arch = opt->value;
            selected_ctx_ext_table = opt->ext_table;
            selected_ext = arm_arch_none;
            selected_cpu = selected_arch;
            strcpy (selected_cpu_name, opt->name);
            ARM_MERGE_FEATURE_SETS (cpu_variant, selected_cpu, selected_fpu);
            *input_line_pointer = saved_char;
            demand_empty_rest_of_line ();
            return;
        }

    as_bad (_("unknown architecture `%s'\n"), name);
    *input_line_pointer = saved_char;
    ignore_rest_of_line ();
}


void
cons_fix_new_arm (fragS *	frag,
		  int		where,
		  int		size,
		  expressionS * exp,
		  bfd_reloc_code_real_type reloc) {
    int pcrel = 0;
    switch (size) {
        case 4:
        default:
            reloc = BFD_RELOC_32;
            break;
    }
    fix_new_exp (frag, where, size, exp, pcrel, reloc);
}

/**
 * @Description: 将重定位信息的内部表示转换为BFD目标格式
 */
arelent *
tc_gen_reloc (asection *section, fixS *fixp) {
    arelent * reloc;
    bfd_reloc_code_real_type code;

    reloc = XNEW (arelent);
    reloc->sym_ptr_ptr = XNEW (asymbol *);
    *reloc->sym_ptr_ptr = symbol_get_bfdsym (fixp->fx_addsy);
    reloc->address = fixp->fx_frag->fr_address + fixp->fx_where;
    reloc->addend = fixp->fx_offset;

    switch (fixp->fx_r_type) {
        case BFD_RELOC_32:
        if (fixp->fx_pcrel) {
            code = BFD_RELOC_32_PCREL;
            break;
        }
        case BFD_RELOC_NONE:
        case BFD_RELOC_ARM_PCREL_BRANCH:
            code = fixp->fx_r_type;
            break;
    }
    reloc->howto = bfd_reloc_type_lookup (stdoutput, code);
    /* HACK: Since arm ELF uses Rel instead of Rela, encode the
     vtable entry to be used in the relocation's section offset.  */
    if (fixp->fx_r_type == BFD_RELOC_VTABLE_ENTRY)
        reloc->address = fixp->fx_offset;

    return reloc;
}

static void
close_automatic_it_block (void) {
    now_pred.mask = 0x10;
    now_pred.block_length = 0;
}

static void
force_automatic_it_block_close (void) {
    if (now_pred.state == AUTOMATIC_PRED_BLOCK) {
        close_automatic_it_block ();
        now_pred.state = OUTSIDE_PRED_BLOCK;
        now_pred.mask = 0;
    }
}

void
arm_frob_label (symbolS * sym) {
    last_label_seen = sym;

    ARM_SET_THUMB (sym, thumb_mode);

#if defined OBJ_COFF || defined OBJ_ELF
    ARM_SET_INTERWORK (sym, support_interwork);
#endif

    force_automatic_it_block_close (); 

    if (label_is_thumb_function_name
      && (S_GET_NAME (sym)[0] != '.' || S_GET_NAME (sym)[1] != 'L')
      && (bfd_section_flags (now_seg) & SEC_CODE) != 0) {
        /* When the address of a Thumb function is taken the bottom
        bit of that address should be set.  This will allow
        interworking between Arm and Thumb functions to work
        correctly.  */

        THUMB_SET_FUNC (sym, 1);

        label_is_thumb_function_name = false;
    }

}

static int
in_pred_block (void) {
    if (!now_pred.state_handled)
        handle_pred_state ();

    return now_pred.state != OUTSIDE_PRED_BLOCK;
}




