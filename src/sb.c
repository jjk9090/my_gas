/*
 * @Author: hxy
 * @Date: 2023-10-02 18:28:49
 * @LastEditTime: 2023-10-04 16:25:03
 * @Description: 操作字符串  字符串缓冲区
 */
#include "as.h"
#include "sb.h"
void
sb_kill (sb *ptr)
{
    free (ptr->ptr);
}
