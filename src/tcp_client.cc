#include "tcp_client.hh"
using namespace std::chrono;

TCPClient::TCPClient() : TCPClient(1, std::string("localhost"), 12345){};

TCPClient::~TCPClient() = default;

// method to connect to one node
void TCPClient::connect_one(sockpp::inet_address sock_addr, uint16_t index) {
  sockpp::tcp_connector &cur_conn = index ? node_conns[index] : conn;

  // keep reconnecting if fail, with simple sleep
  while (!(cur_conn = sockpp::tcp_connector(sock_addr))) {
    std::this_thread::sleep_for(milliseconds(1));
  }
  // send the index of the local host
  cur_conn.write_n(&node_index, sizeof(node_index));
  // std::cout << "connect_one: " << sock_addr << "\n";
}

TCPClient::TCPClient(uint16_t i, const std::string &_host, in_port_t _port)
    : ThreadPool<MigrationInfo>(1),
      dest_host{_host},
      dest_port{_port},
      node_index(i),
      node_conns{nullptr},
      req_mtxs{nullptr} {
  sockpp::socket_initializer sockInit;
  connect_one({_host, _port}, 0);
}

// create with a set of socket addresses and a thread pool
TCPClient::TCPClient(uint16_t i, std::vector<socket_address> &sa, uint thr_num)
    : ThreadPool<MigrationInfo>(thr_num),
      node_index(i),
      conn_num{static_cast<uint16_t>(sa.size())},
      node_conns(new sockpp::tcp_connector[sa.size() + 2]),
      req_mtxs{new std::mutex[sa.size() + 2]} {
  sockpp::socket_initializer sockInit;
  std::thread connect_thr[sa.size() + 2];
  uint16_t j = 1;
  // connect to different nodes concurrently
  for (auto &addr : sa) {
    if (j == node_index) {
      ++j;
    }
    connect_thr[j] = std::thread(&TCPClient::connect_one, this,
                                 sockpp::inet_address{addr.ip, addr.port}, j);
    ++j;
  }
  for (j = 1; j <= conn_num + (node_index ? 1 : 0); ++j) {
    if (j == node_index) {
      continue;
    } else {
      connect_thr[j].join();
    }
  }
  // std::cout << "init TCPClient ok\n";
}

// method to deal with sending task
void TCPClient::run() {
  char *buf = new char[CHUNK_SIZE]();
  static bool lck_flag = req_mtxs != nullptr;

  while (true) {
    // get a task from the queue
    MigrationInfo task = get_task();
    // an empty task means it's time to shutdown
    if (!task.source && !task.target) {
      break;
    }
    uint16_t host = node_index ? task.target : task.source;
    sockpp::tcp_connector *cur_conn;
    // make sure only one worker is sending to a node, to avoid content
    // confusing
    std::unique_lock<std::mutex> lck;
    if (lck_flag) {
      lck = std::unique_lock<std::mutex>(req_mtxs[host]);
      cur_conn = &node_conns[host];
    } else {
      cur_conn = &conn;
    }
    // send a task header first
    if (cur_conn->write_n(&task, sizeof(task)) == -1) {
      std::cerr << "send task header error in task " << task.index << ": "
                << +task.source << "->" << +task.target << std::endl;
      exit(-1);
    }
    // for data-node, it then read from a file and send the data though socket
    if (node_index) {
      FILE *f = fopen("test/stdfile", "r");
      if (!f) {
        std::cerr << "fopen failed in TCPClient\n";
        exit(-1);
      }

      ssize_t n, i = 0, remain = CHUNK_SIZE;
      while (!feof(f) && (n = fread(buf + i, 1, remain, f)) > 0) {
        cur_conn->write_n(buf + i, n);
        i += n;
        remain -= n;
      }
      fclose(f);
    }
  }
  delete[] buf;
}

// for master-node, tell the data-nodes to shutdown
void TCPClient::send_shutdown_signal() {
  MigrationInfo signal;
  for (uint16_t i = 1; i <= conn_num + (node_index ? 1 : 0); ++i) {
    if (i == node_index) {
      continue;
    }
    if (node_conns[i].write_n(&signal, sizeof(signal)) == -1) {
      std::cout << "send shutdown signal error" << std::endl;
      exit(-1);
    }
    node_conns[i].close();
  }
}

// just for test, generate some toy tasks
void TCPClient::demo_tasks() {
  for (uint i = 0; i < 20; ++i) {
    MigrationInfo task(0, 1, i & 1);
    task.index = i;
    if (i & 1) {
      strcpy(task.target_fn, "test/stdfile");
    }
    add_task(std::move(task));
  }
  set_finished();
}

// for data-node, waiting for the master-node to send a starting signal
void TCPClient::wait_start_flag(uint16_t index) {
  char start_flag;
  sockpp::tcp_connector *cur_conn = index ? &node_conns[index] : &conn;
  if (cur_conn->read_n(&start_flag, 1) == -1) {
    std::cerr << "get start flag error in " << +index << std::endl;
    exit(-1);
  }
}

// waiting for other nodes to send a starting signal
void TCPClient::start_client() {
  // std::cout << "TCPClient::start_client\n";
  if (req_mtxs) {
    std::thread wait_thrds[conn_num + 2];
    uint16_t i;
    for (i = 1; i <= conn_num + (node_index ? 1 : 0); ++i) {
      if (i == node_index) {
        continue;
      }
      wait_thrds[i] = std::thread(&TCPClient::wait_start_flag, this, i);
    }
    for (i = 1; i <= conn_num + (node_index ? 1 : 0); ++i) {
      if (i == node_index) {
        continue;
      }
      wait_thrds[i].join();
    }
  } else {
    std::thread wait_thr(&TCPClient::wait_start_flag, this, 0);
    wait_thr.join();
  }

  start_threads();
  wait_for_finish();
  send_shutdown_signal();
}
