#include <stdio.h>
#include <cstdlib>
#include <assert.h>
#include <kernel.h>
#include <rtc.h>

/* User main thread parameters */
//extern const charsceUserMainThreadName[] = "test thread";
//extern const intsceUserMainThreadPriority = SCE_KERNEL_DEFAULT_PRIORITY_USER;
//extern const unsigned intsceUserMainThreadStackSize = SCE_KERNEL_STACK_SIZE_DEFAULT_USER_MAIN;


int main() {
	const unsigned int memSize = 5 * 1024 * 1024;

	SceUID id1 = sceKernelAllocMemBlock("cached", SCE_KERNEL_MEMBLOCK_TYPE_USER_RW, memSize, NULL);
	SceUID id2 = sceKernelAllocMemBlock("uncached", SCE_KERNEL_MEMBLOCK_TYPE_USER_RW_UNCACHE, memSize, NULL);

	char* ptr1, *ptr2;

	sceKernelGetMemBlockBase(id1, (void**)&ptr1);
	sceKernelGetMemBlockBase(id2, (void**)&ptr2);

	SceRtcTick timeA, timeB, timeC;

	sceRtcGetCurrentTick(&timeA);

	for (size_t i = 0; i < memSize; ++i) {
		ptr1[i] = 0xfe;
	}
	char result = 0xff; // So compiler doesn't optimize too hard
	for (size_t i = 0; i < memSize; ++i) {
		result |= ptr1[i];
	}

	sceRtcGetCurrentTick(&timeB);

	for (size_t i = 0; i < memSize; ++i) {
		ptr2[i] = 0xfe;
	}
	result = 0xff; // So compiler doesn't optimize too hard
	for (size_t i = 0; i < memSize; ++i) {
		result |= ptr2[i];
	}

	sceRtcGetCurrentTick(&timeC);

	int firstDiff = (int)(timeB.tick & 0xffffffff) - (int)(timeA.tick & 0xffffffff);
	int secondDiff = (int)(timeC.tick & 0xffffffff) - (int)(timeB.tick & 0xffffffff);

	printf("First: %d, Second: %d\n", firstDiff, secondDiff);

	return 0;
}