/* types.h
   自定义变量类型定义
*/
#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>

/* 基本数据类型定义 */
typedef double REAL;
typedef unsigned int ERROR_ID;

/* 输入输出宏定义 */
#define _IN const
#define _OUT

/* 错误码枚举 */
enum {
    ERR_OK = 0,
    ERR_INVALID_ARG = 1,
    ERR_OOM = 2,
    ERR_DIM_MISMATCH = 3,
    ERR_NOT_INVERTIBLE = 4
};

#endif /* TYPES_H */