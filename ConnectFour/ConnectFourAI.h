#ifndef CONNECTFOURAI_H
#define CONNECTFOURAI_H

#include "ConnectFourBoard.h"

/**
 * @enum AIDifficulty
 * @brief Represents the difficulty levels for the AI.
 */
enum class AIDifficulty {
    None,   // no AI (used for multiplayer)
    Normal, // basic AI strategy
    Hard    // advanced AI strategy
};

/**
 * @class ConnectFourAI
 * @brief Provides AI decision-making for Connect Four.
 * 
 * This class selects moves for the computer player based on the chosen difficulty level.
 * Different strategies are applied depending on the difficulty.
 */
class ConnectFourAI {
public:
    /**
     * @brief Constructs the AI with a specified difficulty.
     * @param diff initial AI difficulty
     */
    ConnectFourAI(AIDifficulty diff = AIDifficulty::None);

    /**
     * @brief Chooses a column for the AI's next move.
     * Selects a column based on the current board state and difficulty level.
     * @param board current game board
     * @param aiPlayer AI player's identifier (1 or 2)
     * @return int column index for the AI's move, or -1 if no valid move exists
     */
    int chooseColumn(const ConnectFourBoard& board, int aiPlayer);

    /**
     * @brief Sets the AI difficulty.
     * @param diff new difficulty level
     */
    void setDifficulty(AIDifficulty diff);

    /**
     * @brief Gets the current AI difficulty.
     * @return AIDifficulty current difficulty
     */
    AIDifficulty getDifficulty() const;

private:
    AIDifficulty difficulty; // current AI difficulty level

    /**
     * @brief Selects a move using the Normal difficulty strategy.
     * @param board current game board
     * @param aiPlayer AI player identifier
     * @return int selected column
     */
    int normalMove(const ConnectFourBoard& board, int aiPlayer);

    /**
     * @brief Selects a move using the Hard difficulty strategy.
     * @param board current game board
     * @param aiPlayer AI player identifier
     * @return int selected column
     */
    int hardMove(const ConnectFourBoard& board, int aiPlayer);

    /**
     * @brief Checks if placing a piece in a column results in a win.
     * @param board current game board
     * @param column column to test
     * @param player player identifier
     * @return true if the move would result in a win
     * @return false otherwise
     */
    bool wouldWin(const ConnectFourBoard& board, int column, int player) const;

    /**
     * @brief Scores a column based on strategic value.
     * Evaluates how strong a move is based on board positioning.
     * @param board current game board
     * @param column column to evaluate
     * @param player player identifier
     * @return int score representing the strength of the move
     */
    int scoreColumn(const ConnectFourBoard& board, int column, int player) const;
};

#endif