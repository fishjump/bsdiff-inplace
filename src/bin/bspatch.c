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

#include <bzlib.h>
#include <err.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <bsdiff/bspatch.h>

#include "defs.h"

static int bz2_read(const struct bspatch_stream *stream, void *buffer,
                    int length) {
  int n;
  int bz2err;
  BZFILE *bz2;

  bz2 = (BZFILE *)stream->opaque;
  n = BZ2_bzRead(&bz2err, bz2, buffer, length);
  if (n != length) {
    printf("[%s %d] n = %d, len = %d\n", __FILE__, __LINE__, n, length);
    return -1;
  }

  return 0;
}

int main(int argc, char *argv[]) {
  FILE *pf;
  int fd;
  int bz2err;

  uint8_t *old;
  BZFILE *bz2;
  int64_t old_sz, new_sz;
  bspatch_stream_t stream;
  header_t header;
  struct stat sb;

  if (argc != 4) {
    errx(1, "usage: %s oldfile newfile patchfile\n", argv[0]);
  }

  // open patch file
  if ((pf = fopen(argv[3], "r")) == NULL) {
    err(1, "failed to open patch file: %s\n", argv[3]);
  }

  // read header
  if (fread(&header, sizeof(header), 1, pf) != 1) {
    if (feof(pf)) {
      errx(1, "corrupt patch: eof while read patch header(%s)\n", argv[3]);
    } else {
      err(1, "failed to read patch header: %s\n", argv[3]);
    }
  }

  // Check for appropriate magic
  if (memcmp(header.signature, SIGNATURE, SIGNATURE_LEN) != 0) {
    errx(1, "malformed patch file signature\n");
  }

  new_sz = header.new_sz;
  if (new_sz < 0) {
    errx(1, "corrupt patch: new size is smaller than 0(%s)\n", argv[3]);
  }

  if (((fd = open(argv[1], O_RDONLY, 0)) < 0) ||
      ((old_sz = lseek(fd, 0, SEEK_END)) == -1) ||
      ((old = malloc(old_sz + 1)) == NULL) || (lseek(fd, 0, SEEK_SET) != 0) ||
      (read(fd, old, old_sz) != old_sz) || (fstat(fd, &sb)) ||
      (close(fd) == -1)) {
    err(1, "failed to open old file: %s\n", argv[1]);
  }

  bz2 = BZ2_bzReadOpen(&bz2err, pf, 0, 0, NULL, 0);
  if (bz2 == NULL) {
    errx(1, "err at BZ2_bzReadOpen, code=%d\n", bz2err);
  }

  stream.read = bz2_read;
  stream.opaque = bz2;
  if (bspatch(old, old_sz, new_sz, &stream)) {
    errx(1, "internal err at bspatch");
  }

  // Clean up the bzip2 reads
  BZ2_bzReadClose(&bz2err, bz2);
  fclose(pf);

  // Write the new file
  if (((fd = open(argv[2], O_CREAT | O_TRUNC | O_WRONLY, sb.st_mode)) < 0) ||
      (write(fd, old, new_sz) != new_sz) || (close(fd) == -1)) {
    err(1, "failed to write the new file at: %s", argv[2]);
  }

  free(old);

  return 0;
}
