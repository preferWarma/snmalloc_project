#include <cstdint>
#include <unistd.h>

using ul_t = unsigned long;

/*
块大小最小为8字节(64位操作系统下), 所以低6位可以做标志位flags
flags in prev_size:
    Free Last First
    2    1    0
*/
struct scf_mblocks_t {
    ul_t magic;     // 标识符, 避免double free
    ul_t prev_size; // 前一个块的大小, 用于形成链表
    ul_t cur_size;  // 当前块的大小
    ul_t free;      // 空闲内存形成的链表
};

// 空闲链表的头指针数组, 空闲链表的空闲块大小差为64字节,即第一条链表里每个空闲块是64字节,第二条链表每个是128字节...
extern scf_mblocks_t* scf_mblocks[30];
extern scf_mblocks_t* scf_free_blocks;
extern uint8_t* scf_last_brk; // 当前的brk位置, free完的空闲内存块的末尾==brk的时候才能使用brk()修改堆的范围

// 参数size是用户需要的字节数
inline uint8_t*
scf_malloc(ul_t size) {
    scf_mblocks_t* b;
    scf_mblocks_t* b2;

    ul_t bytes   = (sizeof(scf_mblocks_t) + size + 63) >> 6 << 6; // 实际需要的字节数, 按64字节对齐
    ul_t nblocks = sizeof(scf_mblocks) / sizeof(scf_mblocks[0]);  // 空闲链表的个数

    ul_t rest;                                                    // 空闲位置
    uint8_t* addr = nullptr;
    uint8_t* p    = nullptr;
    ul_t i;

    // 查找合适的空闲链表
    for (i = 0; i < nblocks; ++i) {
        if ((64 << i) < bytes) {
            continue;
        }
        if (scf_mblocks[i] == nullptr) {
            break;
        }
    }

    // 没有找到合适的空闲链表
    if (i == nblocks) {
        ul_t pages = (bytes + 4095) >> 12; // 以内存页为分配单位, 计算需要分配的页数
        p = static_cast<uint8_t*>(sbrk(static_cast<long>(pages << 12))); // 分配内存(单位:字节), 按内存页大小对齐
        if (!p) {
            return nullptr;
        }
        scf_last_brk = p + (pages << 12);                   // brk位置
        rest         = (pages << 12) - bytes;               // 页内存剩余的字节数
        b            = reinterpret_cast<scf_mblocks_t*>(p); // 用户的新分配的内存块的管理结构

        b->prev_size = 0x3;
        b->cur_size  = bytes;
        b->magic = 0x10f0;
    }

    return nullptr;
}
