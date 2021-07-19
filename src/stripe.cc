#include "stripe.hh"

uint16_t Stripe::node_num;
uint8_t Stripe::rs_k;
uint8_t Stripe::rs_m;

Stripe::Stripe()
    : stripe_index(UINT32_MAX),
      matching_index(UINT32_MAX),
      cur_cost(UINT8_MAX),
      selected{false} {}

Stripe::~Stripe() {}

void Stripe::set_info(uint _stripe_idnex, char16_t *data_dist_src,
                      char16_t *parity_dist_src) {
  stripe_index = _stripe_idnex;
  data_dist.assign(data_dist_src, rs_k);
  parity_dist.assign(parity_dist_src, rs_m);
}

void Stripe::set_params(uint16_t _node_num, uint8_t k, uint8_t m) {
  node_num = _node_num;
  rs_k = k;
  rs_m = m;
}

bool Stripe::cmp_pairty(const Stripe &a, const Stripe &b) {
  return a.parity_dist == b.parity_dist;
}

// compute the merging cost of two given stripes
uint8_t Stripe::compute_cost(const Stripe &a, const Stripe &b) {
  bool flag[node_num] = {false};
  uint8_t i;
  uint8_t cost = 0;

  // check the cost caused by the excess of data chunks
  for (i = 0; i < rs_k; ++i) {
    if (flag[a.data_dist[i]]) {
      ++cost;
    } else {
      flag[a.data_dist[i]] = true;
    }
    if (flag[b.data_dist[i]]) {
      ++cost;
    } else {
      flag[b.data_dist[i]] = true;
    }
  }

  // check the cost of misaligned parity blocks
  //     & the overflow after determining the new parity blocks' placement
  char16_t parity_dest;
  for (i = 0; i < rs_m; ++i) {
    if (a.parity_dist[i] == b.parity_dist[i]) {
      parity_dest = a.parity_dist[i];
    } else {
      ++cost;
      parity_dest =
          flag[a.parity_dist[i]] ? b.parity_dist[i] : a.parity_dist[i];
    }
    if (flag[parity_dest]) {
      // need to move away the extra data chunk
      ++cost;
    } else {
      flag[parity_dest] = true;
    }
  }
  return cost;
}

// generate some stripes randomly
void Stripe::generate_random_distributions(char16_t **raw_dist,
                                           char16_t **parity_dist,
                                           uint16_t node_num, uint stripes_num,
                                           uint8_t rs_k, uint8_t rs_m) {
  std::vector<uint16_t> v;
  for (uint16_t i = 0; i < node_num; ++i) {
    v.push_back(i);
  }
  // use current time as the seed
  srand(time(nullptr));
  for (uint i = 0; i < stripes_num; ++i) {
    // shuffle and trim to get a stripe's distribution
    std::shuffle(v.begin(), v.end(), std::default_random_engine(rand()));
    for (uint8_t j = 0; j < rs_k; ++j) {
      raw_dist[i][j] = v[j];
    }
    for (uint8_t c = 0, j = rs_k; c < rs_m; ++c, ++j) {
      parity_dist[i][c] = v[j];
    }
  }
}