#ifndef _BSDIFF_LIB_IMPL_HELPER_
#define _BSDIFF_LIB_IMPL_HELPER_

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define FASTLZ_BUFFER_SIZE (512) /*  105% of INPUT_SIZE for safety reasons */
#define FASTLZ_INPUT_SIZE (FASTLZ_BUFFER_SIZE / 21 * 20)

#endif // _BSDIFF_LIB_IMPL_HELPER_
