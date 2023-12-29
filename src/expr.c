/*
 * @Author: hxy
 * @Date: 2023-10-02 17:47:38
 * @LastEditTime: 2023-11-16 11:36:30
 * @Description: 请填写简介
 */

#include "as.h"
#include "safe-ctype.h"
#include <limits.h>
#ifndef CHAR_BIT
#define CHAR_BIT 8
#endif
#define STANDARD_MUL_PRECEDENCE 8
static operator_rankT op_rank[O_max] = {
  0,	/* O_illegal */
  0,	/* O_absent */
  0,	/* O_constant */
  0,	/* O_symbol */
  0,	/* O_symbol_rva */
  0,	/* O_secidx */
  0,	/* O_register */
  0,	/* O_big */
  9,	/* O_uminus */
  9,	/* O_bit_not */
  9,	/* O_logical_not */
  8,	/* O_multiply */
  8,	/* O_divide */
  8,	/* O_modulus */
  8,	/* O_left_shift */
  8,	/* O_right_shift */
  7,	/* O_bit_inclusive_or */
  7,	/* O_bit_or_not */
  7,	/* O_bit_exclusive_or */
  7,	/* O_bit_and */
  5,	/* O_add */
  5,	/* O_subtract */
  4,	/* O_eq */
  4,	/* O_ne */
  4,	/* O_lt */
  4,	/* O_le */
  4,	/* O_ge */
  4,	/* O_gt */
  3,	/* O_logical_and */
  2,	/* O_logical_or */
  1,	/* O_index */
};
static symbolS **seen[2];
static unsigned int nr_seen[2];
struct expr_symbol_line {
	struct expr_symbol_line *next;
	symbolS *sym;
	const char *file;
	unsigned int line;
};
static struct expr_symbol_line *expr_symbol_lines;
/**
 * @Description: 设置表达式解析器中操作符的优先级
 */
void
expr_set_precedence (void) {
    op_rank[O_multiply] = STANDARD_MUL_PRECEDENCE;
    op_rank[O_divide] = STANDARD_MUL_PRECEDENCE;
    op_rank[O_modulus] = STANDARD_MUL_PRECEDENCE;
}

LITTLENUM_TYPE generic_bignum[SIZE_OF_LARGE_NUMBER + 6];
FLONUM_TYPE generic_floating_point_number = {
  &generic_bignum[6],		/* low.  (JF: Was 0)  */
  &generic_bignum[SIZE_OF_LARGE_NUMBER + 6 - 1], /* high.  JF: (added +6)  */
  0,				/* leader.  */
  0,				/* exponent.  */
  0				/* sign.  */
};
/**
 * @Description: 初始化表达式解析器
 */
void
expr_begin (void)
{
    expr_set_precedence ();

    /* Verify that X_op field is wide enough.  */
    {
        expressionS e;
        e.X_op = O_max;
        gas_assert (e.X_op == O_max);
    }

    memset (seen, 0, sizeof seen);
    memset (nr_seen, 0, sizeof nr_seen);
      /*为表达式节点中的符号和其所在源文件位置之间建立一个映射关系，
      以方便提供更好的错误信息*/
    expr_symbol_lines = NULL;
}

static void
clean_up_expression (expressionS *expressionP)
{
    switch (expressionP->X_op) {
        case O_illegal:
        case O_absent:
            expressionP->X_add_number = 0;
        /* Fall through.  */
        case O_big:
        case O_constant:
        case O_register:
            expressionP->X_add_symbol = NULL;
        /* Fall through.  */
        case O_symbol:
        case O_uminus:
        case O_bit_not:
            expressionP->X_op_symbol = NULL;
            break;
        default:
            break;
    }
}

/**
 * @Description: 处理汇编代码中含有立即数
 */
