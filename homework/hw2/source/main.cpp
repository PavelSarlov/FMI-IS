#include <chrono>
#include <iostream>
#include <random>
#include <set>
#include <time.h>
#include <vector>

std::mt19937 mt(time(NULL));

void pause() {
  std::cout << "Enter anything to continue..." << std::endl;
  std::cin.clear();
  char c;
  std::cin >> c;
}

void print_board(std::vector<int> &nQueens) {
  for (size_t i = 0; i < nQueens.size(); i++) {
    for (size_t j = 0; j < nQueens.size(); j++) {
      std::cout << (nQueens[j] == (int)i ? '*' : '_') << ' ';
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

int find_conflicts(int n, int row, int col, std::vector<int> &queensPerRow,
                   std::vector<int> &queensPerD1,
                   std::vector<int> &queensPerD2) {
  const int dOffset = n - 1;

  return queensPerRow[row] + queensPerD1[col - row + dOffset] +
         queensPerD2[col + row];
}

bool has_conflicts(std::vector<int> &queensPerRow,
                   std::vector<int> &queensPerD1,
                   std::vector<int> &queensPerD2) {
  for (size_t i = 0; i < queensPerD1.size(); i++) {
    if (queensPerRow[i % queensPerRow.size()] > 1 || queensPerD1[i] > 1 ||
        queensPerD2[i] > 1) {
      return true;
    }
  }

  return false;
}

void init(std::vector<int> &nQueens, std::vector<int> &queensPerRow,
          std::vector<int> &queensPerD1, std::vector<int> &queensPerD2) {
  const int n = nQueens.size();
  const int dOffset = n - 1;

  // horse distance
  //
  /* for (int row = 0, col = 1; row < n; row++) { */
  /*   nQueens[col] = row; */
  /*   queensPerRow[row]++; */
  /*   queensPerD1[col - row + dOffset]++; */
  /*   queensPerD2[col + row]++; */

  /*   col += 2; */
  /*   if (col >= n) { */
  /*     col = 0; */
  /*   } */
  /* } */

  // greedy

  nQueens[0] = mt() % n;
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

    nQueens[col] = rowList[mt() % rowList.size()];
    queensPerRow[nQueens[col]]++;
    queensPerD1[col - nQueens[col] + dOffset]++;
    queensPerD2[col + nQueens[col]]++;
  }
}

int col_max_conflicts(std::vector<int> &nQueens, std::vector<int> &queensPerRow,
                      std::vector<int> &queensPerD1,
                      std::vector<int> &queensPerD2) {
  const int n = nQueens.size();

  int maxConflicts = 0;
  std::vector<int> colList;

  for (int col = 0; col < n; col++) {
    int row = nQueens[col];
    int conflicts =
        find_conflicts(n, row, col, queensPerRow, queensPerD1, queensPerD2) - 3;

    if (maxConflicts < conflicts) {
      maxConflicts = conflicts;
      colList.clear();
      colList.push_back(col);
    } else if (maxConflicts == conflicts) {
      colList.push_back(col);
    }
  }

  if (maxConflicts == 0) {
    return -1;
  }

  return colList[mt() % colList.size()];
}

int row_min_conflicts(int col, std::vector<int> &nQueens,
                      std::vector<int> &queensPerRow,
                      std::vector<int> &queensPerD1,
                      std::vector<int> &queensPerD2) {
  const int n = nQueens.size();
  const int dOffset = n - 1;

  int lastRow = nQueens[col];
  std::vector<int> rowList;

  queensPerRow[lastRow]--;
  queensPerD1[col - lastRow + dOffset]--;
  queensPerD2[col + lastRow]--;

  int minConflicts =
      find_conflicts(n, lastRow, col, queensPerRow, queensPerD1, queensPerD2);

  for (int row = 0; row < n; row++) {
    int conflicts =
        find_conflicts(n, row, col, queensPerRow, queensPerD1, queensPerD2);

    if (minConflicts > conflicts) {
      minConflicts = conflicts;
      rowList.clear();
      rowList.push_back(row);
    } else if (minConflicts == conflicts) {
      rowList.push_back(row);
    }
  }

  int minRow = rowList[mt() % rowList.size()];

  queensPerRow[minRow]++;
  queensPerD1[col - minRow + dOffset]++;
  queensPerD2[col + minRow]++;

  return minRow;
}

bool solve(std::vector<int> &nQueens) {
  int n = nQueens.size();

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

    while (iter++ <= 5 * n) {
      int col =
          col_max_conflicts(nQueens, queensPerRow, queensPerD1, queensPerD2);

      if (col == -1) {
        return true;
      }

      int row = row_min_conflicts(col, nQueens, queensPerRow, queensPerD1,
                                  queensPerD2);

      nQueens[col] = row;
    }
  }
}

int main() {
  int n = 0;

  std::cin >> n;

  std::vector<int> nQueens(n);

  auto start = std::chrono::system_clock::now();

  bool result = solve(nQueens);

  double total_millis = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now() - start)
                            .count();

  if (result) {
    printf("\ntime: %.2f\n\n", total_millis / 1e3);

    if (n <= 20) {
      print_board(nQueens);
    }
  } else {
    printf("No solution found\n");
  }

  pause();

  return 0;
}
