#include <algorithm>
#include <chrono>
#include <iostream>
#include <set>
#include <time.h>
#include <vector>

void print_board(std::vector<int> &nQueens) {
  for (size_t i = 0; i < nQueens.size(); i++) {
    for (size_t j = 0; j < nQueens.size(); j++) {
      std::cout << (nQueens[j] == (int)i ? '*' : '_') << ' ';
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

int attacks(int n) { return (1 + n) * n / 2; }

int find_conflicts(int n, int row, int col, std::vector<int> &queensPerRow,
                   std::vector<int> &queensPerD1,
                   std::vector<int> &queensPerD2) {
  const int dOffset = n - 1;

  return attacks(queensPerRow[row]) +
         attacks(queensPerD1[col - row + dOffset]) +
         attacks(queensPerD2[col + row]);
}

bool has_conflicts(std::vector<int> &queensPerRow,
                   std::vector<int> &queensPerD1,
                   std::vector<int> &queensPerD2) {
  for (auto x : queensPerRow) {
    if (x > 1) {
      return true;
    }
  }

  for (size_t i = 0; i < queensPerD1.size(); i++) {
    if (queensPerD1[i] > 1 || queensPerD2[i] > 1) {
      return true;
    }
  }

  return false;
}

void init(std::vector<int> &nQueens, std::vector<int> &queensPerRow,
          std::vector<int> &queensPerD1, std::vector<int> &queensPerD2) {
  const int n = nQueens.size();
  const int dOffset = n - 1;

  nQueens[0] = rand() % n;
  queensPerRow[nQueens[0]] = 1;
  queensPerD1[-nQueens[0] + dOffset] = 1;
  queensPerD2[nQueens[0]] = 1;

  for (int col = 1; col < n; col++) {
    int minConflicts = INT32_MAX;
    std::vector<int> rowList;

    for (int row = 0; row < n; row++) {
      int conflicts =
          find_conflicts(n, row, col, queensPerRow, queensPerD1, queensPerD2);

      if (minConflicts > conflicts) {
        minConflicts = conflicts;
        rowList.clear();
        rowList.push_back(row);
      }
      if (minConflicts == conflicts) {
        rowList.push_back(row);
      }
    }

    nQueens[col] = rowList[rand() % rowList.size()];
    queensPerRow[nQueens[col]]++;
    queensPerD1[col - nQueens[col] + dOffset]++;
    queensPerD2[col + nQueens[col]]++;
  }
}

int col_max_conflicts(std::vector<int> &nQueens, std::vector<int> &queensPerRow,
                      std::vector<int> &queensPerD1,
                      std::vector<int> &queensPerD2, int *lastCol = nullptr) {
  const int n = nQueens.size();

  int maxConflicts = 0;
  std::vector<int> colList;

  for (int col = 0; col < n; col++) {
    if (lastCol && col == *lastCol) {
      continue;
    }

    int row = nQueens[col];
    int conflicts =
        find_conflicts(n, row, col, queensPerRow, queensPerD1, queensPerD2);

    if (maxConflicts < conflicts) {
      maxConflicts = conflicts;
      colList.clear();
      colList.push_back(col);
    }
    if (maxConflicts == conflicts) {
      colList.push_back(col);
    }
  }

  return colList[rand() % colList.size()];
}

int row_min_conflicts(int col, std::vector<int> &nQueens,
                      std::vector<int> &queensPerRow,
                      std::vector<int> &queensPerD1,
                      std::vector<int> &queensPerD2) {
  const int n = nQueens.size();
  const int dOffset = n - 1;

  int lastRow = nQueens[col];
  std::vector<int> rowList = {lastRow};

  queensPerRow[lastRow]--;
  queensPerD1[col - lastRow + dOffset]--;
  queensPerD2[col + lastRow]--;

  int minConflicts =
      find_conflicts(n, lastRow, col, queensPerRow, queensPerD1, queensPerD2);

  for (int row = 0; row < n; row++) {
    if (row == lastRow) {
      continue;
    }

    int conflicts =
        find_conflicts(n, row, col, queensPerRow, queensPerD1, queensPerD2);

    if (minConflicts > conflicts) {
      minConflicts = conflicts;
      rowList.clear();
      rowList.push_back(row);
    }
    if (minConflicts == conflicts) {
      rowList.push_back(row);
    }
  }

  int minRow = rowList[rand() % rowList.size()];

  queensPerRow[minRow]++;
  queensPerD1[col - minRow + dOffset]++;
  queensPerD2[col + minRow]++;

  return minRow;
}

bool solve(int n, std::vector<int> &nQueens) {
  if (n == 2 || n == 3) {
    return false;
  }

  while (true) {
    std::vector<int> queensPerRow(n);
    std::vector<int> queensPerD1(2 * n - 1);
    std::vector<int> queensPerD2(2 * n - 1);

    init(nQueens, queensPerRow, queensPerD1, queensPerD2);
    int iter = 0;

    if (!has_conflicts(queensPerRow, queensPerD1, queensPerD2)) {
      return true;
    }

    int *lastCol = nullptr;

    while (iter++ <= 10 * n) {
      int col = col_max_conflicts(nQueens, queensPerRow, queensPerD1,
                                  queensPerD2, lastCol);
      delete lastCol;
      lastCol = new int(col);

      int row = row_min_conflicts(col, nQueens, queensPerRow, queensPerD1,
                                  queensPerD2);

      nQueens[col] = row;
    }
  }
}

int main() {
  srand(time(NULL));

  int n = 0;

  std::cin >> n;

  std::vector<int> nQueens(n);

  auto start = std::chrono::system_clock::now();

  bool result = solve(n, nQueens);

  double total_millis = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now() - start)
                            .count();

  if (result) {
    printf("\ntime: %.2f\n\n", total_millis / 1e3);

    /* print_board(nQueens); */
  } else {
    printf("No solution found\n");
  }

  return 0;
}
