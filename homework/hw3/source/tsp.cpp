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
  const double XY_MAX = 2000;
  const double XY_MIN = -2000;
  const double MUTATION_RATE = 0.7;
  const double ELITISM_RATE = 0.3;
  const int MAX_GENERATIONS = 1000;
  const int GENERATION_SIZE = 100;
  const int TOURNAMENT_SIZE = 10;

  vec<town> _towns;
  vec<vec<double>> _distances;

  std::pair<int, int> compute_bounds(int n) {
    std::uniform_int_distribution<int> urd_int(0, n - 1);
    int lower = urd_int(mt);
    urd_int = std::uniform_int_distribution<int>(lower, n);
    int upper = urd_int(mt);

    return {lower, upper};
  }

  template <typename T> vec<T> top_k(vec<T> v, int k) {
    sort(v.begin(), v.end());
    return vec<T>(v.begin(), v.begin() + k);
  }

  double distance(double x1, double y1, double x2, double y2) {
    return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
  }

  double total_distance(genome g) {
    double sum = 0;
    for (size_t i = 1; i < g.path.size(); i++) {
      sum += distance(_towns[g.path[i]].x, _towns[g.path[i]].y,
                      _towns[g.path[i - 1]].x, _towns[g.path[i - 1]].y);
    }

    return sum;
  }

  vec<genome> initialize(int gen_size) {
    vec<genome> gen(gen_size);
    int n = _towns.size();
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

  vec<genome> select_parents(vec<genome> population) {
    std::shuffle(population.begin(), population.end(), mt);
    std::sort(population.begin(), population.begin() + TOURNAMENT_SIZE);

    return vec<genome>(population.begin(), population.begin() + 2);
  }

  vec<genome> reproduction(vec<genome> &parents) {

    genome p1 = parents[0];
    genome p2 = parents[1];
    int n = _towns.size();

    vec<genome> children = {{vec<int>(p1.path.begin(), p1.path.end())},
                            {vec<int>(p2.path.begin(), p2.path.end())}};

    std::pair<int, int> bounds = compute_bounds(n - 1);

    vec<int> section1(p1.path.begin() + bounds.first,
                      p1.path.begin() + bounds.second + 1);
    vec<int> section2(p2.path.begin() + bounds.first,
                      p2.path.begin() + bounds.second + 1);

    for (size_t i = 0; i < section1.size(); i++) {
      auto it1 = std::find(children[0].path.begin(), children[0].path.end(),
                           section2[i]);
      children[0].path.erase(it1);

      auto it2 = std::find(children[1].path.begin(), children[1].path.end(),
                           section1[i]);
      children[1].path.erase(it2);
    }

    std::shuffle(section1.begin(), section1.end(), mt);
    std::shuffle(section2.begin(), section2.end(), mt);

    children[1].path.insert(children[1].path.end(), section1.begin(),
                            section1.end());
    children[0].path.insert(children[0].path.end(), section2.begin(),
                            section2.end());

    return children;
  }

  void mutate(vec<genome> &children) {
    std::uniform_real_distribution<double> urd_mutate =
        std::uniform_real_distribution<double>(0., 1.);

    int n = _towns.size();

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

  void calculate_distances() {
    _distances = vec<vec<double>>(_towns.size(), vec<double>(_towns.size(), 0));

    for (size_t i = 0; i < _towns.size(); i++) {
      for (size_t j = 0; j < i; j++) {
        double dist =
            distance(_towns[j].x, _towns[j].y, _towns[i].x, _towns[i].y);
        _distances[i][j] = dist;
        _distances[j][i] = dist;
      }
    }
  }

public:
  GeneticTSP(vec<town> towns) {
    _towns = towns;
    calculate_distances();
  }

  GeneticTSP(int n = 10) {
    std::uniform_real_distribution<double> urd_coord =
        std::uniform_real_distribution<double>(XY_MIN, XY_MAX);

    vec<town> towns(n);

    for (int i = 0; i < n; i++) {
      towns[i] = {urd_coord(mt), urd_coord(mt)};
    }

    _towns = towns;
    calculate_distances();
  }

  genome solve() {
    const int elitism_offset = ELITISM_RATE * GENERATION_SIZE;

    vec<genome> population = initialize(GENERATION_SIZE);
    evaluate(population);

    genome min = *std::max_element(population.begin(), population.end());

    std::cout << "gen null: " << min << std::endl;

    for (int i = 1; i < MAX_GENERATIONS; i++) {

      vec<genome> new_population;

      if (elitism_offset) {
        new_population = top_k(population, elitism_offset);
      }

      for (int j = elitism_offset; j < GENERATION_SIZE; j++) {
        vec<genome> parents = select_parents(population);
        vec<genome> children = reproduction(parents);

        mutate(children);
        evaluate(children);

        new_population.insert(new_population.end(), children.begin(),
                              children.end());
      }

      population = new_population;

      min = std::min(min,
                     *std::max_element(population.begin(), population.end()));

      if (i % (MAX_GENERATIONS / 3) == 0) {
        std::cout << "gen " << i << ": " << min << std::endl;
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

  csv.open("../UK_TSP/uk12_xy.csv");

  if (csv.is_open()) {
    std::string line;
    for (size_t i = 0; i < towns.size(); i++) {
      std::getline(csv, line);
      sscanf(line.c_str(), "%lf,%lf", &towns[i].x, &towns[i].y);
    }
  }

  csv.close();

  genome result = GeneticTSP(towns).solve();

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
