#include <algorithm>
#include <chrono>
#include <cmath>
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
  vector<int> _classes_counts;
  vector<vector<vector<int>>> _values_counts;

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
      _values_counts[i] =
          vector<vector<int>>(_value_to_idx[i].size(), _classes_counts);
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

        _idx_to_value[j - 1].insert({_value_to_idx.size(), data[i][j]});
        _value_to_idx[j - 1].insert({data[i][j], _value_to_idx.size()});

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

  vector<vector<int>> _get_feature_value_sample(vector<vector<int>> sample,
                                                int feature, int value) {
    vector<vector<int>> result;

    for (auto &s : sample) {
      if (s[feature] == value) {
        result.push_back(s);
      }
    }

    return result;
  }

  double _entropy(vector<vector<int>> sample) {
    vector<int> classes_counts = _get_classes_counts(sample);

    double result = 0;

    for (int i = 0; i < classes_counts.size(); i++) {
      double p = (double)classes_counts[i] / sample.size();
      result += p * log2(p);
    }

    return -result;
  }

  double _gain(vector<vector<int>> sample, int feature) { return 0; }

  void _train_batch(vector<vector<int>> batch) {}

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
    double overall_accuracy = 0;

    vector<vector<vector<int>>> prepared_data = _prepare_data();

    for (int i = 0; i < _N_FOLD; i++) {
      vector<vector<vector<int>>> train_set(prepared_data.begin(),
                                            prepared_data.begin() + i);

      train_set.insert(train_set.end(),
                       prepared_data.begin() +
                           min((i + 1), (int)prepared_data.size()),
                       prepared_data.end());

      /* for (auto &batch : train_set) { */
      /*   _train_batch(batch); */
      /* } */

      /* double accuracy = _predict_batch(prepared_data[i]); */

      /* cout << "Set " << i + 1 << " Accuracy: " << accuracy << endl; */

      /* overall_accuracy += accuracy; */

      _init_probs();
    }

    cout << "Avg. Accuracy: " << overall_accuracy / _N_FOLD << endl;
  }
};

int main() {
  try {
    auto dt = decision_tree("./breast-cancer.data");

  } catch (exception &e) {
    cout << e.what() << endl;
  }

  return 0;
}
