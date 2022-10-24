#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <istream>
#include <limits>
#include <queue>
#include <stdexcept>
#include <vector>

using namespace std;

struct node {
  int heuristic;
  pair<int, int> zero;
  vector<int> board;
  string move;

  void print_board() {
    int side = sqrt(board.size());

    for (size_t i = 0; i < board.size(); i++) {
      cout << board[i] << ' ';
      if (i % side == 0) {
        cout << endl;
      }
    }
  }
};

int manhattan(vector<int> board, int offset_after) {
  int dist = 0;
  int side = sqrt(board.size());

  for (size_t i = 0; i < board.size(); i++) {
    if (board[i] == 0) {
      continue;
    }

    bool offset = board[i] > offset_after;
    int j = (board[i] - 1 + offset);

    int x = j / side;
    int y = j % side;
    int x1 = i / side;
    int y1 = i % side;

    dist += abs(x - x1) + abs(y - y1);
  }

  return dist;
}

bool is_solvable(vector<int> board) {
  int inversions = 0;
  int side = sqrt(board.size());
  int blank = board.size() - 1;

  for (size_t i = 0; i < board.size(); i++) {
    if (board[i] == 0) {
      blank = i;
    }
    for (size_t j = i + 1; j < board.size(); j++) {
      if (board[i] != 0 && board[j] != 0 && board[i] > board[j]) {
        inversions++;
      }
    }
  }

  if (side & 1 || !((blank / side + 1) & 1)) {
    return !(inversions & 1);
  }

  return inversions & 1;
}

int id_search(deque<node> &path, int depth, int &bound, int &side,
              int &offset_after) {
  node top = path.back();

  if (top.heuristic == 0) {
    return -1;
  }

  int cost = depth + top.heuristic;

  if (cost > bound) {
    return cost;
  }

  pair<int, int> zero = top.zero;
  int zero_pos = zero.second * side + zero.first;

  int min = numeric_limits<int>::max();

  if (zero.first < side - 1 && top.move != "right") {
    vector<int> new_state(top.board.begin(), top.board.end());
    swap(new_state[zero_pos], new_state[zero_pos + 1]);

    node n = {manhattan(new_state, offset_after),
              {zero.first + 1, zero.second},
              new_state,
              "left"};
    path.push_back(n);

    int new_bound = id_search(path, depth + 1, bound, side, offset_after);
    if (new_bound == -1) {
      return -1;
    }
    min = std::min(new_bound, min);
    path.pop_back();
  }

  if (zero.first > 0 && top.move != "left") {
    vector<int> new_state(top.board.begin(), top.board.end());
    swap(new_state[zero_pos], new_state[zero_pos - 1]);

    node n = {manhattan(new_state, offset_after),
              {zero.first - 1, zero.second},
              new_state,
              "right"};
    path.push_back(n);

    int new_bound = id_search(path, depth + 1, bound, side, offset_after);
    if (new_bound == -1) {
      return -1;
    }
    min = std::min(new_bound, min);
    path.pop_back();
  }

  if (zero.second < side - 1 && top.move != "down") {
    vector<int> new_state(top.board.begin(), top.board.end());
    swap(new_state[zero_pos], new_state[zero_pos + side]);

    node n = {manhattan(new_state, offset_after),
              {zero.first, zero.second + 1},
              new_state,
              "up"};
    path.push_back(n);

    int new_bound = id_search(path, depth + 1, bound, side, offset_after);
    if (new_bound == -1) {
      return -1;
    }
    min = std::min(new_bound, min);
    path.pop_back();
  }

  if (zero.second > 0 && top.move != "up") {
    vector<int> new_state(top.board.begin(), top.board.end());
    swap(new_state[zero_pos], new_state[zero_pos - side]);

    node n = {manhattan(new_state, offset_after),
              {zero.first, zero.second - 1},
              new_state,
              "down"};
    path.push_back(n);

    int new_bound = id_search(path, depth + 1, bound, side, offset_after);
    if (new_bound == -1) {
      return -1;
    }
    min = std::min(new_bound, min);
    path.pop_back();
  }

  return min;
}

int ida_star(vector<int> board, int offset_after, deque<node> &path) {
  int side = sqrt(board.size());

  pair<int, int> zero = {-1, -1};
  for (size_t i = 0; i < board.size(); i++) {
    if (board[i] == 0) {
      zero = {i % side, i / side};
    }
  }

  if (zero.first == -1) {
    throw invalid_argument("No zero found");
  }

  path.push_back({manhattan(board, offset_after),
                  zero,
                  vector<int>(board.begin(), board.end()),
                  {}});

  int bound = path.back().heuristic;

  while (true) {
    int new_bound = id_search(path, 0, bound, side, offset_after);
    if (new_bound == -1) {
      return 1;
    }
    if (new_bound == numeric_limits<int>::max()) {
      return 0;
    }
    bound = new_bound;
  }
}

int main(int argc, char *argv[]) {
  try {
    (void)argc;
    (void)argv;

    int numbers, empty_tile;
    vector<int> board;

    cin >> numbers >> empty_tile;

    int tiles = numbers + 1;
    int side = sqrt(tiles);

    if (side * side != tiles) {
      throw invalid_argument("Invalid number of tiles.");
    }

    if ((empty_tile != -1 && (empty_tile > numbers || empty_tile < 1))) {
      throw invalid_argument("Invalid zero position. Valid are numbers in "
                             "range 1 to 9 inclusive or -1 for default (9).");
    }

    board = vector<int>(tiles);

    int offset_after = numbers;

    for (int i = 0; i < tiles; i++) {
      cin >> ws;

      int c = cin.peek();

      if (!isdigit(c)) {
        string s;
        cin >> s;
        i--;
        continue;
      }

      bool offset = false;
      cin >> board[i];

      int current_num = i + 1 - offset;

      if (current_num == empty_tile) {
        offset = true;
        offset_after = i;
      }
    }

    if (!is_solvable(board)) {
      throw invalid_argument("Not solvable.");
    }

    deque<node> path;

    auto start = chrono::system_clock::now();

    int result = ida_star(board, offset_after, path);

    double total_millis = chrono::duration_cast<chrono::milliseconds>(
                              chrono::system_clock::now() - start)
                              .count();

    printf("\n%.2f\n", total_millis / 1e3);

    if (result) {
      path.pop_front();
      cout << path.size() << endl;
      for (auto n : path) {
        cout << n.move << endl;
      }
    } else {
      cout << "No solution found" << endl;
    }

  } catch (exception &e) {
    cout << e.what() << endl;
  }

  system("pause");

  return 0;
}
