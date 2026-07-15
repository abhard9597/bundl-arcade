#ifndef CONNECTFOURGAME_H
#define CONNECTFOURGAME_H

#include "ConnectFourBoard.h"

/**
 * @class ConnectFourGame
 * @brief Manages the logic and state of a Connect Four game.
 * 
 * This class controls gameplay, including player turns, move validation, win/draw detection, and overall game state.
 */
class ConnectFourGame {
private:
    ConnectFourBoard board; // game board object
    int currentPlayer; // current player (1 or 2)
    bool isGameOver; // indicates whether the game has ended
    int winner; // winner of the game (0 = draw, 1 = player 1, 2 = player 2)
    int moveCount; // total number of moves made

public:
    /**
     * @brief Constructs a new Connect Four game.
     * Initializes the board and sets default game state values.
     */
    ConnectFourGame();

    /**
     * @brief Drops a piece into a specified column.
     * Attempts to place the current player's piece in the given column.
     * Updates game state based on the result.
     * @param column Column index where the piece is dropped.
     * @return true if the move is valid and executed
     * @return false if the move is invalid
     */
    bool dropPiece(int column);

    /**
     * @brief Checks if the current board contains a winning condition.
     * @return true if a player has won
     * @return false otherwise
     */
    bool checkWin() const;

    /**
     * @brief Checks if the game is a draw.
     * @return true if the board is full and no player has won
     * @return false otherwise
     */
    bool checkDraw() const;

    /**
     * @brief Switches the current player.
     */
    void switchPlayer();

    /**
     * @brief Resets the game to its initial state.
     */
    void resetGame();

    /**
     * @brief Gets the current game board.
     * @return const ConnectFourBoard& Reference to the board
     */
    const ConnectFourBoard& getBoard() const;

    /**
     * @brief Gets the current player
     * @return int current player (1 or 2)
     */
    int getCurrentPlayer() const;

    /**
     * @brief Checks if the game is over
     * @return true if the game has ended
     * @return false otherwise
     */
    bool getIsGameOver() const;

    /**
     * @brief Gets the winner of the game.
     * @return int winner (0 = draw, 1 = player 1, 2 = player 2)
     */
    int getWinner() const;

    /**
     * @brief Gets the number of moves played.
     * @return int total move count
     */
    int getMoveCount() const;
};

#endif