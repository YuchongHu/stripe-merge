#include "ncscale_simulation.hh"

/**
 * Here's just a rough implementation of generating NCScale's tasks for our
 * experiment system. Just some dirty works. Can be skipped.
 */

NCScaleSimulation::NCScaleSimulation(uint k, uint m)
    : rs_k(k), rs_m(m), rs_n(k + m) {
  batch_size = 2 * k * (k + m) * (2 * k + m);
  single_total_cost = rs_n - 1;
  single_mig_cost = k;
  single_delta_cost = m - 1;
  scaling_num = batch_size / 2;
  single_scaling_num = scaling_num / rs_n;
  old_migration_num = k * single_scaling_num;
  old_delta_num = single_delta_cost * single_scaling_num;
  new_recv_num = single_scaling_num * rs_n;
}

uint NCScaleSimulation::get_batch_size() { return batch_size; }

uint NCScaleSimulation::get_total_costs() {
  return scaling_num * single_total_cost;
}

uint NCScaleSimulation::get_similar_batch_size(uint s) {
  uint ratio = s / batch_size;
  return ratio * batch_size;
}

std::vector<MigrationInfo> NCScaleSimulation::get_scheme(uint stripes_num) {
  uint ratio = stripes_num / batch_size;

  if (!ratio) {
    std::cout << "bad input stripes_num. less than single batch_size!"
              << std::endl;
    exit(-1);
  }
  uint per_migration_num = ratio * old_migration_num;
  uint per_delta_num = ratio * old_delta_num;
  uint per_new_recv_num = ratio * new_recv_num;
  std::vector<MigrationInfo> scheme;
  std::vector<uint> new_mig_recv(rs_k, per_new_recv_num);
  std::vector<uint> old_mig_send(rs_n, per_migration_num);
  std::vector<uint> old_delta_send(rs_n, per_delta_num);
  std::vector<uint> old_delta_recv(rs_n, per_delta_num);

  // migrations from old to new
  uint i = 0, j = 0, k = 0;
  uint total_migration = rs_n * per_migration_num;
  while (i < total_migration) {
    while (!old_mig_send[j]) {
      j = ++j % rs_n;
    }
    while (!new_mig_recv[k]) {
      k = ++k % rs_k;
    }
    scheme.emplace_back(j + 1, k + rs_n + 1, 0);
    --old_mig_send[j];
    j = ++j % rs_n;
    --new_mig_recv[k];
    k = ++k % rs_k;
    ++i;
  }
  // delta blocks between the olds
  uint bad_pair_node, bad_pair_count, limit;
  uint send_empty_count = 0, recv_empty_count = 0;
  i = j = 0, k = 1;
  uint total_delta = rs_n * per_delta_num;
  while (i < total_delta) {
    while (!old_delta_send[j]) {
      j = ++j % rs_n;
    }
    while (!old_delta_recv[k]) {
      k = ++k % rs_n;
    }
    if (j == k) {
      if (send_empty_count < rs_n - 1) {
        j = ++j % rs_n;
        continue;
      } else if (recv_empty_count < rs_n - 1) {
        k = ++k % rs_n;
        continue;
      } else {
        bad_pair_count = old_delta_send[j];
        bad_pair_node = j + 1;
        limit = i - 1;
        break;
      }
    } else {
      scheme.emplace_back(j + 1, k + 1, 1);
    }
    if (!(--old_delta_send[j])) {
      ++send_empty_count;
    }
    j = ++j % rs_n;
    if (!(--old_delta_recv[k])) {
      ++recv_empty_count;
    }
    k = ++k % rs_n;
    ++i;
  }
  // check the error status
  if (i < total_delta) {
    std::cout << "-- bad pair when i = " << i << std::endl;
    uint index = total_migration;
    for (uint t = bad_pair_count; t; --t) {
      while (scheme[index].source == bad_pair_node ||
             scheme[index].target == bad_pair_node) {
        index = index < limit ? ++index : total_migration;
      }
      uint b_target = scheme[index].target;
      scheme[index].target = bad_pair_node;
      scheme.emplace_back(bad_pair_node, b_target, 1);
    }
  }
  return scheme;
}
