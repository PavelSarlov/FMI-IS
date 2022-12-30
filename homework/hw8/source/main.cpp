#include <chrono>
#include <cmath>
#include <exception>
#include <iostream>
#include <iterator>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

using serial = unsigned long long;

const vector<vector<double>> TRAIN_INPUT = {{0, 0}, {0, 1}, {1, 0}, {1, 1}};
const vector<vector<double>> TRAIN_OR_OUTPUT = {{0}, {1}, {1}, {1}};
const vector<vector<double>> TRAIN_AND_OUTPUT = {{0}, {0}, {0}, {1}};
const vector<vector<double>> TRAIN_XOR_OUTPUT = {{0}, {1}, {1}, {0}};

template <typename T> ostream &operator<<(ostream &os, const vector<T> &v) {
  for (auto &x : v) {
    os << x << ' ';
  }
  return os;
}

template <typename U, typename V>
ostream &operator<<(ostream &os, const pair<U, V> &p) {
  return os << '{' << p.first << ' ' << p.second << '}';
}

template <typename U, typename V>
ostream &operator<<(ostream &os, const unordered_map<U, V> &m) {
  for (auto &x : m) {
    os << x << ' ';
  }
  return os;
}

class SNN {
  struct Connection;
  struct Neuron;
  struct Layer;

  struct Layer {
    Neuron *bias = nullptr;
    vector<Neuron> neurons;

    Layer() {}

    Layer(vector<Neuron> _neurons, Neuron *_bias = nullptr) {
      this->bias = _bias;
      this->neurons = _neurons;
    }

    Layer(const Layer &_other) { *this = _other; }

    Layer &operator=(const Layer &_other) {
      if (this != &_other) {
        delete this->bias;

        if (_other.bias) {
          this->bias = new Neuron(*_other.bias);
        }
        this->neurons = _other.neurons;
      }
      return *this;
    }

    ~Layer() { delete this->bias; }

    friend ostream &operator<<(ostream &_os, const Layer &l) {
      _os << '{';
      if (l.bias) {
        _os << *l.bias << "; ";
      }
      return _os << l.neurons << '}';
    }
  };

  struct Connection {
    double weight = 0.;
    Neuron *neuron = nullptr;

    Connection() {}

    Connection(double _weight, Neuron *_neuron = nullptr) {
      this->weight = _weight;
      this->neuron = _neuron;
    }

    Connection(const Connection &_other) { *this = _other; }

    Connection &operator=(const Connection &_other) {
      if (this != &_other) {
        this->weight = _other.weight;
        this->neuron = _other.neuron;
      }
      return *this;
    }

    friend ostream &operator<<(ostream &_os, const Connection &c) {
      return _os << '{' << c.weight << "; " << c.neuron << '}';
    }
  };

  struct Neuron {
    double value = 0.;
    vector<Connection> connections;

    Neuron() {}

    Neuron(double _value) { this->value = _value; }

    Neuron(const Neuron &_other) { *this = _other; }

    Neuron &operator=(const Neuron &_other) {
      if (this != &_other) {
        this->value = _other.value;
        this->connections = _other.connections;
      }
      return *this;
    }

    Connection &operator[](int _i) { return this->connections[_i]; }

    friend ostream &operator<<(ostream &_os, const Neuron &n) {
      return _os << "{value: " << n.value << "; connections: " << n.connections
                 << "}";
    }

    double sigmoid(double _value) { return 1. / (1. + exp(-_value)); }

    double tanh(double _value) { return 2. / (1. + exp(-2 * _value)) - 1; }

    double g(double _value) { return this->sigmoid(_value); }

    void activate() { this->value = this->g(this->value); }

    double gradient(double expected) {
      return this->value * (1 - this->value) * (expected - this->value);
    }

    double gradient(vector<double> expected) {
      double sum = 0;

      for (int i = 0; i < expected.size(); i++) {
        sum += this->connections[i].weight *
               this->connections[i].neuron->gradient(expected[i]);
      }

      return this->value * (1 - this->value) * sum;
    }
  };

private:
  void _init_weights() {
    random_device rd;
    mt19937 mt(rd());
    uniform_real_distribution<> urd(-0.5, 0.5);

    for (int i = 0; i < _layers.size() - 1; i++) {
      for (auto &n2 : _layers[i + 1].neurons) {
        for (auto &n1 : _layers[i].neurons) {
          n1.connections.push_back(Connection(urd(mt), &n2));
        }

        if (_layers[i + 1].bias) {
          _layers[i + 1].bias->connections.push_back(Connection(urd(mt), &n2));
        }
      }
    }
  }

