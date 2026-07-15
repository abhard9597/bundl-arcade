#include "ConnectFourAI.h"
#include <cstdlib>
#include <ctime>
#include <limits>
#include <vector>
#include <cmath>

/**
 * @brief Constructs the Connect Four AI.
 * Initializes the AI with the specified difficulty level and seeds the random number generator for move selection.
 * @param difficulty AI difficulty level
 */
ConnectFourAI::ConnectFourAI(AIDifficulty difficulty)
    : difficulty(difficulty)
{
    // seed the random number generator once so random moves are different each run
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
}

/**
 * @brief Chooses a column for the AI's next move.
 * Selects a move based on the current difficulty level.
 * @param board current game board
 * @param aiPlayer AI player identifier
 * @return int selected column, or -1 if no move is available
 */
int ConnectFourAI::chooseColumn(const ConnectFourBoard& board, int aiPlayer) {
    if (difficulty == AIDifficulty::Normal) {
        return normalMove(board, aiPlayer);
    }

    if (difficulty == AIDifficulty::Hard) {
        return hardMove(board, aiPlayer);
    }

    return -1;
}

/**
 * @brief Sets the AI difficulty level.
 * @param diff new difficulty level
 */
void ConnectFourAI::setDifficulty(AIDifficulty diff) {
    difficulty = diff;
}

/**
 * @brief Gets the current AI difficulty level.
 * @return AIDifficulty current difficulty
 */
AIDifficulty ConnectFourAI::getDifficulty() const {
    return difficulty;
}

/**
 * @brief Selects a move using the Normal difficulty strategy.
 * Normal mode chooses randomly from all valid columns.
 * @param board current game board
 * @param aiPlayer AI player identifier
 * @return int selected column, or -1 if no valid moves exist
 */
int ConnectFourAI::normalMove(const ConnectFourBoard& board, int aiPlayer) {
    std::vector<int> validColumns;

    // collect all columns that still have space
    for (int c = 0; c < board.getColumns(); c++) {
        if (!board.isColumnFull(c)) {
            validColumns.push_back(c);
        }
    }
    if (validColumns.empty()) {
        return -1;
    }
    // pick one of the valid columns randomly
    int randomIndex = std::rand() % static_cast<int>(validColumns.size());
    return validColumns[randomIndex];
}

/**
 * @brief Selects a move using the Hard difficulty strategy.
 * Hard mode sometimes behaves like Normal mode, but otherwise attempts to play strategically by
 * choosing winning moves, blocking the opponent, or selecting the strongest available position.
 * @param board current game board
 * @param aiPlayer AI player identifier
 * @return int selected column
 */
int ConnectFourAI::hardMove(const ConnectFourBoard& board, int aiPlayer) {
    int randomChance = std::rand() % 100;
    // hard mode only plays a smart move 50% of the time.
    // the other 50% of the time it behaves like normal mode.
    if (randomChance < 50) {
        return normalMove(board, aiPlayer);
    }
    int opponent = (aiPlayer == 1) ? 2 : 1;
    // first priority: if the AI can win immediately, do it
    for (int c = 0; c < board.getColumns(); c++) {
        if (!board.isColumnFull(c) && wouldWin(board, c, aiPlayer)) {
            return c;
        }
    }
    // second priority: if the opponent could win next turn, block them
    for (int c = 0; c < board.getColumns(); c++) {
        if (!board.isColumnFull(c) && wouldWin(board, c, opponent)) {
            return c;
        }
    }
    // otherwise, score each possible move and choose the best one
    int bestColumn = -1;
    int bestScore = std::numeric_limits<int>::min();

    for (int c = 0; c < board.getColumns(); c++) {
        if (!board.isColumnFull(c)) {
            int score = scoreColumn(board, c, aiPlayer);

            if (score > bestScore) {
                bestScore = score;
                bestColumn = c;
            }
        }
    }

    if (bestColumn != -1) {
        return bestColumn;
    }
    
    return normalMove(board, aiPlayer);
}

/**
 * @brief Checks whether placing a piece in a column would result in a win.
 * Simulates a move in the specified column and evaluates whether it would create a sequence of four connected pieces for the given player.
 * @param board current game board
 * @param column column to test
 * @param player player identifier
 * @return true if the move would produce a winning condition
 * @return false otherwise
 */
bool ConnectFourAI::wouldWin(const ConnectFourBoard& board, int column, int player) const {
    if (column < 0 || column >= board.getColumns() || board.isColumnFull(column)) {
        return false;
    }

    int row = board.getNextOpenRow(column);

    if (row == -1) {
        return false;
    }

    // these are the 4 directions we need to check in Connect Four:
    // horizontal, vertical, diagonal down-right, diagonal down-left
    const int directions[4][2] = {
        {0, 1},
        {1, 0},
        {1, 1},
        {1, -1}
    };

    for (int i = 0; i < 4; i++) {
        int dr = directions[i][0];
        int dc = directions[i][1];
        int count = 1; // start at 1 because we are pretending to place a piece here

        // check forward in this direction
        int r = row + dr;
        int c = column + dc;

        while (r >= 0 && r < board.getRows() &&
               c >= 0 && c < board.getColumns() &&
               board.getCell(r, c) == player) {
            count++;
            r += dr;
            c += dc;
        }

        // check backward in the opposite direction
        r = row - dr;
        c = column - dc;

        while (r >= 0 && r < board.getRows() &&
               c >= 0 && c < board.getColumns() &&
               board.getCell(r, c) == player) {
            count++;
            r -= dr;
            c -= dc;
        }

        // if we can connect 4 or more, this move would win
        if (count >= 4) {
            return true;
        }
    }

    return false;
}

/**
 * @brief Scores a column based on strategic value.
 * Assigns higher scores to stronger moves, such as moves near the center of the board or moves that build longer connections.
 * @param board current game board
 * @param column column to evaluate
 * @param player player identifier
 * @return int strategic score for the column
 */
int ConnectFourAI::scoreColumn(const ConnectFourBoard& board, int column, int player) const {
    if (column < 0 || column >= board.getColumns() || board.isColumnFull(column)) {
        return -100000;
    }

    int row = board.getNextOpenRow(column);
    int score = 0;
    int centerColumn = board.getColumns() / 2;

    // in Connect Four, center columns are usually stronger than edge columns
    score += 10 - (std::abs(centerColumn - column) * 2);

    const int directions[4][2] = {
        {0, 1},
        {1, 0},
        {1, 1},
        {1, -1}
    };

    for (int i = 0; i < 4; i++) {
        int dr = directions[i][0];
        int dc = directions[i][1];
        int count = 1;

        int r = row + dr;
        int c = column + dc;

        while (r >= 0 && r < board.getRows() &&
               c >= 0 && c < board.getColumns() &&
               board.getCell(r, c) == player) {
            count++;
            r += dr;
            c += dc;
        }

        r = row - dr;
        c = column - dc;

        while (r >= 0 && r < board.getRows() &&
               c >= 0 && c < board.getColumns() &&
               board.getCell(r, c) == player) {
            count++;
            r -= dr;
            c -= dc;
        }

        // give a better score to moves that build longer connections
        if (count == 2) {
            score += 6;
        } else if (count == 3) {
            score += 15;
        }
    }

    return score;
}