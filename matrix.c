/* matrix.c
 * 矩阵库实现
 * 提供矩阵创建、运算和高级功能的完整实现
 */

#include "matrix.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * @brief 计算矩阵元素索引
 * @param m 矩阵指针
 * @param r 行索引
 * @param c 列索引
 * @return 元素在data数组中的索引
 * @details 使用行主序存储，索引 = r * cols + c
 */
static inline size_t idx(const MATRIX *m, size_t r, size_t c) {
    return r * m->cols + c;
}

/**
 * @brief 创建矩阵
 * @param rows 矩阵行数
 * @param cols 矩阵列数
 * @param err 错误码输出指针
 * @param ms 内存栈指针（可选）
 * @return 创建的矩阵指针，NULL表示失败
 * @details 分配矩阵结构和数据内存，初始化为0
 *          如果提供内存栈，将data和矩阵指针都注册到栈中
 */
MATRIX *matrix_create(size_t rows, size_t cols, ERROR_ID *err, MEMSTACK *ms) {
    if (rows == 0 || cols == 0) {
        if (err) *err = ERR_INVALID_ARG;
        return NULL;
    }
    MATRIX *m = (MATRIX*)malloc(sizeof(MATRIX));
    if (!m) { if (err) *err = ERR_OOM; return NULL; }
    m->rows = rows; m->cols = cols;
    m->data = (REAL*)calloc(rows * cols, sizeof(REAL));
    if (!m->data) { free(m); if (err) *err = ERR_OOM; return NULL; }

    if (ms) {
        /* 先把 data 注册，再把矩阵指针注册（方便统一释放或检查） */
        if (memstack_push(ms, m->data) != ERR_OK) {
            free(m->data); free(m); if (err) *err = ERR_OOM; return NULL;
        }
        if (memstack_push(ms, m) != ERR_OK) {
            /* 若失败，移除刚压入的 data 并释放 */
            memstack_remove(ms, m->data);
            free(m->data); free(m); if (err) *err = ERR_OOM; return NULL;
        }
    }
    if (err) *err = ERR_OK;
    return m;
}

/**
 * @brief 释放矩阵
 * @param m 矩阵指针
 * @param ms 内存栈指针（可选）
 * @return 错误码
 * @details 释放矩阵数据和结构体内存
 *          如果矩阵在内存栈中，先从栈中移除再释放
 */
ERROR_ID matrix_free(MATRIX *m, MEMSTACK *ms) {
    if (!m) return ERR_INVALID_ARG;
    if (ms) {
        /* 尝试从栈中移除 m 指针（如果在栈中，则节点被销毁，返回 m） */
        void *p = memstack_remove(ms, m);
        if (p == NULL) {
            /* 不在栈中，则直接 free m */
            free(m->data);
            free(m);
            return ERR_OK;
        } else {
            /* m 在栈里，说明它与 data 都登记在 memstack 中 —— 需要同时移除 data */
            void *d = memstack_remove(ms, m->data);
            (void)d; /* 忽略返回值 */
            free(m->data);
            free(m);
            return ERR_OK;
        }
    } else {
        free(m->data);
        free(m);
        return ERR_OK;
    }
}

/**
 * @brief 设置矩阵元素
 * @param m 矩阵指针
 * @param r 行索引
 * @param c 列索引
 * @param v 要设置的值
 * @return 错误码
 * @details 检查索引有效性后设置对应位置的元素值
 */
ERROR_ID matrix_set(MATRIX *m, size_t r, size_t c, REAL v) {
    if (!m || r >= m->rows || c >= m->cols) return ERR_INVALID_ARG;
    m->data[idx(m,r,c)] = v;
    return ERR_OK;
}

/**
 * @brief 获取矩阵元素
 * @param m 矩阵指针
 * @param r 行索引
 * @param c 列索引
 * @param out 输出值的指针
 * @return 错误码
 * @details 检查参数和索引有效性后获取对应位置的元素值
 */
ERROR_ID matrix_get(_IN MATRIX *m, size_t r, size_t c, _OUT REAL *out) {
    if (!m || !out || r >= m->rows || c >= m->cols) return ERR_INVALID_ARG;
    *out = m->data[idx(m,r,c)];
    return ERR_OK;
}

/**
 * @brief 打印矩阵
 * @param m 矩阵指针
 * @details 以格式化的方式打印矩阵内容，每个元素占10个字符宽度，保留4位小数
 */
void matrix_print(_IN MATRIX *m) {
    if (!m) { printf("(null)\n"); return; }
    for (size_t i=0;i<m->rows;i++) {
        for (size_t j=0;j<m->cols;j++) {
            printf("%10.4f ", m->data[idx(m,i,j)]);
        }
        printf("\n");
    }
}

/**
 * @brief 矩阵加法
 * @param A 第一个矩阵
 * @param B 第二个矩阵
 * @param C 结果矩阵指针的指针
 * @param ms 内存栈指针
 * @return 错误码
 * @details 实现矩阵逐元素相加，要求两个矩阵维度相同
 */
