#include "tcp_server.hh"

#include <utility>

TCPServer::TCPServer() : TCPServer(15000, 1, 2, 1, 1) {}

TCPServer::~TCPServer() { acc.close(); }

TCPServer::TCPServer(in_port_t _port, uint n, uint8_t k, uint8_t m,
                     uint workers_num)
    : ThreadPool<MigrationInfo>(1),
      port(_port),
      serving_num(n),
      serving_flag(false),
      blocks_num(0),
      finished_num(0),
      rs_k(k),
      rs_m(m),
      mem_pool(2 * n, CHUNK_SIZE),
      w_worker(workers_num, &mem_pool),
      c_worker(workers_num, &w_worker, k, m),
      thr{new std::thread[n]} {
  sockpp::socket_initializer sockInit;
  acc = sockpp::tcp_acceptor(port);

  if (!acc) {
    std::cerr << "Error creating the acceptor: " << acc.last_error_str()
              << std::endl;
    exit(-1);
  }
  // std::cout << "Awaiting connections on port " << port << "..." << std::endl;
}

void TCPServer::receive_handler(sockpp::tcp_socket sock) {
  std::unique_lock<std::mutex> lck(serving_mtx);
  while (!serving_flag) {
    serving_cv.wait(lck, [&] { return serving_flag; });
  }
  lck.unlock();

  char start_flag = 1;
  if (sock.write_n(&start_flag, 1) == -1) {
    std::cerr << "send start flag error" << std::endl;
    exit(-1);
  }

  struct timeval start_time, end_time;
  double time_1;
  while (true) {
    // get header
    MigrationInfo task;
    if (sock.read_n(&task, sizeof(task)) <= 0) {
      break;
    }

    if (task.source == task.target) {
      break;
    }

    // Request free memory block
    char* buffer = mem_pool.get_block();
    task.mem_ptr = buffer;
    // get chunk from socket
    ssize_t n, i = 0, remain = CHUNK_SIZE;
    while (remain && (n = sock.read_n(buffer + i, remain)) > 0) {
      i += n;
      remain -= n;
    }
    // add cur task to compute queue or write queue
    if (task.coefficient) {
      c_worker.add_task(std::move(task));
    } else {
      w_worker.add_task(std::move(task));
    }
  }
  sock.close();
}

void TCPServer::run() {
  std::unique_lock<std::mutex> lck(serving_mtx);
  while (!serving_flag) {
    serving_cv.wait(lck, [&] { return serving_flag; });
  }
  lck.unlock();
  char start_flag = 1;
  if (master_sock.write_n(&start_flag, 1) == -1) {
    std::cerr << "send start flag error" << std::endl;
    exit(-1);
  }

  while (true) {
    // get header
    MigrationInfo task;
    if (master_sock.read_n(&task, sizeof(task)) <= 0) {
      break;
    }
    add_task(std::move(task));
  }
  set_finished();
  master_sock.close();
}

void TCPServer::wait_for_connection() {
  // wait for all nodes to be connected
  uint16_t i = 0;
  uint16_t client_index;
  while (i < serving_num) {
    sockpp::inet_address peer;
    // Accept a new client connection
    sockpp::tcp_socket sock = acc.accept(&peer);
    if (!sock) {
      std::cerr << "Error accepting incoming connection: "
                << acc.last_error_str() << std::endl;
    } else {
      sock.read_n(&client_index, sizeof(client_index));  // get client index
      if (client_index) {                                // for data node
        thr[i++] =
            std::thread(&TCPServer::receive_handler, this, std::move(sock));
      } else {  // for master node
        master_sock = std::move(sock);
        thr[i++] = std::thread(&TCPServer::run, this);
      }
    }
  }
}

void TCPServer::start_serving() {
  struct timeval start_time, end_time;
  gettimeofday(&start_time, nullptr);
  // start workers
  w_worker.start_threads();
  c_worker.start_threads();
  std::unique_lock<std::mutex> lck(serving_mtx);
  serving_flag = true;
  lck.unlock();
  serving_cv.notify_all();

  for (uint16_t i = 0; i < serving_num; ++i) {
    thr[i].join();
  }
  c_worker.set_finished();
  c_worker.wait_for_finish();
  w_worker.wait_for_finish();
  gettimeofday(&end_time, nullptr);

  double time_1 = (end_time.tv_sec - start_time.tv_sec) +
                  (end_time.tv_usec - start_time.tv_usec) / 1000000.0;
  std::cout << time_1 << "\n";
}