  void _forward(Layer &prev_layer, Layer &next_layer) {
    for (int i = 0; i < next_layer.neurons.size(); i++) {
      double sum =
          next_layer.bias->value * next_layer.bias->connections[i].weight;

      for (int j = 0; j < prev_layer.neurons.size(); j++) {
        sum += prev_layer.neurons[j][i].weight * prev_layer.neurons[j].value;
      }

      next_layer.neurons[i].value = sum;
      next_layer.neurons[i].activate();
    }
  }

  void _backpropagate(vector<double> expected) {
    const auto propagate = [this, expected](Neuron &back, Neuron &front, int i,
                                            int k) {
      back[k].weight += _learning_rate *
                        (i == _layers.size() - 1 ? front.gradient(expected[k])
                                                 : front.gradient(expected)) *
                        back.value;
    };

    for (int i = _layers.size() - 1; i > 0; i--) {
      for (int k = 0; k < _layers[i].neurons.size(); k++) {
        Neuron &front = _layers[i].neurons[k];

        for (int j = 0; j < _layers[i - 1].neurons.size(); j++) {
          propagate(_layers[i - 1].neurons[j], front, i, k);
        }

        propagate((*_layers[i].bias), front, i, k);
      }
    }
  }

private:
  double _learning_rate;
  double _min_error_condition;
  int _max_train_epochs;

  vector<Layer> _layers;

public:
  SNN(int input_size, int hidden_size, int output_size,
      double learning_rate = 0.1, double bias = 1.,
      int max_train_epochs = 100000, double min_error_condition = 0.001) {
    _max_train_epochs = max_train_epochs;
    _learning_rate = learning_rate;
    _min_error_condition = min_error_condition;

    _layers.push_back(Layer(vector<Neuron>(input_size)));
    if (hidden_size > 0) {
      _layers.push_back(Layer(vector<Neuron>(hidden_size), new Neuron(bias)));
    }
    _layers.push_back(Layer(vector<Neuron>(output_size), new Neuron(bias)));

    _init_weights();
  }

  vector<Neuron> predict(vector<double> input) {
    for (int i = 0; i < input.size(); i++) {
      _layers[0].neurons[i].value = input[i];
    }

    for (int i = 0; i < _layers.size() - 1; i++) {
      _forward(_layers[i], _layers[i + 1]);
    }

    return _layers.back().neurons;
  }

  void train(vector<vector<double>> train_input,
             vector<vector<double>> train_output) {
    int epoch = _max_train_epochs;
    double error = 1;

    while (epoch-- && error > _min_error_condition) {
      error = 0;

      for (int i = 0; i < train_input.size(); i++) {
        auto prediction = predict(train_input[i]);

        for (int j = 0; j < prediction.size(); j++) {
          error += pow(prediction[j].value - train_output[i][j], 2);
        }

        _backpropagate(train_output[i]);
      }
    }

    cout << "Finished training in: " << _max_train_epochs - epoch - 1
         << " epochs; error: " << error << endl;
  }
};

int main() {
  try {
    SNN snn_and(2, 0, 1);
    SNN snn_or(2, 0, 1);
    SNN snn_xor(2, 2, 1);

    snn_and.train(TRAIN_INPUT, TRAIN_AND_OUTPUT);
    snn_or.train(TRAIN_INPUT, TRAIN_OR_OUTPUT);
    snn_xor.train(TRAIN_INPUT, TRAIN_XOR_OUTPUT);

    cout << endl;

    for (int i = 0; i < TRAIN_INPUT.size(); i++) {
      cout << "AND -> Input: " << TRAIN_INPUT[i]
           << "; Expected: " << TRAIN_AND_OUTPUT[i][0]
           << "; Actual: " << snn_and.predict(TRAIN_INPUT[i])[0].value << endl;
      cout << "OR  -> Input: " << TRAIN_INPUT[i]
           << "; Expected: " << TRAIN_OR_OUTPUT[i][0]
           << "; Actual: " << snn_or.predict(TRAIN_INPUT[i])[0].value << endl;
      cout << "XOR -> Input: " << TRAIN_INPUT[i]
           << "; Expected: " << TRAIN_XOR_OUTPUT[i][0]
           << "; Actual: " << snn_xor.predict(TRAIN_INPUT[i])[0].value << endl;

      cout << endl;
    }

    system("pause");
  } catch (exception &e) {
    cout << e.what() << endl;
  }

  return 0;
}
