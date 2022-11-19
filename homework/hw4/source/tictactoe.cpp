#include <algorithm>
#include <iostream>
#include <limits>
#include <vector>

template <typename T> using vec = std::vector<T>;

class TicTacToe {
  struct action {
    int value = 0;
    int index = -1;
  };

  enum symbol { first = 'X', second = 'O', empty = ' ', null };

  vec<symbol> _state = vec<symbol>(9, empty);
  symbol _player_symbol = first;
  symbol _computer_symbol = second;

  symbol _game_winner() {
    if (((_state[0] == _state[1] && _state[1] == _state[2]) ||
         (_state[0] == _state[3] && _state[3] == _state[6]) ||
         (_state[0] == _state[4] && _state[4] == _state[8])) &&
        _state[0] != empty) {
      return _state[0];
    }
    if (((_state[3] == _state[4] && _state[4] == _state[5]) ||
         (_state[1] == _state[4] && _state[4] == _state[7]) ||
         (_state[2] == _state[4] && _state[4] == _state[6])) &&
        _state[4] != empty) {
      return _state[4];
    }
    if (((_state[6] == _state[7] && _state[7] == _state[8]) ||
         (_state[2] == _state[5] && _state[5] == _state[8])) &&
        _state[8] != empty) {
      return _state[8];
    }

    return std::find(_state.begin(), _state.end(), empty) == _state.end()
               ? empty
               : null;
  }

  void _print_state() {
    printf("\n");

    for (int i = 0; i < 9; i++) {
      printf(" %c ", _state[i]);

      if ((i + 1) % 3 == 0 && i != 8) {
        printf("\n");
        printf("---+---+---");
        printf("\n");
      } else if (i != 8) {
        printf("|");
      }
    }

    printf("\n");
  }

  action _utility(symbol winner, int depth) {
    return {winner == empty            ? 0
            : winner == _player_symbol ? -10 + depth
                                       : 10 - depth};
  }

  int _minimax() {
    action best_action = _max(0);

    return best_action.index;
  }

  action _max(int depth, int alpha = std::numeric_limits<int>::min(),
              int beta = std::numeric_limits<int>::max()) {
    symbol winner = _game_winner();

    if (winner != null) {
      return _utility(winner, depth);
    }

    action max = {std::numeric_limits<int>::min()};

    for (int i = 0; i < 9; i++) {
      if (_state[i] == empty) {
        _state[i] = _computer_symbol;
        int min = _min(depth + 1).value;
        if (max.value < min) {
          max = {min, i};
        }
        if (max.value >= beta) {
          return max;
        }
        alpha = std::max(alpha, max.value);
        _state[i] = empty;
      }
    }

    return max;
  }

  action _min(int depth, int alpha = std::numeric_limits<int>::min(),
              int beta = std::numeric_limits<int>::max()) {
    symbol winner = _game_winner();

    if (winner != null) {
      return _utility(winner, depth);
    }

    action min = {std::numeric_limits<int>::max()};

    for (int i = 0; i < 9; i++) {
      if (_state[i] == empty) {
        _state[i] = _player_symbol;
        int max = _max(depth + 1).value;
        if (min.value > max) {
          min = {max, i};
        }
        if (min.value <= alpha) {
          return min;
        }
        beta = std::min(beta, min.value);
        _state[i] = empty;
      }
    }

    return min;
  }

public:
  TicTacToe() {}

  void play() {
    std::system("cls");

    printf("Computer(1) or Player(2) is first?: ");

    _player_symbol = getchar() == '2' ? first : second;
    _computer_symbol = _player_symbol == first ? second : first;

    bool invalid_move = false;
    symbol turn = first;

    while (_game_winner() == null) {
      std::system("cls");
      _print_state();

      if (_player_symbol == turn) {
        if (invalid_move) {
          printf("\nInvalid move!");
        }
        printf("\nEnter your move: ");

        int col, row;
        scanf("%d %d", &col, &row);

        if (row < 1 || row > 3 || col < 1 || col > 3 ||
            _state[(col - 1) + 3 * (row - 1)] != empty) {
          invalid_move = true;
          continue;
        }

        invalid_move = false;
        _state[(col - 1) + 3 * (row - 1)] = _player_symbol;
      } else {
        _state[_minimax()] = _computer_symbol;
      }

      turn = turn == first ? second : first;
    }

    std::system("cls");
    _print_state();
    if (_game_winner() == empty) {
      printf("\nIt's a tie!\n");
    } else {
      printf("\n%s wins!\n",
             _game_winner() == _player_symbol ? "Player" : "Computer");
    }
  }
};

int main() {
  TicTacToe game;

  game.play();

  std::system("pause");

  return 0;
}
