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
  std::string name = "";

  friend std::ostream &operator<<(std::ostream &os, const town &t) {
    return (os << t.name << ' ' << t.x << ' ' << t.y);
  }
};

struct genome {
  vec<int> path = {};
  double eval = INFINITY;

  friend std::ostream &operator<<(std::ostream &os, const genome &c) {
    print(c.path, 0, os);
    return (os << " : " << c.eval);
  }

  friend bool operator<(const genome &c1, const genome &c2) {
    return c1.eval < c2.eval;
  };

  friend bool operator>(const genome &c1, const genome &c2) {
    return c1.eval > c2.eval;
  };
};

template <typename T>
using pr_queue = std::priority_queue<T, vec<T>, std::greater<T>>;

class GeneticTSP {
private:
  std::mt19937 mt = std::mt19937(std::random_device{}());
  const double MAX = 2000;
  const double MIN = -2000;
  std::uniform_real_distribution<double> urd_coord =
      std::uniform_real_distribution<double>(MIN, MAX);

  const double MUTATION_RATE = 0.1;
  std::uniform_real_distribution<double> urd_mutate =
      std::uniform_real_distribution<double>(0., 1.);

  const int MAX_ITER = 10000;

  vec<town> _towns;
  vec<vec<double>> _distances;

  std::pair<int, int> compute_bounds(int n) {
    std::uniform_int_distribution<int> urd_int(0, n - 1);
    int lower = urd_int(mt);
    urd_int = std::uniform_int_distribution<int>(lower, n);
    int upper = urd_int(mt);

    return {lower, upper};
  }

  template <typename T> vec<T> top_k(pr_queue<T> q, int k) {
    vec<T> top(k);

    for (int i = 0; i < k; i++) {
      if (q.empty()) {
        break;
      }

      top[i] = q.top();
      q.pop();
    }

    return top;
  }

  double total_distance(genome g) {
    double sum = 0;
    for (size_t i = 1; i < g.path.size(); i++) {
      sum += distance(_towns[g.path[i]].x, _towns[g.path[i]].y,
                      _towns[g.path[i - 1]].x, _towns[g.path[i - 1]].y);
    }

    return sum;
  }

public:
  GeneticTSP(vec<town> towns, vec<vec<double>> distances) {
    this->_towns = towns;
    this->_distances = distances;
  }

  GeneticTSP(int n) {
    this->_towns = vec<town>(n);
    this->_distances = vec<vec<double>>(n, vec<double>(n, 0));

    for (int i = 0; i < n; i++) {
      _towns[i] = {urd_coord(mt), urd_coord(mt)};

      for (int j = 0; j < i; j++) {
        double dist =
            distance(_towns[j].x, _towns[j].y, _towns[i].x, _towns[i].y);
        _distances[i][j] = dist;
        _distances[j][i] = dist;
      }
    }
  }

  vec<genome> initialize(int gen_size, int n) {
    vec<genome> gen(gen_size);
    for (int i = 0; i < gen_size; i++) {
      vec<int> perm(n);
      std::iota(perm.begin(), perm.end(), 0);
      std::shuffle(perm.begin(), perm.end(), mt);
      gen[i] = {perm};
    }
    return gen;
  }

  void evaluate(vec<genome> &generation) {
    for (size_t i = 0; i < generation.size(); i++) {
      generation[i].eval = total_distance(generation[i]);
    }
  }

  vec<genome> select_parents(pr_queue<genome> &q, int k) {
    vec<genome> top = top_k(q, k * 2);
    std::shuffle(top.begin() + k / 2, top.begin() + k * 3 / 2, mt);
    return vec<genome>(top.begin(), top.begin() + k);
  }

