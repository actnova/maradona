//- Copyright (c) 2010 James Grenning and Contributed to Unity Project
/* ==========================================
    Unity Project - A Test Framework for C
    Copyright (c) 2007 Mike Karlesky, Mark VanderVoord, Greg Williams
    [Released under MIT License. Please refer to license.txt for details]
========================================== */

#ifndef UNITY_FIXTURE_MALLOC_OVERRIDES_H_
#define UNITY_FIXTURE_MALLOC_OVERRIDES_H_

#include <stdlib.h>

#define malloc  unity_malloc
#define calloc  unity_calloc
#define realloc unity_realloc
#define free    unity_free

typedef struct GuardBytes
{
    size_t size;
    char guard[sizeof(size_t)];
} Guard;

void *unity_malloc(size_t size);
void *unity_calloc(size_t num, size_t size);
void *unity_realloc(void *oldMem, size_t size);
void  unity_free(void *mem);

#define TEST_ASSERT_MEMSIZE(expected, mem)			TEST_ASSERT_EQUAL(expected, ((Guard*)(((uint32_t)mem) - sizeof(Guard)))->size)

#endif /* UNITY_FIXTURE_MALLOC_OVERRIDES_H_ */
