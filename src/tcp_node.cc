#include "tcp_node.hh"

#include <algorithm>
#include <fstream>
#include <iostream>

TCPNode::TCPNode(uint16_t i, uint _exp_size, uint _scheme_type)
    : self_index(i),
      server{nullptr},
      client{nullptr},
      exp_size(_exp_size),
      scheme_type(_scheme_type) {
  std::string fn("config/nodes_config.ini");
  node_num = parse_config(fn);
}

uint16_t TCPNode::parse_config(const std::string &fn) {
  std::fstream config(fn.c_str(), std::ios_base::in);
  if (!config.is_open()) {
    std::cerr << "Failed to open config." << std::endl;
    exit(-1);
  }
  uint16_t num = 1;
  std::string ip;
  in_port_t port;
  if (!config.eof()) {
    config >> rs_k >> rs_m;
    N = 2 * rs_k + rs_m;
  } else {
    std::cerr << "First line is k, m and N" << std::endl;
    exit(-1);
  }
  while (!config.eof()) {
    config >> ip >> port;
    if (config.fail()) {
      break;
    }
    if (num != self_index) {
      targets.emplace_back(ip, port);
    } else {
      // self_address.ip = ip;
      self_address.port = port;
    }
    ++num;
  }
  if (num < 2) {
    std::cerr << "Nodes num >=2.\n"
              << "[IP_address]  [Port]" << std::endl;
    exit(-1);
  }
  return num - 1;
}

void TCPNode::start() {
  if (self_index) {
    // init dirs
    char dir_base[64], command[128];
    for (uint16_t client_index = 1; client_index <= node_num; ++client_index) {
      if (self_index == client_index) {
        continue;
      }
      sprintf(dir_base, "test/store/%u_%u", self_index, client_index);
      sprintf(command, "mkdir -p %s;rm %s/* -f", dir_base, dir_base);
      system(command);
    }
    // start threads of client and server
    server = new TCPServer(self_address.port, node_num, rs_k, rs_m, node_num);
    std::thread wait_svr_thr([&] { server->wait_for_connection(); });
    client = new TCPClient(self_index, targets);
    std::thread cli_thr([&] { client->start_client(); });
    wait_svr_thr.join();
    std::thread svr_thr([&] { server->start_serving(); });
    // get task from master
    while (true) {
      // the tasks in server's queue come from master
      MigrationInfo task = server->get_task();
      if (!task.source && !task.target) {  // means stop
        break;
      } else {
        // let the client send block to finish task
        client->add_task(std::move(task));
      }
    }
    client->set_finished();
    cli_thr.join();
    svr_thr.join();
    // std::cout << "tcp_node finished!" << std::endl;
  } else {
    client = new TCPClient(self_index, targets);
    std::thread cli_thr([&] { client->start_client(); });

    // get NCScale batch size
    NCScaleSimulation ns(rs_k, rs_m);
    uint actual_size = ns.get_similar_batch_size(exp_size);
    std::cout << "k = " << rs_k << ", m = " << rs_m
              << ", actual_size = " << actual_size << "\n";

    // get scheme
    std::vector<MigrationInfo> scheme;
    if (scheme_type == 2) {
      scheme = std::move(ns.get_scheme(actual_size));
      std::cout << "NCScale ok\n";
    } else {
      char16_t **raw_dist = new char16_t *[actual_size];
      char16_t **parity_dist = new char16_t *[actual_size];
      for (uint i = 0; i < actual_size; ++i) {
        raw_dist[i] = new char16_t[rs_k];
        parity_dist[i] = new char16_t[rs_m];
      }
      Stripe::generate_random_distributions(raw_dist, parity_dist, N,
                                            actual_size, rs_k, rs_m);

      struct timeval start_time, end_time;
      double time_1;
      gettimeofday(&start_time, nullptr);

      MatchingGenerator mg(N, rs_k, rs_m, actual_size);
      mg.input_stripes(raw_dist, parity_dist);
      if (scheme_type) {
        mg.basic_greedy();
        std::cout << "naive_greedy ok\n";
      } else {
        mg.dist_based_search();
        std::cout << "dist_based_search ok\n";
      }
      scheme = std::move(mg.get_sending_scheme());

      gettimeofday(&end_time, nullptr);
      time_1 = (end_time.tv_sec - start_time.tv_sec) +
               (end_time.tv_usec - start_time.tv_usec) / 1000000.0;
      std::cout << "tot_costs = " << scheme.size()
                << "\n** scheme generation_time: " << time_1 << "\n";

      for (uint i = 0; i < actual_size; ++i) {
        delete[] raw_dist[i];
        delete[] parity_dist[i];
      }
      delete[] raw_dist;
      delete[] parity_dist;
    }
    std::random_shuffle(scheme.begin(), scheme.end());
    const uint exp_transfer_size = scheme.size();
    uint i = 0;
    while (i < exp_transfer_size) {
      auto &task = scheme[i];
      task.index = i;
      if (task.coefficient) {
        strcpy(task.target_fn, "test/test_file");
      }
      sprintf(task.store_fn, "test/store/%u_%u/f_%u", task.target, task.source,
              i++);
    }
    std::cout << "exp_transfer_size size: " << exp_transfer_size << "\n";

    // add the tasks from above scheme
    for (i = 0; i < exp_transfer_size; ++i) {
      auto &task = scheme[i];
      if (task.source == task.target) {
        std::cerr << "=== error task " << task.index << " , " << task.source
                  << "->" << task.target << std::endl;
        exit(-1);
      }
      client->add_task(std::move(task));
    }
    client->set_finished();
    cli_thr.join();
    std::cout << "tcp_node of master finished!" << std::endl;
  }
}

TCPNode::~TCPNode() {
  delete server;
  delete client;
}
