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
  uint16_t node_num;  // N
  uint8_t rs_k;
  uint8_t rs_m;
  uint8_t rs_n;  // equals to k+m
  uint stripes_num;
  std::vector<Stripe> stripes;
  std::vector<uint8_t> cost_table;  // use array to store a Symmetric Matrix
  std::list<uint> remaining_stripes;

  /**     - Explanation for the hash-table relevant -
   * We use the hash table to imediately get the set of stripes that complies
   * the given distribution of parity chunks.
   *
   * Here we assume that a distribution of parity chunks can be expressed as a
   * list of number:
   * -    [No.1, ..., No. m] | [vector]    -
   *
   * In the front are the nodes that stores the parity chunks, and in the end we
   * have the vector describing the set of parity positions (in u16's bits).
   *
   * To exploit a hash table, the key is required to be a hashable type. For
   * unordered_map, types in std::string are the only supported ones to present
   * a number's list with an unfixed length. Note that the default string is
   * designed for ASCII, and so it only supports 8-bit values. As an
   * alternative, we can use a u16string as a container to present such a list
   * of number, enabling both the adequate range of values and the convenience
   * of using hash.
   */
  std::unordered_map<std::u16string, std::list<uint>> parity_map;
  std::unordered_map<std::u16string, std::list<uint>> partial_dist_map;
  std::unordered_map<std::u16string, std::list<uint>> zero_map;
  std::vector<uint> non_zero_candidate;

  // for transfer experiment
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