static void
integer_constant (int radix, expressionS *expressionP) {
    char *start;		/* Start of number.  */
    char *suffix = NULL;
    char c;
    valueT number;	/* Offset or (absolute) value.  */
    short int digit;	/* Value of next digit in current radix.  */
    short int maxdig = 0;	/* Highest permitted digit value.  */
    int too_many_digits = 0;	/* If we see >= this number of.  */
    char *name;		/* Points to name of symbol.  */
    symbolS *symbolP;	/* Points to symbol.  */

    int small;			/* True if fits in 32 bits.  */
#ifdef BFD64
#define valuesize 64
#else /* includes non-bfd case, mostly */
#define valuesize 32
#endif
    if (is_end_of_line[(unsigned char) *input_line_pointer]) {
        expressionP->X_op = O_absent;
        return;
    }
    switch (radix) {
        case 2:
            maxdig = 2;
            too_many_digits = valuesize + 1;
            break;
        case 8:
            maxdig = radix = 8;
            too_many_digits = (valuesize + 2) / 3 + 1;
            break;
        case 16:
            maxdig = radix = 16;
            too_many_digits = (valuesize + 3) / 4 + 1;
            break;
        case 10:
            maxdig = radix = 10;
            too_many_digits = (valuesize + 11) / 4; /* Very rough.  */
    }
#undef valuesize
    start = input_line_pointer;
    c = *input_line_pointer++;
    for (number = 0;
       (digit = hex_value (c)) < maxdig;
       c = *input_line_pointer++) {
        number = number * radix + digit;
    }
    small = (input_line_pointer - start - 1) < too_many_digits;
    if (small) {
        if (LOCAL_LABELS_FB && c == 'b') {

        } else {
            expressionP->X_op = O_constant;
            expressionP->X_add_number = number;
            input_line_pointer--;
        }
    } else {
        /* Not a small number.  */
        expressionP->X_op = O_big;
        expressionP->X_add_number = number;	/* Number of littlenums.  */
        input_line_pointer--;	/* -> char following number.  */
    }
}

static void
floating_constant (expressionS *expressionP) {
    /* input_line_pointer -> floating-point constant.  */
    int error_code;

    error_code = atof_generic (&input_line_pointer, ".", EXP_CHARS,
                    &generic_floating_point_number);

    if (error_code) {
        if (error_code == ERROR_EXPONENT_OVERFLOW) {
            as_bad (_("bad floating-point constant: exponent overflow"));
        }
        else {
            as_bad (_("bad floating-point constant: unknown error code=%d"),
                error_code);
        }
    }
    expressionP->X_op = O_big;
    /* input_line_pointer -> just after constant, which may point to
        whitespace.  */
    expressionP->X_add_number = -1;
}

void
current_location (expressionS *expressionp) {
    if (now_seg == absolute_section) {
        expressionp->X_op = O_constant;
        expressionp->X_add_number = abs_section_offset;
    }
  else {
        expressionp->X_op = O_symbol;
        expressionp->X_add_symbol = &dot_symbol;
        expressionp->X_add_number = 0;
    }
}
/**
 * @Description: 返回当前所处的段
 */
