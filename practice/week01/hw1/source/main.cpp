#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <istream>
#include <queue>
#include <vector>

using namespace std;

struct node {
  int weight;
  pair<int, int> zero;
  vector<int> board;
};

int manhattan(vector<int> board, int offsetAfter) {
  int dist = 0;
  int side = sqrt(board.size());

  for (size_t i = 0; i < board.size(); i++) {
    if (board[i] == 0) {
      continue;
    }

    bool offset = board[i] > offsetAfter;
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

  for (size_t i = 0; i < board.size(); i++) {
    for (size_t j = i + 1; j < board.size(); j++) {
      if (board[i] != 0 && board[j] != 0 && board[i] > board[j]) {
        inversions++;
      }
    }
  }

  return !(inversions & 1);
}

vector<string> a_start(vector<int> board) {
  vector<string> result;
  int side = sqrt(board.size());

  pair<int, int> zero;
  for (size_t i = 0; i < board.size(); i++) {
    if (board[i] == 0) {
      zero = {i / side, i % side};
    }
  }

  for (int i = 3;; i += 3) {
    deque<node> s;

    s.push_back({0, zero, vector<int>(board.begin(), board.end())});

    while (!s.empty()) {
      node top = s.back();

      if (node.weight)
    }
  }

  return result;
}

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  int numbers, emptyTile;
  vector<int> board;

  cin >> numbers >> emptyTile;

  int tiles = numbers + 1;
  int side = sqrt(tiles);

  if (side * side != tiles ||
      (emptyTile != -1 && (emptyTile > numbers || emptyTile < 1))) {
    cout << "Invalid number of tiles" << endl;
    return 1;
  }

  board = vector<int>(tiles);

  int offsetAfter = numbers;

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

    if (i + 1 - offset == emptyTile) {
      offset = true;
      offsetAfter = i;
    }
  }

  if (!is_solvable(board, side)) {
    cout << "Not solvable" << endl;
    return 1;
  }

  return 0;
}
