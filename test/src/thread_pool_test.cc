#include "compute_worker.hh"
#include "memory_pool.hh"
#include "write_worker.hh"
using namespace std;
using namespace std::chrono;

int main() {
  const uint producer_num = 2, size = 1 << 26, thread_num = 2, tot_num = 24;
  MemoryPool mp(2 * producer_num, size);
  WriteWorker ww(thread_num, &mp);
  ComputeWorker cw(thread_num, &ww, 4, 2);
  ww.start_threads();
  cw.start_threads();

  for (uint i = 0; i < tot_num; ++i) {
    MigrationInfo info(i, i + 1, i & 1 ? 1 : 0);
    info.index = i;
    info.mem_ptr = mp.get_block();
    sprintf(info.mem_ptr, "This is file_%u", i);
    sprintf(info.store_fn, "test/store/file_%u", i);
    this_thread::sleep_for(milliseconds(500));
    cout << "recv " << i << "\n";
    cw.add_task(move(info));
  }
  cout << "==== all tasks had been added! ===\n";
  ww.wait_for_finish();
  return 0;
}