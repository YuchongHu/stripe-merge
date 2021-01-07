#include "matching_generator.hh"

MatchingGenerator::MatchingGenerator(uint16_t _node_num, uint8_t k, uint8_t m,
                                     uint _stripes_num)
    : node_num{_node_num}, rs_k{k}, rs_m{m}, stripes_num{_stripes_num} {
  rs_n = rs_k + rs_m;
  Stripe::set_params(node_num, rs_k, rs_m);
  stripes.resize(stripes_num);
  size_t table_size = static_cast<size_t>(stripes_num);
  table_size = table_size * (table_size - 1) / 2;
  cost_table.assign(table_size, UINT8_MAX);
}

// initialize the useful mapping data struture for parity distribution
void MatchingGenerator::build_map() {
  auto map_it = parity_map.end();
  auto index_it = partial_dist_map.end();
  const uint8_t upside = 1 << rs_m;
  std::u16string key;
  for (uint index = 0; index < stripes_num; ++index) {
    const std::u16string &pd = stripes[index].parity_dist;
    // map parity dist to its correspondent stripes' indexes
    if ((map_it = parity_map.find(pd)) == parity_map.end()) {
      parity_map[pd] = std::list<uint>{index};
    } else {
      map_it->second.push_back(index);
    }
    remaining_stripes.push_back(index);
    // build multiple index for parity dist
    for (uint8_t s = 1, bits; s < upside; ++s) {
      key.clear();
      bits = s;
      for (auto &x : pd) {
        if (bits & 1) {
          key.push_back(x);
        }
        bits >>= 1;
      }
      key.push_back(s);
      if ((index_it = partial_dist_map.find(key)) == partial_dist_map.end()) {
        partial_dist_map[key] = std::list<uint>{index};
      } else {
        index_it->second.push_back(index);
      }
    }
  }
}

// check table to get cost. If not existed, compute and store it
uint8_t MatchingGenerator::get_cost(uint i, uint j) {
  static const size_t c = 2 * stripes_num - 3;
  if (i > j) {
    std::swap(i, j);
  }
  size_t index = (size_t)i * (c - i) / 2 + j - 1;
  uint8_t &cost = cost_table[index];
  if (cost == UINT8_MAX) {
    cost = Stripe::compute_cost(stripes[i], stripes[j]);
  }
  return cost;
}

inline uint8_t MatchingGenerator::get_cost(uint64_t token) {
  return get_cost(UINT32_MASK & token, UINT32_MASK & (token >> 32));
}

void MatchingGenerator::mark_matched(uint i, uint j, uint8_t cost, bool flag) {
  stripes[i].matching_index = j;
  stripes[j].matching_index = i;
  stripes[i].selected = stripes[j].selected = true;
  stripes[i].cur_cost = stripes[j].cur_cost = cost;
  if (flag) {
    if (!cost) {
      const std::u16string &pd = stripes[i].parity_dist;
      auto map_it = zero_map.find(pd);
      if (map_it != zero_map.end()) {
        map_it->second.push_front(i);
      } else {
        zero_map[pd] = std::list<uint>{i};
      }
    } else if (stripes[i].parity_dist == stripes[j].parity_dist) {
      none_zero_candidate.push_back(i);
    }
  }
}

void MatchingGenerator::input_stripes(char16_t **raw, char16_t **parity) {
  for (uint i = 0; i < stripes_num; ++i) {
    stripes[i].set_info(i, raw[i], parity[i]);
  }
}

uint MatchingGenerator::print_statics(bool print_flag) {
  uint count[rs_n + 1] = {0};
  bool flag[stripes_num] = {0};
  for (uint i = 0; i < stripes_num; ++i) {
    if (!flag[i] && !flag[stripes[i].matching_index]) {
      ++count[stripes[i].cur_cost];
      flag[stripes[i].matching_index] = flag[i] = true;
    }
  }
  if (print_flag) {
    std::cout << "--- zero rate = " << 200.0 * count[0] / stripes_num << "%\n";
  }
  uint total_costs = 0;
  for (uint8_t i = 0; i <= rs_n; ++i) {
    if (!count[i]) {
      continue;
    }
    total_costs += i * count[i];
    if (print_flag) {
      std::cout << "cost = " << +i << " :  " << count[i] << '\n';
    }
  }
  if (print_flag) {
    std::cout << "Total_Costs = " << total_costs << '\n';
  }
  return total_costs;
}

