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
#include <string.h>

#include <bsdiff/legacy/bsdiff.h>
#include <fastlz.h>

#include "bsearch.h"
#include "helper.h"
#include "qsufsort.h"

typedef struct bsdiff_request {
  bsdiff_stream_t *stream;

  const uint8_t *old;
  int64_t oldsize;
  const uint8_t *new;
  int64_t newsize;

  int64_t *sa;
  patch_block_t *block;
  uint8_t *compress_buffer;
} bsdiff_request_t;

typedef struct approximate_match {
  int64_t new_pos; // new[cursor, new_end]
  int64_t old_pos; // old[cursor, old_end]
} approximate_match_t;

typedef approximate_match_t am_t;

static int64_t forward_ext_len(const uint8_t *old, int64_t old_beg,
                               int64_t old_end, const uint8_t *new,
                               int64_t new_beg, int64_t new_end);

static int64_t backward_ext_len(const uint8_t *old, int64_t old_beg,
                                int64_t old_end, const uint8_t *new,
                                int64_t new_beg, int64_t new_end);

static am_t approximate_match(const int64_t *sa, const uint8_t *old,
                              int64_t old_sz, int64_t old_cursor,
                              const uint8_t *new, int64_t new_sz,
                              int64_t new_cursor);

static int bsdiff_internal(const bsdiff_request_t req) {
  am_t match; // approximate match result

  int64_t old_cursor, new_cursor;
  int64_t last_old_cur, last_new_cur;
  int64_t lenf, lenb;
  int64_t len_diff, len_extra, len_skip;
  int64_t i;

  uint8_t fastlz_buffer[FASTLZ_BUFFER_SIZE];

  int64_t *buffer;

  uint64_t out_sz;
  uint8_t last_block_flag;

  buffer = req.stream->malloc((req.oldsize + 1) * sizeof(int64_t));
  if (buffer == NULL) {
    return -1;
  }

  qsufsort(req.sa, buffer, req.old, req.oldsize);
  req.stream->free(buffer);

  new_cursor = 0;
  old_cursor = 0;
  last_new_cur = 0;
  last_old_cur = 0;

  while (new_cursor < req.newsize) {
    match = approximate_match(req.sa,                           // suffix array
                              req.old, req.oldsize, old_cursor, // old
                              req.new, req.newsize, new_cursor  // new
    );

    new_cursor = match.new_pos;
    old_cursor = match.old_pos;

    // get lenf
    lenf = forward_ext_len(req.old, last_old_cur, old_cursor, req.new,
                           last_new_cur, new_cursor);

    // get lenb
    lenb = backward_ext_len(req.old, last_old_cur + lenf, old_cursor, req.new,
                            last_new_cur + lenf, new_cursor);

    len_diff = lenf;
    len_extra = (new_cursor - lenb) - (last_new_cur + lenf);
    len_skip = (old_cursor - lenb) - (last_old_cur + lenf);

    req.block->len_diff = len_diff;
    req.block->len_extra = len_extra;
    req.block->len_skip = len_skip;

    // fill diff
    for (i = 0; i < len_diff; i++) {
      req.block->data[i] =
          req.new[last_new_cur + i] - req.old[last_old_cur + i];
    }

    // fill extra
    for (i = 0; i < len_extra; i++) {
      req.block->data[len_diff + i] = req.new[last_new_cur + lenf + i];
    }

    // write block
    req.stream->write(req.stream, req.block, sizeof(*req.block));

    // compress and write data in block
    for (i = 0; i < len_diff + len_extra; i += FASTLZ_INPUT_SIZE) {
      out_sz = fastlz_compress_level(2, req.block->data + i, FASTLZ_INPUT_SIZE,
                                     fastlz_buffer);
      last_block_flag = 0;
      if ((i + FASTLZ_INPUT_SIZE) >= (len_diff + len_extra)) {
        last_block_flag = 1;
      }
      req.stream->write(req.stream, &out_sz, sizeof(out_sz));
      req.stream->write(req.stream, &last_block_flag, sizeof(last_block_flag));
      req.stream->write(req.stream, fastlz_buffer, out_sz);
    }

    last_new_cur = new_cursor - lenb;
    last_old_cur = old_cursor - lenb;
  }

  return 0;
}

