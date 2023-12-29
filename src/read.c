#include "as.h"
#include "safe-ctype.h"
#include "obj-elf.h"
#include "tc-arm.h"
#include "subsegs.h"
static htab_t po_hash;
char *input_line_pointer;
symbolS *mri_common_symbol;
static void pobegin (void);
char is_end_of_line[256] = {
#ifdef CR_EOL
  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0,	/* @abcdefghijklmno */
#else
  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,	/* @abcdefghijklmno */
#endif
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* _!"#$%&'()*+,-./ */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 0123456789:;<=>? */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0	/* */
};

#ifndef LEX_AT
#define LEX_AT 0
#endif

#ifndef LEX_BR
/* The RS/6000 assembler uses {,},[,] as parts of symbol names.  */
#define LEX_BR 0
#endif

#ifndef LEX_PCT
/* The Delta 68k assembler permits % inside label names.  */
#define LEX_PCT 0
#endif

#ifndef LEX_QM
/* The PowerPC Windows NT assemblers permits ? inside label names.  */
#define LEX_QM 0
#endif

#ifndef LEX_HASH
/* The IA-64 assembler uses # as a suffix designating a symbol.  We include
   it in the symbol and strip it out in tc_canonicalize_symbol_name.  */
#define LEX_HASH 0
#endif

#ifndef LEX_DOLLAR
#define LEX_DOLLAR 3
#endif

#ifndef LEX_TILDE
/* The Delta 68k assembler permits ~ at start of label names.  */
#define LEX_TILDE 0
#endif
/* Used by is_... macros. our ctype[].  */
char lex_type[256] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* @ABCDEFGHIJKLMNO */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* PQRSTUVWXYZ[\]^_ */
  0, 0, 0, LEX_HASH, LEX_DOLLAR, LEX_PCT, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, /* _!"#$%&'()*+,-./ */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, LEX_QM,	/* 0123456789:;<=>? */
  LEX_AT, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,	/* @ABCDEFGHIJKLMNO */
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, LEX_BR, 0, LEX_BR, 0, 3, /* PQRSTUVWXYZ[\]^_ */
  0, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,	/* `abcdefghijklmno */
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, LEX_BR, 0, LEX_BR, LEX_TILDE, 0, /* pqrstuvwxyz{|}~.  */
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3
};

#ifndef TC_CASE_SENSITIVE
char original_case_string[128];
#endif

