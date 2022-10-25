#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <exception>
#include <iostream>
#include <istream>
#include <limits>
#include <queue>
#include <set>
#include <stdexcept>

typedef std::pair<int, int> coords;

enum direction { null, up, right, down, left };
static const char *direction_strings[] = {"null", "up", "right", "down",
                                          "left"};

bool move(std::vector<int> &board, direction move, int &zero_pos, int &side) {

  int zero_y = zero_pos / side;
  int zero_x = zero_pos % side;

  switch (move) {
  case up:
    if (zero_y >= side - 1) {
      return false;
    }
    std::swap(board[zero_pos], board[zero_pos + side]);
    zero_pos += side;
    break;
  case down:
    if (zero_y <= 0) {
      return false;
    }
    std::swap(board[zero_pos], board[zero_pos - side]);
    zero_pos -= side;
    break;
  case left:
    if (zero_x >= side - 1) {
      return false;
    }
    std::swap(board[zero_pos], board[zero_pos + 1]);
    zero_pos += 1;
    break;
  case right:
    if (zero_x <= 0) {
      return false;
    }
    std::swap(board[zero_pos], board[zero_pos - 1]);
    zero_pos -= 1;
    break;
  default:
    break;
  }
  return true;
}

int manhattan(std::vector<int> &board, int &offset_after) {
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

bool is_solvable(std::vector<int> &board) {
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

int find_zero_pos(std::vector<int> &board) {
  for (size_t i = 0; i < board.size(); i++) {
    if (board[i] == 0) {
      return i;
    }
  }

  throw std::invalid_argument("No zero found");
}

int id_search(std::vector<int> &board, std::deque<direction> &path,
              int &zero_pos, int depth, int &bound, int &side,
              int &offset_after) {
  direction last_move = path.back();

  int heuristic = manhattan(board, offset_after);

  if (heuristic == 0) {
    return -1;
  }

  int cost = depth + heuristic;

  if (cost > bound) {
    return cost;
  }

  int min = std::numeric_limits<int>::max();

  if (last_move != direction::right &&
      move(board, direction::left, zero_pos, side)) {
    path.push_back(direction::left);

    int new_bound =
        id_search(board, path, zero_pos, depth + 1, bound, side, offset_after);

    if (new_bound == -1) {
      return -1;
    }

    min = std::min(new_bound, min);
    path.pop_back();

    move(board, direction::right, zero_pos, side);
  }

  if (last_move != direction::left &&
      move(board, direction::right, zero_pos, side)) {
    path.push_back(direction::right);

    int new_bound =
        id_search(board, path, zero_pos, depth + 1, bound, side, offset_after);

    if (new_bound == -1) {
      return -1;
    }

    min = std::min(new_bound, min);
    path.pop_back();

    move(board, direction::left, zero_pos, side);
  }

  if (last_move != direction::up &&
      move(board, direction::down, zero_pos, side)) {
    path.push_back(direction::down);

    int new_bound =
        id_search(board, path, zero_pos, depth + 1, bound, side, offset_after);

    if (new_bound == -1) {
      return -1;
    }
    min = std::min(new_bound, min);
    path.pop_back();

    move(board, direction::up, zero_pos, side);
  }

  if (last_move != direction::down &&
      move(board, direction::up, zero_pos, side)) {
    path.push_back(direction::up);

    int new_bound =
        id_search(board, path, zero_pos, depth + 1, bound, side, offset_after);

    if (new_bound == -1) {
      return -1;
    }
    min = std::min(new_bound, min);
    path.pop_back();

    move(board, direction::down, zero_pos, side);
  }

  return min;
}

int ida_star(std::vector<int> &board, int offset_after,
             std::deque<direction> &path) {
  int side = sqrt(board.size());
  int zero_pos = find_zero_pos(board);

  path.push_back(direction::null);

  int bound = manhattan(board, offset_after);

  while (true) {
    int new_bound =
        id_search(board, path, zero_pos, 0, bound, side, offset_after);
    if (new_bound == -1) {
      return 1;
    }
    if (new_bound == std::numeric_limits<int>::max()) {
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
    std::vector<int> board;

    std::cin >> numbers;

    int tiles = numbers + 1;
    int side = sqrt(tiles);

    if (side * side != tiles) {
      throw std::invalid_argument("Invalid number of tiles.");
    }

    std::cin >> empty_tile;

    if ((empty_tile != -1 && (empty_tile > numbers || empty_tile < 1))) {
      char s[255];
      snprintf(s, 255,
               "Invalid zero position. Valid are numbers in range 1 to %d "
               "inclusive or -1 for default (%d).",
               tiles, tiles);
      throw std::invalid_argument(s);
    }

    board = std::vector<int>(tiles);

    bool offset = false;
    int offset_after = numbers;
    std::set<int> check_set;

    for (int i = 0; i < tiles; i++) {
      std::cin >> std::ws;
      std::cin >> board[i];

      int current_num = i + 1 - offset;

      if (current_num == empty_tile) {
        offset = true;
        offset_after = i;
      }

      check_set.insert(board[i]);
    }

    if (check_set.size() != (size_t)tiles) {
      throw std::invalid_argument("Invalid board (dupicate numbers)");
    }
    for (int n : check_set) {
      if (n < 0 || n >= tiles) {
        throw std::invalid_argument("Invalid board (invalid numbers)");
      }
    }

    if (!is_solvable(board)) {
      throw std::invalid_argument("Not solvable.");
    }

    std::deque<direction> path;

    auto start = std::chrono::system_clock::now();

    int result = ida_star(board, offset_after, path);

    double total_millis = std::chrono::duration_cast<std::chrono::milliseconds>(
                              std::chrono::system_clock::now() - start)
                              .count();

    if (result) {
      path.pop_front();
      std::cout << std::endl << path.size() << std::endl;
      for (auto dir : path) {
        std::cout << direction_strings[dir] << std::endl;
      }
    } else {
      std::cout << "No solution found" << std::endl;
    }

    printf("\ntime: %.2f\n\n", total_millis / 1e3);

  } catch (std::exception &e) {
    std::cout << e.what() << std::endl;
  }

  int a = system("pause");

  return a;
}