static segT
operand (expressionS *expressionP, enum expr_mode mode) {
    char c;
    symbolS *symbolP;	/* Points to symbol.  */
    char *name;		/* Points to name of symbol.  */
    segT segment;
    operatorT op = O_absent; /* For unary operators.  */

    expressionP->X_unsigned = 1;
    expressionP->X_extrabit = 0;

    /* Digits, assume it is a bignum.  */

    SKIP_WHITESPACE ();		/* Leading whitespace is part of operand.  */
    c = *input_line_pointer++;
    if (is_end_of_line[(unsigned char) c])
        goto eol;
    if (c == '\n') {
        printf("换行\n");
    }
    // #3
    switch (c) {
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            input_line_pointer--;

            integer_constant ((NUMBERS_WITH_SUFFIX || flag_m68k_mri)
                    ? 0 : 10,
                    expressionP);
            break;
        case '0':
            c = *input_line_pointer;
            switch (c) {
                default:
                default_case: 
                    if (c && strchr (FLT_CHARS, c)) {
                        input_line_pointer++;
                        floating_constant (expressionP);
                        expressionP->X_add_number = - TOLOWER (c);
                    } else {
                        expressionP->X_op = O_constant;
                        expressionP->X_add_number = 0;
                    }
                    break;
            }
            break;
        case '-':
            op = O_uminus;
        case '+':
            {
            unary: 
                operand (expressionP, mode);
            }
            if (expressionP->X_op == O_constant) {
                if (op == O_uminus) {
                    expressionP->X_add_number
                        = - (addressT) expressionP->X_add_number;
                    expressionP->X_unsigned = 0;
                    if (expressionP->X_add_number)
                        expressionP->X_extrabit ^= 1;
                }
            }
            
            break;
        case '.':
            // .L3  .size
            if (!is_part_of_name (*input_line_pointer)) {
                current_location (expressionP);
                break;
            } else {
                goto isname;
            }
        case ',':
        eol:
            /* Can't imagine any other kind of operand.  */
            expressionP->X_op = O_absent;
            input_line_pointer--;
            break;    
        default:
            if (is_name_beginner (c) || c == '"') {
        isname:
                -- input_line_pointer;
                c = get_symbol_name (&name);
                symbolP = symbol_find_or_make (name);
                segment = S_GET_SEGMENT (symbolP);
                if (mode != expr_defer
                    && segment == absolute_section
                    && !S_FORCE_RELOC (symbolP, 0)) {
                    expressionP->X_op = O_constant;
                    expressionP->X_add_number = S_GET_VALUE (symbolP);
                }
                else if (mode != expr_defer && segment == reg_section) {
                    expressionP->X_op = O_register;
                    expressionP->X_add_number = S_GET_VALUE (symbolP);
                } else {
                    expressionP->X_op = O_symbol;
                    expressionP->X_add_symbol = symbolP;
                    expressionP->X_add_number = 0;
                }

                restore_line_pointer (c);
            }  
            break;
    }
    
    /* It is more 'efficient' to clean up the expressionS when they are
     created.  Doing it here saves lines of code.  */
    clean_up_expression (expressionP);
    SKIP_ALL_WHITESPACE ();		/* -> 1st char after operand.  */
    know (*input_line_pointer != ' ');

    /* The PA port needs this information.  */
    if (expressionP->X_add_symbol)
        symbol_mark_used (expressionP->X_add_symbol);
        
    if (mode != expr_defer) {
        expressionP->X_add_symbol
            = symbol_clone_if_forward_ref (expressionP->X_add_symbol);
        expressionP->X_op_symbol
            = symbol_clone_if_forward_ref (expressionP->X_op_symbol);
    }
    switch (expressionP->X_op) {
        default:
            return absolute_section;
        case O_symbol:
            return S_GET_SEGMENT (expressionP->X_add_symbol);
        case O_register:
            return reg_section;
    }
}

#undef __
#define __ O_illegal
#ifndef O_SINGLE_EQ
#define O_SINGLE_EQ O_illegal
#endif

/* Maps ASCII -> operators.  */
static const operatorT op_encoding[256] = {
  __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,

  __, O_bit_or_not, __, __, __, O_modulus, O_bit_and, __,
  __, __, O_multiply, O_add, __, O_subtract, __, O_divide,
  __, __, __, __, __, __, __, __,
  __, __, __, __, O_lt, O_SINGLE_EQ, O_gt, __,
  __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __,
  __, __, __,
#ifdef NEED_INDEX_OPERATOR
  O_index,
#else
  __,
#endif
  __, __, O_bit_exclusive_or, __,
  __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __,
  __, __, __, __, O_bit_inclusive_or, __, __, __,

  __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __
};


/**
 * @Description: 获取运算符的编码
 */
static inline operatorT
operatorf (int *num_chars) {
    int c;
    operatorT ret;

    c = *input_line_pointer & 0xff;
    *num_chars = 1;

    if (is_end_of_line[c])
        return O_illegal;
    switch (c) {
        default:
            ret = op_encoding[c];
            return ret;

        case '+':
        case '-':
            return op_encoding[c];
    }
}

/**
 * @Description: O_substract
 */
void
subtract_from_result (expressionS *resultP, offsetT amount, int rhs_highbit) {
    valueT ures = resultP->X_add_number;
    valueT uamount = amount;

    resultP->X_add_number -= uamount;

    resultP->X_extrabit ^= rhs_highbit;

    if (ures < uamount)
        resultP->X_extrabit ^= 1;
}

void
add_to_result (expressionS *resultP, offsetT amount, int rhs_highbit) {
    valueT ures = resultP->X_add_number;
    valueT uamount = amount;

    resultP->X_add_number += uamount;

    resultP->X_extrabit ^= rhs_highbit;

    if (ures + uamount < ures)
        resultP->X_extrabit ^= 1;
}

