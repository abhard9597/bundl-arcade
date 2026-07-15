#include "ConnectFourBoard.h"

/**
 * @brief Constructs a Connect Four board with specified dimensions. Initializes a 2D grid with all cells set to 0 (empty).
 * @param r number of rows in the board.
 * @param c number of columns in the board.
 */
ConnectFourBoard::ConnectFourBoard(int r, int c) : rows(r), columns(c) {
    grid = std::vector<std::vector<int>>(rows, std::vector<int>(columns, 0));
}

/**
 * @brief Returns the value of a specific cell on the board
 * @param r row index
 * @param c column index
 * @return int value of the cell (0 = empty, 1 = player 1, 2 = player 2)
 */
int ConnectFourBoard::getCell(int r, int c) const {
    return grid[r][c];
}

/**
 * @brief Sets the value of a specific cell on the board
 * @param r row index.
 * @param c column index
 * @param player player identifier (1 or 2)
 */
void ConnectFourBoard::setCell(int r, int c, int player) {
    grid[r][c] = player;
}

/**
 * @brief Resets the board to its initial empty state (sets all cells in the grid to 0)
 */
void ConnectFourBoard::reset() {
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < columns; c++) {
            grid[r][c] = 0;
        }
    }
}

/**
 * @brief Checks if a column is full (if the top cell is occupied)
 * @param column column index to check
 * @return true if the column is full
 * @return false if the column has at least one empty space
 */
bool ConnectFourBoard::isColumnFull(int column) const {
    return grid[0][column] != 0;
}

/**
 * @brief Finds the next available row in a given column
 * @param column column index
 * @return int row index of the next available slot, or -1 if the column is full
 */
int ConnectFourBoard::getNextOpenRow(int column) const {
    for (int r = rows - 1; r >= 0; r--) {
        if (grid[r][column] == 0) {
            return r;
        }
    }
    return -1;
}

/**
 * @brief Returns the number of rows in the board
 * @return int number of rows
 */
int ConnectFourBoard::getRows() const {
    return rows;
}

/**
 * @brief Returns the number of columns in the board
 * @return int number of columns
 */
int ConnectFourBoard::getColumns() const {
    return columns;
}
