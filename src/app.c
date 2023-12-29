/*
 * @Author: hxy
 * @Date: 2023-10-02 20:28:56
 * @LastEditTime: 2023-11-11 17:33:10
 * @Description: 请填写简介
 */
// 预处理器
#include "as.h"
#include "app.h"
#define LEX_IS_SYMBOL_COMPONENT		1   /* 一个符号的组成部分 */
#define LEX_IS_WHITESPACE		2       /* 空白字符 */
#define LEX_IS_LINE_SEPARATOR		3   /* 行分隔符 ';' */
#define LEX_IS_COMMENT_START		4   /* 注释的起始标记 '@' */
#define LEX_IS_LINE_COMMENT_START	5   /* 行注释的起始标记 '//' */
#define	LEX_IS_TWOCHAR_COMMENT_1ST	6   /* 双字符注释的第一个字符 '/*' */
#define	LEX_IS_STRINGQUOTE		8       /* 字符串引号 */
#define	LEX_IS_COLON			9       /* 冒号 */
#define	LEX_IS_NEWLINE			10      /* 新行字符 \n */
#define	LEX_IS_ONECHAR_QUOTE		11  /* 单字符引号 */
#define LEX_IS_PARALLEL_SEPARATOR	14

#define IS_SYMBOL_COMPONENT(c)		(lex[c] == LEX_IS_SYMBOL_COMPONENT)
#define IS_WHITESPACE(c)		(lex[c] == LEX_IS_WHITESPACE)
#define IS_LINE_SEPARATOR(c)		(lex[c] == LEX_IS_LINE_SEPARATOR)
#define IS_PARALLEL_SEPARATOR(c)	(lex[c] == LEX_IS_PARALLEL_SEPARATOR)
#define IS_COMMENT(c)			(lex[c] == LEX_IS_COMMENT_START)
#define IS_LINE_COMMENT(c)		(lex[c] == LEX_IS_LINE_COMMENT_START)
#define	IS_NEWLINE(c)			(lex[c] == LEX_IS_NEWLINE)

static const char   symver_pseudo[] = ".symver";
static const char * symver_state;
static char last_char;
static char lex[256];
static const char symbol_chars[] =
"$._ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

#define scrub_m68k_mri 0

static const char * symver_state;
static int state;
static int old_state;
static char out_buf[20];
static int add_newlines;
static char *saved_input;
static const char *out_string;
static size_t saved_input_len;
static char input_buffer[32 * 1024];

void
do_scrub_begin (int m68k_mri ATTRIBUTE_UNUSED) {
    // tc-arm中m68k_mri传入的是0
    const char *p;
    int c;
    lex[' '] = LEX_IS_WHITESPACE;
    lex['\t'] = LEX_IS_WHITESPACE;
    lex['\r'] = LEX_IS_WHITESPACE;
    lex['\n'] = LEX_IS_NEWLINE;
    lex[':'] = LEX_IS_COLON;
    lex['"'] = LEX_IS_STRINGQUOTE;
    lex['\''] = LEX_IS_ONECHAR_QUOTE;
    for (p = symbol_chars; *p; ++p)
        lex[(unsigned char) *p] = LEX_IS_SYMBOL_COMPONENT;
    for (c = 128; c < 256; ++c)
        lex[c] = LEX_IS_SYMBOL_COMPONENT;
#ifdef tc_symbol_chars
    for (p = tc_symbol_chars; *p; ++p)
        lex[(unsigned char) *p] = LEX_IS_SYMBOL_COMPONENT;
#endif
    for (p = tc_comment_chars; *p; p++)
        lex[(unsigned char) *p] = LEX_IS_COMMENT_START;
    for (p = line_comment_chars; *p; p++)
        lex[(unsigned char) *p] = LEX_IS_LINE_COMMENT_START;
    for (p = tc_line_separator_chars; *p; p++)
        lex[(unsigned char) *p] = LEX_IS_LINE_SEPARATOR;
    if (lex['/'] == 0)
        lex['/'] = LEX_IS_TWOCHAR_COMMENT_1ST;
}

#define MULTIBYTE_WARN_COUNT_LIMIT 10
static unsigned int multibyte_warn_count = 0;

