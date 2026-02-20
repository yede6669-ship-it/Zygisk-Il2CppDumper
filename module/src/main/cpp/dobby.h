#ifndef DOBBY_H
#define DOBBY_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Dobby 符号解析器
 * @param library_name 动态库名称 (如 "libart.so")
 * @param symbol_name 符号名称 (即你从 MT 管理器复制的长字符串)
 * @return 符号在内存中的绝对地址
 */
void *DobbySymbolResolver(const char *library_name, const char *symbol_name);

/**
 * Dobby Hook 函数
 * @param address 目标函数的起始地址
 * @param replace_call 替换后的新函数地址
 * @param origin_call 用于备份原函数的指针地址
 * @return 状态码 (0 为成功)
 */
int DobbyHook(void *address, void *replace_call, void **origin_call);

#ifdef __cplusplus
}
#endif

#endif
