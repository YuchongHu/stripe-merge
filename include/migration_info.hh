#ifndef MIGRATION_INFO_HH
#define MIGRATION_INFO_HH

#include <cstdint>

class MigrationInfo {
 public:
  uint index;
  int16_t source;
  int16_t target;

  char* mem_ptr;
  uint8_t coefficient;
  char target_fn[32];
  char store_fn[32];

  MigrationInfo() : source(0), target(0), mem_ptr(nullptr){};
  ~MigrationInfo() = default;
  MigrationInfo(int16_t s, int16_t t, uint8_t coef)
      : source(s), target(t), mem_ptr{nullptr}, coefficient(coef) {}
};

#endif