#pragma once
#include <gxm.h>
#include <kernel.h>
#include <stdlib.h>
#include <vectormath.h>
using namespace sce::Vectormath::Simd::Aos;

/*	Helper macro to align a value */
#define ALIGN(x, a) (((x) + ((a)-1)) & ~((a)-1))

/* Alloc used by libgxm */
void* graphicsAlloc(SceKernelMemBlockType type, uint32_t size,
                    uint32_t alignment, uint32_t attribs, SceUID* uid);

bool isEqual(Quat& v1, Quat& v2);