uint MatchingGenerator::basic_greedy(bool remain_flag) {
  if (node_num < 2 * rs_k + rs_m) {
    std::cerr << "bad input parameter!\n";
    exit(-1);
  }
  uint8_t cost;
  uint64_t token;
  uint i, j;
  std::vector<uint64_t> level[rs_n + 1];
  if (!remain_flag) {
    for (i = 0; i < stripes_num; ++i) {
      for (j = i + 1; j < stripes_num; ++j) {
        cost = get_cost(i, j);
        token = (uint64_t)j << 32 | i;
        level[cost].push_back(token);
      }
    }
  } else {
    for (i = 0; i < stripes_num; ++i) {
      if (stripes[i].selected) {
        continue;
      }
      for (j = i + 1; j < stripes_num; ++j) {
        if (stripes[j].selected) {
          continue;
        }
        cost = get_cost(i, j);
        token = (uint64_t)j << 32 | i;
        level[cost].push_back(token);
      }
    }
  }

  uint selected[rs_n + 1] = {0};
  uint num_to_select;
  num_to_select =
      remain_flag ? remaining_stripes.size() >> 1 : stripes_num >> 1;
  bool flag[stripes_num] = {0};
  for (uint8_t cur_cost = 0; cur_cost <= rs_n && num_to_select; ++cur_cost) {
    for (auto &m : level[cur_cost]) {
      i = m & UINT32_MASK;
      j = (m >> 32) & UINT32_MASK;
      if (!flag[i] && !flag[j]) {
        ++selected[cur_cost];
        flag[i] = flag[j] = true;
        mark_matched(i, j, cur_cost, false);
        if (!(--num_to_select)) {
          break;
        }
      }
    }
  }
  if (!remain_flag) {
    uint total_costs = 0;
    for (uint8_t cur_cost = 0; cur_cost <= rs_n; ++cur_cost) {
      if (!selected[cur_cost]) {
        continue;
      }
      total_costs += cur_cost * selected[cur_cost];
    }
    return total_costs;
  } else {
    return 0;
  }
}

bool MatchingGenerator::single_search(uint index,
                                      std::list<uint> &search_domain,
                                      uint8_t target_cost) {
  static std::queue<std::list<uint>::iterator> cleanup_queue;
  auto self_it = search_domain.end();
  auto peer_it = search_domain.end();
  uint8_t min_cost = stripes[index].cur_cost;
  uint8_t tmp_cost;
  uint min_cost_index = UINT32_MAX;
  for (auto it = search_domain.begin(); it != search_domain.end(); ++it) {
    uint &j = *it;
    if (stripes[j].selected) {
      cleanup_queue.push(it);
      continue;
    }
    if (j == index) {
      self_it = it;
      continue;
    }
    if (min_cost > (tmp_cost = get_cost(index, j))) {
      min_cost = tmp_cost;
      peer_it = it;
      min_cost_index = j;
      if (min_cost == target_cost) {
        break;
      }
    }
  }
  if (min_cost < stripes[index].cur_cost) {
    stripes[index].cur_cost = min_cost;
    stripes[index].matching_index = min_cost_index;
    if (min_cost == target_cost) {
      search_domain.erase(peer_it);
      if (self_it != search_domain.end()) {
        search_domain.erase(self_it);
      }
    }
  }
  while (!cleanup_queue.empty()) {
    auto x = cleanup_queue.front();
    cleanup_queue.pop();
    search_domain.erase(x);
  }
  return min_cost == target_cost;
}

bool MatchingGenerator::search_for_matching(uint index, uint8_t target_cost) {
  static std::queue<std::list<std::u16string>::iterator> cleanup_queue;
  static std::u16string key;
  static const uint8_t all_one = (1 << rs_m) - 1;
  const std::u16string &pd = stripes[index].parity_dist;
  if (!target_cost) {
    if (parity_map[pd].size() > 1) {
      return single_search(index, parity_map[pd], target_cost);
    } else {
      return false;
    }
  } else {
    bool success_flag = false;
    int x, y, bits;
    int k = static_cast<int>(rs_m - target_cost), comb = (1 << k) - 1;
    auto index_it = partial_dist_map.end();
    // black magic to get k-subset
    while (!success_flag && comb < all_one) {
      key.clear();
      bits = comb;
      for (auto &x : pd) {
        if (bits & 1) {
          key.push_back(x);
        }
        bits >>= 1;
      }
      key.push_back(static_cast<char16_t>(comb));
      if ((index_it = partial_dist_map.find(key)) != partial_dist_map.end() &&
          index_it->second.size() > 1) {
        success_flag = single_search(index, index_it->second, target_cost);
      }
      // get next comb of k-subset
      x = comb & (-comb), y = x + comb;
      comb = (((comb & ~y) / x) >> 1) | y;
    }
    return success_flag;
  }
}