bool input_from_string = false;
symbolS *line_label;
// 常见的伪指令
static const pseudo_typeS potable[] = {
//   {"abort", s_abort, 0},
//   {"align", s_align_ptwo, 0},
//   // {"altmacro", s_altmacro, 1},
//   {"ascii", stringer, 8+0},
//   {"asciz", stringer, 8+1},
//   {"balign", s_align_bytes, 0},
//   {"balignw", s_align_bytes, -2},
//   {"balignl", s_align_bytes, -4},
// /* block  */
// #ifdef HANDLE_BUNDLE
//   {"bundle_align_mode", s_bundle_align_mode, 0},
//   {"bundle_lock", s_bundle_lock, 0},
//   {"bundle_unlock", s_bundle_unlock, 0},
// #endif
//   {"byte", cons, 1},
  {"comm", s_comm, 0},
//   {"common", s_mri_common, 0},
//   {"common.s", s_mri_common, 1},
//   {"data", s_data, 0},
//   {"dc", cons, 2},
//   {"dc.a", cons, 0},
//   {"dc.b", cons, 1},
//   {"dc.d", float_cons, 'd'},
//   {"dc.l", cons, 4},
//   {"dc.s", float_cons, 'f'},
//   {"dc.w", cons, 2},
//   {"dc.x", float_cons, 'x'},
//   {"dcb", s_space, 2},
//   {"dcb.b", s_space, 1},
//   {"dcb.d", s_float_space, 'd'},
//   {"dcb.l", s_space, 4},
//   {"dcb.s", s_float_space, 'f'},
//   {"dcb.w", s_space, 2},
//   {"dcb.x", s_float_space, 'x'},
//   {"ds", s_space, 2},
//   {"ds.b", s_space, 1},
//   {"ds.d", s_space, 8},
//   {"ds.l", s_space, 4},
//   {"ds.p", s_space, 'p'},
//   {"ds.s", s_space, 4},
//   {"ds.w", s_space, 2},
//   {"ds.x", s_space, 'x'},
//   {"debug", s_ignore, 0},
// #ifdef S_SET_DESC
//   {"desc", s_desc, 0},
// #endif
// /* dim  */
//   {"double", float_cons, 'd'},
// /* dsect  */
//   {"eject", listing_eject, 0},	/* Formfeed listing.  */
//   {"else", s_else, 0},
//   {"elsec", s_else, 0},
//   {"elseif", s_elseif, (int) O_ne},
  {"end", s_end, 0},
//   {"endc", s_endif, 0},
//   {"endfunc", s_func, 1},
//   {"endif", s_endif, 0},
//   // {"endm", s_bad_end, 0},
//   // {"endr", s_bad_end, 1},
// /* endef  */
//   {"equ", s_set, 0},
//   {"equiv", s_set, 1},
//   {"eqv", s_set, -1},
//   {"err", s_err, 0},
//   {"error", s_errwarn, 1},
//   {"exitm", s_mexit, 0},
// /* extend  */
//   {"extern", s_ignore, 0},	/* We treat all undef as ext.  */
//   {"fail", s_fail, 0},
//   {"file", s_file, 0},
//   {"fill", s_fill, 0},
  {"float", float_cons, 'f'},
//   {"format", s_ignore, 0},
//   {"func", s_func, 0},
  {"global", s_globl, 0},
  {"globl", s_globl, 0},
//   {"hword", cons, 2},
//   {"if", s_if, (int) O_ne},
//   {"ifb", s_ifb, 1},
//   {"ifc", s_ifc, 0},
//   {"ifdef", s_ifdef, 0},
//   {"ifeq", s_if, (int) O_eq},
//   {"ifeqs", s_ifeqs, 0},
//   {"ifge", s_if, (int) O_ge},
//   {"ifgt", s_if, (int) O_gt},
//   {"ifle", s_if, (int) O_le},
//   {"iflt", s_if, (int) O_lt},
//   {"ifnb", s_ifb, 0},
//   {"ifnc", s_ifc, 1},
//   {"ifndef", s_ifdef, 1},
//   {"ifne", s_if, (int) O_ne},
//   {"ifnes", s_ifeqs, 1},
//   {"ifnotdef", s_ifdef, 1},
//   {"incbin", s_incbin, 0},
//   {"include", s_include, 0},
//   {"int", cons, 4},
//   {"irp", s_irp, 0},
//   {"irep", s_irp, 0},
//   {"irpc", s_irp, 1},
//   {"irepc", s_irp, 1},
//   {"lcomm", s_lcomm, 0},
//   {"lflags", s_ignore, 0},	/* Listing flags.  */
//   {"linefile", s_linefile, 0},
//   {"linkonce", s_linkonce, 0},
//   {"list", listing_list, 1},	/* Turn listing on.  */
//   {"llen", listing_psize, 1},
//   {"long", cons, 4},
//   {"lsym", s_lsym, 0},
//   {"macro", s_macro, 0},
//   {"mexit", s_mexit, 0},
//   {"mri", s_mri, 0},
//   {".mri", s_mri, 0},	/* Special case so .mri works in MRI mode.  */
//   {"name", s_ignore, 0},
//   // {"noaltmacro", s_altmacro, 0},
//   {"noformat", s_ignore, 0},
//   {"nolist", listing_list, 0},	/* Turn listing off.  */
//   {"nopage", listing_nopage, 0},
//   {"nop", s_nop, 0},
//   {"nops", s_nops, 0},
//   {"octa", cons, 16},
//   {"offset", s_struct, 0},
//   {"org", s_org, 0},
//   {"p2align", s_align_ptwo, 0},
//   {"p2alignw", s_align_ptwo, -2},
//   {"p2alignl", s_align_ptwo, -4},
//   {"page", listing_eject, 0},
//   {"plen", listing_psize, 0},
//   {"print", s_print, 0},
//   {"psize", listing_psize, 0},	/* Set paper size.  */
//   {"purgem", s_purgem, 0},
//   {"quad", cons, 8},
//   // {"reloc", s_reloc, 0},
//   {"rep", s_rept, 0},
//   {"rept", s_rept, 0},
//   {"rva", s_rva, 4},
//   {"sbttl", listing_title, 1},	/* Subtitle of listing.  */
// /* scl  */
// /* sect  */
//   {"set", s_set, 0},
//   {"short", cons, 2},
//   {"single", float_cons, 'f'},
// /* size  */
  {"space", s_space, 0},
//   {"skip", s_space, 0},
//   {"sleb128", s_leb128, 1},
//   {"spc", s_ignore, 0},
//   {"stabd", s_stab, 'd'},
//   {"stabn", s_stab, 'n'},
//   {"stabs", s_stab, 's'},
//   {"string", stringer, 8+1},
//   {"string8", stringer, 8+1},
//   {"string16", stringer, 16+1},
//   {"string32", stringer, 32+1},
//   {"string64", stringer, 64+1},
//   {"struct", s_struct, 0},
// /* tag  */
  {"text", s_text, 0},
  
//   /* This is for gcc to use.  It's only just been added (2/94), so gcc
//      won't be able to use it for a while -- probably a year or more.
//      But once this has been released, check with gcc maintainers
//      before deleting it or even changing the spelling.  */
//   {"this_GCC_requires_the_GNU_assembler", s_ignore, 0},
//   /* If we're folding case -- done for some targets, not necessarily
//      all -- the above string in an input file will be converted to
//      this one.  Match it either way...  */
//   {"this_gcc_requires_the_gnu_assembler", s_ignore, 0},

//   {"title", listing_title, 0},	/* Listing title.  */
//   {"ttl", listing_title, 0},
// /* type  */
//   {"uleb128", s_leb128, 0},
// /* use  */
// /* val  */
//   {"xcom", s_comm, 0},
//   {"xdef", s_globl, 0},
//   {"xref", s_ignore, 0},
//   {"xstabs", s_xstab, 's'},
//   {"warning", s_errwarn, 0},
//   {"weakref", s_weakref, 0},
  {"word", cons, 2},
//   {"zero", s_space, 0},
//   {"2byte", cons, 2},
//   {"4byte", cons, 4},
//   {"8byte", cons, 8},
  {NULL, NULL, 0}			/* End sentinel.  */
};
// 能否覆盖
static int pop_override_ok;
// 哈希表名
static const char *pop_table_name;
// 通过维护这个偏移量，汇编器可以在生成可执行文件时确定绝对节在文件中的位置
addressT abs_section_offset;
// 记录当前函数
static char *current_name;
static char *current_label;
/* 缓冲区 */
static char *buffer;
static char *buffer_limit;	/*->1 + last char in buffer.  */
#define MAXIMUM_NUMBER_OF_CHARS_FOR_FLOAT (16)