/**
 * @Description: 解析一个表达式
 */
segT
expr (int rankarg,		/* Larger # is higher rank.  */
	expressionS *resultP,	/* Deliver result here.  */
	enum expr_mode mode	/* Controls behavior.  */) {
    operator_rankT rank = (operator_rankT) rankarg;
    segT retval;
    expressionS right;
    operatorT op_left;
    operatorT op_right;
    int op_chars;

    know (rankarg >= 0);

    /* Save the value of dot for the fixup code.  */
    if (rank == 0) {
        // 保存当前片段的偏移
        dot_value = frag_now_fix ();
        // 保存当前frag
        dot_frag = frag_now;
    }

    // 返回当前的seg  处理resultP的信息
    retval = operand (resultP, mode);

    /* operand () gobbles spaces.  */
    know (*input_line_pointer != ' ');

    // 获取输入行中运算符的编码
    op_left = operatorf (&op_chars);
    while (op_left != O_illegal && op_rank[(int) op_left] > rank) {
        segT rightseg;
        bool is_unsigned;
        offsetT frag_off;

        input_line_pointer += op_chars;	/* -> after operator.  */

        right.X_md = 0;
        rightseg = expr (op_rank[(int) op_left], &right, mode);
        if (right.X_op == O_absent) {
            as_warn (_("missing operand; zero assumed"));
            right.X_op = O_constant;
            right.X_add_number = 0;
            right.X_add_symbol = NULL;
            right.X_op_symbol = NULL;
        }
        know (*input_line_pointer != ' ');
        op_right = operatorf (&op_chars);

        know (op_right == O_illegal || op_left == O_index
            || op_rank[(int) op_right] <= op_rank[(int) op_left]);
        know ((int) op_left >= (int) O_multiply);
#ifndef md_operator
        know ((int) op_left <= (int) O_index);
#else
      know ((int) op_left < (int) O_max);
#endif
        is_unsigned = resultP->X_unsigned && right.X_unsigned;
        if (op_left == O_subtract
	       && right.X_op == O_symbol
	       && resultP->X_op == O_symbol
	       && retval == rightseg
#ifdef md_allow_local_subtract
	       && md_allow_local_subtract (resultP, & right, rightseg)
#endif
	       && ((SEG_NORMAL (rightseg)
		    && !S_FORCE_RELOC (resultP->X_add_symbol, 0)
		    && !S_FORCE_RELOC (right.X_add_symbol, 0))
		   || right.X_add_symbol == resultP->X_add_symbol)
	       && frag_offset_fixed_p (symbol_get_frag (resultP->X_add_symbol),
				       symbol_get_frag (right.X_add_symbol),
				       &frag_off)) {
            offsetT symval_diff = S_GET_VALUE (resultP->X_add_symbol)
                        - S_GET_VALUE (right.X_add_symbol);
            subtract_from_result (resultP, right.X_add_number, right.X_extrabit);
            subtract_from_result (resultP, frag_off / OCTETS_PER_BYTE, 0);
            add_to_result (resultP, symval_diff, symval_diff < 0);
            resultP->X_op = O_constant;
            resultP->X_add_symbol = 0;
            is_unsigned = false;
        }
        if (resultP->X_op == O_symbol
	       && right.X_op == O_symbol
	       && (op_left == O_add
		   || op_left == O_subtract
		   || (resultP->X_add_number == 0
		       && right.X_add_number == 0))) {
            /* Symbol OP symbol.  */
            resultP->X_op = op_left;
            resultP->X_op_symbol = right.X_add_symbol;
            if (op_left == O_subtract) {
                subtract_from_result (resultP, right.X_add_number,
                right.X_extrabit);
                if (retval == rightseg
                    && SEG_NORMAL (retval)
                    && !S_FORCE_RELOC (resultP->X_add_symbol, 0)
                    && !S_FORCE_RELOC (right.X_add_symbol, 0)) {
                    retval = absolute_section;
                    rightseg = absolute_section;
                }
            }
        }
        resultP->X_unsigned = is_unsigned;
        if (retval != rightseg) {
            if (retval == undefined_section)
                ;
            else if (rightseg == undefined_section)
                retval = rightseg;
            else if (retval == expr_section)
                ;
            else if (rightseg == expr_section)
                retval = rightseg;
            else if (retval == reg_section)
                ;
            else if (rightseg == reg_section)
                retval = rightseg;
            else if (rightseg == absolute_section)
                ;
            else if (retval == absolute_section)
                retval = rightseg;
#ifdef DIFF_EXPR_OK
            else if (op_left == O_subtract)
                ;
#endif
            else
                as_bad (_("operation combines symbols in different segments"));
        }

        op_left = op_right;
    }
    if (resultP->X_add_symbol)
        // 标记已使用该符号 main  func
        symbol_mark_used (resultP->X_add_symbol);

    // 遇到.data
    if (rank == 0 && mode == expr_evaluate)
        resolve_expression (resultP);
    return resultP->X_op == O_constant ? absolute_section : retval;
}

