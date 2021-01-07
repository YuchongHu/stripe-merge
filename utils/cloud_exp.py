import subprocess
import threading
import sys

user_name = "yao"
dir_base = "~/stripe-merge"
exp_loop = 10

results = []


def send_one(index, program_flag):
    if program_flag:
        cmd = "scp ./build/bin/node_main {}@node{:0>2d}:{}/".format(user_name,
                                                                    index, dir_base)
    else:
        cmd = "scp ./config/nodes_config.ini {}@node{:0>2d}:{}/config/".format(user_name,
                                                                               index, dir_base)
    subprocess.call(cmd, shell=True, stdout=subprocess.DEVNULL)


def update(n, update_type):
    thread_list = []
    if update_type == "all":
        for i in range(1, n + 1):
            t = threading.Thread(target=send_one, args=(i, True))
            t.start()
            thread_list.append(t)
            t = threading.Thread(target=send_one, args=(i, False))
            t.start()
            thread_list.append(t)
    else:
        for i in range(1, n + 1):
            t = threading.Thread(target=send_one, args=(
                i, update_type == "program"))
            thread_list.append(t)
            t.start()
    for t in thread_list:
        t.join()
    print("== update ok ==")


def call_single_node(index):
    cmd = "ssh {}@node{:0>2d} \"cd ;./node_main {}\"".format(
        user_name, index, index)
    get_bytes = subprocess.check_output(cmd, shell=True)
    result = get_bytes.decode('utf-8')
    results.append(float(result))


def call_nodes(n):
    thread_list = []
    for i in range(1, n + 1):
        t = threading.Thread(target=call_single_node, args=(i,))
        t.start()
        thread_list.append(t)
    for t in thread_list:
        t.join()
    print("== call nodes ok ==")


def call_master(size, exp_type):
    subprocess.call(
        "./build/bin/node_main 0 {} {}".format(size, exp_type), shell=True)


def generate_config(k, m, nodes_num):
    lines = []
    with open("config/nodes_ip.txt", "r") as f:
        lines = f.readlines()
    if lines:
        with open("config/nodes_config.ini", "w") as wf:
            wf.write("{} {}\n".format(k, m))
            for i in range(nodes_num):
                wf.write(lines[i].strip() + " 15000\n")
    print("generate_config ok")


if __name__ == "__main__":
    if len(sys.argv) == 5 and sys.argv[1] == "get_config":
        generate_config(int(sys.argv[2]), int(sys.argv[3]), int(sys.argv[4]))
    elif len(sys.argv) == 4 and sys.argv[1] == "update":
        update(int(sys.argv[3]), sys.argv[2])
    elif len(sys.argv) == 7:
        nodes_num = int(sys.argv[5])
        for i in range(exp_loop):
            results.clear()
            print("---- i = %d ----" % i)
            master_t = threading.Thread(
                target=call_master, args=(sys.argv[2], sys.argv[6]))
            master_t.start()
            nodes_t = threading.Thread(target=call_nodes, args=(nodes_num,))
            nodes_t.start()
            master_t.join()
            nodes_t.join()
            results.sort(reverse=True)
            print("%f\n" % results[0])

    else:
        print("Usage: exp  stripes_size  k  m  nodes_num  [0-p, 1-g, 2-ncs]")
        print("   Or: update  [all/config/program]  nodes_num")
        print("   Or: get_config  k  m  nodes_num")