static bool
in_bss (void) {
    flagword flags = bfd_section_flags (now_seg);

    return (flags & SEC_ALLOC) && !(flags & (SEC_LOAD | SEC_HAS_CONTENTS));
}

/**
 * @Description: 遇到.comm
 */
void
s_comm (int ignore) {
    s_comm_internal (0, elf_common_parse);
}

void
s_space (int mult) {
    expressionS exp;
    expressionS val;
    char *p = 0;
    char *stop = NULL;
    char stopc = 0;
    int bytes;
#ifdef md_cons_align
    md_cons_align (1);
#endif
    bytes = mult;

    expression (&exp);

    SKIP_WHITESPACE ();
    if (*input_line_pointer == ',') {
        ++input_line_pointer;
        expression (&val);
    } else {
        val.X_op = O_constant;
        val.X_add_number = 0;
    }

    if ((val.X_op != O_constant
        || val.X_add_number < - 0x80
        || val.X_add_number > 0xff
        || (mult != 0 && mult != 1 && val.X_add_number != 0))
        && (now_seg != absolute_section && !in_bss ())) {

    } else {
        if (now_seg == absolute_section || mri_common_symbol != NULL)
            resolve_expression (&exp);
        if (exp.X_op == O_constant) {
            addressT repeat = exp.X_add_number;
            addressT total;

            bytes = 0;
            if ((offsetT) repeat < 0) {
                as_warn (_(".space repeat count is negative, ignored"));
                goto getout;
            }
            if (repeat == 0) {
                if (!flag_mri)
                    as_warn (_(".space repeat count is zero, ignored"));
                goto getout;
            }
            if ((unsigned int) mult <= 1)
                total = repeat;
            else if (gas_mul_overflow (repeat, mult, &total)
                || (offsetT) total < 0) {
                as_warn (_(".space repeat count overflow, ignored"));
                goto getout;
            }

            bytes = total;
            /* If we are in the absolute section, just bump the offset.  */
            if (now_seg == absolute_section) {
                if (val.X_op != O_constant || val.X_add_number != 0)
                    as_warn (_("ignoring fill value in absolute section"));
                abs_section_offset += total;
                goto getout;
            }

            if (!need_pass_2)
                p = frag_var (rs_fill, 1, 1, (relax_substateT) 0, (symbolS *) 0,
                    (offsetT) total, (char *) 0);
        }

        if ((val.X_op != O_constant || val.X_add_number != 0) && in_bss ())
            as_warn (_("ignoring fill value in section `%s'"),
                segment_name (now_seg));
        else if (p)
            *p = val.X_add_number;
    }
getout:

  demand_empty_rest_of_line ();
}
void
ignore_rest_of_line (void) {
    while (input_line_pointer <= buffer_limit)
        if (is_end_of_line[(unsigned char) *input_line_pointer++])
            break;
}

/**
 * @Description: 解析float型
 */
static int
parse_one_float (int float_type, char temp[MAXIMUM_NUMBER_OF_CHARS_FOR_FLOAT])
{
    int length;

    SKIP_WHITESPACE ();
    if (input_line_pointer[0] == '0'
        && ISALPHA (input_line_pointer[1]))
        input_line_pointer += 2;

    /* Accept :xxxx, where the x's are hex digits, for a floating point
        with the exact digits specified.  */
    if (input_line_pointer[0] == ':') {
        ++input_line_pointer;
        // length = hex_float (float_type, temp);
        if (length < 0) {
            ignore_rest_of_line ();
            return length;
        }
    } else {
        const char *err;

        err = md_atof (float_type, temp, &length);
        know (length <= MAXIMUM_NUMBER_OF_CHARS_FOR_FLOAT);
        know (err != NULL || length > 0);
        if (err) {
            as_bad (_("bad floating literal: %s"), err);
            ignore_rest_of_line ();
            return -1;
        }
    }

    return length;
}
// 处理.text
void s_text (int ignore ATTRIBUTE_UNUSED) {
    int temp;

    temp = get_absolute_expression ();
    subseg_set (text_section, (subsegT) temp);
    demand_empty_rest_of_line ();
}

