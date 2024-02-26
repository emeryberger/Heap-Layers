#ifndef HL_ARCH_H
#define HL_ARCH_H


#if __APPLE__
#include <TargetConditionals.h>
#if TARGET_CPU_ARM64
#define HL_APPLE_SILICON
#endif
#endif


#endif
