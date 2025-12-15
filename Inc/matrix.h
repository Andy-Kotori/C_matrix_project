/* matrix.h
 * 矩阵库头文件
 * 提供矩阵创建、运算和高级功能接口
 */

#ifndef MATRIX_H
#define MATRIX_H

#include "types.h"
#include "memstack.h"

/**
 * @brief 矩阵对象结构体
 * @details 采用行主序存储，数据在内存中连续存放
 */
typedef struct {
    size_t rows;    /**< 矩阵行数 */
    size_t cols;    /**< 矩阵列数 */
    REAL *data;     /**< 矩阵数据数组，长度为 rows * cols */
} MATRIX;

/**
 * @brief 创建矩阵
 * @param rows 矩阵行数
 * @param cols 矩阵列数
 * @param err 错误码输出指针
 * @param ms 内存栈指针（可选）
 * @return 创建的矩阵指针，NULL表示失败
 * @details 如果提供非NULL的MEMSTACK指针，会将data和矩阵指针压入栈中
 */
MATRIX *matrix_create(size_t rows, size_t cols, ERROR_ID *err, MEMSTACK *ms);

/**
 * @brief 释放矩阵
 * @param m 矩阵指针
 * @param ms 内存栈指针（可选）
 * @return 错误码
 * @details 如果矩阵在内存栈中，会从栈中移除并释放
 */
ERROR_ID matrix_free(MATRIX *m, MEMSTACK *ms);

/**
 * @brief 设置矩阵元素
 * @param m 矩阵指针
 * @param r 行索引
 * @param c 列索引
 * @param v 要设置的值
 * @return 错误码
 */
ERROR_ID matrix_set(MATRIX *m, size_t r, size_t c, REAL v);

/**
 * @brief 获取矩阵元素
 * @param m 矩阵指针
 * @param r 行索引
 * @param c 列索引
 * @param out 输出值的指针
 * @return 错误码
 */
ERROR_ID matrix_get(_IN MATRIX *m, size_t r, size_t c, _OUT REAL *out);

/**
 * @brief 打印矩阵
 * @param m 矩阵指针
 */
void matrix_print(_IN MATRIX *m);

/**
 * @brief 矩阵加法
 * @param A 第一个矩阵
 * @param B 第二个矩阵
 * @param C 结果矩阵指针的指针
 * @param ms 内存栈指针
 * @return 错误码
 */
ERROR_ID matrix_add(_IN MATRIX *A, _IN MATRIX *B, _OUT MATRIX **C, MEMSTACK *ms);

/**
 * @brief 矩阵减法
 * @param A 第一个矩阵
 * @param B 第二个矩阵
 * @param C 结果矩阵指针的指针
 * @param ms 内存栈指针
 * @return 错误码
 */
ERROR_ID matrix_sub(_IN MATRIX *A, _IN MATRIX *B, _OUT MATRIX **C, MEMSTACK *ms);

/**
 * @brief 矩阵标量乘法
 * @param A 矩阵
 * @param k 标量系数
 * @param C 结果矩阵指针的指针
 * @param ms 内存栈指针
 * @return 错误码
 */
ERROR_ID matrix_scalar_mul(_IN MATRIX *A, REAL k, _OUT MATRIX **C, MEMSTACK *ms);

/**
 * @brief 矩阵转置
 * @param A 原矩阵
 * @param T 转置矩阵指针的指针
 * @param ms 内存栈指针
 * @return 错误码
 */
ERROR_ID matrix_transpose(_IN MATRIX *A, _OUT MATRIX **T, MEMSTACK *ms);

/**
 * @brief 矩阵乘法
 * @param A 第一个矩阵
 * @param B 第二个矩阵
 * @param C 结果矩阵指针的指针
 * @param ms 内存栈指针
 * @return 错误码
 */
ERROR_ID matrix_multiply(_IN MATRIX *A, _IN MATRIX *B, _OUT MATRIX **C, MEMSTACK *ms);

/**
 * @brief 计算矩阵行列式（递归Laplace展开）
 * @param A 方阵
 * @param det 行列式值输出指针
 * @return 错误码
 * @details 使用递归Laplace展开，适合教学演示
 */
ERROR_ID matrix_determinant(_IN MATRIX *A, _OUT REAL *det);

/**
 * @brief 计算矩阵行列式（优化的递归展开法）
 * @param A 方阵
 * @param det 行列式值输出指针
 * @return 错误码
 * @details 使用优化的Laplace展开，添加循环展开和缓存优化
 */
ERROR_ID matrix_determinant_recursive(_IN MATRIX *A, _OUT REAL *det);

/**
 * @brief 计算矩阵行列式（高斯消元法）
 * @param A 方阵
 * @param det 行列式值输出指针
 * @return 错误码
 * @details 使用高斯消元将矩阵化为上三角矩阵，行列式等于对角线元素乘积
 */
ERROR_ID matrix_determinant_gaussian(_IN MATRIX *A, _OUT REAL *det);

/**
 * @brief 计算矩阵行列式（LU分解法）
 * @param A 方阵
 * @param det 行列式值输出指针
 * @return 错误码
 * @details 使用LU分解，行列式等于U矩阵对角线元素乘积乘以排列矩阵的行列式
 */
ERROR_ID matrix_determinant_lu(_IN MATRIX *A, _OUT REAL *det);

/**
 * @brief 计算伴随矩阵
 * @param A 方阵
 * @param adj 伴随矩阵指针的指针
 * @param ms 内存栈指针
 * @return 错误码
 * @details 伴随矩阵是代数余子式矩阵的转置
 */
ERROR_ID matrix_adjugate(_IN MATRIX *A, _OUT MATRIX **adj, MEMSTACK *ms);

/**
 * @brief 计算逆矩阵（伴随矩阵法）
 * @param A 方阵
 * @param inv 逆矩阵指针的指针
 * @param ms 内存栈指针
 * @return 错误码
 * @details 使用公式 A^{-1} = adj(A) / det(A)，采用优化的递归行列式计算
 */
ERROR_ID matrix_inverse_adjugate(_IN MATRIX *A, _OUT MATRIX **inv, MEMSTACK *ms);

/**
 * @brief 计算逆矩阵（Gauss-Jordan消元法）
 * @param A 方阵
 * @param inv 逆矩阵指针的指针
 * @param ms 内存栈指针
 * @return 错误码
 * @details 通过增广矩阵[A|I]进行行变换得到[I|A^{-1}]，使用循环展开优化
 */
ERROR_ID matrix_inverse_gauss_jordan(_IN MATRIX *A, _OUT MATRIX **inv, MEMSTACK *ms);

/**
 * @brief 计算逆矩阵（LU分解法）
 * @param A 方阵
 * @param inv 逆矩阵指针的指针
 * @param ms 内存栈指针
 * @return 错误码
 * @details 先进行LU分解，然后求解线性方程组得到逆矩阵，利用现有LU优化
 */
ERROR_ID matrix_inverse_lu(_IN MATRIX *A, _OUT MATRIX **inv, MEMSTACK *ms);

/**
 * @brief 计算逆矩阵（兼容接口，使用伴随矩阵法）
 * @param A 方阵
 * @param inv 逆矩阵指针的指针
 * @param ms 内存栈指针
 * @return 错误码
 * @details 保持向后兼容，调用matrix_inverse_adjugate
 */
ERROR_ID matrix_inverse(_IN MATRIX *A, _OUT MATRIX **inv, MEMSTACK *ms);

#endif /* MATRIX_H */
