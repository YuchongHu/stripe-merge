#include "tcp_node.hh"

using namespace std;

int main(int argc, char *argv[]) {
  if (!((argc == 2 && argv[1][0] != '0') || (argc == 4 && argv[1][0] == '0'))) {
    cerr << "Usage: "
         << "[Node_index] <exp_size> <0_StripeMerge-P, 1_StripeMerge-G, "
            "2_NCScale>"
         << endl;
    exit(-1);
  }

  if (argv[1][0] != '0') {
    TCPNode node(strtoul(argv[1], nullptr, 10));
    node.start();
  } else {
    uint exp_type = strtoul(argv[3], nullptr, 10);
    if (exp_type > 2) {
      cerr << "exp_type error!" << endl;
      exit(-1);
    }
    TCPNode node(strtoul(argv[1], nullptr, 10), strtoul(argv[2], nullptr, 10),
                 exp_type);
    node.start();

    return 0;
  }
}