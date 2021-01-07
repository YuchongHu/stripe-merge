// tcpecho.cpp
//
// Simple TCP echo client
//
// --------------------------------------------------------------------------
// This file is part of the "sockpp" C++ socket library.
//
// Copyright (c) 2014-2017 Frank Pagliughi
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// --------------------------------------------------------------------------

#include <sys/time.h>

#include <cstdlib>
#include <iostream>
#include <string>

#include "sockpp/tcp_connector.h"

using namespace std;
using namespace std::chrono;

#define BUF_SIZE 1 << 16

int main(int argc, char *argv[]) {
  cout << "Sample TCP echo client for 'sockpp' " << endl;

  string host = (argc > 1) ? argv[1] : "localhost";
  in_port_t port = (argc > 2) ? atoi(argv[2]) : 12345;

  sockpp::socket_initializer sockInit;

  // Implicitly creates an inet_address from {host,port}
  // and then tries the connection.

  sockpp::tcp_connector conn({host, port});
  if (!conn) {
    cerr << "Error connecting to server at " << sockpp::inet_address(host, port)
         << "\n\t" << conn.last_error_str() << endl;
    return 1;
  }

  cout << "Created a connection from " << conn.address() << endl;

  // Set a timeout for the responses
  if (!conn.read_timeout(seconds(5))) {
    cerr << "Error setting timeout on TCP stream: " << conn.last_error_str()
         << endl;
  }

  FILE *f = fopen("test/test_file", "r");
  if (!f) {
    cerr << "fopen failed\n";
    exit(-1);
  }

  struct timeval start_time, end_time;
  gettimeofday(&start_time, nullptr);
  ssize_t n;
  char *buf = new char[BUF_SIZE];
  while (!feof(f) && (n = fread(buf, 1, BUF_SIZE, f)) > 0) {
    conn.write_n(buf, n);
  }
  fclose(f);
  conn.close();

  gettimeofday(&end_time, nullptr);

  cout << "send ok\n";
  double time_1 = (end_time.tv_sec - start_time.tv_sec) * 1000 +
                  (end_time.tv_usec - start_time.tv_usec) / 1000;
  cout << "finish time: " << time_1 << "ms\n";

  return (!conn) ? 1 : 0;
}
