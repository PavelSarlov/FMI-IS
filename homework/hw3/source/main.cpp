#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <math.h>
#include <random>
#include <set>
#include <vector>

std::mt19937 mt(std::random_device{}());
const double MAX = 2000;
const double MIN = -2000;
std::uniform_real_distribution<double> urd(MIN, MAX);

struct town {
  double x = 0, y = 0;
  std::map<int, double> edges = {};
  std::string name = "";

  friend std::ostream &operator<<(std::ostream &os, const town &t) {
    return (os << t.name << ' ' << t.x << ' ' << t.y);
  }
};

template <typename T> using vec = std::vector<T>;
using eval = std::pair<double, int>;

template <typename T> void print(vec<T> v, bool nl = false) {
  for (auto t : v) {
    std::cout << t;
    if (nl) {
      std::cout << std::endl;
    } else {
      std::cout << ' ';
    }
  }
}

double distance(double x1, double y1, double x2, double y2) {
  return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

template <typename T> vec<T> merge(vec<T> &p1, vec<T> &p2) {
  vec<T> result(p1);
  result.insert(result.end(), p2.begin(), p2.end());
  return result;
}

template <typename T> vec<T> slice(vec<T> &p, int start, int end) {
  vec<T> lower(p.begin(), p.begin() + start);
  vec<T> upper(p.begin() + end + 1, p.end());
  return merge(lower, upper);
}

vec<town> generate_towns(int n) {
  vec<town> towns(n);
  for (int i = 0; i < n; i++) {
    towns[i] = {urd(mt), urd(mt)};

    for (int j = 0; j < i; j++) {
      double dist = distance(towns[j].x, towns[j].y, towns[i].x, towns[i].y);
      towns[j].edges.insert({i, dist});
      towns[i].edges.insert({j, dist});
    }
  }

  return towns;
}

vec<vec<int>> initialize(int n) {
  vec<vec<int>> gen(n);
  for (int i = 0; i < n; i++) {
    vec<int> perm(n);
    std::iota(perm.begin(), perm.end(), 0);
    std::random_shuffle(perm.begin(), perm.end());
    gen[i] = perm;
  }
  return gen;
}

std::set<eval> evaluate(vec<town> &towns, vec<vec<int>> &gen) {
  std::set<eval> result;
  for (size_t i = 0; i < gen.size(); i++) {
    for (size_t j = 1; j < gen[i].size(); j++) {
      if ((int)j - 1 != gen[i][j]) {
        result.insert({towns[gen[i][j]].edges.at(j - 1), i});
      }
    }
  }

  return result;
}

template <typename T> vec<T> top_k(vec<T> list) {
  std::sort(list.begin(), list.end());
  slice(list, 0, 9);
}

/* vec<int> select_parents(vec<vec<int>> gen) {} */

void tsp(vec<town> &towns) {

  const int GEN_SIZE = towns.size() * towns.size() / 2;

  vec<vec<int>> gen = initialize(GEN_SIZE);
  std::set<eval> gen_eval = evaluate(towns, gen);

  std::cout << "gen first: " << gen_eval.begin()->first << std::endl;

  int max_iter = 1000;
  int iter = max_iter;
  eval min = *gen_eval.begin();

  while (iter--) {

    if (gen_eval.begin()->first >= min.first) {
      break;
    }

    if (iter % (max_iter / 3) == 0) {
      std::cout << "gen " << max_iter - iter << ": " << gen_eval.begin()->first
                << std::endl;
    }
  }

  std::cout << "gen last:" << min.first << std::endl;
}

void test_towns() {

  vec<town> towns;
  std::ifstream csv;
  std::string name;

  csv.open("../UK_TSP/uk12_name.csv");

  if (csv.is_open()) {
    while (std::getline(csv, name)) {
      towns.push_back({0, 0, {}, name});
    }
  }

  csv.close();

  csv.open("../UK_TSP/uk12_xy.csv");

  if (csv.is_open()) {
    std::string line;
    for (size_t i = 0; i < towns.size(); i++) {
      std::getline(csv, line);
      sscanf(line.c_str(), "%lf,%lf", &towns[i].x, &towns[i].y);
      for (size_t j = 0; j < i; j++) {
        double dist = distance(towns[j].x, towns[j].y, towns[i].x, towns[i].y);
        towns[j].edges.insert({i, dist});
        towns[i].edges.insert({j, dist});
      }
    }
  }

  csv.close();

  tsp(towns);
}

int main(int argc, char *argv[]) {
  std::cout << std::setprecision(10);

  try {
    if (argc > 1 && std::string(argv[1]) == "test") {
      test_towns();
    } else {
      int n;
      std::cin >> n;

      vec<town> towns = generate_towns(n);

      tsp(towns);
    }
  } catch (std::exception &e) {
    std::cout << e.what() << std::endl;
  }

  std::system("pause");

  return 0;
}
