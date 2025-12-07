/* main.c
   矩阵计算库测试程序 - 实现四大基础功能测试
 */

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "types.h"
#include "memstack.h"
#include "matrix.h"

/* 性能测试函数 */
void performance_test(void) {
    printf("\n========== 性能测试 ==========\n");
    
    size_t sizes[] = {100, 500, 1000, 10000};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    
    for (int i = 0; i < num_sizes; i++) {
        size_t n = sizes[i];
        printf("\n测试 %zux%zu 矩阵...\n", n, n);
        
        MEMSTACK ms;
        memstack_init(&ms);
        ERROR_ID e;
        
        /* 创建测试矩阵 */
        MATRIX *A = matrix_create(n, n, &e, &ms);
        MATRIX *B = matrix_create(n, n, &e, &ms);
        
        if (!A || !B) {
            printf("矩阵创建失败\n");
            memstack_free_all(&ms);
            continue;
        }
        
        /* 初始化矩阵元素 */
        for (size_t i = 0; i < n; i++) {
            for (size_t j = 0; j < n; j++) {
                matrix_set(A, i, j, (double)rand() / RAND_MAX);
                matrix_set(B, i, j, (double)rand() / RAND_MAX);
            }
        }
        
        clock_t start, end;
        double cpu_time;
        
        /* 测试矩阵加法 */
        start = clock();
        MATRIX *C_add = NULL;
        e = matrix_add(A, B, &C_add, &ms);
        end = clock();
        cpu_time = ((double)(end - start)) / CLOCKS_PER_SEC;
        printf("矩阵加法: %.4f 秒\n", cpu_time);
        
        /* 测试矩阵标量乘法 */
        start = clock();
        MATRIX *C_scalar = NULL;
        e = matrix_scalar_mul(A, 2.0, &C_scalar, &ms);
        end = clock();
        cpu_time = ((double)(end - start)) / CLOCKS_PER_SEC;
        printf("矩阵标量乘法: %.4f 秒\n", cpu_time);
        
        /* 测试矩阵转置 */
        start = clock();
        MATRIX *C_transpose = NULL;
        e = matrix_transpose(A, &C_transpose, &ms);
        end = clock();
        cpu_time = ((double)(end - start)) / CLOCKS_PER_SEC;
        printf("矩阵转置: %.4f 秒\n", cpu_time);
        
        /* 测试矩阵乘法 */
        start = clock();
        MATRIX *C_multiply = NULL;
        e = matrix_multiply(A, B, &C_multiply, &ms);
        end = clock();
        cpu_time = ((double)(end - start)) / CLOCKS_PER_SEC;
        printf("矩阵乘法: %.4f 秒\n", cpu_time);
        
        memstack_free_all(&ms);
    }
}

/* 基础功能测试 */
void basic_function_test(void) {
    printf("========== 基础功能测试 ==========\n");
    
    MEMSTACK ms;
    memstack_init(&ms);
    ERROR_ID e;
    
    /* 创建测试矩阵 */
    size_t n = 3;
    MATRIX *A = matrix_create(n, n, &e, &ms);
    MATRIX *B = matrix_create(n, n, &e, &ms);
    
    if (!A || !B) {
        printf("矩阵创建失败\n");
        return;
    }
    
    /* 初始化矩阵A */
    printf("矩阵 A:\n");
    double values_A[3][3] = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            matrix_set(A, i, j, values_A[i][j]);
        }
    }
    matrix_print(A);
    
    /* 初始化矩阵B */
    printf("\n矩阵 B:\n");
    double values_B[3][3] = {{9, 8, 7}, {6, 5, 4}, {3, 2, 1}};
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            matrix_set(B, i, j, values_B[i][j]);
        }
    }
    matrix_print(B);
    
    /* 测试矩阵加法 */
    printf("\n矩阵加法 A + B:\n");
    MATRIX *C_add = NULL;
    e = matrix_add(A, B, &C_add, &ms);
    if (e == ERR_OK) {
        matrix_print(C_add);
    } else {
        printf("计算失败，错误码 %u\n", e);
    }
    
    /* 测试矩阵标量乘法 */
    printf("\n矩阵标量乘法 A * 2:\n");
    MATRIX *C_scalar = NULL;
    e = matrix_scalar_mul(A, 2.0, &C_scalar, &ms);
    if (e == ERR_OK) {
        matrix_print(C_scalar);
    } else {
        printf("计算失败，错误码 %u\n", e);
    }
    
    /* 测试矩阵转置 */
    printf("\n矩阵转置 A^T:\n");
    MATRIX *C_transpose = NULL;
    e = matrix_transpose(A, &C_transpose, &ms);
    if (e == ERR_OK) {
        matrix_print(C_transpose);
    } else {
        printf("计算失败，错误码 %u\n", e);
    }
    
    /* 测试矩阵乘法 */
    printf("\n矩阵乘法 A × B:\n");
    MATRIX *C_multiply = NULL;
    e = matrix_multiply(A, B, &C_multiply, &ms);
    if (e == ERR_OK) {
        matrix_print(C_multiply);
    } else {
        printf("计算失败，错误码 %u\n", e);
    }
    
    memstack_free_all(&ms);
}

int main(void) {
    /* 初始化随机数种子 */
    srand(time(NULL));
    
    /* 测试基础功能 */
    basic_function_test();
    
    /* 测试性能 */
    performance_test();
    
    return 0;
}
