#include <algorithm>
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

class decision_tree {
  struct node {
    int feature;
    vector<node> children;
    int class_result = -1;
  };

  const int _N_FOLD = 10;

  map<string, int> _class_to_idx;
  map<int, string> _idx_to_class;

  vector<map<string, int>> _value_to_idx;
  vector<map<int, string>> _idx_to_value;

  vector<vector<int>> _dataset;
  vector<int> _classes_counts;
  vector<vector<vector<int>>> _values_counts;

  template <typename T> T _flatten(vector<T> arr) {
    return accumulate(arr.begin(), arr.end(), T{}, [](auto &dest, auto &src) {
      dest.insert(dest.end(), src.begin(), src.end());
      return dest;
    });
  }

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

  void _reset_counts() {
    _classes_counts = vector<int>(_class_to_idx.size());
    _values_counts = vector<vector<vector<int>>>(_value_to_idx.size());

    for (int i = 0; i < _value_to_idx.size(); i++) {
      _values_counts[i] = vector<vector<int>>(_value_to_idx[i].size(),
                                              vector<int>(_classes_counts));
    }
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

        _idx_to_value[j - 1].insert({_value_to_idx[j - 1].size(), data[i][j]});
        _value_to_idx[j - 1].insert({data[i][j], _value_to_idx[j - 1].size()});

        _dataset[i][j] = _value_to_idx[j - 1].at(data[i][j]);
      }
    }

    _reset_counts();

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

  vector<vector<int>> _get_feature_value_sample(vector<vector<int>> &sample,
                                                int feature, int value) {
    vector<vector<int>> result;

    for (auto &s : sample) {
      if (s[feature + 1] == value) {
        result.push_back(s);
      }
    }

    return result;
  }

  void _calculate_counts(vector<vector<int>> sample) {
    _reset_counts();

    for (int i = 0; i < sample.size(); i++) {
      _classes_counts[sample[i][0]]++;

      for (int j = 1; j < sample[i].size(); j++) {
        _values_counts[j - 1][sample[i][j]][sample[i][0]]++;
      }
    }
  }

  double _entropy(vector<int> counts) {
    int total = accumulate(counts.begin(), counts.end(), 0);
    double result = 0;

    for (int i = 0; i < counts.size(); i++) {
      double p = (double)counts[i] / total;
      result += p ? p * log2(p) : 0;
    }

    return -result;
  }

  double _gain(vector<int> classes_counts, vector<vector<int>> feature_counts) {
    double gain = _entropy(classes_counts);
    int sample_size =
        accumulate(classes_counts.begin(), classes_counts.end(), 0);

    for (int i = 0; i < feature_counts.size(); i++) {
      double value_entropy = _entropy(feature_counts[i]);

      gain -= (double)accumulate(feature_counts[i].begin(),
                                 feature_counts[i].end(), 0) /
              sample_size * value_entropy;
    }

    return gain;
  }

  pair<double, int> _get_highest_gain_feature(vector<bool> &visited) {
    pair<double, int> highest_gain_feature = {-1, 0};

    for (int i = 0; i < _values_counts.size(); i++) {
      if (visited[i]) {
        continue;
      }

      double gain = _gain(_classes_counts, _values_counts[i]);

      if (gain > highest_gain_feature.first) {
        highest_gain_feature = {gain, i};
      }
    }

    return highest_gain_feature;
  }

  node _recurse(vector<vector<int>> sample, vector<bool> &visited,
                const int k = 0) {
    _calculate_counts(sample);

    if (sample.size() < k) {
      node root;
      root.feature = -1;
      root.class_result =
          distance(_classes_counts.begin(),
                   max_element(_classes_counts.begin(), _classes_counts.end()));

      return root;
    }

    auto highest_gain_feature = _get_highest_gain_feature(visited);

    node root;
    root.feature = highest_gain_feature.second;
    root.children =
        vector<node>(_values_counts[highest_gain_feature.second].size());

    if (highest_gain_feature.first <= 0) {
      root.class_result = sample[0][0];
      return root;
    }

    visited[highest_gain_feature.second] = true;

    for (int i = 0; i < _values_counts[highest_gain_feature.second].size();
         i++) {
      root.children[i] = _recurse(
          _get_feature_value_sample(sample, highest_gain_feature.second, i),
          visited);
    }

    return root;
  }

  node _build_decision_tree(vector<vector<int>> sample, const int k = 0) {
    vector<bool> visited(_values_counts.size(), false);

    return _recurse(sample, visited, k);
  }

  int _predict(node n, vector<int> &data) {
    if (n.class_result > -1) {
      return n.class_result;
    }

    return _predict(n.children[data[n.feature + 1]], data);
  }

  double _predict_batch(vector<vector<int>> batch, node root,
                        vector<node> random_forest = {}) {
    int correct_count = 0;

    for (auto &s : batch) {
      if (!random_forest.empty()) {
        vector<int> classes_counts(_class_to_idx.size());

        for (auto &n : random_forest) {
          classes_counts[_predict(n, s)]++;
        }

        if (s[0] == distance(classes_counts.begin(),
                             max_element(classes_counts.begin(),
                                         classes_counts.end()))) {
          correct_count++;
        }

      } else if (s[0] == _predict(root, s)) {
        correct_count++;
      }
    }

    return (double)correct_count / batch.size();
  }

public:
  decision_tree(string path) {
    ifstream file(path);

    vector<vector<string>> raw_data;

    if (file.is_open()) {
      string line;
      while (getline(file, line)) {
        raw_data.push_back(_split(line, ","));
      }
    }

    file.close();

    cout << "Data size: " << raw_data.size() << endl;
    _parse_dataset(raw_data);
  }

  void cross_validate() {
    double overall_accuracy_k = 0;
    double overall_accuracy_rf = 0;

    vector<vector<vector<int>>> prepared_data = _prepare_data();

    for (int i = 0; i < _N_FOLD; i++) {
      vector<vector<vector<int>>> train_set(prepared_data.begin(),
                                            prepared_data.begin() + i);

      train_set.insert(train_set.end(),
                       prepared_data.begin() +
                           min((i + 1), (int)prepared_data.size()),
                       prepared_data.end());

      node root = _build_decision_tree(_flatten(train_set), 10);

      vector<node> random_forest(train_set.size());
      for (int j = 0; j < train_set.size(); j++) {
        random_forest[j] = _build_decision_tree(train_set[j]);
      }

      double accuracy_k = _predict_batch(prepared_data[i], root);
      double accuracy_rf =
          _predict_batch(prepared_data[i], root, random_forest);

      cout << "Set " << i + 1 << " Accuracy: K -> " << accuracy_k << "; RF -> "
           << accuracy_rf << endl;

      overall_accuracy_k += accuracy_k;
      overall_accuracy_rf += accuracy_rf;
    }

    cout << "Avg. Accuracy: K -> " << overall_accuracy_k / _N_FOLD << "; RF -> "
         << overall_accuracy_rf / _N_FOLD << endl;
  }
};

int main() {
  cout << setprecision(2);

  try {
    auto dt = decision_tree("./breast-cancer.data");

    dt.cross_validate();
  } catch (exception &e) {
    cout << e.what() << endl;
  }

  return 0;
}