void MatchingGenerator::dist_based_search() {
  if (node_num < 2 * rs_k + rs_m) {
    std::cerr << "bad input parameter!\n";
    exit(-1);
  }
  build_map();

  std::queue<std::list<uint>::iterator> it_to_del;
  for (uint8_t cost = 0; cost < rs_m && !remaining_stripes.empty(); ++cost) {
    for (auto cur_it = remaining_stripes.begin();
         cur_it != remaining_stripes.end(); ++cur_it) {
      uint &cur_stripe = *cur_it;
      if (stripes[cur_stripe].selected) {
        it_to_del.push(cur_it);
        continue;
      }
      bool succeed = false;
      if (stripes[cur_stripe].cur_cost == cost) {
        if (!stripes[stripes[cur_stripe].matching_index].selected) {
          succeed = true;
        } else {
          stripes[cur_stripe].matching_index = UINT32_MAX;
          stripes[cur_stripe].cur_cost = UINT8_MAX;
          succeed = search_for_matching(cur_stripe, cost);
        }
      } else {
        succeed = search_for_matching(cur_stripe, cost);
      }
      if (succeed) {
        mark_matched(cur_stripe, stripes[cur_stripe].matching_index,
                     stripes[cur_stripe].cur_cost);
        it_to_del.push(cur_it);
      }
    }
    while (!it_to_del.empty()) {
      auto x = it_to_del.front();
      it_to_del.pop();
      remaining_stripes.erase(x);
    }
  }

  // ****  former method to deal with remaning stripes, partitial greedy for
  // current stripe
  // ***
  // **
  // while (!remaining_stripes.empty()) {
  //   auto it = remaining_stripes.begin();
  //   uint index = *it;
  //   auto min_it = it;
  //   uint8_t min_cost = UINT8_MAX;
  //   uint8_t tmp_cost = 0;
  //   if (stripes[index].selected) {
  //     remaining_stripes.erase(it);
  //     continue;
  //   }
  //   if (remaining_stripes.size() > 1) {
  //     while (++it != remaining_stripes.end()) {
  //       if (stripes[*it].selected) {
  //         it_to_del.push(it);
  //         continue;
  //       }
  //       if (min_cost > (tmp_cost = get_cost(index, *it))) {
  //         min_cost = tmp_cost;
  //         min_it = it;
  //       }
  //     }
  //     mark_matched(index, *min_it, min_cost);
  //     while (!it_to_del.empty()) {
  //       auto x = it_to_del.front();
  //       it_to_del.pop();
  //       remaining_stripes.erase(x);
  //     }
  //     remaining_stripes.erase(min_it);
  //     remaining_stripes.erase(remaining_stripes.begin());
  //   } else {
  //     std::cerr << "stripes size error!\n";
  //     break;
  //   }
  // }

  // new change: call naive_greedy with remaning stripes
  if (!remaining_stripes.empty()) {
    basic_greedy(true);
  }

  make_optimization();
}

void MatchingGenerator::make_optimization() {
  auto map_it = zero_map.end();
  // uint count = 0;
  for (auto cur_i : none_zero_candidate) {
    const uint cur_j = stripes[cur_i].matching_index;
    auto &pd = stripes[cur_i].parity_dist;
    if ((map_it = zero_map.find(pd)) != zero_map.end()) {
      for (auto i : map_it->second) {
        const uint j = stripes[i].matching_index;
        if (!get_cost(cur_i, i) && !get_cost(cur_j, j)) {
          mark_matched(cur_j, j, 0);
          mark_matched(cur_i, i, 0, false);
          // ++count;
          break;
        } else if (!get_cost(cur_i, j) && !get_cost(cur_j, i)) {
          mark_matched(cur_i, j, 0);
          mark_matched(cur_j, i, 0, false);
          // ++count;
          break;
        }
      }
    }
  }
  // std::cout << "\nmake_optimization for: " << count << "\n";
}

