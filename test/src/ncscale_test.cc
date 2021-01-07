#include <sys/time.h>

#include <memory>

#include "ncscale_simulation.hh"
using namespace std;

int main(int argc, char *argv[]) {
  if (argc != 4) {
    cerr << "Usage: [k] [m] [upside_num]" << endl;
    exit(-1);
  }

  struct timeval start_time, end_time;
  gettimeofday(&start_time, nullptr);

  uint8_t k = strtoul(argv[1], nullptr, 10);
  uint8_t m = strtoul(argv[2], nullptr, 10);
  uint upside_num = strtoul(argv[3], nullptr, 10);

  NCScaleSimulation ns(k, m);
  uint batch_size = ns.get_batch_size();
  uint tot_costs = ns.get_total_costs();
  uint similar = ns.get_similar_batch_size(upside_num);
  uint target_costs = similar / batch_size * tot_costs;
  // cout << "*** input:  k = " << +k << ", m = " << +m
  //      << ", upside_num = " << upside_num << "\n";
  // cout << "Batch size: " << batch_size << "\ntheoretical costs: " <<
  // tot_costs
  //      << "\nsimilar_size: " << similar << "\ntarget_costs: " << target_costs
  //      << endl;

  auto scheme = move(ns.get_scheme(similar));
  // cout << "scheme_size: " << scheme.size()
  //      << (scheme.size() != target_costs ? "\n### error scheme size!\n" :
  //      "\n");

  for (auto x : scheme) {
    if (x.source == x.target) {
      cerr << "!!!!!!!!   bad pair: " << +x.source << "->" << +x.target << endl;
      exit(-1);
    }
    // cout << +x.source << "->" << +x.target << "\n";
  }
  gettimeofday(&end_time, nullptr);
  double time_1 = (end_time.tv_sec - start_time.tv_sec) +
                  (end_time.tv_usec - start_time.tv_usec) / 1000000.0;
  cout << "time: " << time_1 << endl;

  if (scheme.size()) {
    cout << "All pairs are OK!" << endl;
  }

  return 0;
}