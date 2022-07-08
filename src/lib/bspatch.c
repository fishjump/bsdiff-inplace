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

#include <limits.h>

#include <bsdiff/bspatch.h>

#include "helper.h"

int bspatch(uint8_t *old, int64_t old_sz, int64_t new_sz,
            struct bspatch_stream *stream) {
  const char magic[] = "BSDIFF";

  patch_block_t block;
  uint8_t byte;
  int64_t old_cursor, new_cursor;
  int64_t i;

  old_cursor = 0;
  new_cursor = 0;
  while (new_cursor < new_sz) {
    // Read control data
    if (stream->read(stream, &block, sizeof(block))) {
      return -1;
    }

    // Sanity-check
    if (block.len_diff < 0 || block.len_diff > INT_MAX ||   // len_diff
        block.len_extra < 0 || block.len_extra > INT_MAX || // len_extra
        new_cursor + block.len_diff > new_sz                // overflow
    ) {
      return -1;
    }

    // Read diff string and add
    for (i = 0; i < block.len_diff; i++) {
      if (stream->read(stream, &byte, 1)) {
        return -1;
      }
      if (old_cursor + i < old_sz) {
        old[new_cursor + i] = old[old_cursor + i] + byte;
      }
    }

    // Adjust pointers
    new_cursor += block.len_diff;
    old_cursor += block.len_diff;

    // Sanity-check
    if (new_cursor + block.len_extra > new_sz) {
      return -1;
    }

    // Move old for the extra string space
    for (i = old_sz - 1; i >= old_cursor; i--) {
      old[i + block.len_extra] = old[i];
    }

    // Read extra string
    if (stream->read(stream, old + new_cursor, block.len_extra)) {
      return -1;
    }

    // Adjust pointers
    new_cursor += block.len_extra;
    old_cursor += block.len_extra + block.len_skip;
  }

  return 0;
}
