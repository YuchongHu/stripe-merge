#include "tcp_client.hh"

#include <iostream>
#include <string>
using namespace std;

int main(int argc, char *argv[]) {
  if (argc != 2) {
    cerr << "index is required" << endl;
    exit(1);
  }
  TCPClient client(strtoul(argv[1], nullptr, 10), string("localhost"), 15000);
  client.start_client();
}