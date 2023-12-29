/*
 * @Author: hxy
 * @Date: 2023-10-06 21:00:58
 * @LastEditTime: 2023-11-27 21:19:35
 * @Description: 请填写简介
 */

#include "as.h"
#include "input-file.h"
#include "safe-ctype.h"
#define BUFFER_SIZE (32 * 1024)
/*标记当前正在读取的文件是否需要被预处理 */
int preprocess = 0;
static FILE *f_in;
static const char *file_name;

void
input_file_begin (void)
{
    f_in = (FILE *) 0;
}

size_t
input_file_buffer_size (void)
{
    return (BUFFER_SIZE);
}
/**
 * @Description: 打开一个不为空的文件
 */
void
input_file_open (const char *filename,
		 int pre) {
    int c;
    char buf[80];

    preprocess = pre;  
    gas_assert (filename != 0);     
    if (filename[0]) {
        f_in = fopen (filename, FOPEN_RT);
        file_name = filename;
    } else {
        /* Use stdin for the input file.  */
        f_in = stdin;
        /* For error messages.  */
        file_name = _("{standard input}");
    }
    if (f_in == NULL) {
        as_bad (_("can't open %s for reading: %s"),
            file_name, xstrerror (errno));
        return;
    }
    // 读取一个字符
    c = getc (f_in);

    // 检查文件读取是否发生了错误
    if (ferror (f_in)) {
        as_bad (_("can't read from %s: %s"),
            file_name, xstrerror (errno));

        fclose (f_in);
        f_in = NULL;
        return;
    }
    /* 对于空的文件 */
    if (feof (f_in)) {
        fclose (f_in);
        f_in = NULL;
        return;
    }
    gas_assert (c != EOF);
    if (c == '#') {
        /* Begins with comment, may not want to preprocess.  */
        c = getc (f_in);
        if (c == 'N') {
            char *p = fgets (buf, sizeof (buf), f_in);
            if (p && startswith (p, "O_APP") && ISSPACE (p[5]))
                // 不需要进行预处理
                preprocess = 0;
            if (!p || !strchr (p, '\n'))
                // 将字符#放回文件流中
                ungetc ('#', f_in);
            else
                ungetc ('\n', f_in);
        }  else if (c == 'A') {
            char *p = fgets (buf, sizeof (buf), f_in);
            if (p && startswith (p, "PP") && ISSPACE (p[2]))
                preprocess = 1;
            if (!p || !strchr (p, '\n'))
                ungetc ('#', f_in);
            else
                ungetc ('\n', f_in);
        }
        else if (c == '\n')
            ungetc ('\n', f_in);
        else
            ungetc ('#', f_in);
    } else
        ungetc (c, f_in);
}

/**
 * @Description: 确定文件大小
 */
static size_t
input_file_get (char *buf, size_t buflen) {
    size_t size;
    if (feof(f_in)) {
        return 0;
    }
    size = fread(buf, sizeof (char), buflen, f_in);
    if (ferror (f_in))
        as_bad (_("can't read from %s: %s"), file_name, xstrerror (errno));
    return size;
}

/**
 * @Description: 预处理输入文件的所有内容  比如注释之类的
 */
char *input_file_give_next_buffer (char *where) {
    // 最后一个字符
    char *return_value;		
    size_t size = 0;
    if (f_in == (FILE *) 0) 
        return 0;
    // 判断是否需要预处理
    if (preprocess) {
        size = do_scrub_chars (input_file_get, where, BUFFER_SIZE);
    } else {
        size = input_file_get (where, BUFFER_SIZE);
    }
    if (size)
        return_value = where + size;
    else {
        if (fclose (f_in)) {
            as_warn (_("can't close %s: %s"), file_name, xstrerror (errno));
        }
        f_in = (FILE *) 0;
        return_value = 0;
    }
    return return_value;
}

void
input_file_close (void)
{
    /* Don't close a null file pointer.  */
    if (f_in != NULL)
        fclose (f_in);

    f_in = 0;
}

void
input_file_end (void) {
}