void MatchingGenerator::get_one_scheme(uint index) {
  uint8_t host_blocks_num[node_num] = {0};
  const uint &pair_index = stripes[index].matching_index;
  const Stripe &a = stripes[index];
  const Stripe &b = stripes[pair_index];

  for (uint8_t i = 0; i < rs_k; ++i) {
    ++host_blocks_num[a.data_dist[i]];
    ++host_blocks_num[b.data_dist[i]];
  }

  char16_t parity_dest;
  for (uint8_t i = 0; i < rs_m; ++i) {
    if (a.parity_dist[i] == b.parity_dist[i]) {
      parity_dest = a.parity_dist[i];
    } else {
      if (!host_blocks_num[a.parity_dist[i]]) {
        parity_dest = a.parity_dist[i];
        blocks_to_sent.emplace_back(b.parity_dist[i] + 1, parity_dest + 1,
                                    i + 1);
      } else {
        parity_dest = b.parity_dist[i];
        blocks_to_sent.emplace_back(a.parity_dist[i] + 1, parity_dest + 1,
                                    i + 1);
      }
    }
    ++host_blocks_num[parity_dest];
  }
  uint8_t j;
  std::deque<uint16_t> available_nodes;
  std::queue<uint16_t> remain_nodes;
  uint16_t i, target;
  for (i = 0; i < node_num; ++i) {
    if (host_blocks_num[i] > 1) {
      remain_nodes.push(i);
    } else if (!host_blocks_num[i]) {
      available_nodes.push_back(i);
    }
  }
  std::shuffle(available_nodes.begin(), available_nodes.end(),
               std::default_random_engine(index));
  while (!remain_nodes.empty()) {
    i = remain_nodes.front();
    remain_nodes.pop();
    for (j = host_blocks_num[i] - 1; !available_nodes.empty() && j; --j) {
      target = available_nodes.front();
      available_nodes.pop_back();
      ++host_blocks_num[target];
      blocks_to_sent.emplace_back(i + 1, target + 1, 0);
    }
  }
}

std::vector<MigrationInfo> MatchingGenerator::get_sending_scheme() {
  if (blocks_to_sent.empty()) {
    bool flag[stripes_num] = {0};
    for (uint i = 0; i < stripes_num; ++i) {
      if (!flag[i] && !flag[stripes[i].matching_index]) {
        flag[stripes[i].matching_index] = flag[i] = true;
        if (stripes[i].cur_cost) {
          get_one_scheme(i);
        }
      }
    }
  }
  return blocks_to_sent;
}

void MatchingGenerator::random_match() {
  std::vector<uint> pos;
  for (uint i = 0; i < stripes_num; ++i) {
    pos.push_back(i);
  }
  srand(time(nullptr));
  std::random_shuffle(pos.begin(), pos.end());
  for (uint i = 0; i < stripes_num; i += 2) {
    uint8_t cost = get_cost(pos[i], pos[i + 1]);
    mark_matched(pos[i], pos[i + 1], cost, false);
  }
}

void print_mem_size(size_t s) {
  if (s < (1 << 10)) {
    std::cout << s << " Bytes\n";
  } else if (s < (1 << 20)) {
    std::cout << s / 1024.0 << " KiB\n";
    // } else if (s < (1 << 30)) {
  } else {
    std::cout << s / 1024.0 / 1024.0 << " MiB\n";
    // std::cout << s / 1024.0 / 1024.0 / 1024.0 << " GiB\n";
  }
}

void MatchingGenerator::build_map_test() {
  build_map();
  size_t p_mem = 0;
  size_t key_len_count = 0, key_count = 0, set_size_count = 0;
  for (auto it = parity_map.begin(); it != parity_map.end();
       ++it, ++key_count) {
    key_len_count += it->first.length();
    set_size_count += it->second.size();
  }
  for (auto it = partial_dist_map.begin(); it != partial_dist_map.end();
       ++it, ++key_count) {
    key_len_count += it->first.length();
    set_size_count += it->second.size();
  }
  p_mem = key_len_count * sizeof(char16_t) + key_count * sizeof(parity_map) +
          set_size_count * sizeof(std::list<uint>) +
          remaining_stripes.size() * sizeof(uint);
  // std::cout << "key_count = " << key_count << "\n";
  // std::cout << "key_len_count = " << key_len_count << "\n";
  // std::cout << "set_size_count = " << set_size_count << "\n";
  std::cout << "parity-based relevant: ";
  print_mem_size(p_mem);

  size_t common_mem = sizeof(MatchingGenerator) + p_mem;
  common_mem += stripes_num * (sizeof(Stripe) + sizeof(char16_t) * rs_n);
  common_mem += cost_table.size();

  std::cout << "all elements size: ";
  print_mem_size(common_mem);
}