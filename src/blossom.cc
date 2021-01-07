#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "matching_generator.hh"
using namespace std;

#define ll long long
#define llu unsigned ll
#define UP_SIDE 100
const int maxn = 10;
const int inf = 0x3f3f3f3f;

struct node {
  int x, y, z;
  node() {}
  node(int a, int b, int c) { x = a, y = b, z = c; }
} g[maxn][maxn];

int n, nx, t, lab[maxn], match[maxn], slack[maxn];
int st[maxn], pa[maxn], flower_from[maxn][maxn], S[maxn], vis[maxn];
vector<int> flower[maxn];
deque<int> q;

int dist(node e) { return lab[e.x] + lab[e.y] - g[e.x][e.y].z * 2; }

void update_slack(int x, int y) {
  if (!slack[y] || dist(g[x][y]) < dist(g[slack[y]][y])) slack[y] = x;
}

void set_slack(int y) {
  slack[y] = 0;
  for (int x = 1; x <= n; x++)
    if (g[x][y].z > 0 && st[x] != y && S[st[x]] == 0) update_slack(x, y);
}

void q_push(int x) {
  if (x <= n) return q.push_back(x);
  for (int i = 0; i < flower[x].size(); i++) q_push(flower[x][i]);
}

void set_st(int x, int b) {
  st[x] = b;
  if (x <= n) return;
  for (int i = 0; i < flower[x].size(); i++) set_st(flower[x][i], b);
}

int get_pr(int b, int xr) {
  int pr = find(flower[b].begin(), flower[b].end(), xr) - flower[b].begin();
  if (pr % 2 == 1) {
    reverse(flower[b].begin() + 1, flower[b].end());
    return flower[b].size() - pr;
  } else
    return pr;
}

void set_match(int x, int y) {
  match[x] = g[x][y].y;
  if (x <= n) return;
  node e = g[x][y];
  int xr = flower_from[x][e.x], pr = get_pr(x, xr);
  for (int i = 0; i < pr; i++) set_match(flower[x][i], flower[x][i ^ 1]);

  set_match(xr, y);
  rotate(flower[x].begin(), flower[x].begin() + pr, flower[x].end());
}

void augment(int x, int y) {
  int xnv = st[match[x]];
  set_match(x, y);
  if (!xnv) return;
  set_match(xnv, st[pa[xnv]]);
  augment(st[pa[xnv]], xnv);
}

int get_lca(int x, int y) {
  for (++t; x || y; swap(x, y)) {
    if (x == 0) continue;
    if (vis[x] == t) return x;
    vis[x] = t;
    x = st[match[x]];
    if (x) x = st[pa[x]];
  }
  return 0;
}

void add_blossom(int x, int lca, int y) {
  int b = n + 1;
  while (b <= nx && st[b]) b++;
  if (b > nx) nx++;
  lab[b] = 0, S[b] = 0;
  match[b] = match[lca];
  flower[b].clear();
  flower[b].push_back(lca);

  for (int xx = x, yy; xx != lca; xx = st[pa[yy]])
    flower[b].push_back(xx), flower[b].push_back(yy = st[match[xx]]),
        q_push(yy);
  reverse(flower[b].begin() + 1, flower[b].end());
  for (int xx = y, yy; xx != lca; xx = st[pa[yy]])
    flower[b].push_back(xx), flower[b].push_back(yy = st[match[xx]]),
        q_push(yy);

  set_st(b, b);
  for (int xx = 1; xx <= nx; xx++) g[b][xx].z = g[xx][b].z = 0;
  for (int xx = 1; xx <= n; xx++) flower_from[b][xx] = 0;
  for (int i = 0; i < flower[b].size(); i++) {
    int xs = flower[b][i];
    for (int xx = 1; xx <= nx; xx++)
      if (g[b][xx].z == 0 || dist(g[xs][xx]) < dist(g[b][xx]))
        g[b][xx] = g[xs][xx], g[xx][b] = g[xx][xs];
    for (int xx = 1; xx <= n; xx++)
      if (flower_from[xs][xx]) flower_from[b][xx] = xs;
  }
  set_slack(b);
}

