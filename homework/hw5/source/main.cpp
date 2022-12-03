#include <algorithm>
#include <cmath>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
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

template <typename K, typename V>
ostream &operator<<(ostream &os, const map<K, V> &m) {
  for (auto &x : m) {
    os << '[' << x.first << "; " << x.second << ']' << ' ';
  }
  os << endl;
  return os;
}

class naive_bayes_classifier {
  const int _N_FOLD = 10;
  const double LAMBDA = 1;

  int _features_count = 0;

  map<string, int> _class_to_idx;
  map<int, string> _idx_to_class;

  map<string, int> _value_to_idx;
  map<int, string> _idx_to_value;

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
        _features_count,
        vector<vector<double>>(_value_to_idx.size(), _classes_probs));
  }

  void _parse_dataset(vector<vector<string>> &data) {
    _dataset = vector<vector<int>>(data.size());

    _features_count = data[0].size() - 1;

    for (size_t i = 0; i < data.size(); i++) {
      _dataset[i] = vector<int>(data[i].size());

      _idx_to_class.insert({_class_to_idx.size(), data[i][0]});
      _class_to_idx.insert({data[i][0], _class_to_idx.size()});

      _dataset[i][0] = _class_to_idx.at(data[i][0]);

      for (size_t j = 1; j < data[i].size(); j++) {

        _idx_to_value.insert({_value_to_idx.size(), data[i][j]});
        _value_to_idx.insert({data[i][j], _value_to_idx.size()});

        _dataset[i][j] = _value_to_idx.at(data[i][j]);
      }
    }

    _init_probs();

    cout << "Classes: " << _classes_probs.size() << endl;
    cout << "Features: " << _features_probs.size() << endl;
    cout << "Values per feature: " << _value_to_idx.size() << endl << endl;
  }

  void _train_batch(vector<vector<int>> dataset) {
    vector<int> classes_counts(_classes_probs.size(), 0);

    for (int i = 0; i < dataset.size(); i++) {
      classes_counts[dataset[i][0]]++;
    }
    for (int i = 0; i < _classes_probs.size(); i++) {
      _classes_probs[i] = (double)classes_counts[i] / dataset.size();
    }

    vector<vector<vector<int>>> features_counts(
        _features_count,
        vector<vector<int>>(_value_to_idx.size(),
                            vector<int>(classes_counts.size(), 0)));

    for (int i = 0; i < dataset.size(); i++) {
      for (int j = 0; j < _features_count; j++) {
        features_counts[j][dataset[i][j + 1]][dataset[i][0]]++;
      }
    }
    for (int i = 0; i < _features_probs.size(); i++) {
      for (int j = 0; j < _features_probs[i].size(); j++) {
        for (int k = 0; k < _features_probs[i][j].size(); k++) {
          _features_probs[i][j][k] =
              (double)features_counts[i][j][k] / classes_counts[j];
        }
      }
    }
  }

  int _predict(vector<int> &data) {
    vector<double> probs(_classes_probs.size(), 0);

    for (int i = 0; i < probs.size(); i++) {
      probs[i] = log(_classes_probs[i]);
      for (int j = 0; j < _features_count; j++) {
        probs[i] += log(_features_probs[j][data[j + 1]][i]);
      }
    }

    return max_element(probs.begin(), probs.end()) - probs.begin();
  }

  double _predict_batch(vector<vector<int>> &data_batch) {
    int correct = 0;

    for (auto &data : data_batch) {
      if (_predict(data) == data[0]) {
        correct++;
      }
    }

    return (double)correct / data_batch.size();
  }

public:
  naive_bayes_classifier(string path) {
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
    const int SET_SIZE = _dataset.size() / _N_FOLD;

    double overall_accuracy = 0;

    for (int i = 0; i < _N_FOLD; i++) {
      vector<vector<int>> train_set(
          _dataset.begin(),
          _dataset.begin() + min(i * SET_SIZE, (int)_dataset.size()));
      train_set.insert(train_set.end(),
                       _dataset.begin() +
                           min((i + 1) * SET_SIZE, (int)_dataset.size()),
                       _dataset.end());
      vector<vector<int>> predict_set(
          _dataset.begin() + min(i * SET_SIZE, (int)_dataset.size()),
          _dataset.begin() + min((i + 1) * SET_SIZE, (int)_dataset.size()));

      _train_batch(train_set);

      double accuracy = _predict_batch(predict_set);

      cout << "Set " << i + 1 << " Accuracy: " << accuracy << endl;

      overall_accuracy += accuracy;

      _init_probs();
    }

    cout << "Avg. Accuracy: " << overall_accuracy / _N_FOLD << endl;
  }
};

int main() {
  try {
    cout << setprecision(2);

    auto nbc = naive_bayes_classifier("house-votes-84.data");

    nbc.cross_validate();
  } catch (exception &e) {
    cout << e.what() << endl;
  }

  return 0;
}
