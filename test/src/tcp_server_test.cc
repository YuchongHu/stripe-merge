#include "tcp_server.hh"

#include <iostream>
using namespace std;

int main(int argc, char* argv[]) {
  if (argc != 2) {
    cerr << "serving_num is required." << endl;
    exit(-1);
  }
  uint n = strtoul(argv[1], nullptr, 10);
  TCPServer server(15000, n, 4, 2, n);
  server.start_serving();
}