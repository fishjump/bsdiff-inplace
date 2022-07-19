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

#include <bsdiff/bspatch.h>
#include <fastlz.h>

#include "helper.h"

typedef struct {
  uint8_t compressed[FASTLZ_BUFFER_SIZE];
  uint8_t decompressed[FASTLZ_BUFFER_SIZE];
  uint64_t compressed_size;
  uint64_t decompressed_size;
  uint8_t last_block_flag;

  const bsdiff_stream_t *patch;

  size_t cursor;
} __attribute__((packed)) fastlz_ctx_t;

static void fastlz_ctx_init(fastlz_ctx_t *ctx, const bsdiff_stream_t *patch) {
  ctx->compressed_size = 0;
  ctx->decompressed_size = 0;
  ctx->last_block_flag = 0;
  ctx->cursor = 0;
  ctx->patch = patch;
}

static int fastlz_ctx_next(fastlz_ctx_t *ctx) {
  if (ctx->last_block_flag) {
    return BSPATCH_DECOMPRESS_ERR;
  }

  if (ctx->patch->read(ctx->patch, &ctx->compressed_size,
                       sizeof(ctx->compressed_size)) !=
      sizeof(ctx->compressed_size)) {
    return BSPATCH_READ_PATCH_ERR;
  }

  if (ctx->patch->read(ctx->patch, &ctx->last_block_flag,
                       sizeof(ctx->last_block_flag)) !=
      sizeof(ctx->last_block_flag)) {
    return BSPATCH_READ_PATCH_ERR;
  }

  if (ctx->patch->read(ctx->patch, &ctx->compressed, ctx->compressed_size) !=
      ctx->compressed_size) {
    return BSPATCH_READ_PATCH_ERR;
  }

  ctx->decompressed_size =
      fastlz_decompress(ctx->compressed, ctx->compressed_size,
                        ctx->decompressed, FASTLZ_BUFFER_SIZE);
  ctx->cursor = 0;

  return BSPATCH_SUCCESS;
}

static int fastlz_ctx_read(fastlz_ctx_t *ctx, void *buffer, size_t size) {
  size_t i;
  int ret;
  for (size_t i = 0; i < size; i++) {
    if (ctx->cursor >= ctx->decompressed_size) {
      ret = fastlz_ctx_next(ctx);
      if (ret != BSPATCH_SUCCESS) {
        return ret;
      }
    }

    ((uint8_t *)buffer)[i] = ctx->decompressed[ctx->cursor];
    ctx->cursor++;
  }

  return BSPATCH_SUCCESS;
}

static void fastlz_ctx_reset(fastlz_ctx_t *ctx) {
  ctx->compressed_size = 0;
  ctx->decompressed_size = 0;
  ctx->last_block_flag = 0;
  ctx->cursor = 0;
}

int bspatch(bsdiff_array_like_t *old, const bsdiff_stream_t *patch,
            size_t *new_size) {
  bsdiff_header_t header;
  patch_block_t block;
  uint8_t p_byte, o_byte; // patch/old byte
  int64_t old_cursor, new_cursor;
  int64_t i;
  uint64_t old_sz, new_sz;

  fastlz_ctx_t ctx;

  fastlz_ctx_init(&ctx, patch);

  if (patch->read(patch, &header, sizeof(header)) != sizeof(header)) {
    return BSPATCH_READ_PATCH_ERR;
  }

  if (memcmp(header.signature, BSDIFF_SIGNATURE, BSDIFF_SIGNATURE_LEN) != 0) {
    return BSPATCH_SIGNATURE_INCONSISTENCY_ERR;
  }

  old_sz = old->len(old);
  new_sz = header.new_sz;

  old_cursor = 0;
  new_cursor = 0;
  while (new_cursor < header.new_sz) {
    if (patch->read(patch, &block, sizeof(block)) != sizeof(block)) {
      return BSPATCH_READ_PATCH_ERR;
    }

    fastlz_ctx_reset(&ctx);

    // sanity-check
    if (block.len_diff < 0 || block.len_diff > INT_MAX ||   // len_diff
        block.len_extra < 0 || block.len_extra > INT_MAX || // len_extra
        new_cursor + block.len_diff > header.new_sz         // overflow
    ) {
      return BSPATCH_SANITY_CHECK_ERR;
    }

    // Read diff string and add
    for (i = 0; i < block.len_diff; i++) {
      if (fastlz_ctx_read(&ctx, &p_byte, 1) != 0) {
        return BSPATCH_READ_PATCH_ERR;
      }

      if (old_cursor + i < old_sz) {
        /* Codebelow is similar to
         * old[new_cursor + i] = old[old_cursor + i] + byte;
         */
        if (old->read(old, old_cursor + i, &o_byte, 1) != 1) {
          return BSPATCH_READ_OLD_ERR;
        }
        o_byte += p_byte;
        if (old->write(old, new_cursor + i, &o_byte, 1) != 1) {
          return BSPATCH_WRITE_OLD_ERR;
        }
      }
    }

    // adjust pointers
    new_cursor += block.len_diff;
    old_cursor += block.len_diff;

    // sanity-check
    if (new_cursor + block.len_extra > header.new_sz) {
      return BSPATCH_SANITY_CHECK_ERR;
    }

    // move old for the extra string space
    for (i = old_sz - 1; i >= old_cursor; i--) {
      /* Code below is similar to
       * old[i + block.len_extra] = old[i];
       */
      if (old->read(old, i, &o_byte, 1) != 1) {
        return BSPATCH_READ_OLD_ERR;
      }
      if (old->write(old, i + block.len_extra, &o_byte, 1) != 1) {
        return BSPATCH_WRITE_OLD_ERR;
      }
    }

    // read extra string
    for (i = 0; i < block.len_extra; i++) {
      if (fastlz_ctx_read(&ctx, &p_byte, 1) != 0) {
        return BSPATCH_READ_PATCH_ERR;
      }

      if (old->write(old, new_cursor + i, &p_byte, 1) != 1) {
        return BSPATCH_WRITE_OLD_ERR;
      }
    }

    // adjust pointers
    new_cursor += block.len_extra;
    old_cursor += block.len_extra + block.len_skip;
  }

  *new_size = new_sz;

  return BSPATCH_SUCCESS;
}