void expand_blossom(int b) {
  for (int i = 0; i < flower[b].size(); i++) set_st(flower[b][i], flower[b][i]);
  int xr = flower_from[b][g[b][pa[b]].x], pr = get_pr(b, xr);
  for (int i = 0; i < pr; i += 2) {
    int xs = flower[b][i], xns = flower[b][i + 1];
    pa[xs] = g[xns][xs].x;
    S[xs] = 1, S[xns] = 0;
    slack[xs] = 0, set_slack(xns);
    q_push(xns);
  }
  S[xr] = 1, pa[xr] = pa[b];
  for (int i = pr + 1; i < flower[b].size(); i++) {
    int xs = flower[b][i];
    S[xs] = -1, set_slack(xs);
  }
  st[b] = 0;
}
bool on_found_edge(const node &e) {
  int x = st[e.x], y = st[e.y];
  if (S[y] == -1) {
    pa[y] = e.x, S[y] = 1;
    int nu = st[match[y]];
    slack[y] = slack[nu] = 0;
    S[nu] = 0, q_push(nu);
  } else if (S[y] == 0) {
    int lca = get_lca(x, y);
    if (!lca)
      return augment(x, y), augment(y, x), true;
    else
      add_blossom(x, lca, y);
  }
  return false;
}

bool matching(void) {
  memset(S, -1, sizeof(S));
  memset(slack, 0, sizeof(slack));
  q.clear();
  for (int x = 1; x <= nx; x++)
    if (st[x] == x && !match[x]) pa[x] = 0, S[x] = 0, q_push(x);
  if (q.empty()) return false;

  while (1) {
    while (q.size()) {
      int x = q.front();
      q.pop_front();
      if (S[st[x]] == 1) continue;
      for (int y = 1; y <= n; y++)
        if (g[x][y].z > 0 && st[x] != st[y]) {
          if (dist(g[x][y]) == 0) {
            if (on_found_edge(g[x][y])) return true;
          } else
            update_slack(x, st[y]);
        }
    }
    int d = inf;
    for (int b = n + 1; b <= nx; b++)
      if (st[b] == b && S[b] == 1) d = min(d, lab[b] / 2);
    for (int x = 1; x <= nx; x++)
      if (st[x] == x && slack[x]) {
        if (S[x] == -1)
          d = min(d, dist(g[slack[x]][x]));
        else if (S[x] == 0)
          d = min(d, dist(g[slack[x]][x]) / 2);
      }
    for (int x = 1; x <= n; x++) {
      if (S[st[x]] == 0) {
        if (lab[x] <= d) return false;
        lab[x] -= d;
      } else if (S[st[x]] == 1)
        lab[x] += d;
    }
    for (int b = n + 1; b <= nx; b++)
      if (st[b] == b) {
        if (S[st[b]] == 0)
          lab[b] += d * 2;
        else if (S[st[b]] == 1)
          lab[b] -= d * 2;
      }
    q.clear();
    for (int x = 1; x <= nx; x++)
      if (st[x] == x && slack[x] && st[slack[x]] != x &&
          dist(g[slack[x]][x]) == 0)
        if (on_found_edge(g[slack[x]][x])) return true;
    for (int b = n + 1; b <= nx; b++)
      if (st[b] == b && S[b] == 1 && lab[b] == 0) expand_blossom(b);
  }
  return false;
}

pair<ll, int> weight_blossom(void) {
  memset(match, 0, sizeof(match));
  nx = n;
  int n_matches = 0;
  ll tot_weight = 0;
  for (int x = 0; x <= n; x++) st[x] = x, flower[x].clear();
  int w_max = 0;
  for (int x = 1; x <= n; x++)
    for (int y = 1; y <= n; y++) {
      flower_from[x][y] = (x == y ? x : 0);
      w_max = max(w_max, g[x][y].z);
    }
  for (int x = 1; x <= n; x++) lab[x] = w_max;

  while (matching()) ++n_matches;
  for (int x = 1; x <= n; x++)
    if (match[x] && match[x] < x) tot_weight += (ll)g[x][match[x]].z;
  return make_pair(tot_weight, n_matches);
}

ll MatchingGenerator::blossom() {
  if (n * 1.5 > maxn) {
    return -1;
  }
  n = stripes_num;
  // init edges
  for (int x = 1; x <= n; x++)
    for (int y = 1; y <= n; y++) g[x][y] = node(x, y, 0);

  // input edges' weights
  for (int i = 0, x = 1; x <= n; ++x, ++i) {
    for (int j = 0, y = 1; y <= n; ++y, ++j) {
      if (i == j) {
        continue;
      }
      g[x][y].z = g[y][x].z = UP_SIDE - get_cost(i, j);
    }
  }

  // call process
  return (((ll)stripes_num / 2) * UP_SIDE) - weight_blossom().first;
}
