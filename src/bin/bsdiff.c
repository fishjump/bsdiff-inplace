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

#include <sys/types.h>

#include <bzlib.h>
#include <err.h>
#include <fcntl.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <bsdiff/bsdiff.h>

#include "defs.h"

static int bz2_write(struct bsdiff_stream *stream, const void *buffer,
                     int size) {
  int bz2err;
  BZFILE *bz2;

  bz2 = (BZFILE *)stream->opaque;
  BZ2_bzWrite(&bz2err, bz2, (void *)buffer, size);

  if (bz2err != BZ_STREAM_END && bz2err != BZ_OK) {
    return -1;
  }

  return 0;
}

int main(int argc, char *argv[]) {

  int fd;
  int bz2err;
  uint8_t *old, *new;
  off_t old_sz, new_sz;
  FILE *pf;
  bsdiff_stream_t stream;
  BZFILE *bz2;

  header_t header = {
      .signature = SIGNATURE,
      .new_sz = 0,
  };

  if (argc != 4) {
    errx(1, "usage: %s oldfile newfile patchfile\n", argv[0]);
  }

  if (((fd = open(argv[1], O_RDONLY, 0)) < 0) ||   // open old
      ((old_sz = lseek(fd, 0, SEEK_END)) == -1) || // get old_sz
      ((old = malloc(old_sz + 1)) == NULL) ||      // alloc mem
      (lseek(fd, 0, SEEK_SET) != 0) ||             // move file cursor to begin
      (read(fd, old, old_sz) != old_sz) ||         // read all the file
      (close(fd) == -1)                            // close
  ) {
    err(1, "failed to read old: %s\n", argv[1]);
  }

  if (((fd = open(argv[2], O_RDONLY, 0)) < 0) ||   // open new
      ((new_sz = lseek(fd, 0, SEEK_END)) == -1) || // get new_sz
      ((new = malloc(new_sz + 1)) == NULL) ||      // alloc mem
      (lseek(fd, 0, SEEK_SET) != 0) ||             // move file cursor to begin
      (read(fd, new, new_sz) != new_sz) ||         // read all the file
      (close(fd) == -1)                            // close
  ) {
    err(1, "failed to read new: %s\n", argv[2]);
  }

  if ((pf = fopen(argv[3], "w")) == NULL) {
    err(1, "failed to create patch: %s\n", argv[3]);
  }

  // write header (signature+new_sz)
  header.new_sz = new_sz;
  if (fwrite(&header, sizeof(header), 1, pf) != 1) {
    err(1, "failed to write header\n");
  }

  bz2 = BZ2_bzWriteOpen(&bz2err, pf, 9, 0, 0);
  if (bz2 == NULL) {
    errx(1, "BZ2_bzWriteOpen, bz2err=%d", bz2err);
  }

  stream.malloc = malloc;
  stream.free = free;
  stream.write = bz2_write;
  stream.opaque = bz2;

  if (bsdiff(old, old_sz, new, new_sz, &stream)) {
    err(1, "internal err at bsdiff\n");
    return -1;
  }

  BZ2_bzWriteClose(&bz2err, bz2, 0, NULL, NULL);
  if (bz2err != BZ_OK) {
    err(1, "BZ2_bzWriteClose, bz2err=%d", bz2err);
  }

  if (fclose(pf)) {
    err(1, "internal err at fclose\n");
  }

  free(old);
  free(new);

  return 0;
}