ERROR_ID matrix_add(_IN MATRIX *A, _IN MATRIX *B, _OUT MATRIX **C, MEMSTACK *ms) {
    if (!A || !B || !C) return ERR_INVALID_ARG;
    if (A->rows != B->rows || A->cols != B->cols) return ERR_DIM_MISMATCH;
    ERROR_ID e;
    MATRIX *R = matrix_create(A->rows, A->cols, &e, ms);
    if (!R) return e;
    size_t n = A->rows * A->cols;
    for (size_t k=0;k<n;k++) R->data[k] = A->data[k] + B->data[k];
    *C = R;
    return ERR_OK;
}

/**
 * @brief 矩阵减法
 * @param A 第一个矩阵
 * @param B 第二个矩阵
 * @param C 结果矩阵指针的指针
 * @param ms 内存栈指针
 * @return 错误码
 * @details 实现矩阵逐元素相减，要求两个矩阵维度相同
 */
ERROR_ID matrix_sub(_IN MATRIX *A, _IN MATRIX *B, _OUT MATRIX **C, MEMSTACK *ms) {
    if (!A || !B || !C) return ERR_INVALID_ARG;
    if (A->rows != B->rows || A->cols != B->cols) return ERR_DIM_MISMATCH;
    ERROR_ID e;
    MATRIX *R = matrix_create(A->rows, A->cols, &e, ms);
    if (!R) return e;
    size_t n = A->rows * A->cols;
    for (size_t k=0;k<n;k++) R->data[k] = A->data[k] - B->data[k];
    *C = R;
    return ERR_OK;
}

/**
 * @brief 矩阵标量乘法
 * @param A 矩阵
 * @param k 标量系数
 * @param C 结果矩阵指针的指针
 * @param ms 内存栈指针
 * @return 错误码
 * @details 实现矩阵与标量的乘法，每个元素乘以标量k
 */
ERROR_ID matrix_scalar_mul(_IN MATRIX *A, REAL k, _OUT MATRIX **C, MEMSTACK *ms) {
    if (!A || !C) return ERR_INVALID_ARG;
    ERROR_ID e;
    MATRIX *R = matrix_create(A->rows, A->cols, &e, ms);
    if (!R) return e;
    size_t n = A->rows * A->cols;
    for (size_t i=0;i<n;i++) R->data[i] = A->data[i] * k;
    *C = R;
    return ERR_OK;
}

/**
 * @brief 矩阵转置
 * @param A 原矩阵
 * @param T 转置矩阵指针的指针
 * @param ms 内存栈指针
 * @return 错误码
 * @details 实现矩阵的行列互换，新矩阵的行数等于原矩阵列数，列数等于原矩阵行数
 */
ERROR_ID matrix_transpose(_IN MATRIX *A, _OUT MATRIX **T, MEMSTACK *ms) {
    if (!A || !T) return ERR_INVALID_ARG;
    ERROR_ID e;
    MATRIX *R = matrix_create(A->cols, A->rows, &e, ms);
    if (!R) return e;
    for (size_t i=0;i<A->rows;i++) for (size_t j=0;j<A->cols;j++)
        R->data[idx(R,j,i)] = A->data[idx(A,i,j)];
    *T = R;
    return ERR_OK;
}

/**
 * @brief 矩阵乘法（朴素实现）
 * @param A 第一个矩阵
 * @param B 第二个矩阵
 * @param C 结果矩阵指针的指针
 * @param ms 内存栈指针
 * @return 错误码
 * @details 使用三重循环实现矩阵乘法，时间复杂度O(n^3)
 *          要求A的列数等于B的行数
 */
ERROR_ID matrix_multiply(_IN MATRIX *A, _IN MATRIX *B, _OUT MATRIX **C, MEMSTACK *ms) {
    if (!A || !B || !C) return ERR_INVALID_ARG;
    if (A->cols != B->rows) return ERR_DIM_MISMATCH;
    ERROR_ID e;
    MATRIX *R = matrix_create(A->rows, B->cols, &e, ms);
    if (!R) return e;
    for (size_t i=0;i<A->rows;i++) {
        for (size_t k=0;k<A->cols;k++) {
            REAL a = A->data[idx(A,i,k)];
            for (size_t j=0;j<B->cols;j++) {
                R->data[idx(R,i,j)] += a * B->data[idx(B,k,j)];
            }
        }
    }
    *C = R;
    return ERR_OK;
}

/**
 * @brief 生成子矩阵（去掉指定行和列）
 * @param A 原矩阵
 * @param ex_r 要排除的行索引
 * @param ex_c 要排除的列索引
 * @param err 错误码输出指针
 * @param ms 内存栈指针
 * @return 子矩阵指针，NULL表示失败
 * @details 生成去掉指定行和列的子矩阵，用于行列式计算
 *          调用者负责释放返回的矩阵
 */
