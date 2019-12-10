#pragma once
#include <gxm.h>
#include <gxt.h>
#include <kernel.h>
#include <libdbg.h>
#include <stdlib.h>
#include <string.h>

/*	Helper macro to align a value */
#define ALIGN(x, a) (((x) + ((a)-1)) & ~((a)-1))

/* Alloc used by libgxm */
void *graphicsAlloc(SceKernelMemBlockType type, uint32_t size,
                    uint32_t alignment, uint32_t attribs, SceUID *uid);

void loadTexture(SceGxmTexture *texture, const char *filename);
