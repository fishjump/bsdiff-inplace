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

#ifndef __BSDIFF_ADAPTER_H__
#define __BSDIFF_ADAPTER_H__

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct bsdiff_array_like bsdiff_array_like_t;
typedef size_t (*fd_read_func_t)(const bsdiff_array_like_t *arr, size_t offset,
                                 void *buffer, size_t size);
typedef size_t (*fd_write_func_t)(bsdiff_array_like_t *arr, size_t offset,
                                  void *buffer, size_t size);
typedef size_t (*fd_len_func_t)(bsdiff_array_like_t *arr);

struct bsdiff_array_like {
  void *opaque;
  fd_read_func_t read;
  fd_write_func_t write;
  fd_len_func_t len;
};

typedef struct bsdiff_stream bsdiff_stream_t;
typedef size_t (*fs_read_func_t)(const bsdiff_stream_t *fs, void *buffer,
                                 size_t size);
typedef size_t (*fs_write_func_t)(bsdiff_stream_t *fs, void *buffer,
                                  size_t size);

struct bsdiff_stream {
  void *opaque;
  fs_read_func_t read;
  fs_write_func_t write;
};

#ifdef __cplusplus
}
#endif

#endif // __BSDIFF_ADAPTER_H__