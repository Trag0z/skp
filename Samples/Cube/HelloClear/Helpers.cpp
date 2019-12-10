#pragma once
#include "Helpers.h"

void *graphicsAlloc(SceKernelMemBlockType type, uint32_t size,
                    uint32_t alignment, uint32_t attribs, SceUID *uid) {
    /*	Since we are using sceKernelAllocMemBlock directly, we cannot directly
            use the alignment parameter.  Instead, we must allocate the size to
       the minimum for this memblock type, and just SCE_DBG_ALWAYS_ASSERT that
       this will cover our desired alignment.

            Developers using their own heaps should be able to use the alignment
            parameter directly for more minimal padding.
    */

    if (type == SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RWDATA) {
        /* CDRAM memblocks must be 256KiB aligned */
        SCE_DBG_ALWAYS_ASSERT(alignment <= 256 * 1024);
        size = ALIGN(size, 256 * 1024);
    } else {
        /* LPDDR memblocks must be 4KiB aligned */
        SCE_DBG_ALWAYS_ASSERT(alignment <= 4 * 1024);
        size = ALIGN(size, 4 * 1024);
    }

    /* allocate some memory */
    *uid = sceKernelAllocMemBlock("simple", type, size, NULL);
    SCE_DBG_ALWAYS_ASSERT(*uid >= SCE_OK);

    /* grab the base address */
    void *mem = NULL;
    int err = sceKernelGetMemBlockBase(*uid, &mem);
    SCE_DBG_ALWAYS_ASSERT(err == SCE_OK);

    /* map for the GPU */
    err = sceGxmMapMemory(mem, size, attribs);
    SCE_DBG_ALWAYS_ASSERT(err == SCE_OK);

    /* done */
    return mem;
}

void loadTexture(SceGxmTexture *texture, const char *filename) {
    // NOTE: Dangling file pointers here?
    SceUID fileID = sceIoOpen(filename, SCE_O_RDONLY, SCE_STM_RU);
    SceOff fileSize = sceIoLseek(fileID, 0, SCE_SEEK_END);
    sceIoLseek(fileID, 0, SCE_SEEK_SET);


    // Why this randomly allocated memory that never gets freed?
    void *dummyMem = malloc(fileSize);
	sceIoRead(fileID, dummyMem, fileSize);

    const void *dataSrc = sceGxtGetDataAddress(dummyMem);
    const uint32_t dataSize = sceGxtGetDataSize(dummyMem);
    SceUID texID;
    void *texPtr = graphicsAlloc(SCE_KERNEL_MEMBLOCK_TYPE_USER_RW_UNCACHE,
                                 dataSize, SCE_GXM_TEXTURE_ALIGNMENT,
                                 SCE_GXM_MEMORY_ATTRIB_READ, &texID);

    memcpy(texPtr, dataSrc, dataSize);
    sceGxtInitTexture(texture, texPtr, dataSrc, 0);
};