/**
 * @Description: 验证我们是否在一行的末尾。如果不是，则发出错误并跳到EOL
 */
void
demand_empty_rest_of_line (void) {
    SKIP_WHITESPACE ();
    if (input_line_pointer > buffer_limit)
        return;
    if (is_end_of_line[(unsigned char) *input_line_pointer])
        input_line_pointer++;
    else {
        if (ISPRINT (*input_line_pointer))
            as_bad (_("junk at end of line, first unrecognized character is `%c'"),
            *input_line_pointer);
        else
            as_bad (_("junk at end of line, first unrecognized character valued 0x%x"),
            *input_line_pointer);
        ignore_rest_of_line ();
    }
}

#define MAXIMUM_NUMBER_OF_CHARS_FOR_FLOAT (16)

static void
cons_worker (int nbytes,	/* 1=.byte, 2=.word, 4=.long.  */
	     int rva) {
    int c;
    expressionS exp;
    char *stop = NULL;
    char stopc = 0;
}
// 根据占据的字节分配
void
cons (int size) {
    cons_worker (size, 0);
}

/**
 * @Description: 读到.float时它对应的处理函数
 */
void
float_cons (/* Clobbers input_line-pointer, checks end-of-line.  */
	    int float_type	/* 'f':.ffloat ... 'F':.float ...  */) {
    // 遇到.float || ...
    char *p;
    char temp[MAXIMUM_NUMBER_OF_CHARS_FOR_FLOAT];
    int length;
    if (now_seg == absolute_section) {
        as_bad (_("attempt to store float in absolute section"));
        ignore_rest_of_line ();
        return;
    }
    if (in_bss ()) {
        as_bad (_("attempt to store float in section `%s'"),
            segment_name (now_seg));
        ignore_rest_of_line ();
        return;
    }
#ifdef md_cons_align
    md_cons_align (1);
#endif
    // .float 4.5
    do {
        // 处理浮点数
        length = parse_one_float(float_type,temp);
        if (length < 0) 
            return;
        // 创建flag
        if (!need_pass_2) {
            int count = 1;
            while(--count >= 0) {
                // 得到一块内存空间
                p = frag_more(length);
                memcpy (p, temp, (unsigned int) length);
            }
        }
        SKIP_WHITESPACE ();
        
    } while(*input_line_pointer++ == ',');// 处理下一个浮点数
     --input_line_pointer;
    demand_empty_rest_of_line();
}

/**
 * @Description: 字符串的下一个字符
 */
unsigned int
next_char_of_string (void)
{
    unsigned int c;

    c = *input_line_pointer++ & CHAR_MASK;
    switch (c) {
        case 0:
            /* PR 20902: Do not advance past the end of the buffer.  */
            -- input_line_pointer;
            c = NOT_A_CHAR;
            break;

        case '\"':
            c = NOT_A_CHAR;
            break;

        case '\n':
            as_warn (_("unterminated string; newline inserted"));
            bump_line_counters ();
            break;

        case '\\':
            if (!TC_STRING_ESCAPES)
                break;
            switch (c = *input_line_pointer++ & CHAR_MASK) {
                case 'b':
                    c = '\b';
                    break;

                case 'f':
                    c = '\f';
                    break;

                case 'n':
                    c = '\n';
                    break;

                case 'r':
                    c = '\r';
                    break;

                case 't':
                    c = '\t';
                    break;

                case 'v':
                    c = '\013';
                    break;

                case '\\':
                case '"':
                    break;		/* As itself.  */

                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                {
                    unsigned number;
                    int i;

                    for (i = 0, number = 0;
                    ISDIGIT (c) && i < 3;
                    c = *input_line_pointer++, i++) {
                        number = number * 8 + c - '0';
                    }

                    c = number & CHAR_MASK;
                }
                    --input_line_pointer;
                    break;

                case 'x':
                case 'X':
                {
                    unsigned number;

                    number = 0;
                    c = *input_line_pointer++;
                    while (ISXDIGIT (c)) {
                        if (ISDIGIT (c))
                            number = number * 16 + c - '0';
                        else if (ISUPPER (c))
                            number = number * 16 + c - 'A' + 10;
                        else
                            number = number * 16 + c - 'a' + 10;
                        c = *input_line_pointer++;
                    }
                    c = number & CHAR_MASK;
                    --input_line_pointer;
                }
                break;

                case '\n':
                    /* To be compatible with BSD 4.2 as: give the luser a linefeed!!  */
                    as_warn (_("unterminated string; newline inserted"));
                    c = '\n';
                    bump_line_counters ();
                    break;

                case 0:
                    /* Do not advance past the end of the buffer.  */
                    -- input_line_pointer;
                    c = NOT_A_CHAR;
                    break;

                default:
                    break;
            }
            break;

        default:
            break;
    }
    return (c);
}

/**
 * @Description: 从input_line_pointer中读取符号名
 */
