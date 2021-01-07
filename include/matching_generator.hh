#ifndef MATCHING_GENERATOR_HH
#define MATCHING_GENERATOR_HH

#include <iostream>
#include <list>
#include <memory>
#include <queue>
#include <unordered_map>
#include <vector>

#include "migration_info.hh"
#include "stripe.hh"

#define UINT32_MASK 0xFFFFFFFF

class MatchingGenerator {
  uint16_t node_num;
  uint8_t rs_k;
  uint8_t rs_m;
  uint8_t rs_n;
  uint stripes_num;
  std::vector<Stripe> stripes;
  std::vector<uint8_t> cost_table;
  std::list<uint> remaining_stripes;
  std::unordered_map<std::u16string, std::list<uint>> parity_map;
  std::unordered_map<std::u16string, std::list<uint>> partial_dist_map;
  std::unordered_map<std::u16string, std::list<uint>> zero_map;
  std::vector<uint> none_zero_candidate;
  std::vector<MigrationInfo> blocks_to_sent;

 public:
  MatchingGenerator() {}
  ~MatchingGenerator() {}
  MatchingGenerator(uint16_t _node_num, uint8_t k, uint8_t m,
                    uint _stripes_num);
  void input_stripes(char16_t **raw, char16_t **parity);
  uint8_t get_cost(uint i, uint j);
  uint print_statics(bool print_flag = true);
  uint basic_greedy(bool remain_flag = false);
  void dist_based_search();
  void random_match();
  long long blossom();
  std::vector<MigrationInfo> get_sending_scheme();
  void build_map_test();

 private:
  void build_map();
  uint8_t get_cost(uint64_t token);
  void mark_matched(uint i, uint j, uint8_t cost, bool flag = true);
  bool single_search(uint index, std::list<uint> &search_domain,
                     uint8_t target_cost);
  bool search_for_matching(uint index, uint8_t target_cost);
  void make_optimization();
  void get_one_scheme(uint index);
};

#endif
