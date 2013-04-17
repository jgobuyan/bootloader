/*
 * Copyright (c) 2007, Cameron Rich
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, 
 *   this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice, 
 *   this list of conditions and the following disclaimer in the documentation 
 *   and/or other materials provided with the distribution.
 * * Neither the name of the axTLS project nor the names of its contributors 
 *   may be used to endorse or promote products derived from this software 
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file os_port.c
 *
 * OS specific functions.
 * This port is for the STM32 secure bootloader not running under an OS.
 * A simple malloc/free implementation from CCAN is used.
 */
#include <stdlib.h>
#include <string.h>
#include "os_port.h"
#include "alloc.h"

#define ALLOC_POOL_SIZE 16384
unsigned char AllocPool[ALLOC_POOL_SIZE];
void malloc_init(void)
{
    alloc_init(AllocPool, ALLOC_POOL_SIZE);
}

/* 
 * Some functions that call display some error trace and then call abort().
 * This just makes life much easier on embedded systems, since we're 
 * suffering major trauma...
 */
EXP_FUNC void * STDCALL ax_malloc(size_t s)
{
    void *x;

    x = alloc_get(AllocPool, ALLOC_POOL_SIZE, s, sizeof(int));

    return x;
}

EXP_FUNC void * STDCALL ax_realloc(void *y, size_t s)
{
    void *x;

    x = alloc_get(AllocPool, ALLOC_POOL_SIZE, s, sizeof(int));
    memcpy (x, y, s);
    alloc_free(AllocPool, ALLOC_POOL_SIZE, y);
    return x;

}

EXP_FUNC void * STDCALL ax_calloc(size_t n, size_t s)
{
    void *x;

    x = alloc_get(AllocPool, ALLOC_POOL_SIZE, n * s, sizeof(int));
    if (x)
    {
        memset(x, 0, n * s);
    }
    return x;
}

EXP_FUNC void STDCALL ax_free(void *x)
{

    alloc_free(AllocPool, ALLOC_POOL_SIZE, x);

}
