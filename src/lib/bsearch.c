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

#include <string.h>

#include "bsearch.h"
#include "helper.h"

static int64_t matchlen(const uint8_t *old, int64_t old_sz, const uint8_t *new,
                        int64_t new_sz) {
  int64_t i, min;
  min = MIN(old_sz, new_sz);

  for (i = 0; i < min; i++) {
    if (old[i] != new[i]) {
      break;
    }
  }

  return i;
}

int64_t bsearch(const int64_t *I, const uint8_t *old, int64_t old_sz,
                const uint8_t *new, int64_t new_sz, int64_t beg, int64_t end,
                int64_t *pos) {
  int64_t x, y;

  if (end - beg < 2) {
    x = matchlen(old + I[beg], old_sz - I[beg], new, new_sz);
    y = matchlen(old + I[end], old_sz - I[end], new, new_sz);

    if (x > y) {
      *pos = I[beg];
      return x;
    } else {
      *pos = I[end];
      return y;
    }
  }

  x = beg + (end - beg) / 2;
  if (memcmp(old + I[x], new, MIN(old_sz - I[x], new_sz)) < 0) {
    return bsearch(I, old, old_sz, new, new_sz, x, end, pos);
  } else {
    return bsearch(I, old, old_sz, new, new_sz, beg, x, pos);
  }
}