  vec<genome> reproduction(vec<genome> &parents) {
    vec<genome> children(parents.size());

    auto crossover = [this](const genome &p1, const genome &p2) -> vec<genome> {
      int n = p1.path.size();
      vec<genome> cross = {{vec<int>(p1.path.begin(), p1.path.end())},
                           {vec<int>(p2.path.begin(), p2.path.end())}};

      std::pair<int, int> bounds = compute_bounds(n - 1);
      vec<int> section1(p1.path.begin() + bounds.first,
                        p1.path.begin() + bounds.second + 1);
      vec<int> section2(p2.path.begin() + bounds.first,
                        p2.path.begin() + bounds.second + 1);

      for (size_t i = 0; i < section1.size(); i++) {
        auto it1 =
            std::find(cross[0].path.begin(), cross[0].path.end(), section2[i]);
        cross[0].path.erase(it1);

        auto it2 =
            std::find(cross[1].path.begin(), cross[1].path.end(), section1[i]);
        cross[1].path.erase(it2);
      }

      std::shuffle(section1.begin(), section1.end(), mt);
      std::shuffle(section2.begin(), section2.end(), mt);

      cross[1].path.insert(cross[1].path.end(), section1.begin(),
                           section1.end());
      cross[0].path.insert(cross[0].path.end(), section2.begin(),
                           section2.end());

      return cross;
    };

    for (size_t i = 0; i < parents.size() - 1; i += 2) {
      vec<genome> cross = crossover(parents[i], parents[i + 1]);
      children[i] = cross[0];
      children[i + 1] = cross[1];
    }

    return children;
  }

  void mutate(vec<genome> &children) {
    int n = children[0].path.size();

    for (auto &child : children) {
      if (urd_mutate(mt) < MUTATION_RATE) {
        std::pair<int, int> bounds = compute_bounds(n - 1);

        for (int i = 0; i <= (bounds.second - bounds.first) / 2; i++) {
          std::swap(child.path[bounds.first + i],
                    child.path[bounds.second - i]);
        }
      }
    };
  }

  genome solve() {

    const int GENERATION_SIZE = _towns.size() * _towns.size() / 2;
    const int PARENTS_SIZE = (GENERATION_SIZE / 3) & 1 ? GENERATION_SIZE / 3 + 1
                                                       : GENERATION_SIZE / 3;

    vec<genome> generation = initialize(GENERATION_SIZE, _towns.size());
    evaluate(generation);
    pr_queue<genome> pq(generation.begin(), generation.end());

    std::cout << "gen first: " << pq.top() << std::endl;

    int iter = MAX_ITER;
    genome min = pq.top();

    while (--iter) {

      vec<genome> parents = select_parents(pq, PARENTS_SIZE);
      vec<genome> children = reproduction(parents);

      mutate(children);
      evaluate(children);

      for (auto &child : children) {
        pq.push(child);
      }

      min = pq.top();
      generation = top_k(pq, GENERATION_SIZE);

      pq = pr_queue<genome>(generation.begin(), generation.end());

      if (iter % (MAX_ITER / 3) == 0) {
        std::cout << "gen number " << MAX_ITER - iter << ": " << min
                  << std::endl;
      }
    }

    std::cout << "gen last: " << min << std::endl;

    return min;
  }
};

void test_towns() {

  vec<town> towns;

  std::ifstream csv;
  std::string name;

  csv.open("../UK_TSP/uk12_name.csv");

  if (csv.is_open()) {
    while (std::getline(csv, name)) {
      towns.push_back({0, 0, name});
    }
  }

  csv.close();

  vec<vec<double>> distances(towns.size(), vec<double>(towns.size(), 0));

  csv.open("../UK_TSP/uk12_xy.csv");

  if (csv.is_open()) {
    std::string line;
    for (size_t i = 0; i < towns.size(); i++) {
      std::getline(csv, line);
      sscanf(line.c_str(), "%lf,%lf", &towns[i].x, &towns[i].y);
      for (size_t j = 0; j < i; j++) {
        double dist = distance(towns[j].x, towns[j].y, towns[i].x, towns[i].y);
        distances[i][j] = dist;
        distances[j][i] = dist;
      }
    }
  }

  csv.close();

  genome result = GeneticTSP(towns, distances).solve();

  std::cout << std::endl;

  for (int t : result.path) {
    std::cout << towns[t].name << std::endl;
  }
}

int main(int argc, char *argv[]) {
  std::cout << std::setprecision(10);

  try {
    if (argc > 1 && std::string(argv[1]) == "test") {
      test_towns();
    } else {
      int n;
      std::cin >> n;

      GeneticTSP tsp = GeneticTSP(n);

      tsp.solve();
    }
  } catch (std::exception &e) {
    std::cout << e.what() << std::endl;
  }

  std::system("pause");

  return 0;
}
