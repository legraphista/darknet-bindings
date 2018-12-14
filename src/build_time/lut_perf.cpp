#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <emmintrin.h>

int frame_size = 3840 * 2160 * 3;
float magic_number = 1 / 255.f;

inline void convert(float *to, const uint8_t *from, int length) {
  // align to's front
  while ((intptr_t) to & 0x0f && length) {
    --length;
    *to++ = (float) *from++;
  }
  // align to's back
  while (length & 0x0f) {
    --length;
    to[length] = (float) from[length];
  }
  length /= 16;

  __m128i z = _mm_setzero_si128();
  __m128i *F = (__m128i *) from;
  if ((intptr_t) from & 0x0f) // if from is memory aligned
    for (int i = 0; i < length; ++i) {
      __m128i m = _mm_loadu_si128(F + i);
      __m128i L = _mm_unpacklo_epi8(m, z);
      __m128i H = _mm_unpackhi_epi8(m, z);
      __m128i L0 = _mm_unpacklo_epi8(L, z);
      __m128i L1 = _mm_unpackhi_epi8(L, z);
      __m128i H0 = _mm_unpacklo_epi8(H, z);
      __m128i H1 = _mm_unpackhi_epi8(H, z);
      _mm_store_ps(to + 16 * i, _mm_cvtepi32_ps(L0));
      _mm_store_ps(to + 16 * i + 4, _mm_cvtepi32_ps(L1));
      _mm_store_ps(to + 16 * i + 8, _mm_cvtepi32_ps(H0));
      _mm_store_ps(to + 16 * i + 12, _mm_cvtepi32_ps(H1));
    }
  else // if from is not memory aligned
    for (int i = 0; i < length; ++i) {
      __m128i m = _mm_load_si128(F + i);
      __m128i L = _mm_unpacklo_epi8(m, z);
      __m128i H = _mm_unpackhi_epi8(m, z);
      __m128i L0 = _mm_unpacklo_epi8(L, z);
      __m128i L1 = _mm_unpackhi_epi8(L, z);
      __m128i H0 = _mm_unpacklo_epi8(H, z);
      __m128i H1 = _mm_unpackhi_epi8(H, z);
      _mm_store_ps(to + 16 * i, _mm_cvtepi32_ps(L0));
      _mm_store_ps(to + 16 * i + 4, _mm_cvtepi32_ps(L1));
      _mm_store_ps(to + 16 * i + 8, _mm_cvtepi32_ps(H0));
      _mm_store_ps(to + 16 * i + 12, _mm_cvtepi32_ps(H1));
    }
}

int main() {
  unsigned char *pseudo_frame = (unsigned char *) malloc(frame_size);
  float *pseudo_frame_float = (float *) malloc(frame_size * sizeof(float));
  clock_t begin, end;
  double time_spent;

  for (int i = 0; i < frame_size; i++) {
    pseudo_frame[i] = i % 256;
  }

  float *proc_frame = (float *) malloc(frame_size * sizeof(float));

  begin = clock();

  // uint8t -> float
  {

    convert(pseudo_frame_float, pseudo_frame, frame_size);

  }

//  begin = clock();

// frame * 1/128
  {
    float *frame_moving_pointer = pseudo_frame_float;

    float *mult_float = (float *) malloc(4 * sizeof(float));
    mult_float[0] = mult_float[1] = mult_float[2] = mult_float[3] = 1 / 255.f;

    __m128 *mult = (__m128 *) mult_float;
    __m128 *ptr = (__m128 *) frame_moving_pointer;
    int nb_iters = frame_size / 4;

    for (int i = 0; i < nb_iters; ++i, ++ptr, frame_moving_pointer += 4) {
      _mm_store_ps(frame_moving_pointer, _mm_mul_ps(*ptr, *mult));
    }
  }
  end = clock();

  time_spent = (double) (end - begin) / CLOCKS_PER_SEC * 1000;

  printf("PROC %f %f\n", time_spent, pseudo_frame_float[frame_size - 1]);

}