#include <cmath>

#define int8_t char                 // NOLINT
#define int16_t short               // NOLINT
#define int32_t int                 // NOLINT
#define int64_t long int            // NOLINT
#define uint8_t unsigned char       // NOLINT
#define uint16_t unsigned short     // NOLINT
#define uint32_t unsigned int       // NOLINT
#define uint64_t unsigned long int  // NOLINT

#define INCREASE(x, l) ((x + 1) >= (l) ? (x) : ((x) + 1))

void resize_preprocess(uint64_t src_h, uint64_t src_w, uint64_t dst_h, uint64_t dst_w,
                       int16_t* __restrict__ cubfh, int16_t* __restrict__ cubfw,
                       int32_t* __restrict__ inth, int32_t* __restrict__ intw) {
  float scale_h = double(src_h) / dst_h;
  float scale_w = double(src_w) / dst_w;

  for (int j = 0; j < dst_h; ++j) {
    float fh = (float)((j + 0.5) * scale_h - 0.5f);
    int sh = floor(fh);
    fh -= sh;
    if (sh < 0) {
      fh = 0;
      sh = 0;
    }
    if (sh >= src_h) {
      fh = 0;
      sh = src_h - 1;
    }

    int int_h1 = INCREASE(sh, src_h);

    fh = fh * 2048;
    cubfh[j] = rint(2048 - fh);
    cubfh[dst_h + j] = rint(fh);

    inth[j] = sh;
    inth[dst_h + j] = int_h1;
  }

  for (int i = 0; i < dst_w; ++i) {
    float fw = (float)((i + 0.5) * scale_w - 0.5f);
    int sw = floor(fw);
    fw -= sw;

    if (sw < 0) {
      fw = 0;
      sw = 0;
    }
    if (sw >= src_w) {
      fw = 0;
      sw = src_w - 1;
    }
    int int_w1 = INCREASE(sw, src_w);
    fw = fw * 2048;
    cubfw[i] = rint(2048 - rint(fw));
    cubfw[dst_w + i] = rint(fw);

    intw[i] = sw;
    intw[dst_w + i] = int_w1;
  }
}
