#ifndef TCP_NODE_HH
#define TCP_NODE_HH

#include <string>
#include <vector>

#include "matching_generator.hh"
#include "ncscale_simulation.hh"
#include "tcp_client.hh"
#include "tcp_server.hh"

/**
 * A TCPNode is consited of two components: TCPClient and TCPServer, where
 * TCPClient is used to connect to other nodes and send data to other nodes,
 * while serving connections and receiving data from other nodes are
 * accomplished in TCPServer.
 *
 * There is one instance of TCPNode running in every node, and we can assign its
 * role by passing specific parameters in the program's starting command.
 *
 * In master-node, TCPNode generates the merging scheme, and send the control
 * commands to corresponding data-nodes. In data-node, TCPNode listens for
 * control commands from the master-node, actions to execute the commands (send
 * data to others or receive data from others).
 */
class TCPNode {
 private:
  TCPClient *client;
  TCPServer *server;
  uint16_t self_index;
  socket_address self_address;
  std::vector<socket_address> targets;
  uint16_t node_num;

  uint rs_k;
  uint rs_m;
  uint exp_size;
  uint N;
  uint scheme_type;

 public:
  explicit TCPNode(uint16_t i, uint _exp_size = 0, uint _scheme_type = 0);
  ~TCPNode();

  uint16_t parse_config(const std::string &fn);
  void start();
};

#endif