bool
scan_for_multibyte_characters (const unsigned char *  start,
			       const unsigned char *  end,
			       bool                   warn)
{
    if (end <= start)
        return false;

    if (warn && multibyte_warn_count > MULTIBYTE_WARN_COUNT_LIMIT)
        return false;

    bool found = false;

    while (start < end) {
        unsigned char c;

        if ((c = * start++) <= 0x7f)
            continue;

        if (!warn)
            return true;

        found = true;

        const char * filename;
        unsigned int lineno;

        filename = as_where (& lineno);
        if (filename == NULL)
            as_warn (_("multibyte character (%#x) encountered in input"), c);
        else if (lineno == 0)
            as_warn (_("multibyte character (%#x) encountered in %s"), c, filename);
        else
            as_warn (_("multibyte character (%#x) encountered in %s at or near line %u"), c, filename, lineno);

        if (++ multibyte_warn_count == MULTIBYTE_WARN_COUNT_LIMIT) {
            as_warn (_("further multibyte character warnings suppressed"));
            break;
        }
    }

    return found;
}

/**
 * @Description: 具体处理输入的文件，进行预处理
 */
size_t
do_scrub_chars (size_t (*get) (char *, size_t), char *tostart, size_t tolen) {
    // 输出缓冲流
    char *to = tostart;
    char *toend = tostart + tolen;

    // 输入缓冲流
    char *from;
    char *fromend;
    size_t fromlen;
    int ch, ch2 = 0;
    /* 正在处理的字符串的起始字符 */
    static char quotechar;

/* 获取下一个输入字符 */
#define GET()							\
  (from < fromend						\
   ? * (unsigned char *) (from++)				\
   : (saved_input = NULL,					\
      fromlen = (*get) (input_buffer, sizeof input_buffer),	\
      from = input_buffer,					\
      fromend = from + fromlen,					\
      (fromlen == 0						\
       ? EOF							\
       : * (unsigned char *) (from++))))    

/* 将字符推回输入流*/
#define UNGET(uch) (*--from = (uch))

/* 这个宏将一个字符放入输出缓冲区。
    如果这字符填充输出缓冲区，此宏跳转到标签TOFULL。
    处理两种不同的终止条件:输入端的EOF流，以及一个完整的输出缓冲区。*/
#define PUT(pch)				\
  do						\
    {						\
      *to++ = (pch);				\
      if (to >= toend)				\
	goto tofull;				\
    }						\
  while (0)

    if (saved_input != NULL) {
        from = saved_input;
        fromend = from + saved_input_len;
    } else {
        // 输入流的长度即输入文件的长度
        fromlen = (*get) (input_buffer, sizeof input_buffer);
        if (fromlen == 0) 
            return 0;
        from = input_buffer;
        fromend = from + fromlen;
    }
    
    while (1) {
        switch (state) {
            case -1:
                ch = *out_string++;
                if(out_string == '\0') {
                    state = old_state;
                    // 继续解析下一个操作数
                    old_state = 3;
                }
                PUT(ch);
                continue;
            // 处理'*/' 
            case -2:
                for (;;) {
                    do {
                        ch = GET();
                        if (ch == EOF) {
                            as_warn (_("end of file in comment"));
                            goto fromeof;
                        }
                        if (ch == '\n')
                            PUT ('\n');                      
                    } while (ch != '*');
                    while ((ch = GET ()) == '*') ;
                    if (ch == EOF) {
                        as_warn (_("end of file in comment"));
                        goto fromeof;
                    }
                    if (ch == '/')
                        break;
                    UNGET(ch);
                }
                state = old_state;
                UNGET(' ');
                continue;
            case 4:
                ch = GET ();
                if (ch == EOF)
                    goto fromeof;
                else if (ch >= '0' && ch <= '9')
                    PUT (ch);
                else {
                    while (ch != EOF && IS_WHITESPACE (ch))
                    ch = GET ();
                    if (ch == '"') {
                        quotechar = ch;
                        state = 5;
                        old_state = 3;
                        PUT (ch);
                    }
                    else {
                        while (ch != EOF && ch != '\n')
                            ch = GET ();
                        state = 0;
                        PUT (ch);
                    }
                }
                continue;
            // 解析 字符串，返回旧状态
            case 5:
                {
                    char *s;
                    ptrdiff_t len;
                    for (s = from; s < fromend; s++) {
                        if (ch == '\\'
                            || ch == quotechar
                            || ch == '\n')
                            break;
                    }
                    len = s - from;
                    if (len > toend - to)
                        len = toend - to;
                    if (len > 0) {
                        memcpy (to, from, len);
                        to += len;
                        from += len;
                        if (to >= toend)
                            goto tofull;
                    }
                }
                ch = GET ();
                if (ch == EOF) {
                    static char one_char_buf[1];
                    as_warn(_("end of file in string; '%c' inserted"),quotechar);
                    state = old_state;
                    from = fromend = one_char_buf + 1;
                    fromlen = 1;
                    UNGET('\n');
                    PUT (quotechar);
                } else if (ch == quotechar) {
                    state = old_state;
                    PUT (ch);
                } else {
                    PUT(ch);
                }
                continue;
            
            default:
                break;
        }
        
        // 从第一个字符开始解析
        ch = GET();
        recycle:
        if (symver_state == NULL) {
            if ((state == 0 || state == 1) && ch == symver_pseudo[0])
                symver_state = symver_pseudo + 1;
        } else {
            if (ch != '\0' && (*symver_state == ch))
                ++symver_state;
            else if (*symver_state != '\0')
                symver_state = NULL;
            else {
                if (IS_NEWLINE (ch))
                    symver_state = NULL; 
            }
        }
        if (ch == EOF) {
            if (state != 0) {
                as_warn (_("end of file not at end of a line; newline inserted"));
                state = 0;
                PUT ('\n');
            }
            goto fromeof;
        }
        switch (lex[ch]) {
            // 遇到空白字符
            case LEX_IS_WHITESPACE:
                do {
                    ch = GET();
                } while (ch != EOF && IS_WHITESPACE(ch));
                if (ch == EOF) 
                    goto fromeof;
                if (state == 0) {
                    // 遇到第一个非空字符  .text.还给t
                    state = 1;
                    UNGET (ch);
                    // 保留这个字符前面的空白字符 代表一行的开始
                    PUT (' ');
                    break;
                }
                if (IS_COMMENT (ch)
                    || IS_LINE_SEPARATOR (ch)
                    || IS_PARALLEL_SEPARATOR (ch)) {
                    goto recycle;
                }
                if ((state == 2 || state == 11)
                    && lex[ch] == LEX_IS_COLON) {
                    // :后
                    state = 1;
                    // 放到输出流
                    PUT (ch);
                    break;
                }
                switch (state) {
                    case 1:
                        goto recycle;
                    case 2:
                        state = 3;
                        if (to + 1 < toend) {
                            PUT (' ');
                            goto recycle;
                        }
                        // 继续解析
                        UNGET (ch);
                        PUT (' ');
                        break;
                    case 3:
                        goto recycle;
                    case 9:
                    case 10:
                        state = 10;
                        goto recycle;
                    case 11:
                        // 能否允许符号不带:
                        if (LABELS_WITHOUT_COLONS) {
                            state = 1; 
                        } else {
                            state = 3;
                        }
                        // 把\n符号去掉
                        // UNGET (ch);
                        PUT (' ');	/* Sp after label definition.  */
                        break;
                    default:
                        BAD_CASE (state);
                }
            case LEX_IS_TWOCHAR_COMMENT_1ST:  /* 遇到双字符注释的开始 */
                ch2 = GET ();
	            if (ch2 == '*') {
                    for (;;) {
                        do {
                            ch2 = GET();
                            if (ch2 != EOF && IS_NEWLINE (ch2)) {
                                add_newlines++;
                            } 
                        } while (ch2 != EOF && ch2 != '*');
                        while (ch2 == '*') 
                            ch2 = GET();

                        if (ch2 == EOF || ch2 == '/')
                            break;
                            
                        UNGET (ch2);
                    }
                    if (ch2 == EOF)
                        as_warn (_("end of file in multiline comment"));
                    ch = ' ';
                    goto recycle;
                }
            #ifdef DOUBLESLASH_LINE_COMMENTS
                else if (ch2 == '/') {
                    // 双斜杠行注释
                    do {
                        ch = GET();
                    } while (ch != EOF && !IS_NEWLINE (ch));
                    if (ch == EOF)
                        as_warn ("end of file in comment; newline inserted");
                    state = 0;
                    PUT('\n');
                    break;
                }
            #endif
                else {
                    if (ch2 != EOF) {
                        UNGET (ch2);
                    }
                    if (state == 9 || state == 10) 
                        state = 3;
                    PUT (ch);
                }
                break;
            case LEX_IS_COLON:
                if (state == 9 || state == 10)
                    state = 3;
                else if (state != 3)
                    state = 1;
                
                PUT (ch);
                break; 
            case LEX_IS_NEWLINE:
                if (add_newlines) {
                    --add_newlines;
                    UNGET (ch);
                }
            case LEX_IS_LINE_SEPARATOR:
                state = 0;
                PUT (ch);
                break;
            case LEX_IS_PARALLEL_SEPARATOR:
                state = 1;
                PUT (ch);
                break;
            case LEX_IS_LINE_COMMENT_START:
                if (ch == '/') {
                    ch2 = GET();
                    if (ch2 == '*') {
                        old_state = 3;
                        state = -2;
                        break;
                    } else if (ch2 != EOF){
                        UNGET (ch2);
                    }
                }
                if (state == 0 || state == 1) {	/* Only comment at start of line.  */
                    int startch;

                    startch = ch;

                    do {
                        ch = GET ();
                    } while (ch != EOF && IS_WHITESPACE (ch));

                    if (ch == EOF) {
                        as_warn (_("end of file in comment; newline inserted"));
                        PUT ('\n');
                        break;
                    }

                    if (ch < '0' || ch > '9' || state != 0 || startch != '#') {
                    /* Not a cpp line.  */
                        while (ch != EOF && !IS_NEWLINE (ch))
                            ch = GET ();
                        if (ch == EOF) {
                            as_warn (_("end of file in comment; newline inserted"));
                            PUT ('\n');
                        } else {
                            UNGET(ch);
                        }
                        state = 0;
                        break;
                    }

                    UNGET (ch);
                    old_state = 4;
                    state = -1;
                    out_string = "\t.linefile ";
                    PUT (*out_string++);
                    break;
                }
                if (strchr (tc_comment_chars, ch) == NULL
                    && (! scrub_m68k_mri
                    || (ch != '!' && ch != '*')))
                    goto de_fault;
                if (scrub_m68k_mri
                    && (ch == '!' || ch == '*' || ch == '#')
                    && state != 1
                    && state != 10)
                    goto de_fault;
            case LEX_IS_COMMENT_START:
                if ((symver_state != NULL) && (*symver_state == 0))
                    goto de_fault;
                if ((to > tostart ? to[-1] : last_char) == '\\')
                    /* Do not treat the <comment-char> as a start-of-comment.  */
                    goto de_fault;
                do {
                    ch = GET ();
                } while (ch != EOF && !IS_NEWLINE (ch));
                if (ch == EOF)
                    as_warn (_("end of file in comment; newline inserted"));
                state = 0;
                PUT ('\n');
                break;
            case LEX_IS_SYMBOL_COMPONENT:
                if (state == 10) {
                    UNGET(ch);
                    state = 3;
                    PUT(' ');
                    break;
                } 
                if (state == 3)
                    state = 9;
                if (to + 1 < toend && symver_state == NULL) {
                    char *s;
                    ptrdiff_t len;
                    for (s = from;s < fromend;s++) {
                        int type;

                        ch2 = *(unsigned char *)s;
                        type = lex[ch2];
                        if (type != 0 && type != LEX_IS_SYMBOL_COMPONENT) {
                            break;
                        }
                    }
                    if (s > from) {
                        --s;
                    }
                    len = s - from;
                    if (len > (toend - to) - 1)
                        len = (toend - to) - 1;
                    if (len > 0) {
                        PUT(ch);
                        memcpy (to, from, len);
                        to += len;
                        from += len;
                        if (to >= toend)
                            goto tofull;
                        ch = GET(); 
                    }
                }
            // case LEX_IS_COMMENT_START:
            //     if ((symver_state != NULL) && (*symver_state == 0))
            //         goto de_fault;

            //     /* Care is needed not to damage occurrences of \<comment-char>
            //         by stripping the <comment-char> onwards.  Yuck.  */
            //     if ((to > tostart ? to[-1] : last_char) == '\\')
            //         /* Do not treat the <comment-char> as a start-of-comment.  */
            //         goto de_fault;
            //     do {
            //         ch = GET ();
            //     } while (ch != EOF && !IS_NEWLINE (ch));
            //     if(ch == EOF) {
            //         as_warn (_("end of file in comment; newline inserted"));
            //     }
            //     state = 0;
            //     PUT ('\n');
            //     break;
            default:
            de_fault:  
                if (state == 0) {
                    state = 11;	
                } else if (state == 1) {
                    state = 2;
                } else if (state == 9) {
                    if (!IS_SYMBOL_COMPONENT (ch))
                        state = 3;
                } else if (state == 10) {
                    if (ch == '\\') {
                        if (to + 1 >= toend) {
                            UNGET (ch);
                            goto tofull;
                        }
                        *to++ = ' ';
                    }
                    state = 3;
                }
                PUT (ch);
                break;                
        }
    }
fromeof:
    if (to > tostart)
        last_char = to[-1];
    return to - tostart;
tofull:
    if (fromend > from) {
        saved_input = from;
        saved_input_len = fromend - from;
    } else
        saved_input = NULL;

    if (to > tostart)
        last_char = to[-1];
    return to - tostart;
}