// 判断是否是一个symbol
char
get_symbol_name (char ** ilp_return)
{
    char c;

    * ilp_return = input_line_pointer;
    /* We accept FAKE_LABEL_CHAR in a name in case this is being called with a
        constructed string.  */
    if (is_name_beginner (c = *input_line_pointer++)
        || (input_from_string && c == FAKE_LABEL_CHAR))
        {
        while (is_part_of_name (c = *input_line_pointer++)
            || (input_from_string && c == FAKE_LABEL_CHAR))
        ;
        if (is_name_ender (c))
        c = *input_line_pointer++;
        }
    else if (c == '"')
        {
        char *dst = input_line_pointer;

        * ilp_return = input_line_pointer;
        for (;;)
        {
        c = *input_line_pointer++;

        if (c == 0)
            {
            as_warn (_("missing closing '\"'"));
            break;
            }

        if (c == '"')
            {
            char *ilp_save = input_line_pointer;

            SKIP_WHITESPACE ();
            if (*input_line_pointer == '"')
            {
            ++input_line_pointer;
            continue;
            }
            input_line_pointer = ilp_save;
            break;
            }

        if (c == '\\')
            switch (*input_line_pointer)
            {
            case '"':
            case '\\':
            c = *input_line_pointer++;
            break;

            default:
            if (c != 0)
            as_warn (_("'\\%c' in quoted symbol name; "
                    "behavior may change in the future"),
                *input_line_pointer);
            break;
            }

        *dst++ = c;
        }
        *dst = 0;
        }
    *--input_line_pointer = 0;
    return c;
}


/**
 * @Description: 替代NULL字符 如果是""""就跳过
 */
char
restore_line_pointer (char c)
{
  * input_line_pointer = c;
  if (c == '"')
    c = * ++ input_line_pointer;
  return c;
}

/**
 * @Description: 解析表达式
 */