int bsdiff(const uint8_t *old, int64_t old_sz, const uint8_t *new,
           int64_t new_sz, bsdiff_stream_t *stream) {
  int ret;
  size_t block_sz;
  bsdiff_request_t req;

  req.sa = stream->malloc((req.oldsize + 1) * sizeof(int64_t));
  if (req.sa == NULL) {
    return -1;
  }

  block_sz = 2 * (req.newsize + 1) * sizeof(int64_t);
  req.block = stream->malloc(block_sz);
  if (req.block == NULL) {
    stream->free(req.sa);
    return -1;
  }

  req.old = old;
  req.oldsize = old_sz;
  req.new = new;
  req.newsize = new_sz;
  req.stream = stream;

  ret = bsdiff_internal(req);

  stream->free(req.sa);
  stream->free(req.block);

  return ret;
}

static am_t approximate_match(const int64_t *sa, const uint8_t *old,
                              int64_t old_sz, int64_t old_cursor,
                              const uint8_t *new, int64_t new_sz,
                              int64_t new_cursor) {
  am_t match;

  int64_t len;       // length of exact match
  int64_t pos;       // exact match pos in old
  int64_t match_cnt; // the matched bytes in a approximate match
  int64_t tmp;
  int64_t offset;

  match_cnt = 0;
  offset = old_cursor - new_cursor;
  tmp = new_cursor;
  while (new_cursor < new_sz) {
    len = bsearch(           // search a exact match region
        sa,                  // suffix array
        old, old_sz,         // old data and length
        new + new_cursor,    // new data
        new_sz - new_cursor, // new length,
        0, old_sz, &pos      // begin, end, pos(output)
    );

    /* We already know the result in range [tmp, new_cursor + len]. The tmp is
     * initialized as new_cursor. We don't reset the match_cnt and tmp in every
     * loop to use the known result for better performance.
     */
    for (; tmp < new_cursor + len; tmp++) {
      if (tmp + offset < old_sz && old[tmp + offset] == new[tmp]) {
        match_cnt++;
      }
    }

    /* It is a exact match, we don't have to next_cursor++.
     * Let's do a huge jump!
     *
     * * Why not move the match_cnt into the loop and reset it every time? *
     *
     * You may already know it, for performance reasons. You don't have to count
     * it over and over, because we already know parts of the result, that is,
     * [cursor + 1, cursor + len]. We only need to count the difference between
     * [cursor + len, the-new-cursor + the-new-len]
     *
     * * Some complaints *
     *
     * The code from Colin Percival is too confusing. He mixed this logic into
     * the logic of generating a diff block. I took times to understand it!
     *
     * He used break here and utilized the external loop to help him come back
     * to this loop, it is too confusing. We should exit this loop only when we
     * find the approximate match we want. It's better to use continue than
     * break.
     */
    if (len != 0 && len == match_cnt) {
      new_cursor += len;
      tmp = new_cursor;
      match_cnt = 0;
      continue;
    }

    if (len - match_cnt > 8) {
      break;
    }

    if ((new_cursor + offset < old_sz) &&
        (old[new_cursor + offset] == new[new_cursor])) {
      match_cnt--;
    }

    new_cursor++;
  } // while (new_cursor < new_sz)

  match.new_pos = new_cursor;
  match.old_pos = pos;

  return match;
}

static int64_t forward_ext_len(const uint8_t *old, int64_t old_beg,
                               int64_t old_end, const uint8_t *new,
                               int64_t new_beg, int64_t new_end) {

  // Double pointer iteration
  int64_t lenf = 0;                       // length of forward extension
  int64_t ln = new_beg, rn = new_end - 1; // left and right pointer for new
  int64_t lo = old_beg, ro = old_end - 1; // left and right pointer for old
  int64_t lm = 0;                         // left match count
  int64_t lr_best = 0;                    // left best match rate
  while (lo < ro && ln < rn) {
    if (old[lo] == new[ln]) {
      lm++;
    }
    ln++;
    lo++;
    if (lm * 2 - (ln - new_beg) > lr_best) {
      lr_best = lm * 2 - (ln - new_beg);
      lenf = ln - new_beg;
    }
  }

  return lenf;
}

static int64_t backward_ext_len(const uint8_t *old, int64_t old_beg,
                                int64_t old_end, const uint8_t *new,
                                int64_t new_beg, int64_t new_end) {
  // Double pointer iteration
  int64_t lenb = 0;                       // length of backward extension
  int64_t ln = new_beg, rn = new_end - 1; // left and right pointer for new
  int64_t lo = old_beg, ro = old_end - 1; // left and right pointer for old
  int64_t rm = 0;                         // right match count
  int64_t rr_best = 0;                    // right best match rate

  while (lo < ro && ln < rn) {
    if (old[ro] == new[rn]) {
      rm++;
    }
    ro--;
    rn--;
    if (rm * 2 - (new_end - rn) > rr_best) {
      rr_best = rm * 2 - (new_end - rn);
      lenb = new_end - rn;
    }
  }

  return lenb;
}
