/* memstack.c
 * 内存分配栈实现
 * 提供统一的内存管理机制，便于批量释放内存
 */

#include "memstack.h"
#include <stdlib.h>

/**
 * @brief 初始化内存栈
 * @param s 内存栈指针
 * @details 将栈顶指针置为NULL，准备接收新的内存指针
 *          如果传入NULL指针则直接返回
 */
void memstack_init(MEMSTACK *s) {
    if (!s) return;
    s->top = NULL;
}

/**
 * @brief 将指针压入内存栈
 * @param s 内存栈指针
 * @param p 要存储的指针
 * @return 错误码，ERR_OK表示成功，ERR_INVALID_ARG表示参数无效，ERR_OOM表示内存不足
 * @details 创建新节点并将指针存入栈顶，采用头插法
 *          如果内存栈或指针为NULL，返回错误码
 *          如果内存分配失败，返回ERR_OOM
 */
ERROR_ID memstack_push(MEMSTACK *s, void *p) {
    if (!s || !p) return ERR_INVALID_ARG;
    MemNode *n = (MemNode*)malloc(sizeof(MemNode));
    if (!n) return ERR_OOM;
    n->ptr = p;
    n->next = s->top;
    s->top = n;
    return ERR_OK;
}

/**
 * @brief 从栈中移除特定指针
 * @param s 内存栈指针
 * @param p 要移除的指针
 * @return 被移除的指针，NULL表示未找到或参数无效
 * @details 从栈中查找并移除指定指针，但不释放该指针指向的内存
 *          采用遍历查找方式，找到后从链表中移除对应节点
 *          如果内存栈或指针为NULL，返回NULL
 */
void *memstack_remove(MEMSTACK *s, void *p) {
    if (!s || !p) return NULL;
    MemNode *cur = s->top, *prev = NULL;
    while (cur) {
        if (cur->ptr == p) {
            if (prev) prev->next = cur->next;
            else s->top = cur->next;
            void *ret = cur->ptr;
            free(cur);
            return ret;
        }
        prev = cur;
        cur = cur->next;
    }
    return NULL;
}

/**
 * @brief 释放内存栈中所有指针
 * @param s 内存栈指针
 * @details 释放栈中所有记录的指针及其节点，最后将栈顶置空
 *          遍历整个链表，先释放每个节点记录的指针，再释放节点本身
 *          如果内存栈为NULL则直接返回
 */
void memstack_free_all(MEMSTACK *s) {
    if (!s) return;
    MemNode *cur = s->top;
    while (cur) {
        free(cur->ptr);
        MemNode *t = cur;
        cur = cur->next;
        free(t);
    }
    s->top = NULL;
}