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

#include "qsufsort.h"

static void split(int64_t *I, int64_t *V, int64_t start, int64_t len,
                  int64_t h) {
  int64_t i, j, k, x, tmp, jj, kk;

  if (len < 16) {
    for (k = start; k < start + len; k += j) {
      j = 1;
      x = V[I[k] + h];
      for (i = 1; k + i < start + len; i++) {
        if (V[I[k + i] + h] < x) {
          x = V[I[k + i] + h];
          j = 0;
        };
        if (V[I[k + i] + h] == x) {
          tmp = I[k + j];
          I[k + j] = I[k + i];
          I[k + i] = tmp;
          j++;
        };
      };
      for (i = 0; i < j; i++)
        V[I[k + i]] = k + j - 1;
      if (j == 1)
        I[k] = -1;
    };
    return;
  };

  x = V[I[start + len / 2] + h];
  jj = 0;
  kk = 0;
  for (i = start; i < start + len; i++) {
    if (V[I[i] + h] < x)
      jj++;
    if (V[I[i] + h] == x)
      kk++;
  };
  jj += start;
  kk += jj;

  i = start;
  j = 0;
  k = 0;
  while (i < jj) {
    if (V[I[i] + h] < x) {
      i++;
    } else if (V[I[i] + h] == x) {
      tmp = I[i];
      I[i] = I[jj + j];
      I[jj + j] = tmp;
      j++;
    } else {
      tmp = I[i];
      I[i] = I[kk + k];
      I[kk + k] = tmp;
      k++;
    };
  };

  while (jj + j < kk) {
    if (V[I[jj + j] + h] == x) {
      j++;
    } else {
      tmp = I[jj + j];
      I[jj + j] = I[kk + k];
      I[kk + k] = tmp;
      k++;
    };
  };

  if (jj > start)
    split(I, V, start, jj - start, h);

  for (i = 0; i < kk - jj; i++)
    V[I[jj + i]] = kk - 1;
  if (jj == kk - 1)
    I[jj] = -1;

  if (start + len > kk)
    split(I, V, kk, start + len - kk, h);
}

void qsufsort(int64_t *I, int64_t *V, const uint8_t *old, int64_t old_sz) {
  int64_t buckets[256];
  int64_t i, h, len;

  for (i = 0; i < 256; i++)
    buckets[i] = 0;
  for (i = 0; i < old_sz; i++)
    buckets[old[i]]++;
  for (i = 1; i < 256; i++)
    buckets[i] += buckets[i - 1];
  for (i = 255; i > 0; i--)
    buckets[i] = buckets[i - 1];
  buckets[0] = 0;

  for (i = 0; i < old_sz; i++)
    I[++buckets[old[i]]] = i;
  I[0] = old_sz;
  for (i = 0; i < old_sz; i++)
    V[i] = buckets[old[i]];
  V[old_sz] = 0;
  for (i = 1; i < 256; i++)
    if (buckets[i] == buckets[i - 1] + 1)
      I[buckets[i]] = -1;
  I[0] = -1;

  for (h = 1; I[0] != -(old_sz + 1); h += h) {
    len = 0;
    for (i = 0; i < old_sz + 1;) {
      if (I[i] < 0) {
        len -= I[i];
        i -= I[i];
      } else {
        if (len)
          I[i - len] = -len;
        len = V[I[i]] + 1 - i;
        split(I, V, i, len, h);
        i += len;
        len = 0;
      };
    };
    if (len)
      I[i - len] = -len;
  };

  for (i = 0; i < old_sz + 1; i++)
    I[V[i]] = i;
}
