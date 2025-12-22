/* main.c
   矩阵计算库测试程序 - 实现四大基础功能测试
 */

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "types.h"
#include "memstack.h"
#include "matrix.h"

/* 性能测试函数 - 多次测试并计算平均值 */
void performance_test(void) {
    printf("\n========== 性能测试 ==========\n");
    
    /* 打开结果文件 */
    FILE *fp = fopen("results.txt", "w");
    if (!fp) {
        printf("无法创建结果文件 results.txt\n");
        return;
    }
    
    /* 打开JSON结果文件 */
    FILE *json_fp = fopen("results.json", "w");
    if (!json_fp) {
        printf("无法创建JSON结果文件 results.json\n");
        fclose(fp);
        return;
    }
    
    /* 写入JSON文件头 */
    fprintf(json_fp, "{\n");
    
    fprintf(fp, "矩阵运算性能测试结果\n");
    fprintf(fp, "====================\n\n");
    fprintf(fp, "测试环境:\n");
    fprintf(fp, "- 矩阵大小: 10, 50, 100, 500\n");
    fprintf(fp, "- 测试次数: 小矩阵测试20次, 大矩阵测试5次\n");
    fprintf(fp, "- 单位: 秒 (平均值)\n\n");
    
    size_t sizes[] = {1100, 1200, 1300, 1400, 1500, 1600, 1700, 1800, 1900, 2000};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    
    for (int size_idx = 0; size_idx < num_sizes; size_idx++) {
        size_t n = sizes[size_idx];
        printf("\n测试 %zux%zu 矩阵...\n", n, n);
        fprintf(fp, "矩阵大小: %zux%zu\n", n, n);
        fprintf(fp, "-------------------\n");
        
        /* 写入JSON对象开始 */
        if (size_idx > 0) {
            fprintf(json_fp, ",\n");
        }
        fprintf(json_fp, "  \"%zux%zu\": {\n", n, n);
        
        /* 确定测试次数：500x500矩阵测试5次，其他测试20次 */
        int num_tests = 20;
        int num_multiply_tests = 20;
        int num_inverse_tests = 20;
         
        /* 初始化累加器 */
        double total_add = 0.0, total_scalar = 0.0, total_transpose = 0.0, total_multiply = 0.0;
        double total_det_recursive = 0.0, total_det_gaussian = 0.0, total_det_lu = 0.0;
        double total_inv_adjugate = 0.0, total_inv_gauss_jordan = 0.0, total_inv_lu = 0.0;
        
        /* 创建数组存储每次测试的时间 */
        double *times_add = (double*)malloc(num_tests * sizeof(double));
        double *times_scalar = (double*)malloc(num_tests * sizeof(double));
        double *times_transpose = (double*)malloc(num_tests * sizeof(double));
        double *times_multiply = (double*)malloc(num_multiply_tests * sizeof(double));
        double *times_det_recursive = (n < 10 ) ? (double*)malloc(num_tests * sizeof(double)) : NULL;
        double *times_det_gaussian = (double*)malloc(num_tests * sizeof(double));
        double *times_det_lu = (double*)malloc(num_tests * sizeof(double));
        double *times_inv_adjugate = (n < 10 ) ? (double*)malloc(num_inverse_tests * sizeof(double)) : NULL;
        double *times_inv_gauss_jordan = (double*)malloc(num_inverse_tests * sizeof(double));
        double *times_inv_lu = (double*)malloc(num_inverse_tests * sizeof(double));
        
        for (int test = 0; test < num_tests; test++) {
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
            total_add += cpu_time;
            times_add[test] = cpu_time;
            printf("  第 %d 次 - 矩阵加法: %.4f 秒\n", test + 1, cpu_time);
            
            /* 测试矩阵标量乘法 */
            start = clock();
            MATRIX *C_scalar = NULL;
            e = matrix_scalar_mul(A, 2.0, &C_scalar, &ms);
            end = clock();
            cpu_time = ((double)(end - start)) / CLOCKS_PER_SEC;
            total_scalar += cpu_time;
            times_scalar[test] = cpu_time;
            printf("  第 %d 次 - 矩阵标量乘法: %.4f 秒\n", test + 1, cpu_time);
            
            /* 测试矩阵转置 */
            start = clock();
            MATRIX *C_transpose = NULL;
            e = matrix_transpose(A, &C_transpose, &ms);
            end = clock();
            cpu_time = ((double)(end - start)) / CLOCKS_PER_SEC;
            total_transpose += cpu_time;
            times_transpose[test] = cpu_time;
            printf("  第 %d 次 - 矩阵转置: %.4f 秒\n", test + 1, cpu_time);
            
            /* 测试矩阵乘法（根据矩阵大小决定测试次数） */
            if (test < num_multiply_tests) {
                start = clock();
                MATRIX *C_multiply = NULL;
                e = matrix_multiply(A, B, &C_multiply, &ms);
                end = clock();
                cpu_time = ((double)(end - start)) / CLOCKS_PER_SEC;
                total_multiply += cpu_time;
                times_multiply[test] = cpu_time;
                printf("  第 %d 次 - 矩阵乘法: %.4f 秒\n", test + 1, cpu_time);
            }
            
            /* 测试行列式计算 */
            REAL det_recursive = 0.0;
            if (n < 10 ) {  /* 只测试50x50及以下的矩阵 */
                start = clock();
                e = matrix_determinant_recursive(A, &det_recursive);
                end = clock();
                cpu_time = ((double)(end - start)) / CLOCKS_PER_SEC;
                total_det_recursive += cpu_time;
                times_det_recursive[test] = cpu_time;
                printf("  第 %d 次 - 行列式计算-递归展开法: %.4f 秒\n", test + 1, cpu_time);
            }
            
            REAL det_gaussian = 0.0;
            start = clock();
            e = matrix_determinant_gaussian(A, &det_gaussian);
            end = clock();
            cpu_time = ((double)(end - start)) / CLOCKS_PER_SEC;
            total_det_gaussian += cpu_time;
            times_det_gaussian[test] = cpu_time;
            printf("  第 %d 次 - 行列式计算-高斯消元法: %.4f 秒\n", test + 1, cpu_time);
            
            REAL det_lu = 0.0;
            start = clock();
            e = matrix_determinant_lu(A, &det_lu);
            end = clock();
            cpu_time = ((double)(end - start)) / CLOCKS_PER_SEC;
            total_det_lu += cpu_time;
            times_det_lu[test] = cpu_time;
            printf("  第 %d 次 - 行列式计算-LU分解法: %.4f 秒\n", test + 1, cpu_time);
            
            /* 测试逆矩阵计算（只测试较小的矩阵） */
            if (test < num_inverse_tests) {
                /* 测试伴随矩阵法 */
                if (n < 10 ) {  /* 只测试50x50及以下的矩阵 */
                    start = clock();
                    MATRIX *inv_adjugate = NULL;
                    e = matrix_inverse_adjugate(A, &inv_adjugate, &ms);
                    end = clock();
                    cpu_time = ((double)(end - start)) / CLOCKS_PER_SEC;
                    total_inv_adjugate += cpu_time;
                    times_inv_adjugate[test] = cpu_time;
                    printf("  第 %d 次 - 逆矩阵计算-伴随矩阵法: %.4f 秒\n", test + 1, cpu_time);
                }
                
                /* 测试Gauss-Jordan消元法 */
                start = clock();
                MATRIX *inv_gauss_jordan = NULL;
                e = matrix_inverse_gauss_jordan(A, &inv_gauss_jordan, &ms);
                end = clock();
                cpu_time = ((double)(end - start)) / CLOCKS_PER_SEC;
                total_inv_gauss_jordan += cpu_time;
                times_inv_gauss_jordan[test] = cpu_time;
                printf("  第 %d 次 - 逆矩阵计算-Gauss-Jordan消元法: %.4f 秒\n", test + 1, cpu_time);
                
                /* 测试LU分解法 */
                start = clock();
                MATRIX *inv_lu = NULL;
                e = matrix_inverse_lu(A, &inv_lu, &ms);
                end = clock();
                cpu_time = ((double)(end - start)) / CLOCKS_PER_SEC;
                total_inv_lu += cpu_time;
                times_inv_lu[test] = cpu_time;
                printf("  第 %d 次 - 逆矩阵计算-LU分解法: %.4f 秒\n", test + 1, cpu_time);
            }
            
            memstack_free_all(&ms);
        }
        
        /* 计算平均值 */
        double avg_add = total_add / num_tests;
        double avg_scalar = total_scalar / num_tests;
        double avg_transpose = total_transpose / num_tests;
        double avg_multiply = total_multiply / num_multiply_tests;
        
        /* 打印和保存结果 */
        printf("矩阵加法 (平均): %.4f 秒\n", avg_add);
        printf("矩阵标量乘法 (平均): %.4f 秒\n", avg_scalar);
        printf("矩阵转置 (平均): %.4f 秒\n", avg_transpose);
        printf("矩阵乘法 (平均): %.4f 秒\n", avg_multiply);
        
        fprintf(fp, "矩阵加法: %.4f 秒 (测试 %d 次)\n", avg_add, num_tests);
        fprintf(fp, "矩阵标量乘法: %.4f 秒 (测试 %d 次)\n", avg_scalar, num_tests);
        fprintf(fp, "矩阵转置: %.4f 秒 (测试 %d 次)\n", avg_transpose, num_tests);
        fprintf(fp, "矩阵乘法: %.4f 秒 (测试 %d 次)\n", avg_multiply, num_multiply_tests);
        
        /* 打印行列式计算结果 */
        fprintf(fp, "\n行列式计算性能:\n");
        if (n <= 10) {
            double avg_det_recursive = total_det_recursive / num_tests;
            printf("行列式计算-递归展开法 (平均): %.4f 秒\n", avg_det_recursive);
            fprintf(fp, "递归展开法: %.4f 秒 (测试 %d 次)\n", avg_det_recursive, num_tests);
        } else {
            fprintf(fp, "递归展开法: 未测试 (矩阵过大)\n");
        }
        
        double avg_det_gaussian = total_det_gaussian / num_tests;
        double avg_det_lu = total_det_lu / num_tests;
        
        printf("行列式计算-高斯消元法 (平均): %.4f 秒\n", avg_det_gaussian);
        printf("行列式计算-LU分解法 (平均): %.4f 秒\n", avg_det_lu);
        
        fprintf(fp, "高斯消元法: %.4f 秒 (测试 %d 次)\n", avg_det_gaussian, num_tests);
        fprintf(fp, "LU分解法: %.4f 秒 (测试 %d 次)\n", avg_det_lu, num_tests);
        
        /* 打印逆矩阵计算结果 */
        fprintf(fp, "\n逆矩阵计算性能:\n");
        if (n <= 10) {
            double avg_inv_adjugate = total_inv_adjugate / num_inverse_tests;
            printf("逆矩阵计算-伴随矩阵法 (平均): %.4f 秒\n", avg_inv_adjugate);
            fprintf(fp, "伴随矩阵法: %.4f 秒 (测试 %d 次)\n", avg_inv_adjugate, num_inverse_tests);
        } else {
            fprintf(fp, "伴随矩阵法: 未测试 (矩阵过大)\n");
        }
        
        double avg_inv_gauss_jordan = total_inv_gauss_jordan / num_inverse_tests;
        double avg_inv_lu = total_inv_lu / num_inverse_tests;
        
        printf("逆矩阵计算-Gauss-Jordan消元法 (平均): %.4f 秒\n", avg_inv_gauss_jordan);
        printf("逆矩阵计算-LU分解法 (平均): %.4f 秒\n", avg_inv_lu);
        
        fprintf(fp, "Gauss-Jordan消元法: %.4f 秒 (测试 %d 次)\n", avg_inv_gauss_jordan, num_inverse_tests);
        fprintf(fp, "LU分解法: %.4f 秒 (测试 %d 次)\n", avg_inv_lu, num_inverse_tests);
        
        fprintf(fp, "\n");
        
        /* 写入JSON数据 */
        fprintf(json_fp, "    \"matrix_addition\": {\n");
        fprintf(json_fp, "      \"average_time\": %.4f,\n", avg_add);
        fprintf(json_fp, "      \"test_count\": %d,\n", num_tests);
        fprintf(json_fp, "      \"times\": [");
        for (int i = 0; i < num_tests; i++) {
            fprintf(json_fp, "%.4f", times_add[i]);
            if (i < num_tests - 1) fprintf(json_fp, ", ");
        }
        fprintf(json_fp, "]\n    },\n");
        
        fprintf(json_fp, "    \"matrix_scalar_multiplication\": {\n");
        fprintf(json_fp, "      \"average_time\": %.4f,\n", avg_scalar);
        fprintf(json_fp, "      \"test_count\": %d,\n", num_tests);
        fprintf(json_fp, "      \"times\": [");
        for (int i = 0; i < num_tests; i++) {
            fprintf(json_fp, "%.4f", times_scalar[i]);
            if (i < num_tests - 1) fprintf(json_fp, ", ");
        }
        fprintf(json_fp, "]\n    },\n");
        
        fprintf(json_fp, "    \"matrix_transpose\": {\n");
        fprintf(json_fp, "      \"average_time\": %.4f,\n", avg_transpose);
        fprintf(json_fp, "      \"test_count\": %d,\n", num_tests);
        fprintf(json_fp, "      \"times\": [");
        for (int i = 0; i < num_tests; i++) {
            fprintf(json_fp, "%.4f", times_transpose[i]);
            if (i < num_tests - 1) fprintf(json_fp, ", ");
        }
        fprintf(json_fp, "]\n    },\n");
        
        fprintf(json_fp, "    \"matrix_multiplication\": {\n");
        fprintf(json_fp, "      \"average_time\": %.4f,\n", avg_multiply);
        fprintf(json_fp, "      \"test_count\": %d,\n", num_multiply_tests);
        fprintf(json_fp, "      \"times\": [");
        for (int i = 0; i < num_multiply_tests; i++) {
            fprintf(json_fp, "%.4f", times_multiply[i]);
            if (i < num_multiply_tests - 1) fprintf(json_fp, ", ");
        }
        fprintf(json_fp, "]\n    },\n");
        
        /* 行列式计算JSON数据 */
        fprintf(json_fp, "    \"determinant_calculation\": {\n");
        
        if (n <= 10 && times_det_recursive) {
            double avg_det_recursive = total_det_recursive / num_tests;
            fprintf(json_fp, "      \"recursive_expansion\": {\n");
            fprintf(json_fp, "        \"average_time\": %.4f,\n", avg_det_recursive);
            fprintf(json_fp, "        \"test_count\": %d,\n", num_tests);
            fprintf(json_fp, "        \"times\": [");
            for (int i = 0; i < num_tests; i++) {
                fprintf(json_fp, "%.4f", times_det_recursive[i]);
                if (i < num_tests - 1) fprintf(json_fp, ", ");
            }
            fprintf(json_fp, "]\n      },\n");
        }
        
        fprintf(json_fp, "      \"gaussian_elimination\": {\n");
        fprintf(json_fp, "        \"average_time\": %.4f,\n", avg_det_gaussian);
        fprintf(json_fp, "        \"test_count\": %d,\n", num_tests);
        fprintf(json_fp, "        \"times\": [");
        for (int i = 0; i < num_tests; i++) {
            fprintf(json_fp, "%.4f", times_det_gaussian[i]);
            if (i < num_tests - 1) fprintf(json_fp, ", ");
        }
        fprintf(json_fp, "]\n      },\n");
        
        fprintf(json_fp, "      \"lu_decomposition\": {\n");
        fprintf(json_fp, "        \"average_time\": %.4f,\n", avg_det_lu);
        fprintf(json_fp, "        \"test_count\": %d,\n", num_tests);
        fprintf(json_fp, "        \"times\": [");
        for (int i = 0; i < num_tests; i++) {
            fprintf(json_fp, "%.4f", times_det_lu[i]);
            if (i < num_tests - 1) fprintf(json_fp, ", ");
        }
        fprintf(json_fp, "]\n      }\n    },\n");
        
        /* 逆矩阵计算JSON数据 */
        fprintf(json_fp, "    \"inverse_calculation\": {\n");
        
        if (n <= 10 && times_inv_adjugate) {
            double avg_inv_adjugate = total_inv_adjugate / num_inverse_tests;
            fprintf(json_fp, "      \"adjugate_method\": {\n");
            fprintf(json_fp, "        \"average_time\": %.4f,\n", avg_inv_adjugate);
            fprintf(json_fp, "        \"test_count\": %d,\n", num_inverse_tests);
            fprintf(json_fp, "        \"times\": [");
            for (int i = 0; i < num_inverse_tests; i++) {
                fprintf(json_fp, "%.4f", times_inv_adjugate[i]);
                if (i < num_inverse_tests - 1) fprintf(json_fp, ", ");
            }
            fprintf(json_fp, "]\n      },\n");
        }
        
        fprintf(json_fp, "      \"gauss_jordan_elimination\": {\n");
        fprintf(json_fp, "        \"average_time\": %.4f,\n", avg_inv_gauss_jordan);
        fprintf(json_fp, "        \"test_count\": %d,\n", num_inverse_tests);
        fprintf(json_fp, "        \"times\": [");
        for (int i = 0; i < num_inverse_tests; i++) {
            fprintf(json_fp, "%.4f", times_inv_gauss_jordan[i]);
            if (i < num_inverse_tests - 1) fprintf(json_fp, ", ");
        }
        fprintf(json_fp, "]\n      },\n");
        
        fprintf(json_fp, "      \"lu_decomposition\": {\n");
        fprintf(json_fp, "        \"average_time\": %.4f,\n", avg_inv_lu);
        fprintf(json_fp, "        \"test_count\": %d,\n", num_inverse_tests);
        fprintf(json_fp, "        \"times\": [");
        for (int i = 0; i < num_inverse_tests; i++) {
            fprintf(json_fp, "%.4f", times_inv_lu[i]);
            if (i < num_inverse_tests - 1) fprintf(json_fp, ", ");
        }
        fprintf(json_fp, "]\n      }\n    }\n");
        
        /* 写入JSON对象结束 */
        fprintf(json_fp, "  }");
        
        /* 输出每次测试的时间数组 */
        printf("\n详细测试时间数组:\n");
        fprintf(fp, "\n详细测试时间数组:\n");
        
        /* 输出矩阵加法时间数组 */
        printf("矩阵加法: [");
        fprintf(fp, "矩阵加法: [");
        for (int i = 0; i < num_tests; i++) {
            printf("%.4f", times_add[i]);
            fprintf(fp, "%.4f", times_add[i]);
            if (i < num_tests - 1) {
                printf(", ");
                fprintf(fp, ", ");
            }
        }
        printf("]\n");
        fprintf(fp, "]\n");
        
        /* 输出矩阵标量乘法时间数组 */
        printf("矩阵标量乘法: [");
        fprintf(fp, "矩阵标量乘法: [");
        for (int i = 0; i < num_tests; i++) {
            printf("%.4f", times_scalar[i]);
            fprintf(fp, "%.4f", times_scalar[i]);
            if (i < num_tests - 1) {
                printf(", ");
                fprintf(fp, ", ");
            }
        }
        printf("]\n");
        fprintf(fp, "]\n");
        
        /* 输出矩阵转置时间数组 */
        printf("矩阵转置: [");
        fprintf(fp, "矩阵转置: [");
        for (int i = 0; i < num_tests; i++) {
            printf("%.4f", times_transpose[i]);
            fprintf(fp, "%.4f", times_transpose[i]);
            if (i < num_tests - 1) {
                printf(", ");
                fprintf(fp, ", ");
            }
        }
        printf("]\n");
        fprintf(fp, "]\n");
        
        /* 输出矩阵乘法时间数组 */
        printf("矩阵乘法: [");
        fprintf(fp, "矩阵乘法: [");
        for (int i = 0; i < num_multiply_tests; i++) {
            printf("%.4f", times_multiply[i]);
            fprintf(fp, "%.4f", times_multiply[i]);
            if (i < num_multiply_tests - 1) {
                printf(", ");
                fprintf(fp, ", ");
            }
        }
        printf("]\n");
        fprintf(fp, "]\n");
        
        /* 输出行列式计算时间数组 */
        if (n < 10  && times_det_recursive) {
            printf("行列式计算-递归展开法: [");
            fprintf(fp, "行列式计算-递归展开法: [");
            for (int i = 0; i < num_tests; i++) {
                printf("%.4f", times_det_recursive[i]);
                fprintf(fp, "%.4f", times_det_recursive[i]);
                if (i < num_tests - 1) {
                    printf(", ");
                    fprintf(fp, ", ");
                }
            }
            printf("]\n");
            fprintf(fp, "]\n");
        }
        
        printf("行列式计算-高斯消元法: [");
        fprintf(fp, "行列式计算-高斯消元法: [");
        for (int i = 0; i < num_tests; i++) {
            printf("%.4f", times_det_gaussian[i]);
            fprintf(fp, "%.4f", times_det_gaussian[i]);
            if (i < num_tests - 1) {
                printf(", ");
                fprintf(fp, ", ");
            }
        }
        printf("]\n");
        fprintf(fp, "]\n");
        
        printf("行列式计算-LU分解法: [");
        fprintf(fp, "行列式计算-LU分解法: [");
        for (int i = 0; i < num_tests; i++) {
            printf("%.4f", times_det_lu[i]);
            fprintf(fp, "%.4f", times_det_lu[i]);
            if (i < num_tests - 1) {
                printf(", ");
                fprintf(fp, ", ");
            }
        }
        printf("]\n");
        fprintf(fp, "]\n");
        
        /* 输出逆矩阵计算时间数组 */
        if (n < 10  && times_inv_adjugate) {
            printf("逆矩阵计算-伴随矩阵法: [");
            fprintf(fp, "逆矩阵计算-伴随矩阵法: [");
            for (int i = 0; i < num_inverse_tests; i++) {
                printf("%.4f", times_inv_adjugate[i]);
                fprintf(fp, "%.4f", times_inv_adjugate[i]);
                if (i < num_inverse_tests - 1) {
                    printf(", ");
                    fprintf(fp, ", ");
                }
            }
            printf("]\n");
            fprintf(fp, "]\n");
        }
        
        printf("逆矩阵计算-Gauss-Jordan消元法: [");
        fprintf(fp, "逆矩阵计算-Gauss-Jordan消元法: [");
        for (int i = 0; i < num_inverse_tests; i++) {
            printf("%.4f", times_inv_gauss_jordan[i]);
            fprintf(fp, "%.4f", times_inv_gauss_jordan[i]);
            if (i < num_inverse_tests - 1) {
                printf(", ");
                fprintf(fp, ", ");
            }
        }
        printf("]\n");
        fprintf(fp, "]\n");
        
        printf("逆矩阵计算-LU分解法: [");
        fprintf(fp, "逆矩阵计算-LU分解法: [");
        for (int i = 0; i < num_inverse_tests; i++) {
            printf("%.4f", times_inv_lu[i]);
            fprintf(fp, "%.4f", times_inv_lu[i]);
            if (i < num_inverse_tests - 1) {
                printf(", ");
                fprintf(fp, ", ");
            }
        }
        printf("]\n");
        fprintf(fp, "]\n");
        
        /* 释放内存 */
        free(times_add);
        free(times_scalar);
        free(times_transpose);
        free(times_multiply);
        if (times_det_recursive) free(times_det_recursive);
        free(times_det_gaussian);
        free(times_det_lu);
        if (times_inv_adjugate) free(times_inv_adjugate);
        free(times_inv_gauss_jordan);
        free(times_inv_lu);
    }
    
    /* 写入JSON文件尾 */
    fprintf(json_fp, "\n}\n");
    
    fclose(fp);
    fclose(json_fp);
    printf("\n测试结果已保存到 results.txt 和 results.json\n");
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
