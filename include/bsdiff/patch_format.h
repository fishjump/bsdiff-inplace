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

#ifndef __BSDIFF_PATCH_FORMAT_H__
#define __BSDIFF_PATCH_FORMAT_H__

#include <stddef.h>
#include <stdint.h>

#define BSDIFF_SIGNATURE "YUEYU/BSDIFF"
#define BSDIFF_SIGNATURE_LEN (sizeof(BSDIFF_SIGNATURE) - 1) /* -1 for '\0' */

#define PATCH_BLK_SZ(BLK_PTR)                                                  \
  (sizeof(*(BLK_PTR)) + (BLK_PTR)->len_diff + (BLK_PTR)->len_extra)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct bsdiff_header {
  char signature[BSDIFF_SIGNATURE_LEN];
  uint64_t new_sz;
} __attribute__((packed)) bsdiff_header_t;

typedef struct patch_block {
  uint64_t len_diff;  // read len_diff bytes as diff
  uint64_t len_extra; // read len_extra bytes as extra
  uint64_t len_skip;  // skip len_skip bytes in old file
  uint8_t data[0];    // the actual size of data is len_diff+len_extra
} __attribute__((packed)) patch_block_t;

/**
 * Format of data:
 * +-------------------------+
 * | size of compressed data |
 * +-------------------------+
 * | end flag                |
 * +-------------------------+
 * | compressed              |
 * +-------------------------+
 */

#ifdef __cplusplus
}
#endif

#endif // __BSDIFF_PATCH_FORMAT_H__