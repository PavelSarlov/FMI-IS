#include <chrono>
#include <cmath>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <numeric>
#include <random>
#include <string>
#include <vector>

using namespace std;

template <typename T> ostream &operator<<(ostream &os, const vector<T> &v) {
  for (auto &x : v) {
    os << x << ' ';
  }
  os << endl;
  return os;
}

template <typename U, typename V>
ostream &operator<<(ostream &os, const pair<U, V> &p) {
  os << p.first << ' ' << p.second << endl;
  return os;
}

int main(int argc, char *argv[]) {
  try {
    system("pause");
  } catch (exception &e) {
    cout << e.what() << endl;
  }

  return 0;
}