int
resolve_expression (expressionS *expressionP)
{
  /* Help out with CSE.  */
  valueT final_val = expressionP->X_add_number;
  symbolS *add_symbol = expressionP->X_add_symbol;
  symbolS *orig_add_symbol = add_symbol;
  symbolS *op_symbol = expressionP->X_op_symbol;
  operatorT op = expressionP->X_op;
  valueT left, right;
  segT seg_left, seg_right;
  fragS *frag_left, *frag_right;
  offsetT frag_off;

  switch (op)
    {
    default:
      return 0;

    case O_constant:
    case O_register:
      left = 0;
      break;

    case O_symbol:
    case O_symbol_rva:
      if (!snapshot_symbol (&add_symbol, &left, &seg_left, &frag_left))
	return 0;

      break;

    case O_uminus:
    case O_bit_not:
    case O_logical_not:
      if (!snapshot_symbol (&add_symbol, &left, &seg_left, &frag_left))
	return 0;

      if (seg_left != absolute_section)
	return 0;

      if (op == O_logical_not)
	left = !left;
      else if (op == O_uminus)
	left = -left;
      else
	left = ~left;
      op = O_constant;
      break;

    case O_multiply:
    case O_divide:
    case O_modulus:
    case O_left_shift:
    case O_right_shift:
    case O_bit_inclusive_or:
    case O_bit_or_not:
    case O_bit_exclusive_or:
    case O_bit_and:
    case O_add:
    case O_subtract:
    case O_eq:
    case O_ne:
    case O_lt:
    case O_le:
    case O_ge:
    case O_gt:
    case O_logical_and:
    case O_logical_or:
      if (!snapshot_symbol (&add_symbol, &left, &seg_left, &frag_left)
	  || !snapshot_symbol (&op_symbol, &right, &seg_right, &frag_right))
	return 0;

      /* Simplify addition or subtraction of a constant by folding the
	 constant into X_add_number.  */
      if (op == O_add)
	{
	  if (seg_right == absolute_section)
	    {
	      final_val += right;
	      op = O_symbol;
	      break;
	    }
	  else if (seg_left == absolute_section)
	    {
	      final_val += left;
	      left = right;
	      seg_left = seg_right;
	      add_symbol = op_symbol;
	      orig_add_symbol = expressionP->X_op_symbol;
	      op = O_symbol;
	      break;
	    }
	}
      else if (op == O_subtract)
	{
	  if (seg_right == absolute_section)
	    {
	      final_val -= right;
	      op = O_symbol;
	      break;
	    }
	}

      /* Equality and non-equality tests are permitted on anything.
	 Subtraction, and other comparison operators are permitted if
	 both operands are in the same section.
	 Shifts by constant zero are permitted on anything.
	 Multiplies, bit-ors, and bit-ands with constant zero are
	 permitted on anything.
	 Multiplies and divides by constant one are permitted on
	 anything.
	 Binary operations with both operands being the same register
	 or undefined symbol are permitted if the result doesn't depend
	 on the input value.
	 Otherwise, both operands must be absolute.  We already handled
	 the case of addition or subtraction of a constant above.  */
      frag_off = 0;
      if (!(seg_left == absolute_section
	       && seg_right == absolute_section)
	  && !(op == O_eq || op == O_ne)
	  && !((op == O_subtract
		|| op == O_lt || op == O_le || op == O_ge || op == O_gt)
	       && seg_left == seg_right
	       && (finalize_syms
		   || frag_offset_fixed_p (frag_left, frag_right, &frag_off)
		   || (op == O_gt
		       && frag_gtoffset_p (left, frag_left,
					   right, frag_right, &frag_off)))
	       && (seg_left != reg_section || left == right)
	       && (seg_left != undefined_section || add_symbol == op_symbol)))
	{
	  if ((seg_left == absolute_section && left == 0)
	      || (seg_right == absolute_section && right == 0))
	    {
	      if (op == O_bit_exclusive_or || op == O_bit_inclusive_or)
		{
		  if (!(seg_right == absolute_section && right == 0))
		    {
		      seg_left = seg_right;
		      left = right;
		      add_symbol = op_symbol;
		      orig_add_symbol = expressionP->X_op_symbol;
		    }
		  op = O_symbol;
		  break;
		}
	      else if (op == O_left_shift || op == O_right_shift)
		{
		  if (!(seg_left == absolute_section && left == 0))
		    {
		      op = O_symbol;
		      break;
		    }
		}
	      else if (op != O_multiply
		       && op != O_bit_or_not && op != O_bit_and)
	        return 0;
	    }
	  else if (op == O_multiply
		   && seg_left == absolute_section && left == 1)
	    {
	      seg_left = seg_right;
	      left = right;
	      add_symbol = op_symbol;
	      orig_add_symbol = expressionP->X_op_symbol;
	      op = O_symbol;
	      break;
	    }
	  else if ((op == O_multiply || op == O_divide)
		   && seg_right == absolute_section && right == 1)
	    {
	      op = O_symbol;
	      break;
	    }
	  else if (!(left == right
		     && ((seg_left == reg_section && seg_right == reg_section)
			 || (seg_left == undefined_section
			     && seg_right == undefined_section
			     && add_symbol == op_symbol))))
	    return 0;
	  else if (op == O_bit_and || op == O_bit_inclusive_or)
	    {
	      op = O_symbol;
	      break;
	    }
	  else if (op != O_bit_exclusive_or && op != O_bit_or_not)
	    return 0;
	}

      right += frag_off / OCTETS_PER_BYTE;
      switch (op)
	{
	case O_add:			left += right; break;
	case O_subtract:		left -= right; break;
	case O_multiply:		left *= right; break;
	case O_divide:
	  if (right == 0)
	    return 0;
	  left = (offsetT) left / (offsetT) right;
	  break;
	case O_modulus:
	  if (right == 0)
	    return 0;
	  left = (offsetT) left % (offsetT) right;
	  break;
	case O_left_shift:
	  if (right >= sizeof (left) * CHAR_BIT)
	    left = 0;
	  else
	    left <<= right;
	  break;
	case O_right_shift:
	  if (right >= sizeof (left) * CHAR_BIT)
	    left = 0;
	  else
	    left >>= right;
	  break;
	case O_bit_inclusive_or:	left |= right; break;
	case O_bit_or_not:		left |= ~right; break;
	case O_bit_exclusive_or:	left ^= right; break;
	case O_bit_and:			left &= right; break;
	case O_eq:
	case O_ne:
	  left = (left == right
		  && seg_left == seg_right
		  && (finalize_syms || frag_left == frag_right)
		  && (seg_left != undefined_section
		      || add_symbol == op_symbol)
		  ? ~ (valueT) 0 : 0);
	  if (op == O_ne)
	    left = ~left;
	  break;
	case O_lt:
	  left = (offsetT) left <  (offsetT) right ? ~ (valueT) 0 : 0;
	  break;
	case O_le:
	  left = (offsetT) left <= (offsetT) right ? ~ (valueT) 0 : 0;
	  break;
	case O_ge:
	  left = (offsetT) left >= (offsetT) right ? ~ (valueT) 0 : 0;
	  break;
	case O_gt:
	  left = (offsetT) left >  (offsetT) right ? ~ (valueT) 0 : 0;
	  break;
	case O_logical_and:	left = left && right; break;
	case O_logical_or:	left = left || right; break;
	default:		abort ();
	}

      op = O_constant;
      break;
    }

  if (op == O_symbol)
    {
      if (seg_left == absolute_section)
	op = O_constant;
      else if (seg_left == reg_section && final_val == 0)
	op = O_register;
      else if (!symbol_same_p (add_symbol, orig_add_symbol))
	final_val += left;
      expressionP->X_add_symbol = add_symbol;
    }
  expressionP->X_op = op;

  if (op == O_constant || op == O_register)
    final_val += left;
  expressionP->X_add_number = final_val;

  return 1;
}