char *
read_symbol_name (void)
{
    char * name;
    char * start;
    char c;

    c = *input_line_pointer++;

    if (c == '"') {
#define SYM_NAME_CHUNK_LEN 128
        ptrdiff_t len = SYM_NAME_CHUNK_LEN;
        char * name_end;
        unsigned int C;

        start = name = XNEWVEC (char, len + 1);

        name_end = name + SYM_NAME_CHUNK_LEN;

        while (is_a_char (C = next_char_of_string ())) {
            if (name >= name_end) {
                ptrdiff_t sofar;

                sofar = name - start;
                len += SYM_NAME_CHUNK_LEN;
                start = XRESIZEVEC (char, start, len + 1);
                name_end = start + len;
                name = start + sofar;
            }

            *name++ = (char) C;
        }
        *name = 0;

        /* Since quoted symbol names can contain non-ASCII characters,
        check the string and warn if it cannot be recognised by the
        current character set.  */
        /* PR 29447: mbstowcs ignores the third (length) parameter when
        the first (destination) parameter is NULL.  For clarity sake
        therefore we pass 0 rather than 'len' as the third parameter.  */
        if (mbstowcs (NULL, name, 0) == (size_t) -1)
            as_warn (_("symbol name not recognised in the current locale"));
    } else if (is_name_beginner (c) || (input_from_string && c == FAKE_LABEL_CHAR)) {
        ptrdiff_t len;

        name = input_line_pointer - 1;

        /* We accept FAKE_LABEL_CHAR in a name in case this is
        being called with a constructed string.  */
        while (is_part_of_name (c = *input_line_pointer++)
            || (input_from_string && c == FAKE_LABEL_CHAR))
        ;

        len = (input_line_pointer - name) - 1;
        start = XNEWVEC (char, len + 1);

        memcpy (start, name, len);
        start[len] = 0;

        /* Skip a name ender char if one is present.  */
        if (! is_name_ender (c))
            --input_line_pointer;
    } else
        name = start = NULL;

    if (name == start) {
        as_bad (_("expected symbol name"));
        ignore_rest_of_line ();
        free (start);
        return NULL;
    }

    SKIP_WHITESPACE ();
    return start;
}


/**
 * @Description: 读到.global  表示后面的符号声明为全局符号，以使得该标志符
 * 可以在链接过程中被其他模块访问和使用
 */
void
s_globl (int ignore ATTRIBUTE_UNUSED)
{
    char *name;
    int c;
    symbolS *symbolP;
    char *stop = NULL;
    char stopc = 0;

    // 为.globl 后面的符号创建SymbolP
    do {
        if ((name = read_symbol_name ()) == NULL)
	        return;
        // 创建.globl后面 _start的符号
        symbolP = symbol_find_or_make (name);
        // 标记为外部符号
        S_SET_EXTERNAL (symbolP);
        SKIP_WHITESPACE ();
        c = *input_line_pointer;
        if (c == ',') {
            input_line_pointer++;
            SKIP_WHITESPACE ();
            if (is_end_of_line[(unsigned char) *input_line_pointer])
                c = '\n';
        }
        free (name);
    } while (c == ',');
    // 此时input_line_pointer指向的内容也跳过.globl后面的符号
    demand_empty_rest_of_line ();
}

void
pop_insert (const pseudo_typeS *table)
{
  const pseudo_typeS *pop;
  for (pop = table; pop->poc_name; pop++) {
    if (str_hash_insert (po_hash, pop->poc_name, pop, 0) != NULL) {
      if (!pop_override_ok)
        as_fatal (_("error constructing %s pseudo-op table"),
            pop_table_name);
    }
  }
}
#ifndef md_pop_insert
#define md_pop_insert()		pop_insert(md_pseudo_table)
#endif

#ifdef md_frag_max_var
# define HANDLE_BUNDLE
#endif

#ifdef HANDLE_BUNDLE
static unsigned int bundle_align_p2;

// 是否需要忽略
#define HANDLE_CONDITIONAL_ASSEMBLY(num_read)				\
  if (ignore_input ())							\
    {									\
      char *eol = find_end_of_line (input_line_pointer - (num_read),	\
				    flag_m68k_mri);			\
      input_line_pointer = (input_line_pointer <= buffer_limit		\
			    && eol >= buffer_limit)			\
			   ? buffer_limit				\
			   : eol + 1;					\
      continue;								\
    }

static fragS *bundle_lock_frag;
static frchainS *bundle_lock_frchain;

/* This is incremented by .bundle_lock and decremented by .bundle_unlock,
   to allow nesting.  */
static unsigned int bundle_lock_depth;
#endif

int target_big_endian = TARGET_BYTES_BIG_ENDIAN;


// #ifndef cfi_pop_insert
// #define cfi_pop_insert()	pop_insert(cfi_pseudo_table)
// #endif
static offsetT
get_absolute_expr (expressionS *exp)
{
    expression_and_evaluate (exp);

    if (exp->X_op != O_constant)  {
        if (exp->X_op != O_absent)
	        as_bad (_("bad or irreducible absolute expression"));
        exp->X_add_number = 0;
    }
    return exp->X_add_number;
}


static const char *pop_table_name;
offsetT
get_absolute_expression(void) {
    expressionS exp;
    return get_absolute_expr (&exp);
}

