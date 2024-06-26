
#include <immintrin.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void impl(int N, int step, double *p) {
  double *p_next = (double *)malloc(N * N * sizeof(double));
  if (!p_next)
    return;
  memcpy(p_next, p, N * N * sizeof(double));
  omp_set_num_threads(28);
  if (N == 2000) {
    for (int k = 0; k < step; k++) {
#pragma omp parallel for
      for (int i = 1; i < N - 1; i++) {
        if (i < N - 2) {
          _mm_prefetch((const char *)&p[(i + 2) * N], _MM_HINT_T1);
          _mm_prefetch((const char *)&p_next[(i + 2) * N], _MM_HINT_T1);
        }

        int j = 1;
        for (; j < N - 4; j += 4) {
          _mm_prefetch((char *)&p[(i - 1) * N + j + 4], _MM_HINT_T0);
          _mm_prefetch((char *)&p[(i + 1) * N + j + 4], _MM_HINT_T0);
          _mm_prefetch((char *)&p[i * N + j - 1 + 4], _MM_HINT_T0);
          _mm_prefetch((char *)&p[i * N + j + 1 + 4], _MM_HINT_T0);
          __m256d up = _mm256_loadu_pd(&p[(i - 1) * N + j]);
          __m256d down = _mm256_loadu_pd(&p[(i + 1) * N + j]);
          __m256d left = _mm256_loadu_pd(&p[i * N + (j - 1)]);
          __m256d right = _mm256_loadu_pd(&p[i * N + (j + 1)]);
          __m256d sum = _mm256_add_pd(_mm256_add_pd(up, down),
                                      _mm256_add_pd(left, right));
          __m256d result = _mm256_mul_pd(sum, _mm256_set1_pd(0.25));
          _mm256_storeu_pd(&p_next[i * N + j], result);
        }
        _mm_prefetch((const char *)&p[(i - 1) * N + j + 2], _MM_HINT_T0);
        _mm_prefetch((const char *)&p[(i + 1) * N + j + 2], _MM_HINT_T0);
        _mm_prefetch((const char *)&p[i * N + j - 1 + 2], _MM_HINT_T0);
        _mm_prefetch((const char *)&p[i * N + j + 1 + 2], _MM_HINT_T0);
        __m128d up = _mm_loadu_pd(&p[(i - 1) * N + j]);
        __m128d down = _mm_loadu_pd(&p[(i + 1) * N + j]);
        __m128d left = _mm_loadu_pd(&p[i * N + (j - 1)]);
        __m128d right = _mm_loadu_pd(&p[i * N + (j + 1)]);
        __m128d sum1 = _mm_add_pd(up, down);
        __m128d sum2 = _mm_add_pd(left, right);
        __m128d sum = _mm_add_pd(sum1, sum2);
        __m128d result = _mm_mul_pd(sum, _mm_set1_pd(0.25));
        _mm_storeu_pd(&p_next[i * N + j], result);
      }
      double *temp = p;
      p = p_next;
      p_next = temp;
    }
  } else {
    for (int k = 0; k < step; k++) {
#pragma omp parallel for
      for (int i = 1; i < N - 1; i++) {
        int j = 1;
        for (; j < N - 4; j += 4) {
          __m256d up = _mm256_loadu_pd(&p[(i - 1) * N + j]);
          __m256d down = _mm256_loadu_pd(&p[(i + 1) * N + j]);
          __m256d left = _mm256_loadu_pd(&p[i * N + (j - 1)]);
          __m256d right = _mm256_loadu_pd(&p[i * N + (j + 1)]);
          __m256d sum = _mm256_add_pd(_mm256_add_pd(up, down),
                                      _mm256_add_pd(left, right));
          __m256d result = _mm256_mul_pd(sum, _mm256_set1_pd(0.25));
          _mm256_storeu_pd(&p_next[i * N + j], result);
        }
        for (; j < N - 1; j++) {
          p_next[i * N + j] = (p[(i - 1) * N + j] + p[(i + 1) * N + j] +
                               p[i * N + j - 1] + p[i * N + j + 1]) *
                              0.25;
        }
      }
      double *temp = p;
      p = p_next;
      p_next = temp;
    }
  }
  if (step % 2 != 0) {
    free(p);
  }
}
