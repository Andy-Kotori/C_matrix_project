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
    
    /* 循环展开优化 - 每次处理4个元素 */
    size_t k;
    for (k = 0; k + 3 < n; k += 4) {
        R->data[k]   = A->data[k]   + B->data[k];
        R->data[k+1] = A->data[k+1] + B->data[k+1];
        R->data[k+2] = A->data[k+2] + B->data[k+2];
        R->data[k+3] = A->data[k+3] + B->data[k+3];
    }
    /* 处理剩余元素 */
    for (; k < n; k++) {
        R->data[k] = A->data[k] + B->data[k];
    }
    
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
    
    /* 循环展开优化 - 每次处理4个元素 */
    size_t i;
    for (i = 0; i + 3 < n; i += 4) {
        R->data[i]   = A->data[i]   * k;
        R->data[i+1] = A->data[i+1] * k;
        R->data[i+2] = A->data[i+2] * k;
        R->data[i+3] = A->data[i+3] * k;
    }
    /* 处理剩余元素 */
    for (; i < n; i++) {
        R->data[i] = A->data[i] * k;
    }
    
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
    
    /* 分块转置优化 - 使用8x8块大小提高缓存命中率 */
    const size_t BLOCK_SIZE = 8;
    size_t i, j;
    
    /* 分块处理主区域 */
    for (i = 0; i + BLOCK_SIZE - 1 < A->rows; i += BLOCK_SIZE) {
        for (j = 0; j + BLOCK_SIZE - 1 < A->cols; j += BLOCK_SIZE) {
            /* 处理每个块 */
            for (size_t ii = i; ii < i + BLOCK_SIZE; ii++) {
                for (size_t jj = j; jj < j + BLOCK_SIZE; jj++) {
                    R->data[idx(R, jj, ii)] = A->data[idx(A, ii, jj)];
                }
            }
        }
    }
    
    /* 处理剩余的行 */
    for (i = A->rows - (A->rows % BLOCK_SIZE); i < A->rows; i++) {
        for (j = 0; j < A->cols; j++) {
            R->data[idx(R, j, i)] = A->data[idx(A, i, j)];
        }
    }
    
    /* 处理剩余的列 */
    for (j = A->cols - (A->cols % BLOCK_SIZE); j < A->cols; j++) {
        for (i = 0; i < A->rows - (A->rows % BLOCK_SIZE); i++) {
            R->data[idx(R, j, i)] = A->data[idx(A, i, j)];
        }
    }
    
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
    
    /* 分块矩阵乘法优化 - 提高缓存命中率 */
    const size_t BLOCK_SIZE = 32;
    size_t i, j, k, ii, jj, kk;
    
    /* 初始化结果矩阵为0 */
    size_t n = A->rows * B->cols;
    for (size_t idx = 0; idx < n; idx++) {
        R->data[idx] = 0.0;
    }
    
    /* 分块矩阵乘法 */
    for (i = 0; i < A->rows; i += BLOCK_SIZE) {
        for (j = 0; j < B->cols; j += BLOCK_SIZE) {
            for (k = 0; k < A->cols; k += BLOCK_SIZE) {
                /* 处理每个块 */
                size_t i_end = (i + BLOCK_SIZE < A->rows) ? i + BLOCK_SIZE : A->rows;
                size_t j_end = (j + BLOCK_SIZE < B->cols) ? j + BLOCK_SIZE : B->cols;
                size_t k_end = (k + BLOCK_SIZE < A->cols) ? k + BLOCK_SIZE : A->cols;
                
                for (ii = i; ii < i_end; ii++) {
                    for (kk = k; kk < k_end; kk++) {
                        REAL a = A->data[idx(A, ii, kk)];
                        for (jj = j; jj < j_end; jj++) {
                            R->data[idx(R, ii, jj)] += a * B->data[idx(B, kk, jj)];
                        }
                    }
                }
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
 * @brief 计算矩阵行列式（优化的递归展开法）
 * @param A 方阵
 * @param det 行列式值输出指针
 * @return 错误码
 * @details 使用优化的Laplace展开，添加循环展开和缓存优化
 */
ERROR_ID matrix_determinant_recursive(_IN MATRIX *A, _OUT REAL *det) {
    if (!A || !det) return ERR_INVALID_ARG;
    if (A->rows != A->cols) return ERR_DIM_MISMATCH;
    size_t n = A->rows;
    
    /* 基础情况 */
    if (n == 1) { *det = A->data[0]; return ERR_OK; }
    if (n == 2) {
        *det = A->data[0]*A->data[3] - A->data[1]*A->data[2];
        return ERR_OK;
    }
    if (n == 3) {
        /* 3x3矩阵直接使用公式，避免递归开销 */
        *det = A->data[0] * (A->data[4]*A->data[8] - A->data[5]*A->data[7]) -
               A->data[1] * (A->data[3]*A->data[8] - A->data[5]*A->data[6]) +
               A->data[2] * (A->data[3]*A->data[7] - A->data[4]*A->data[6]);
        return ERR_OK;
    }
    
    REAL sum = 0.0;
    MEMSTACK ms;
    memstack_init(&ms);
    
    /* 循环展开 - 每次处理4列 */
    size_t c;
    for (c = 0; c + 3 < n; c += 4) {
        /* 处理4列 */
        for (size_t col = c; col < c + 4 && col < n; col++) {
            ERROR_ID e;
            MATRIX *sub = matrix_minor(A, 0, col, &e, &ms);
            if (!sub) { memstack_free_all(&ms); return e; }
            REAL subdet = 0.0;
            e = matrix_determinant_recursive(sub, &subdet);
            if (e != ERR_OK) { memstack_free_all(&ms); return e; }
            REAL cofactor = A->data[idx(A, 0, col)] * subdet;
            if ((col & 1) != 0) cofactor = -cofactor;
            sum += cofactor;
        }
    }
    
    /* 处理剩余的列 */
    for (; c < n; c++) {
        ERROR_ID e;
        MATRIX *sub = matrix_minor(A, 0, c, &e, &ms);
        if (!sub) { memstack_free_all(&ms); return e; }
        REAL subdet = 0.0;
        e = matrix_determinant_recursive(sub, &subdet);
        if (e != ERR_OK) { memstack_free_all(&ms); return e; }
        REAL cofactor = A->data[idx(A, 0, c)] * subdet;
        if ((c & 1) != 0) cofactor = -cofactor;
        sum += cofactor;
    }
    
    memstack_free_all(&ms);
    *det = sum;
    return ERR_OK;
}

/**
 * @brief 计算矩阵行列式（高斯消元法）
 * @param A 方阵
 * @param det 行列式值输出指针
 * @return 错误码
 * @details 使用高斯消元将矩阵化为上三角矩阵，行列式等于对角线元素乘积
 */
ERROR_ID matrix_determinant_gaussian(_IN MATRIX *A, _OUT REAL *det) {
    if (!A || !det) return ERR_INVALID_ARG;
    if (A->rows != A->cols) return ERR_DIM_MISMATCH;
    size_t n = A->rows;
    
    /* 创建矩阵副本，避免修改原矩阵 */
    MEMSTACK ms;
    memstack_init(&ms);
    MATRIX *M = matrix_create(n, n, NULL, &ms);
    if (!M) { memstack_free_all(&ms); return ERR_OOM; }
    
    /* 复制矩阵数据 */
    size_t total_elements = n * n;
    size_t k;
    for (k = 0; k + 3 < total_elements; k += 4) {
        M->data[k]   = A->data[k];
        M->data[k+1] = A->data[k+1];
        M->data[k+2] = A->data[k+2];
        M->data[k+3] = A->data[k+3];
    }
    for (; k < total_elements; k++) {
        M->data[k] = A->data[k];
    }
    
    REAL determinant = 1.0;
    
    /* 高斯消元 */
    for (size_t i = 0; i < n; i++) {
        /* 寻找主元 */
        size_t max_row = i;
        REAL max_val = M->data[idx(M, i, i)];
        if (max_val < 0) max_val = -max_val;
        
        for (size_t row = i + 1; row < n; row++) {
            REAL val = M->data[idx(M, row, i)];
            if (val < 0) val = -val;
            if (val > max_val) {
                max_val = val;
                max_row = row;
            }
        }
        
        /* 如果主元为0，行列式为0 */
        if (max_val < 1e-12) {
            memstack_free_all(&ms);
            *det = 0.0;
            return ERR_OK;
        }
        
        /* 交换行（如果需要） */
        if (max_row != i) {
            /* 循环展开优化行交换 */
            for (size_t col = 0; col + 3 < n; col += 4) {
                REAL temp = M->data[idx(M, i, col)];
                M->data[idx(M, i, col)] = M->data[idx(M, max_row, col)];
                M->data[idx(M, max_row, col)] = temp;
                
                temp = M->data[idx(M, i, col+1)];
                M->data[idx(M, i, col+1)] = M->data[idx(M, max_row, col+1)];
                M->data[idx(M, max_row, col+1)] = temp;
                
                temp = M->data[idx(M, i, col+2)];
                M->data[idx(M, i, col+2)] = M->data[idx(M, max_row, col+2)];
                M->data[idx(M, max_row, col+2)] = temp;
                
                temp = M->data[idx(M, i, col+3)];
                M->data[idx(M, i, col+3)] = M->data[idx(M, max_row, col+3)];
                M->data[idx(M, max_row, col+3)] = temp;
            }
            for (size_t col = n - (n % 4); col < n; col++) {
                REAL temp = M->data[idx(M, i, col)];
                M->data[idx(M, i, col)] = M->data[idx(M, max_row, col)];
                M->data[idx(M, max_row, col)] = temp;
            }
            determinant = -determinant; /* 行交换改变符号 */
        }
        
        /* 消元 */
        REAL pivot = M->data[idx(M, i, i)];
        for (size_t row = i + 1; row < n; row++) {
            REAL factor = M->data[idx(M, row, i)] / pivot;
            if (factor != 0.0) {
                /* 循环展开优化消元 */
                for (size_t col = i; col + 3 < n; col += 4) {
                    M->data[idx(M, row, col)]   -= factor * M->data[idx(M, i, col)];
                    M->data[idx(M, row, col+1)] -= factor * M->data[idx(M, i, col+1)];
                    M->data[idx(M, row, col+2)] -= factor * M->data[idx(M, i, col+2)];
                    M->data[idx(M, row, col+3)] -= factor * M->data[idx(M, i, col+3)];
                }
                for (size_t col = n - (n % 4); col < n; col++) {
                    M->data[idx(M, row, col)] -= factor * M->data[idx(M, i, col)];
                }
            }
        }
    }
    
    /* 计算对角线元素乘积 */
    for (size_t i = 0; i < n; i++) {
        determinant *= M->data[idx(M, i, i)];
    }
    
    memstack_free_all(&ms);
    *det = determinant;
    return ERR_OK;
}

/**
 * @brief 计算矩阵行列式（LU分解法）
 * @param A 方阵
 * @param det 行列式值输出指针
 * @return 错误码
 * @details 使用LU分解，行列式等于U矩阵对角线元素乘积乘以排列矩阵的行列式
 */
ERROR_ID matrix_determinant_lu(_IN MATRIX *A, _OUT REAL *det) {
    if (!A || !det) return ERR_INVALID_ARG;
    if (A->rows != A->cols) return ERR_DIM_MISMATCH;
    size_t n = A->rows;
    
    /* 创建矩阵副本 */
    MEMSTACK ms;
    memstack_init(&ms);
    MATRIX *M = matrix_create(n, n, NULL, &ms);
    if (!M) { memstack_free_all(&ms); return ERR_OOM; }
    
    /* 复制矩阵数据 */
    size_t total_elements = n * n;
    size_t k;
    for (k = 0; k + 3 < total_elements; k += 4) {
        M->data[k]   = A->data[k];
        M->data[k+1] = A->data[k+1];
        M->data[k+2] = A->data[k+2];
        M->data[k+3] = A->data[k+3];
    }
    for (; k < total_elements; k++) {
        M->data[k] = A->data[k];
    }
    
    REAL determinant = 1.0;
    int sign = 1; /* 记录行交换次数的奇偶性 */
    
    /* LU分解 */
    for (size_t i = 0; i < n; i++) {
        /* 寻找主元 */
        size_t max_row = i;
        REAL max_val = M->data[idx(M, i, i)];
        if (max_val < 0) max_val = -max_val;
        
        for (size_t row = i + 1; row < n; row++) {
            REAL val = M->data[idx(M, row, i)];
            if (val < 0) val = -val;
            if (val > max_val) {
                max_val = val;
                max_row = row;
            }
        }
        
        /* 如果主元为0，行列式为0 */
        if (max_val < 1e-12) {
            memstack_free_all(&ms);
            *det = 0.0;
            return ERR_OK;
        }
        
        /* 交换行（如果需要） */
        if (max_row != i) {
            /* 循环展开优化行交换 */
            for (size_t col = 0; col + 3 < n; col += 4) {
                REAL temp = M->data[idx(M, i, col)];
                M->data[idx(M, i, col)] = M->data[idx(M, max_row, col)];
                M->data[idx(M, max_row, col)] = temp;
                
                temp = M->data[idx(M, i, col+1)];
                M->data[idx(M, i, col+1)] = M->data[idx(M, max_row, col+1)];
                M->data[idx(M, max_row, col+1)] = temp;
                
                temp = M->data[idx(M, i, col+2)];
                M->data[idx(M, i, col+2)] = M->data[idx(M, max_row, col+2)];
                M->data[idx(M, max_row, col+2)] = temp;
                
                temp = M->data[idx(M, i, col+3)];
                M->data[idx(M, i, col+3)] = M->data[idx(M, max_row, col+3)];
                M->data[idx(M, max_row, col+3)] = temp;
            }
            for (size_t col = n - (n % 4); col < n; col++) {
                REAL temp = M->data[idx(M, i, col)];
                M->data[idx(M, i, col)] = M->data[idx(M, max_row, col)];
                M->data[idx(M, max_row, col)] = temp;
            }
            sign = -sign; /* 行交换改变符号 */
        }
        
        /* 计算L和U矩阵 */
        REAL pivot = M->data[idx(M, i, i)];
        for (size_t row = i + 1; row < n; row++) {
            REAL factor = M->data[idx(M, row, i)] / pivot;
            M->data[idx(M, row, i)] = factor; /* 存储L矩阵元素 */
            
            /* 更新U矩阵元素 */
            for (size_t col = i + 1; col + 3 < n; col += 4) {
                M->data[idx(M, row, col)]   -= factor * M->data[idx(M, i, col)];
                M->data[idx(M, row, col+1)] -= factor * M->data[idx(M, i, col+1)];
                M->data[idx(M, row, col+2)] -= factor * M->data[idx(M, i, col+2)];
                M->data[idx(M, row, col+3)] -= factor * M->data[idx(M, i, col+3)];
            }
            for (size_t col = n - (n % 4); col < n; col++) {
                M->data[idx(M, row, col)] -= factor * M->data[idx(M, i, col)];
            }
        }
    }
    
    /* 计算U矩阵对角线元素乘积 */
    for (size_t i = 0; i < n; i++) {
        determinant *= M->data[idx(M, i, i)];
    }
    
    /* 乘以排列矩阵的行列式（sign） */
    determinant *= sign;
    
    memstack_free_all(&ms);
    *det = determinant;
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
 * @brief 计算逆矩阵（伴随矩阵法）
 * @param A 方阵
 * @param inv 逆矩阵指针的指针
 * @param ms 内存栈指针
 * @return 错误码
 * @details 使用公式 A^{-1} = adj(A) / det(A) 计算逆矩阵
 *          采用优化的递归行列式计算，利用循环展开和缓存优化
 */
ERROR_ID matrix_inverse_adjugate(_IN MATRIX *A, _OUT MATRIX **inv, MEMSTACK *ms) {
    if (!A || !inv) return ERR_INVALID_ARG;
    if (A->rows != A->cols) return ERR_DIM_MISMATCH;
    
    /* 使用优化的递归行列式计算 */
    REAL det = 0.0;
    ERROR_ID e = matrix_determinant_recursive(A, &det);
    if (e != ERR_OK) return e;
    if (det == 0.0) return ERR_NOT_INVERTIBLE;
    
    /* 计算伴随矩阵 */
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
    
    *inv = R;
    return ERR_OK;
}

/**
 * @brief 计算逆矩阵（Gauss-Jordan消元法）
 * @param A 方阵
 * @param inv 逆矩阵指针的指针
 * @param ms 内存栈指针
 * @return 错误码
 * @details 通过增广矩阵[A|I]进行行变换得到[I|A^{-1}]
 *          使用循环展开优化，提高计算效率
 */
ERROR_ID matrix_inverse_gauss_jordan(_IN MATRIX *A, _OUT MATRIX **inv, MEMSTACK *ms) {
    if (!A || !inv) return ERR_INVALID_ARG;
    if (A->rows != A->cols) return ERR_DIM_MISMATCH;
    size_t n = A->rows;
    
    MEMSTACK local_ms;
    memstack_init(&local_ms);
    
    /* 创建增广矩阵 [A|I] */
    MATRIX *M = matrix_create(n, 2 * n, NULL, &local_ms);
    if (!M) { memstack_free_all(&local_ms); return ERR_OOM; }
    
    /* 复制A到左半部分，I到右半部分 */
    size_t total_elements = n * n;
    size_t k;
    for (k = 0; k + 3 < total_elements; k += 4) {
        M->data[k]   = A->data[k];
        M->data[k+1] = A->data[k+1];
        M->data[k+2] = A->data[k+2];
        M->data[k+3] = A->data[k+3];
    }
    for (; k < total_elements; k++) {
        M->data[k] = A->data[k];
    }
    
    /* 设置右半部分为单位矩阵 */
    for (size_t i = 0; i < n; i++) {
        M->data[idx(M, i, n + i)] = 1.0;
    }
    
    /* Gauss-Jordan消元 */
    for (size_t i = 0; i < n; i++) {
        /* 寻找主元 */
        size_t max_row = i;
        REAL max_val = M->data[idx(M, i, i)];
        if (max_val < 0) max_val = -max_val;
        
        for (size_t row = i + 1; row < n; row++) {
            REAL val = M->data[idx(M, row, i)];
            if (val < 0) val = -val;
            if (val > max_val) {
                max_val = val;
                max_row = row;
            }
        }
        
        /* 如果主元为0，矩阵不可逆 */
        if (max_val < 1e-12) {
            memstack_free_all(&local_ms);
            return ERR_NOT_INVERTIBLE;
        }
        
        /* 交换行（如果需要） */
        if (max_row != i) {
            /* 循环展开优化行交换 */
            for (size_t col = 0; col + 3 < 2 * n; col += 4) {
                REAL temp = M->data[idx(M, i, col)];
                M->data[idx(M, i, col)] = M->data[idx(M, max_row, col)];
                M->data[idx(M, max_row, col)] = temp;
                
                temp = M->data[idx(M, i, col+1)];
                M->data[idx(M, i, col+1)] = M->data[idx(M, max_row, col+1)];
                M->data[idx(M, max_row, col+1)] = temp;
                
                temp = M->data[idx(M, i, col+2)];
                M->data[idx(M, i, col+2)] = M->data[idx(M, max_row, col+2)];
                M->data[idx(M, max_row, col+2)] = temp;
                
                temp = M->data[idx(M, i, col+3)];
                M->data[idx(M, i, col+3)] = M->data[idx(M, max_row, col+3)];
                M->data[idx(M, max_row, col+3)] = temp;
            }
            for (size_t col = 2 * n - ((2 * n) % 4); col < 2 * n; col++) {
                REAL temp = M->data[idx(M, i, col)];
                M->data[idx(M, i, col)] = M->data[idx(M, max_row, col)];
                M->data[idx(M, max_row, col)] = temp;
            }
        }
        
        /* 归一化当前行 */
        REAL pivot = M->data[idx(M, i, i)];
        /* 循环展开优化行归一化 */
        for (size_t col = 0; col + 3 < 2 * n; col += 4) {
            M->data[idx(M, i, col)]   /= pivot;
            M->data[idx(M, i, col+1)] /= pivot;
            M->data[idx(M, i, col+2)] /= pivot;
            M->data[idx(M, i, col+3)] /= pivot;
        }
        for (size_t col = 2 * n - ((2 * n) % 4); col < 2 * n; col++) {
            M->data[idx(M, i, col)] /= pivot;
        }
        
        /* 消去其他行 */
        for (size_t row = 0; row < n; row++) {
            if (row == i) continue;
            REAL factor = M->data[idx(M, row, i)];
            if (factor != 0.0) {
                /* 循环展开优化消元 */
                for (size_t col = 0; col + 3 < 2 * n; col += 4) {
                    M->data[idx(M, row, col)]   -= factor * M->data[idx(M, i, col)];
                    M->data[idx(M, row, col+1)] -= factor * M->data[idx(M, i, col+1)];
                    M->data[idx(M, row, col+2)] -= factor * M->data[idx(M, i, col+2)];
                    M->data[idx(M, row, col+3)] -= factor * M->data[idx(M, i, col+3)];
                }
                for (size_t col = 2 * n - ((2 * n) % 4); col < 2 * n; col++) {
                    M->data[idx(M, row, col)] -= factor * M->data[idx(M, i, col)];
                }
            }
        }
    }
    
    /* 提取逆矩阵（右半部分） */
    MATRIX *R = matrix_create(n, n, NULL, ms);
    if (!R) { memstack_free_all(&local_ms); return ERR_OOM; }
    
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            R->data[idx(R, i, j)] = M->data[idx(M, i, n + j)];
        }
    }
    
    memstack_free_all(&local_ms);
    *inv = R;
    return ERR_OK;
}

/**
 * @brief 计算逆矩阵（LU分解法）
 * @param A 方阵
 * @param inv 逆矩阵指针的指针
 * @param ms 内存栈指针
 * @return 错误码
 * @details 先进行LU分解，然后求解A*X=I得到逆矩阵
 *          利用现有的LU分解优化技术
 */
ERROR_ID matrix_inverse_lu(_IN MATRIX *A, _OUT MATRIX **inv, MEMSTACK *ms) {
    if (!A || !inv) return ERR_INVALID_ARG;
    if (A->rows != A->cols) return ERR_DIM_MISMATCH;
    size_t n = A->rows;
    
    MEMSTACK local_ms;
    memstack_init(&local_ms);
    
    /* 创建矩阵副本用于LU分解 */
    MATRIX *M = matrix_create(n, n, NULL, &local_ms);
    if (!M) { memstack_free_all(&local_ms); return ERR_OOM; }
    
    /* 复制矩阵数据 */
    size_t total_elements = n * n;
    size_t k;
    for (k = 0; k + 3 < total_elements; k += 4) {
        M->data[k]   = A->data[k];
        M->data[k+1] = A->data[k+1];
        M->data[k+2] = A->data[k+2];
        M->data[k+3] = A->data[k+3];
    }
    for (; k < total_elements; k++) {
        M->data[k] = A->data[k];
    }
    
    int sign = 1; /* 记录行交换次数的奇偶性 */
    size_t *perm = (size_t*)malloc(n * sizeof(size_t));
    if (!perm) { memstack_free_all(&local_ms); return ERR_OOM; }
    
    /* 初始化排列数组 */
    for (size_t i = 0; i < n; i++) {
        perm[i] = i;
    }
    
    /* LU分解（复用现有优化代码） */
    for (size_t i = 0; i < n; i++) {
        /* 寻找主元 */
        size_t max_row = i;
        REAL max_val = M->data[idx(M, i, i)];
        if (max_val < 0) max_val = -max_val;
        
        for (size_t row = i + 1; row < n; row++) {
            REAL val = M->data[idx(M, row, i)];
            if (val < 0) val = -val;
            if (val > max_val) {
                max_val = val;
                max_row = row;
            }
        }
        
        /* 如果主元为0，矩阵不可逆 */
        if (max_val < 1e-12) {
            free(perm);
            memstack_free_all(&local_ms);
            return ERR_NOT_INVERTIBLE;
        }
        
        /* 交换行（如果需要） */
        if (max_row != i) {
            /* 循环展开优化行交换 */
            for (size_t col = 0; col + 3 < n; col += 4) {
                REAL temp = M->data[idx(M, i, col)];
                M->data[idx(M, i, col)] = M->data[idx(M, max_row, col)];
                M->data[idx(M, max_row, col)] = temp;
                
                temp = M->data[idx(M, i, col+1)];
                M->data[idx(M, i, col+1)] = M->data[idx(M, max_row, col+1)];
                M->data[idx(M, max_row, col+1)] = temp;
                
                temp = M->data[idx(M, i, col+2)];
                M->data[idx(M, i, col+2)] = M->data[idx(M, max_row, col+2)];
                M->data[idx(M, max_row, col+2)] = temp;
                
                temp = M->data[idx(M, i, col+3)];
                M->data[idx(M, i, col+3)] = M->data[idx(M, max_row, col+3)];
                M->data[idx(M, max_row, col+3)] = temp;
            }
            for (size_t col = n - (n % 4); col < n; col++) {
                REAL temp = M->data[idx(M, i, col)];
                M->data[idx(M, i, col)] = M->data[idx(M, max_row, col)];
                M->data[idx(M, max_row, col)] = temp;
            }
            
            /* 交换排列数组 */
            size_t temp_perm = perm[i];
            perm[i] = perm[max_row];
            perm[max_row] = temp_perm;
            
            sign = -sign;
        }
        
        /* 计算L和U矩阵 */
        REAL pivot = M->data[idx(M, i, i)];
        for (size_t row = i + 1; row < n; row++) {
            REAL factor = M->data[idx(M, row, i)] / pivot;
            M->data[idx(M, row, i)] = factor; /* 存储L矩阵元素 */
            
            /* 更新U矩阵元素 */
            for (size_t col = i + 1; col + 3 < n; col += 4) {
                M->data[idx(M, row, col)]   -= factor * M->data[idx(M, i, col)];
                M->data[idx(M, row, col+1)] -= factor * M->data[idx(M, i, col+1)];
                M->data[idx(M, row, col+2)] -= factor * M->data[idx(M, i, col+2)];
                M->data[idx(M, row, col+3)] -= factor * M->data[idx(M, i, col+3)];
            }
            for (size_t col = n - (n % 4); col < n; col++) {
                M->data[idx(M, row, col)] -= factor * M->data[idx(M, i, col)];
            }
        }
    }
    
    /* 创建逆矩阵 */
    MATRIX *R = matrix_create(n, n, NULL, ms);
    if (!R) { free(perm); memstack_free_all(&local_ms); return ERR_OOM; }
    
    /* 求解线性方程组 A * X = I */
    for (size_t col = 0; col < n; col++) {
        /* 创建右端向量（单位矩阵的列） */
        REAL *b = (REAL*)calloc(n, sizeof(REAL));
        if (!b) { free(perm); memstack_free_all(&local_ms); return ERR_OOM; }
        b[perm[col]] = 1.0; /* 考虑行交换 */
        
        /* 前向替换求解 Ly = b */
        REAL *y = (REAL*)calloc(n, sizeof(REAL));
        if (!y) { free(b); free(perm); memstack_free_all(&local_ms); return ERR_OOM; }
        
        for (size_t i = 0; i < n; i++) {
            y[i] = b[i];
            for (size_t j = 0; j < i; j++) {
                y[i] -= M->data[idx(M, i, j)] * y[j];
            }
        }
        
        /* 后向替换求解 Ux = y */
        for (size_t i = n; i-- > 0; ) {
            REAL sum = y[i];
            for (size_t j = i + 1; j < n; j++) {
                sum -= M->data[idx(M, i, j)] * R->data[idx(R, j, col)];
            }
            R->data[idx(R, i, col)] = sum / M->data[idx(M, i, i)];
        }
        
        free(b);
        free(y);
    }
    
    free(perm);
    memstack_free_all(&local_ms);
    *inv = R;
    return ERR_OK;
}

/**
 * @brief 计算逆矩阵（兼容接口，使用伴随矩阵法）
 * @param A 方阵
 * @param inv 逆矩阵指针的指针
 * @param ms 内存栈指针
 * @return 错误码
 * @details 保持向后兼容，调用matrix_inverse_adjugate
 */
ERROR_ID matrix_inverse(_IN MATRIX *A, _OUT MATRIX **inv, MEMSTACK *ms) {
    return matrix_inverse_adjugate(A, inv, ms);
}
