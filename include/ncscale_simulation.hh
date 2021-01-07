#ifndef NCSCALE_SIMULATION_HH
#define NCSCALE_SIMULATION_HH

#include <iostream>
#include <vector>

#include "migration_info.hh"

class NCScaleSimulation {
 private:
  uint rs_k;
  uint rs_m;
  uint rs_n;
  uint batch_size;
  uint scaling_num;
  uint single_scaling_num;
  uint single_total_cost;
  uint single_mig_cost;
  uint single_delta_cost;
  uint old_migration_num;
  uint old_delta_num;
  uint new_recv_num;

 public:
  NCScaleSimulation() = default;
  ~NCScaleSimulation() = default;
  NCScaleSimulation(uint k, uint m);

  uint get_batch_size();
  uint get_total_costs();
  uint get_similar_batch_size(uint s);
  std::vector<MigrationInfo> get_scheme(uint stripes_num);
};

#endif