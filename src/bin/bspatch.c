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

#include <bsdiff/adapters/array_like_adapter.h>
#include <bsdiff/bspatch.h>

// static int bz2_read(const file_stream_t *stream, void *buffer, int size) {
//   int n;
//   int bz2err;
//   BZFILE *bz2;

//   bz2 = (BZFILE *)stream->opaque;
//   if (BZ2_bzRead(&bz2err, bz2, buffer, size) != size) {
//     return -1;
//   }

//   return 0;
// }

static size_t file_read(const bsdiff_stream_t *stream, void *buffer,
                        size_t size) {
  return fread(buffer, 1, size, (FILE *)stream->opaque);
}

int main(int argc, char *argv[]) {
  FILE *fp;
  int fd;
  int bz2err;

  uint8_t *old_buffer;
  struct stat sb;

  size_t old_sz;
  array_like_t old_array_like;
  bsdiff_array_like_t old;
  bsdiff_stream_t patch;
  size_t new_sz;

  if (argc != 4) {
    errx(1, "usage: %s oldfile newfile patchfile\n", argv[0]);
  }

  if (((fd = open(argv[1], O_RDONLY, 0)) < 0) ||
      ((old_sz = lseek(fd, 0, SEEK_END)) == -1) ||
      ((old_buffer = malloc(old_sz + 1)) == NULL) ||
      (lseek(fd, 0, SEEK_SET) != 0) ||
      (read(fd, old_buffer, old_sz) != old_sz) || (fstat(fd, &sb)) ||
      (close(fd) == -1)) {
    err(1, "failed to open old file: %s\n", argv[1]);
  }

  make_array_like(&old_array_like, old_buffer, old_sz);
  make_array_like_adapter(&old, &old_array_like);

  // open patch file
  fp = fopen(argv[3], "r");
  if (fp == NULL) {
    err(1, "failed to open patch file: %s\n", argv[3]);
  }

  patch.opaque = fp;
  patch.read = file_read;
  patch.write = NULL; // patch will not be writen

  if (bspatch(&old, &patch, &new_sz)) {
    errx(1, "internal err at bspatch");
  }

  fclose(fp);

  // write the new file
  if (((fd = open(argv[2], O_CREAT | O_TRUNC | O_WRONLY, sb.st_mode)) < 0) ||
      (write(fd, old_buffer, new_sz) != old_sz) || (close(fd) == -1)) {
    err(1, "failed to write the new file at: %s", argv[2]);
  }

  free(old_buffer);

  return 0;
}
