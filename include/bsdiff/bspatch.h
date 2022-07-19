/*-
 * Copyright 2003-2005 Colin Percival
 * Copyright 2012 Matthew Endsley
 * Copyright 2022 Yue Yu
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted providing that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __BSDIFF_BSPATCH_H__
#define __BSDIFF_BSPATCH_H__

#include "adapter.h"
#include "patch_format.h"

#define BSPATCH_SUCCESS 0
#define BSPATCH_READ_PATCH_ERR 1
#define BSPATCH_READ_OLD_ERR 2
#define BSPATCH_WRITE_OLD_ERR 3
#define BSPATCH_SIGNATURE_INCONSISTENCY_ERR 4
#define BSPATCH_SANITY_CHECK_ERR 5
#define BSPATCH_DECOMPRESS_ERR 6

#ifdef __cplusplus
extern "C" {
#endif

int bspatch(bsdiff_array_like_t *old, const bsdiff_stream_t *patch,
            size_t *new_size);

#ifdef __cplusplus
}
#endif

#endif // __BSDIFF_BSPATCH_H__
