/* memstack.h
 * 内存分配栈头文件
 * 提供统一的内存管理机制，便于批量释放内存
 */

#ifndef MEMSTACK_H
#define MEMSTACK_H

#include "types.h"

/**
 * @brief 内存节点结构体
 * @details 用于构建内存栈的链表节点
 */
typedef struct MemNode {
    void *ptr;              /**< 存储的指针 */
    struct MemNode *next;   /**< 下一个节点指针 */
} MemNode;

/**
 * @brief 内存栈结构体
 * @details 管理动态分配的内存指针，支持统一释放
 */
typedef struct {
    MemNode *top;   /**< 栈顶指针 */
} MEMSTACK;

/**
 * @brief 初始化内存栈
 * @param s 内存栈指针
 * @details 将栈顶指针置为NULL，准备接收新的内存指针
 */
void memstack_init(MEMSTACK *s);

/**
 * @brief 将指针压入内存栈
 * @param s 内存栈指针
 * @param p 要存储的指针
 * @return 错误码，ERR_OK表示成功
 * @details 创建新节点并将指针存入栈顶
 */
ERROR_ID memstack_push(MEMSTACK *s, void *p);

/**
 * @brief 从栈中移除特定指针
 * @param s 内存栈指针
 * @param p 要移除的指针
 * @return 被移除的指针，NULL表示未找到
 * @details 从栈中查找并移除指定指针，但不释放该指针指向的内存
 */
void *memstack_remove(MEMSTACK *s, void *p);

/**
 * @brief 释放内存栈中所有指针
 * @param s 内存栈指针
 * @details 释放栈中所有记录的指针及其节点，最后将栈顶置空
 */
void memstack_free_all(MEMSTACK *s);

#endif /* MEMSTACK_H */