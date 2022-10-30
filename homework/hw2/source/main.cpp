#include <iostream>
#include <set>
#include <time.h>
#include <vector>

std::vector<int> init(int n) {
  std::vector<int> nQueens(n);
  std::vector<int> queensPerRow(n);
  std::vector<int> queensPerD1(2 * n - 1);
  std::vector<int> queensPerD2(2 * n - 1);

  const int dOffset = n - 1;

  nQueens[0] = rand() % n;
  queensPerRow[nQueens[0]] = 1;
  queensPerD1[-nQueens[0] + dOffset] = 1;
  queensPerD2[nQueens[0]] = 1;

  for (int i = 1; i < n; i++) {
    int minConflicts = INT32_MAX;
    int minRow = 0;

    for (int j = 0; j < n; j++) {
      int conflicts =
          queensPerRow[j] + queensPerD1[i - j + dOffset] + queensPerD2[i + j];

      if (minConflicts > conflicts) {
        minConflicts = conflicts;
        minRow = j;
      }
      if (minConflicts == conflicts) {
        minRow = rand() % 2 ? j : minRow;
      }
    }

    nQueens[i] = minRow;
    queensPerRow[nQueens[i]]++;
    queensPerD1[i - nQueens[i] + dOffset]++;
    queensPerD2[i + nQueens[i]]++;
  }

  return nQueens;
}

void solve(int n) {
  std::vector<int> nQueens = init(n);

  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      std::cout << (nQueens[j] == i ? '*' : '_') << ' ';
    }
    std::cout << std::endl;
  }
}

int main() {
  srand(time(NULL));

  int n = 0;

  std::cin >> n;

  solve(n);

  return 0;
}
