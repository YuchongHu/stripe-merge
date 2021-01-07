import subprocess

result_base = "exp/data/cloud/"
std_cmds = ["exp/bin/matching_exp", "1000", "72", "16", "4"]

pos = {
    "k": 3,
    "m": 4,
    "N": 2,
    "S": 1
}

exp_range = {
    "k": [2**i for i in range(2, 7)],
    "m": [4],  # range(2, 5),
    "S": [20000*i for i in range(1, 6)]
}


def run_exp(m):
    # print("====== m = %d ======" % m)
    with open(result_base + "num_run_time_%d.csv" % m, "w") as f:
        cmds = [x for x in std_cmds]
        cmds[pos["m"]] = str(m)
        for S in exp_range["S"]:
            cmds[pos["S"]] = str(S)
            info = "*** S = %d ***" % S
            print(info)
            f.write("S=%d, " % S)
            # N = 2*(2*k + m)
            # cmds[pos["N"]] = str(N)
            output = subprocess.check_output(cmds).decode("utf-8")
            f.write(output)
            f.flush()

# def run_exp(m):
#     print("====== m = %d ======" % m)
#     with open(result_base + "N1N2_scale_m_%d.csv" % m, "w") as f:
#         cmds = [x for x in std_cmds]
#         cmds[pos["m"]] = str(m)
#         for k in exp_range["k"]:
#             cmds[pos["k"]] = str(k)
#             info = "*** k=%d ***" % k
#             print(info)
#             f.write("%s\n" % info)
#             n = 2 * k + m
#             for N in [n*i for i in [1, 2]]:
#                 cmds[pos["N"]] = str(N)
#                 f.write("--- N = %d ---\n" % N)
#                 output = subprocess.check_output(cmds).decode("utf-8")
#                 f.write(output)
#                 f.flush()


if __name__ == "__main__":
    for m in exp_range["m"]:
        run_exp(m)
