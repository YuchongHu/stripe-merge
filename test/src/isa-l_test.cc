#include <isa-l.h>
#include <sys/time.h>

#include <cstring>
#include <iostream>
using namespace std;

#define LEN 1 << 26
#define N 3
#define K 2
#define M N - K

int main() {
  uint8_t *encode_matrix = new uint8_t[6]{1, 0, 0, 1, 1, 2};
  uint8_t *encode_gftbl = new uint8_t[32 * 2]();
  uint8_t **data = new uint8_t *[N];
  for (uint8_t i = 0; i < N; ++i) {
    data[i] = new uint8_t[LEN]();
  }

  // gf_gen_rs_matrix(encode_matrix, N, K);
  // printf("encode_matrix:\n");
  // for (uint i = 0; i < N * K;) {
  //   cout << +encode_matrix[i] << "\t";
  //   i++;
  //   if (i % K == 0) cout << "\n";
  // }
  ec_init_tables(K, M, encode_matrix, encode_gftbl);
  struct timeval start_time, end_time;
  double time;

  gettimeofday(&start_time, nullptr);
  ec_encode_data(LEN, K, M, encode_gftbl, data, data);
  gettimeofday(&end_time, nullptr);
  time = (end_time.tv_sec - start_time.tv_sec) * 1000.0 +
         (end_time.tv_usec - start_time.tv_usec) / 1000.0;
  cout << "encode ok  normal: " << time << endl;

  gettimeofday(&start_time, nullptr);
  ec_encode_data_sse(LEN, K, M, encode_gftbl, data, data);
  gettimeofday(&end_time, nullptr);
  time = (end_time.tv_sec - start_time.tv_sec) * 1000.0 +
         (end_time.tv_usec - start_time.tv_usec) / 1000.0;
  cout << "encode ok  with sse: " << time << endl;

  gettimeofday(&start_time, nullptr);
  ec_encode_data_avx(LEN, K, M, encode_gftbl, data, data);
  gettimeofday(&end_time, nullptr);
  time = (end_time.tv_sec - start_time.tv_sec) * 1000.0 +
         (end_time.tv_usec - start_time.tv_usec) / 1000.0;
  cout << "encode ok  with avx: " << time << endl;

  gettimeofday(&start_time, nullptr);
  ec_encode_data_avx2(LEN, K, M, encode_gftbl, data, data);
  gettimeofday(&end_time, nullptr);
  time = (end_time.tv_sec - start_time.tv_sec) * 1000.0 +
         (end_time.tv_usec - start_time.tv_usec) / 1000.0;
  cout << "encode ok  with avx2: " << time << endl;

  return 0;
}