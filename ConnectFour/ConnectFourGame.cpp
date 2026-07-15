#include "ConnectFourGame.h"

/**
 * @brief Constructs a new Connect Four game.
 * Initializes a standard 6x7 board, sets the starting player to Player 1, and resets all game state variables.
 */
ConnectFourGame::ConnectFourGame()
    : board(6, 7), currentPlayer(1), isGameOver(false), winner(0), moveCount(0) {}

/**
 * @brief Drops a piece into the specified column.
 * Places the current player's piece in the lowest available row of the column.
 * Updates the game state (win, draw, or switch player).
 * @param column column index where the piece is dropped
 * @return true if the move was valid and executed
 * @return false if the move is invalid (e.g., column full or game over)
 */
bool ConnectFourGame::dropPiece(int column) {
    // reject invalid moves (game over, out of bounds, or column full)
    if (isGameOver || column < 0 || column >= board.getColumns() || board.isColumnFull(column)) {
        return false;
    }

    int row = board.getNextOpenRow(column); // find the lowest available row in the selected column
    if (row == -1) {
        return false;
    }

    board.setCell(row, column, currentPlayer); // place the piece for the current player
    moveCount++;

    if (checkWin()) { // check if the move resulted in a win, otherwise switch player
        isGameOver = true;
        winner = currentPlayer;
    } else if (checkDraw()) {
        isGameOver = true;
        winner = 0;
    } else {
        switchPlayer();
    }
    return true;
}

/**
 * @brief Switches the current player (alternates between Player 1 and Player 2)
 */
void ConnectFourGame::switchPlayer() {
    currentPlayer = (currentPlayer == 1) ? 2 : 1;
}

/**
 * @brief Checks if the game is a draw (when all columns are full and no player has won)
 * @return true if the board is full
 * @return false otherwise
 */
bool ConnectFourGame::checkDraw() const {
    for (int c = 0; c < board.getColumns(); c++) {
        if (!board.isColumnFull(c)) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Checks if the current board contains a winning condition.
 * Evaluates horizontal, vertical, and diagonal sequences of four consecutive pieces for the same player.
 * @return true if a winning sequence is found
 * @return false otherwise
 */
bool ConnectFourGame::checkWin() const {
    int rows = board.getRows();
    int cols = board.getColumns();
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            int player = board.getCell(r, c);
            if (player == 0) continue;
            if (c + 3 < cols && // check horizontal
                board.getCell(r, c + 1) == player &&
                board.getCell(r, c + 2) == player &&
                board.getCell(r, c + 3) == player) {
                return true;
            }
            if (r + 3 < rows &&  // check vertical
                board.getCell(r + 1, c) == player &&
                board.getCell(r + 2, c) == player &&
                board.getCell(r + 3, c) == player) {
                return true;
            }
            if (r + 3 < rows && c + 3 < cols && // check diagonal (down-right)
                board.getCell(r + 1, c + 1) == player &&
                board.getCell(r + 2, c + 2) == player &&
                board.getCell(r + 3, c + 3) == player) {
                return true;
            }
            if (r + 3 < rows && c - 3 >= 0 && // check diagonal (down-left)
                board.getCell(r + 1, c - 1) == player &&
                board.getCell(r + 2, c - 2) == player &&
                board.getCell(r + 3, c - 3) == player) {
                return true;
            }
        }
    }
    return false;
}

/**
 * @brief Resets the game to its initial state. Clears the board and resets all game variables.
 */
void ConnectFourGame::resetGame() {
    board.reset();
    currentPlayer = 1;
    isGameOver = false;
    winner = 0;
    moveCount = 0;
}

/**
 * @brief Gets the current game board.
 * @return const ConnectFourBoard& Reference to the board
 */
const ConnectFourBoard& ConnectFourGame::getBoard() const {
    return board;
}

/**
 * @brief Gets the current player
 * @return int current player (1 or 2)
 */
int ConnectFourGame::getCurrentPlayer() const {
    return currentPlayer;
}

/**
 * @brief Checks if the game is over
 * @return true if the game has ended
 * @return false otherwise
 */
bool ConnectFourGame::getIsGameOver() const {
    return isGameOver;
}

/**
 * @brief Gets the winner of the game
 * @return int winner (0 = draw, 1 = player 1, 2 = player 2)
 */
int ConnectFourGame::getWinner() const {
    return winner;
}

/**
 * @brief Gets the number of moves played
 * @return int total move count
 */
int ConnectFourGame::getMoveCount() const {
    return moveCount;
}
