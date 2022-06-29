#ifndef _LIBBSPATCH_H_
#define _LIBBSPATCH_H_

#include <stdint.h>

struct bspatch_stream {
  void *opaque;
  int (*read)(const struct bspatch_stream *stream, void *buffer, int length);
};

int bspatch(const uint8_t *old, int64_t oldsize, uint8_t *new, int64_t newsize,
            struct bspatch_stream *stream);

#endif // _LIBBSPATCH_H_