/**
 * @Description: 扫描到.data
 */
void
s_data (int ignore ATTRIBUTE_UNUSED) {
    segT section;
    int temp;

    temp = get_absolute_expression ();
    // 是否将数据段设置为只读  并追加到text section
    if (flag_readonly_data_in_text) {
        section = text_section;
        temp += 1000;
    } else
        section = data_section;

    subseg_set (section, (subsegT) temp);

    demand_empty_rest_of_line ();
}

void
s_end (int ignore ATTRIBUTE_UNUSED) {
}

offsetT
parse_align (int align_bytes) {
    expressionS exp;
    addressT align;
    SKIP_WHITESPACE ();

    if (*input_line_pointer != ',') {
    no_align:
        as_bad (_("expected alignment after size"));
        ignore_rest_of_line ();
        return -1;
    }

    input_line_pointer++;
    SKIP_WHITESPACE ();

    align = get_absolute_expr (&exp);
    return align;
}

void
do_align (unsigned int n, char *fill, unsigned int len, unsigned int max) {
    if (now_seg == absolute_section || in_bss ()) {

    }
#ifdef md_do_align
    md_do_align (n, fill, len, max, just_record_alignment);
#endif

#ifdef md_do_align
    just_record_alignment: ATTRIBUTE_UNUSED_LABEL
#endif
    if (n > OCTETS_PER_BYTE_POWER)
        record_alignment (now_seg, n - OCTETS_PER_BYTE_POWER);
}

#ifndef TC_ALIGN_LIMIT
#define TC_ALIGN_LIMIT (stdoutput->arch_info->bits_per_address - 1)
#endif

static void
s_align (signed int arg, int bytes_p) {
    unsigned int align_limit = TC_ALIGN_LIMIT;
    addressT align;
    char *stop = NULL;
    char stopc = 0;
    offsetT fill = 0;
    unsigned int max;
    int fill_p;
    if (is_end_of_line[(unsigned char) *input_line_pointer]) {
        if (arg < 0)
            align = 0;
        else
            align = arg;	/* Default value from pseudo-op table.  */
    } else {
        align = get_absolute_expression ();
        SKIP_WHITESPACE ();
#ifdef TC_ALIGN_ZERO_IS_DEFAULT
        if (arg > 0 && align == 0)
            align = arg;
#endif
    }
    if (*input_line_pointer != ',') {
        fill_p = 0;
        max = 0;
    }
    if (!fill_p) {
        if (arg < 0)
            as_warn (_("expected fill pattern missing"));
        do_align (align, (char *) NULL, 0, max);
    }
    demand_empty_rest_of_line ();
}

void
s_align_ptwo (int arg) {
    s_align (arg, 0);
}

// 伪指令表初始化
static void
pobegin (void) {
    po_hash = str_htab_create ();
    
    // 执行特定于目标的伪操作
    pop_table_name = "md";
    pop_override_ok = 0;
    md_pop_insert ();

    // 初始化目标文件相关指令
    pop_table_name = "obj";
    pop_override_ok = 1;
    obj_pop_insert ();

    // 通用的伪指令处理
    pop_table_name = "standard";
    pop_insert (potable);

    // 符合 CFI 规范的伪指令处理
    // 用于调试 暂时不实现
    // pop_table_name = "cfi";
    // cfi_pop_insert ();
}

static char *
_find_end_of_line (char *s, int mri_string, int insn ATTRIBUTE_UNUSED,
		   int in_macro)
{
  char inquote = '\0';
  int inescape = 0;

  while (!is_end_of_line[(unsigned char) *s]
	 || (inquote && !ISCNTRL (*s))
	 || (inquote == '\'' && flag_mri)
#ifdef TC_EOL_IN_INSN
	 || (insn && TC_EOL_IN_INSN (s))
#endif
	 /* PR 6926:  When we are parsing the body of a macro the sequence
	    \@ is special - it refers to the invocation count.  If the @
	    character happens to be registered as a line-separator character
	    by the target, then the is_end_of_line[] test above will have
	    returned true, but we need to ignore the line separating
	    semantics in this particular case.  */
	 || (in_macro && inescape && *s == '@')
	)
    {
      if (mri_string && *s == '\'')
	inquote ^= *s;
      else if (inescape)
	inescape = 0;
      else if (*s == '\\')
	inescape = 1;
      else if (!inquote
	       ? *s == '"'
#ifdef TC_SINGLE_QUOTE_STRINGS
		 || (TC_SINGLE_QUOTE_STRINGS && *s == '\'')
#endif
	       : *s == inquote)
	inquote ^= *s;
      ++s;
    }
  if (inquote)
    as_warn (_("missing closing `%c'"), inquote);
  if (inescape && !ignore_input ())
    as_warn (_("stray `\\'"));
  return s;
}

char *
find_end_of_line (char *s, int mri_string) {
    return _find_end_of_line (s, mri_string, 0, 0);
}

/**
 * @Description:  In:	Input_line_pointer->next character.
 * Do:	Skip input_line_pointer over all whitespace.
 * Out:	1 if input_line_pointer->end-of-line.
 */
