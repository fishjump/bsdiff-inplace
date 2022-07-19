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

#ifndef __BSDIFF_ARRAY_LIKE_ADAPTER_H__
#define __BSDIFF_ARRAY_LIKE_ADAPTER_H__

#include <stddef.h>
#include <stdint.h>

#include "../adapter.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct array_like {
  uint8_t *arr;
  size_t sz;
} array_like_t;

__attribute__((weak)) size_t array_like_read(const bsdiff_array_like_t *arr,
                                             size_t offset, void *buffer,
                                             size_t size) {
  const array_like_t *data;
  size_t i;

  data = (const array_like_t *)arr->opaque;

  for (i = 0; i < size; i++) {
    ((uint8_t *)buffer)[i] = data->arr[offset + i];
  }

  return size;
}

__attribute__((weak)) size_t array_like_write(bsdiff_array_like_t *arr,
                                              size_t offset, void *buffer,
                                              size_t size) {
  array_like_t *data;
  size_t i;

  data = (array_like_t *)arr->opaque;

  for (i = 0; i < size; i++) {
    data->arr[offset + i] = ((uint8_t *)buffer)[i];
  }

  return size;
}

__attribute__((weak)) size_t array_like_len(bsdiff_array_like_t *arr) {
  const array_like_t *data;

  data = (const array_like_t *)arr->opaque;

  return data->sz;
}

__attribute__((weak)) void make_array_like(array_like_t *array_like, void *arr,
                                           size_t sz) {
  array_like->arr = arr;
  array_like->sz = sz;
}

__attribute__((weak)) void make_array_like_adapter(bsdiff_array_like_t *arr,
                                                   array_like_t *array_like) {
  arr->opaque = array_like;
  arr->len = array_like_len;
  arr->write = array_like_write;
  arr->read = array_like_read;
}

#ifdef __cplusplus
}
#endif

#endif // __BSDIFF_ARRAY_LIKE_ADAPTER_H__