#include <algorithm>
#include <chrono>
#include <exception>
#include <fstream>
#include <iostream>
#include <map>
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

class decision_tree {
  const int _N_FOLD = 10;

  map<string, int> _class_to_idx;
  map<int, string> _idx_to_class;

  vector<map<string, int>> _value_to_idx;
  vector<map<int, string>> _idx_to_value;

  vector<vector<int>> _dataset;
  vector<double> _classes_probs;
  vector<vector<vector<double>>> _features_probs;

  vector<string> _split(string text, string delim) {
    auto start = 0;
    auto end = text.find(delim);

    vector<string> result;

    while (end != string::npos) {
      result.push_back(text.substr(start, end - start));
      start = end + delim.length();
      end = text.find(delim, start);
    }

    result.push_back(text.substr(start));
    return result;
  }

  void _init_probs() {
    _classes_probs = vector<double>(_class_to_idx.size());
    _features_probs = vector<vector<vector<double>>>(
        _value_to_idx.size(),
        vector<vector<double>>(_value_to_idx.size(), _classes_probs));
  }

  void _parse_dataset(vector<vector<string>> &data) {
    _dataset = vector<vector<int>>(data.size());

    _value_to_idx = vector<map<string, int>>(data[0].size() - 1);
    _idx_to_value = vector<map<int, string>>(data[0].size() - 1);

    for (size_t i = 0; i < data.size(); i++) {
      _dataset[i] = vector<int>(data[i].size());

      _idx_to_class.insert({_class_to_idx.size(), data[i][0]});
      _class_to_idx.insert({data[i][0], _class_to_idx.size()});

      _dataset[i][0] = _class_to_idx.at(data[i][0]);

      for (size_t j = 1; j < data[i].size(); j++) {

        _idx_to_value[j - 1].insert({_value_to_idx.size(), data[i][j]});
        _value_to_idx[j - 1].insert({data[i][j], _value_to_idx.size()});

        _dataset[i][j] = _value_to_idx[j - 1].at(data[i][j]);
      }
    }

    _init_probs();

    cout << "Classes: " << _class_to_idx.size() << endl;
    cout << "Features: " << _value_to_idx.size() << endl;
  }

  vector<int> _get_classes_counts(vector<vector<int>> dataset) {
    vector<int> classes_counts(_class_to_idx.size(), 0);

    for (int i = 0; i < dataset.size(); i++) {
      classes_counts[dataset[i][0]]++;
    }

    return classes_counts;
  }

  vector<vector<vector<int>>> _prepare_data() {
    const unsigned seed =
        chrono::system_clock::now().time_since_epoch().count();

    shuffle(_dataset.begin(), _dataset.end(), default_random_engine(seed));

    vector<vector<vector<int>>> preparation(_N_FOLD);
    vector<vector<vector<int>>> classes(_class_to_idx.size());
    vector<int> classes_counts = _get_classes_counts(_dataset);
    double total_sample_size = (double)_dataset.size() / _N_FOLD;

    for (int i = 0; i < _dataset.size(); i++) {
      classes[_dataset[i][0]].push_back(_dataset[i]);
    }

    for (auto &c : classes) {
      shuffle(c.begin(), c.end(), default_random_engine(seed));
    }

    for (int i = 0; i < classes_counts.size(); i++) {
      classes_counts[i] =
          total_sample_size / _dataset.size() * classes_counts[i];
    }

    for (int i = 0; i < _N_FOLD; i++) {
      vector<int> cur_classes_counts = classes_counts;

      for (int j = 0; j < cur_classes_counts.size() && classes[j].size(); j++) {
        while (cur_classes_counts[j]--) {
          preparation[i].push_back(classes[j].back());
          classes[j].pop_back();
        }
      }
    }

    return preparation;
  }

public:
};

int main() {
  try {
    auto dt = decision_tree();

  } catch (exception &e) {
    cout << e.what() << endl;
  }

  return 0;
}
