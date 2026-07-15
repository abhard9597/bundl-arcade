#ifndef CONNECTFOURBOARD_H
#define CONNECTFOURBOARD_H

#include <vector>

/**
 * @class ConnectFourBoard
 * @brief Represents the game board for a Connect Four game
 * 
 * This class manages a 2D grid representing the board state.
 * Each cell stores an integer value (0 = empty, 1 = player 1, 2 = player 2)
 */

class ConnectFourBoard {
private:
    int rows; // number of rows in the board
    int columns; // number of columns in the board
    std::vector<std::vector<int>> grid; // 2D grid storing board state

public:
    /**
     * @brief Constructs a Connect Four board
     * Initializes a board with the proper dimensions (6x7)
     * 
     * @param r number of rows
     * @param c number of columns
     */
    ConnectFourBoard(int r = 6, int c = 7);

    /**
     * @brief Gets the value of a specific cell
     * @param r row index
     * @param c column index
     * @return int value of the cell
     */
    int getCell(int r, int c) const;

    /**
     * @brief Sets the value of a specific cell
     * @param r row index
     * @param c column index
     * @param player player identifier (1 or 2)
     */
    void setCell(int r, int c, int player);

    /**
     * @brief Resets the board to an empty state
     */
    void reset();

    /**
     * @brief Checks if a column is full
     * @param column column index
     * @return true if the column is full
     * @return false Otherwise
     */
    bool isColumnFull(int column) const;

    /**
     * @brief Finds the next available row in a column
     * @param column column index
     * @return int row index of the next available position, or -1 if full
     */
    int getNextOpenRow(int column) const;

    /**
     * @brief Gets the number of rows
     * @return int number of rows
     */
    int getRows() const;

    /**
     * @brief Gets the number of columns
     * @return int number of columns
     */
    int getColumns() const;
};

#endif
