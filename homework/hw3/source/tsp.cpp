#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <math.h>
#include <queue>
#include <random>
#include <set>
#include <vector>

template <typename T> using vec = std::vector<T>;

std::mt19937 mt(std::random_device{}());
const double MAX = 2000;
const double MIN = -2000;
std::uniform_real_distribution<double> urd(MIN, MAX);

template <typename T>
void print(vec<T> v, bool nl = false, std::ostream &os = std::cout) {
  for (auto t : v) {
    os << t;
    if (nl) {
      os << std::endl;
    } else {
      os << ' ';
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

struct town {
  double x = 0, y = 0;
  std::map<int, double> edges = {};
  std::string name = "";

  friend std::ostream &operator<<(std::ostream &os, const town &t) {
    return (os << t.name << ' ' << t.x << ' ' << t.y);
  }
};

struct chromosome {
  vec<int> path = {};
  double eval = INFINITY;

  friend std::ostream &operator<<(std::ostream &os, const chromosome &c) {
    print(c.path, 0, os);
    return (os << " : " << c.eval);
  }

  friend bool operator<(const chromosome &c1, const chromosome &c2) {
    return c1.eval < c2.eval;
  };

  friend bool operator>(const chromosome &c1, const chromosome &c2) {
    return c1.eval > c2.eval;
  };
};

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

vec<chromosome> initialize(int gen_size, int n) {
  vec<chromosome> gen(gen_size);
  for (int i = 0; i < gen_size; i++) {
    vec<int> perm(n);
    std::iota(perm.begin(), perm.end(), 0);
    std::random_shuffle(perm.begin(), perm.end());
    gen[i] = {perm};
  }
  return gen;
}

void evaluate(vec<town> &towns, vec<chromosome> &gen) {
  for (size_t i = 0; i < gen.size(); i++) {
    double sum = 0;

    for (size_t j = 1; j < gen[i].path.size(); j++) {
      if ((int)j - 1 != gen[i].path[j]) {
        sum += towns[gen[i].path[j]].edges.at(j - 1);
      }
    }

    gen[i].eval = sum;
  }
}

template <typename T> vec<T> top_k(vec<T> list) {
  std::sort(list.begin(), list.end());
  slice(list, 0, 9);
}

vec<int> select_parents(vec<chromosome> &gen) {
  (void)gen;
  return {};
}

vec<chromosome> reproduction(vec<chromosome> &generation, vec<int> &parents) {
  (void)parents;
  (void)generation;
  return {};
}

void mutate() {}

void tsp(vec<town> &towns) {

  const int GENERATION_SIZE = towns.size() * towns.size() / 2;

  vec<chromosome> generation = initialize(GENERATION_SIZE, towns.size());
  evaluate(towns, generation);
  std::priority_queue<chromosome, vec<chromosome>, std::greater<chromosome>> pq(
      generation.begin(), generation.end());

  std::cout << "gen first: " << pq.top() << std::endl;

  int max_iter = 1000;
  int iter = max_iter;
  chromosome min = pq.top();

  while (iter--) {
    vec<int> parents = select_parents(generation);
    vec<chromosome> children = reproduction(generation, parents);

    if (pq.top().eval >= min.eval) {
      break;
    }

    min = pq.top();

    if (iter % (max_iter / 3) == 0) {
      std::cout << "gen " << max_iter - iter << ": " << min << std::endl;
    }
  }

  std::cout << "gen last: " << min << std::endl;
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
