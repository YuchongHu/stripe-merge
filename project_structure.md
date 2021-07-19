# Guide

In this documentation, you can find some useful suggestions on how to explore this project and try to understand how every component works.

## Overall View

The prototype system of ***StripeMerge*** includes two parts: implementations of the two algorithms, which also supports standalone simulations with randomly generated stripes, and a rough distributed system for experiments. The latter follows a classic "master-slave" pattern, where the master is in charge of generating the merging schemes, sending tasks to different nodes, and data-nodes, playing roles of slaves, receive tasks from the master, and perform the tasks (receive data from other data-nodes or send their data to other data-nodes). The system exploits multithreading to improve the performance, including the efforts of the thread pool and inner operations pipelining.

## Project Structure

```shell
├── README.md
├── config
│   ├── nodes_config.ini
│   └── nodes_ip.txt
├── include
│   ├── compute_worker.hh
│   ├── ec_setting.hh
│   ├── matching_generator.hh
│   ├── memory_pool.hh
│   ├── migration_info.hh
│   ├── ncscale_simulation.hh
│   ├── stripe.hh
│   ├── tcp_client.hh
│   ├── tcp_node.hh
│   ├── tcp_server.hh
│   ├── thread_pool.hh
│   └── write_worker.hh
├── makefile
├── project_structure.md
├── src
│   ├── blossom.cc
│   ├── compute_worker.cc
│   ├── matching_generator.cc
│   ├── matching_main.cc
│   ├── memory_pool.cc
│   ├── ncscale_simulation.cc
│   ├── node_main.cc
│   ├── stripe.cc
│   ├── tcp_client.cc
│   ├── tcp_node.cc
│   ├── tcp_server.cc
│   └── write_worker.cc
├── test
│   └── src
│       ├── isa-l_test.cc
│       ├── ncscale_test.cc
│       ├── tcp_client_test.cc
│       ├── tcp_server_test.cc
│       ├── tcpecho.cc
│       ├── tcpechosvr.cc
│       └── thread_pool_test.cc
└── utils
    ├── cloud_exp.py
    ├── combination_compute.py
    └── sim_exp.py
```

## Descriptions of Components

1. matching_generator: the implementation of the two algorithms (StripeMerge-G and StripeMerge-P).
2. matching_main: an entrance of calling matching_generator.
3. thread_pool: a thread pool template, which is consisted of some worker-threads, and a shared task queue.
4. compute_worker: thread pool of ec computation.
5. write_worker: thread pool of writing data to a file.
6. migration_info: info of task in experiments.
7. stripe: info of a general stripe.
8. memory_pool: a pool of pre-allocated and fixed-size memory blocks, which is multithreading-safe.
9. ncscale_simulation: a rough implementation of generating NCScale's tasks for our experiment system.
10. tcp_client: it is used to connect to other nodes and send data to other nodes.
11. tcp_node: it is consited of two components: TCPClient and TCPServer, and there is one instance of TCPNode running in every node, and we can assign its role by passing specific parameters in the program's starting command.
12. tcp_server: serving connections and receiving data from other nodes are accomplished in TCPServer.
13. node_main: an entrance of calling tcp_node.

Note that the blossom.cc implements a demo for solving the Maximum Weighted Matching Problem in General Graphs. However this demo was not included or called in the final-version system.

