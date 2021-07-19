#include <sys/time.h>

#include "matching_generator.hh"

using namespace std;

#define COMPARE true

int main(int argc, char **argv) {
  if (argc != 5) {
    cerr << "Usage: ./matching_main [stripes_num] [host_num] [rs_k] [rs_m]"
         << endl;
    exit(-1);
  }
  // get parameters
  uint stripes_num = strtoul(argv[1], nullptr, 10);
  uint16_t node_num = strtoul(argv[2], nullptr, 10);
  uint8_t rs_k = strtoul(argv[3], nullptr, 10);
  uint8_t rs_m = strtoul(argv[4], nullptr, 10);

  // prepare for hash table
  char16_t **raw_dist = new char16_t *[stripes_num];
  char16_t **parity_dist = new char16_t *[stripes_num];
  for (uint i = 0; i < stripes_num; ++i) {
    raw_dist[i] = new char16_t[rs_k];
    parity_dist[i] = new char16_t[rs_m];
  }

  // get distributions
  Stripe::generate_random_distributions(raw_dist, parity_dist, node_num,
                                        stripes_num, rs_k, rs_m);

  struct timeval start_time, end_time;

  uint tot_costs[4] = {0};
  double fin_time[4] = {0.0};

  // call StripeMerge-P
  cout << "=========== dist_based_search -> StripeMerge-P  ===========\n";
  gettimeofday(&start_time, nullptr);
  MatchingGenerator *mg1 =
      new MatchingGenerator(node_num, rs_k, rs_m, stripes_num);
  mg1->input_stripes(raw_dist, parity_dist);
  mg1->dist_based_search();
  gettimeofday(&end_time, nullptr);
  tot_costs[0] = mg1->print_statics();

  fin_time[0] = (end_time.tv_sec - start_time.tv_sec) * 1000 +
                (end_time.tv_usec - start_time.tv_usec) / 1000;
  cout << "finish time: " << fin_time[0] << "ms\n";
  // auto scheme = mg1->get_sending_scheme();
  // std::cout << "get_sending_scheme size: " << scheme.size() << "\n";
  delete mg1;

  // call StripeMerge-G
  if (COMPARE) {
    cout << "\n=========== basic_greedy -> StripeMerge-G ===========\n";
    gettimeofday(&start_time, nullptr);
    MatchingGenerator *mg2 =
        new MatchingGenerator(node_num, rs_k, rs_m, stripes_num);
    mg2->input_stripes(raw_dist, parity_dist);
    tot_costs[1] = mg2->basic_greedy();
    gettimeofday(&end_time, nullptr);
    auto scheme = mg2->get_sending_scheme();
    std::cout << "scheme size = " << scheme.size() << "\n";
    delete mg2;
    fin_time[1] = (end_time.tv_sec - start_time.tv_sec) * 1000 +
                  (end_time.tv_usec - start_time.tv_usec) / 1000;
    cout << "finish time: " << fin_time[1] << "ms\n";

    // cout << "\n*** Speedup ratio = " << time_2 / time_1 << " ***\n";
  }

  for (uint i = 0; i < stripes_num; ++i) {
    delete[] raw_dist[i];
    delete[] parity_dist[i];
  }
  delete[] raw_dist;
  delete[] parity_dist;

  return 0;
}
