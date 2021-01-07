from scipy.special import comb, perm

N = 40
k = 8
m = 4
n = k + m


def get_n_cost():
    s = 0
    for i in range(m + 1):
        s += perm(k, i) * perm(N - n, m - i)
    return s


total_size = perm(N, m) * comb(N - m, k)
print(total_size)

zero = comb(N - n, k)
print(zero)
print(zero/total_size)
print(total_size/zero)

# n_cost = get_n_cost()
# none_n = total_size - n_cost