static MATRIX *matrix_minor(_IN MATRIX *A, size_t ex_r, size_t ex_c, ERROR_ID *err, MEMSTACK *ms) {
    if (!A || A->rows != A->cols) { if (err) *err = ERR_INVALID_ARG; return NULL; }
    size_t n = A->rows;
    MATRIX *M = matrix_create(n-1, n-1, err, ms);
    if (!M) return NULL;
    size_t rr = 0;
    for (size_t i=0;i<n;i++) {
        if (i == ex_r) continue;
        size_t cc = 0;
        for (size_t j=0;j<n;j++) {
            if (j == ex_c) continue;
            M->data[idx(M, rr, cc)] = A->data[idx(A, i, j)];
            cc++;
        }
        rr++;
    }
    if (err) *err = ERR_OK;
    return M;
}

/**
 * @brief 计算矩阵行列式（递归实现）
 * @param A 方阵
 * @param det 行列式值输出指针
 * @return 错误码
 * @details 使用Laplace展开递归计算行列式，适合教学演示
 *          基础情况：1×1矩阵返回唯一元素，2×2矩阵使用公式
 *          递归情况：按第一行展开，使用代数余子式
 */
ERROR_ID matrix_determinant(_IN MATRIX *A, _OUT REAL *det) {
    if (!A || !det) return ERR_INVALID_ARG;
    if (A->rows != A->cols) return ERR_DIM_MISMATCH;
    size_t n = A->rows;
    if (n == 1) { *det = A->data[0]; return ERR_OK; }
    if (n == 2) {
        *det = A->data[0]*A->data[3] - A->data[1]*A->data[2];
        return ERR_OK;
    }
    REAL sum = 0.0;
    for (size_t c=0;c<n;c++) {
        ERROR_ID e;
        MATRIX *sub = matrix_minor(A, 0, c, &e, NULL); /* minor 不使用全局 memstack，caller 释放 */
        if (!sub) return e;
        REAL subdet = 0.0;
        e = matrix_determinant(sub, &subdet);
        /* 释放子矩阵 */
        matrix_free(sub, NULL);
        if (e != ERR_OK) return e;
        REAL cofactor = A->data[idx(A,0,c)] * subdet;
        if ((c & 1) != 0) cofactor = -cofactor;
        sum += cofactor;
    }
    *det = sum;
    return ERR_OK;
}

/**
 * @brief 计算伴随矩阵
 * @param A 方阵
 * @param adj 伴随矩阵指针的指针
 * @param ms 内存栈指针
 * @return 错误码
 * @details 伴随矩阵是代数余子式矩阵的转置
 *          对每个元素计算其代数余子式，然后转置放置
 */
ERROR_ID matrix_adjugate(_IN MATRIX *A, _OUT MATRIX **adj, MEMSTACK *ms) {
    if (!A || !adj) return ERR_INVALID_ARG;
    if (A->rows != A->cols) return ERR_DIM_MISMATCH;
    size_t n = A->rows;
    ERROR_ID e;
    MATRIX *M = matrix_create(n, n, &e, ms);
    if (!M) return e;
    for (size_t i=0;i<n;i++) {
        for (size_t j=0;j<n;j++) {
            ERROR_ID em;
            MATRIX *sub = matrix_minor(A, i, j, &em, NULL);
            if (!sub) { matrix_free(M, ms); return em; }
            REAL subdet = 0.0;
            em = matrix_determinant(sub, &subdet);
            matrix_free(sub, NULL);
            if (em != ERR_OK) { matrix_free(M, ms); return em; }
            REAL cof = (( (i + j) & 1) ? -subdet : subdet);
            /* note: adjugate is transpose of cofactor matrix */
            M->data[idx(M, j, i)] = cof; /* transposed placement */
        }
    }
    *adj = M;
    return ERR_OK;
}

/**
 * @brief 计算逆矩阵
 * @param A 方阵
 * @param inv 逆矩阵指针的指针
 * @param ms 内存栈指针
 * @return 错误码
 * @details 使用公式 A^{-1} = adj(A) / det(A) 计算逆矩阵
 *          要求矩阵可逆（行列式不为0）
 */
ERROR_ID matrix_inverse(_IN MATRIX *A, _OUT MATRIX **inv, MEMSTACK *ms) {
    if (!A || !inv) return ERR_INVALID_ARG;
    if (A->rows != A->cols) return ERR_DIM_MISMATCH;
    REAL det = 0.0;
    ERROR_ID e = matrix_determinant(A, &det);
    if (e != ERR_OK) return e;
    if (det == 0.0) return ERR_NOT_INVERTIBLE;
    MATRIX *adj = NULL;
    e = matrix_adjugate(A, &adj, ms);
    if (e != ERR_OK) return e;
    /* inv = adj / det */
    MATRIX *R = NULL;
    e = matrix_scalar_mul(adj, 1.0 / det, &R, ms);
    if (e != ERR_OK) {
        matrix_free(adj, ms);
        return e;
    }
    /* adj may have been registered in ms; user should be aware duplicates */
    *inv = R;
    return ERR_OK;
}
