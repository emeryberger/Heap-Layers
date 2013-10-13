#ifndef X86JUMP_H
#define X86JUMP_H

#include <new>
#include <stdint.h>

#ifdef _MSC_VER
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop) )
#else
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif


PACK(
struct X86Jump32 {
    volatile uint8_t jmp_opcode;
    volatile uint32_t jmp_offset;

    X86Jump32(void *target) {
        jmp_opcode = 0xE9;
        jmp_offset = (uint32_t)((intptr_t)target - (intptr_t)this) - sizeof(struct X86Jump32);
    }
});

PACK(
struct X86Jump64 {
    volatile uint32_t sub_8_rsp;
    volatile uint32_t mov_imm_0rsp;
    volatile uint32_t target_low;
    volatile uint32_t mov_imm_4rsp;
    volatile uint32_t target_high;
    volatile uint8_t retq;

    X86Jump64(void *target) {
        /* x86_64 doesn't have an immediate 64 bit jump, so build one:
         *  1. Move down 8 bytes on the stack
         *  2. Put the target address on the stack in 32 bit chunks
         *  3. Return
         */
        sub_8_rsp = 0x08EC8348;     // move the stack pointer down 8 bytes
        mov_imm_0rsp = 0x002444C7;  // move an immediate to 0(%rsp)
        target_low = (uint32_t)(int64_t)target;
        mov_imm_4rsp = 0x042444C7;  // move an immediate to 4(%rsp)
        target_high = (uint32_t)((int64_t)target >> 32);
        retq = 0xC3;
    }
});

PACK(
struct X86_64Jump {
    union {
        uint8_t jmp32[sizeof(X86Jump32)];
        uint8_t jmp64[sizeof(X86Jump64)];
    };
    
    X86_64Jump(void *target) {
        if((uintptr_t)target - (uintptr_t)this <= 0x00000000FFFFFFFFu || (uintptr_t)this - (uintptr_t)target <= 0x00000000FFFFFFFFu) {
            new(this) X86Jump32(target);
        } else {
            new(this) X86Jump64(target);
        }
    }
});

#if defined(__LP64__) || defined(_LP64) || defined(__APPLE__) || defined(_WIN64) || defined(__x86_64__)
typedef X86_64Jump X86Jump;
#else 
typedef X86Jump32 X86Jump;
#endif

#endif