static const expressionS zero = { .X_op = O_constant };
/* Build a dummy symbol to hold a complex expression.  This is how we
   build expressions up out of other expressions.  The symbol is put
   into the fake section expr_section.  */

symbolS *
make_expr_symbol (const expressionS *expressionP) {
    symbolS *symbolP;
    struct expr_symbol_line *n;

    if (expressionP->X_op == O_symbol
        && expressionP->X_add_number == 0)
        return expressionP->X_add_symbol;

    if (expressionP->X_op == O_big) {
        /* This won't work, because the actual value is stored in
        generic_floating_point_number or generic_bignum, and we are
        going to lose it if we haven't already.  */
        if (expressionP->X_add_number > 0)
            as_bad (_("bignum invalid"));
        else
            as_bad (_("floating point number invalid"));
        expressionP = &zero;
    }

    /* Putting constant symbols in absolute_section rather than
        expr_section is convenient for the old a.out code, for which
        S_GET_SEGMENT does not always retrieve the value put in by
        S_SET_SEGMENT.  */
    symbolP = symbol_create (FAKE_LABEL_NAME,
                (expressionP->X_op == O_constant
                    ? absolute_section
                    : expressionP->X_op == O_register
                    ? reg_section
                    : expr_section),
                &zero_address_frag, 0);
    symbol_set_value_expression (symbolP, expressionP);

    if (expressionP->X_op == O_constant)
        resolve_symbol_value (symbolP);

    n = notes_alloc (sizeof (*n));
    n->sym = symbolP;
    n->file = as_where (&n->line);
    n->next = expr_symbol_lines;
    expr_symbol_lines = n;

    return symbolP;
}
int
expr_symbol_where (symbolS *sym, const char **pfile, unsigned int *pline)
{
  struct expr_symbol_line *l;

  for (l = expr_symbol_lines; l != NULL; l = l->next)
    {
      if (l->sym == sym)
	{
	  *pfile = l->file;
	  *pline = l->line;
	  return 1;
	}
    }

  return 0;
}

void
expr_end (void) {
    for (size_t i = 0; i < ARRAY_SIZE (seen); i++)
        free (seen[i]);
}