int
is_it_end_of_statement (void) {
    SKIP_WHITESPACE ();
    return (is_end_of_line[(unsigned char) *input_line_pointer]);
}


symbolS *
s_comm_internal (int param,
		 symbolS *(*comm_parse_extra) (int, symbolS *, addressT)) {
    char *name;
    offsetT temp, size;
    symbolS *symbolP = NULL;
    char *stop = NULL;
    char stopc = 0;
    expressionS exp;      

    if ((name = read_symbol_name ()) == NULL)
        goto out;  
    if (*input_line_pointer == ',')
        input_line_pointer++;
    temp = get_absolute_expr (&exp);
    size = temp;
    size &= ((addressT) 2 << (stdoutput->arch_info->bits_per_address - 1)) - 1;

    symbolP = symbol_find_or_make (name);
    size = S_GET_VALUE (symbolP);

    if (size == 0)
        size = temp;
    else if (size != temp)
        as_warn (_("size of \"%s\" is already %ld; not changing to %ld"),
            name, (long) size, (long) temp);
    if (comm_parse_extra != NULL)
        symbolP = (*comm_parse_extra) (param, symbolP, size);
    else {
        S_SET_VALUE (symbolP, (valueT) size);
        S_SET_EXTERNAL (symbolP);
        S_SET_SEGMENT (symbolP, bfd_com_section_ptr);
    }
    demand_empty_rest_of_line ();
out:
    free (name);
    return symbolP;
}


char arm_line_separator_chars[] = ";";
void
read_begin (void) {
    const char *p;
    pobegin();
    obstack_begin (&cond_obstack, chunksize);
    /* Use machine dependent syntax.  */
    for (p = tc_line_separator_chars; *p; p++)
        is_end_of_line[(unsigned char) *p] = 2;
    abs_section_offset = 0;
    current_name = NULL;
    current_label = NULL;
}

/**
 * @Description: 处理一条汇编语言
 */
static void
assemble_one (char *line) {
    // 初始化代码片段
    fragS *insn_start_frag = NULL;
    md_assemble (line);
}

void
read_a_source_file (const char *name) {
    char nul_char;
    char next_char;
    char *s;		/* String of symbol, '\0' appended.  */
    long temp;
    const pseudo_typeS *pop;
    
    buffer = input_scrub_new_file (name);
    listing_file (name);
    listing_newline (NULL);
    // 放进文件依赖链中
    register_dependency (name);
    // int flag = 0;
    int i = 5;
    buffer_limit = input_scrub_next_buffer (&input_line_pointer);
    
    while (input_line_pointer < buffer_limit) {      
        bool was_new_line;
        was_new_line = is_end_of_line[(unsigned char) input_line_pointer[-1]];
        if (was_new_line) {
            symbol_set_value_now (&dot_symbol);
        #ifdef md_start_line_hook
            md_start_line_hook ();
        #endif
            if (input_line_pointer[-1] == '\n') {
                // 处理行数
                bump_line_counters ();
            }
            line_label = NULL;
        }
        // 遇到空格 . 
        do
            nul_char = next_char = *input_line_pointer++;
        while (next_char == '\t' || next_char == ' ' || next_char == '\f');
        // 遇到一个有效字符 .data my_float .float符号
        if (is_name_beginner (next_char) || next_char == '"') {
            char *rest;
            HANDLE_CONDITIONAL_ASSEMBLY(1);
            // 把.等多拿的符号还回去
            --input_line_pointer;
            // s得到一个符号  s的内存空间就是input_line_pointer 名字的分界线
            // s -> .data nul_char -> ' '
            // s -> my_float nul_char -> ':'
            // s -> .float nul_char -> ' '
            // s -> .globl nul_char -> ' '
            // s -> _start nul_char -> ':'
            // s -> mov nul_char -> ' '
            nul_char = get_symbol_name (& s);	/* name's delimiter.  */
            // 下一个字符
            next_char = (nul_char == '"' ? input_line_pointer[1] : nul_char);
            // 除去该符号剩下的内容
            rest = input_line_pointer + (nul_char == '"' ? 2 : 1);
            // printf("%s\n",s);
            // 符号后的字符。即标号，例如:myfloat:
            if (TC_START_LABEL (s, nul_char, next_char)) {
                // s -> 指向的空间恢复成还有my_float:的样子 s不和input_line_ponter共享空间了
                // _start
                line_label = colon(s);
                // 但是input_line_pointer
                restore_line_pointer (nul_char);
                // 除去:符号
                ++input_line_pointer;
                SKIP_WHITESPACE ();
            } else {
                // 处理普通字符 即伪指令和指令
                pop = NULL;
                char *s2 = s;
                // 将字符放在数组original_case_string中
                strncpy (original_case_string, s2,
                    sizeof (original_case_string) - 1);
                original_case_string[sizeof (original_case_string) - 1] = 0;

                while (*s2) {
                    *s2 = TOLOWER (*s2);
                    s2++;
                }
                if (pop != NULL
                    || (!flag_m68k_mri && *s == '.')) {
                    // 伪指令 .data .float
                    if (pop == NULL) {
                        // 在符号表寻找段   .data 找到data
                        pop = str_hash_find(po_hash, s + 1);                
                    } else if (pop && !pop->poc_handler)
                        pop = NULL;
                    
                    /* 让input_line_pointer得到正确的值 除去该符号的剩下的内容*/
                    next_char = restore_line_pointer (nul_char);
                    
                    if (next_char == ' ' || next_char == '\t')
                        // 去除空格
                        input_line_pointer++;
                    // 节处理函数 已经定义过
                    (*pop->poc_handler) (pop->poc_val);
                    // 到达.end结尾
                    if (pop->poc_handler == s_end)
                        goto quit;
                } else {
                    // 处理指令  mov
                    // 让s恢复成还有mov指令的状态
                    (void) restore_line_pointer (nul_char);
                    input_line_pointer = _find_end_of_line (input_line_pointer, flag_m68k_mri, 1, 0);
                    next_char = nul_char = *input_line_pointer;
                    
                    *input_line_pointer = '\0';
                    assemble_one (s); 
                    
                    if (input_line_pointer == NULL)
                        as_fatal (_("unable to continue with assembly."));
                    *input_line_pointer++ = nul_char;
                }             
            } 
            continue;              
        }

        /* Empty statement?  */
        if (is_end_of_line[(unsigned char) next_char])
            continue;      
    }
        
    quit:
    symbol_set_value_now (&dot_symbol);
#ifdef md_cleanup
  md_cleanup ();
#endif
    // input_line_pointer = NULL;
    // 关闭文件
    input_scrub_close ();
}

static void
convert_to_bignum (expressionS *exp, int sign) {
    valueT value;
    unsigned int i;

    value = exp->X_add_number;
    for (i = 0; i < sizeof (exp->X_add_number) / CHARS_PER_LITTLENUM; i++) {
        generic_bignum[i] = value & LITTLENUM_MASK;
        value >>= LITTLENUM_NUMBER_OF_BITS;
    }
    /* Add a sequence of sign bits if the top bit of X_add_number is not
        the sign of the original value.  */
    if ((exp->X_add_number < 0) == !sign)
        generic_bignum[i++] = sign ? LITTLENUM_MASK : 0;
    exp->X_op = O_big;
    exp->X_add_number = i;
}

void
emit_expr_with_reloc (expressionS *exp,
		      unsigned int nbytes,
		      TC_PARSE_CONS_RETURN_TYPE reloc) {
    operatorT op;
    char *p;
    valueT extra_digit = 0;

    /* Don't do anything if we are going to make another pass.  */
    if (need_pass_2)
        return;
    frag_grow (nbytes);
    dot_value = frag_now_fix ();
    dot_frag = frag_now;

    if (check_eh_frame (exp, &nbytes))
        return;

    op = exp->X_op;

    p = frag_more ((int) nbytes);
    /* If we have an integer, but the number of bytes is too large to
     pass to md_number_to_chars, handle it as a bignum.  */
    if (op == O_constant && nbytes > sizeof (valueT)) {
        extra_digit = exp->X_unsigned ? 0 : -1;
        convert_to_bignum (exp, !exp->X_unsigned);
        op = O_big;
    }
    if (op == O_constant) {
        valueT get;
        valueT use;
        valueT mask;
        valueT unmask;

        if (nbytes >= sizeof (valueT)) {
            know (nbytes == sizeof (valueT));
            mask = 0;
        } else {
            /* Don't store these bits.  */
            mask = ~(valueT) 0 << (BITS_PER_CHAR * nbytes);
        }

        unmask = ~mask;		/* Do store these bits.  */
        get = exp->X_add_number;
        use = get & unmask;

        if ((get & mask) != 0 && (-get & mask) != 0) {
            /* Leading bits contain both 0s & 1s.  */
            as_warn (_("value 0x%" PRIx64 " truncated to 0x%" PRIx64),
                (uint64_t) get, (uint64_t) use);
        }
        /* Put bytes in right order.  */
        md_number_to_chars (p, use, (int) nbytes);
    } else {
        emit_expr_fix (exp, nbytes, frag_now, p, TC_PARSE_CONS_RETURN_NONE);
    }
}

void
emit_expr_fix (expressionS *exp, unsigned int nbytes, fragS *frag, char *p,
	       TC_PARSE_CONS_RETURN_TYPE r ATTRIBUTE_UNUSED) {
    int offset = 0;
    unsigned int size = nbytes;

    memset (p, 0, size);  

    TC_CONS_FIX_NEW (frag, p - frag->fr_literal + offset, size, exp, r);         
}
/**
 * @Description: 将表达式EXP的内容放入object文件中NBYTES字节
 */
void
emit_expr (expressionS *exp, unsigned int nbytes) {
    emit_expr_with_reloc (exp, nbytes, TC_PARSE_CONS_RETURN_NONE);
}

static void
poend (void) {
    htab_delete (po_hash);
}
void
read_end (void) {
    // stabs_end ();
    poend ();
    _obstack_free (&cond_obstack, NULL);
    free (current_name);
    free (current_label);
}