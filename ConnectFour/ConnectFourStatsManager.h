/**
 * @file ConnectFourStatsManager.h
 * @brief Declares the data structures and manager used to store Connect Four statistics.
 *
 * This file defines the per-mode statistics containers and the manager class that
 * loads, saves, resets, and updates locally stored Connect Four results. The data
 * is saved on the current device so each machine keeps its own separate record.
 *
 */

#ifndef CONNECTFOURSTATSMANAGER_H
#define CONNECTFOURSTATSMANAGER_H

#include <string>

/**
 * @struct ConnectFourModeStats
 * @brief Stores statistics for a single Connect Four mode.
 *
 * A mode section tracks the total number of completed games and how many of
 * those games ended in a win, loss, or draw from the perspective used by the
 * application for that mode.
 *
 */
struct ConnectFourModeStats {
    int gamesPlayed = 0; 
    int wins = 0;        
    int losses = 0;      
    int draws = 0;       
};

/**
 * @struct ConnectFourStats
 * @brief Groups statistics for every Connect Four mode tracked by the game.
 *
 * The application keeps separate totals for normal single player, hard single
 * player, and multiplayer so the statistics window can display each section
 * independently.
 *
 */
struct ConnectFourStats {
    ConnectFourModeStats normalSinglePlayer; 
    ConnectFourModeStats hardSinglePlayer;   
    ConnectFourModeStats multiplayer;        
};

/**
 * @class ConnectFourStatsManager
 * @brief Loads, saves, resets, and updates local Connect Four statistics.
 *
 * This manager owns the current statistics snapshot and the path of the text
 * file where the values are stored. It provides helper functions for recording
 * results for each mode and hides the file parsing details from the rest of the
 * game.
 *
 */
class ConnectFourStatsManager {
public:
    /**
     * @brief Loads statistics from a text file.
     *
     * The file is expected to contain simple key=value pairs. Unknown keys or
     * malformed lines are ignored so that a partially damaged file does not
     * crash the game.
     *
     * @param filepath Path to the statistics file that should be loaded.
     * @return true if the file was opened successfully and the manager updated its path.
     * @return false if the file could not be opened.
     */
    bool Load(const std::string& filepath);

    /**
     * @brief Saves the current statistics snapshot to the active file path.
     *
     * This function overwrites the existing file contents with the latest set
     * of values stored in the manager.
     *
     * @return true if the file was written successfully.
     * @return false if the file path is empty or the file could not be opened.
     */
    bool Save() const;

    /**
     * @brief Resets all tracked statistics back to zero.
     *
     * After clearing the in-memory data, the function immediately saves the
     * reset values to disk so the statistics window reflects the change on the
     * current device.
     */
    void Reset();

    /**
     * @brief Records the result of a normal single player game.
     *
     * The winner value is interpreted as 0 for a draw, 1 for a player win,
     * and any other value for a loss.
     *
     * @param winner Winner code returned by the game logic.
     */
    void RecordNormalSinglePlayer(int winner);

    /**
     * @brief Records the result of a hard single player game.
     *
     * The winner value is interpreted as 0 for a draw, 1 for a player win,
     * and any other value for a loss.
     *
     * @param winner Winner code returned by the game logic.
     */
    void RecordHardSinglePlayer(int winner);

    /**
     * @brief Records the result of a multiplayer game.
     *
     * In multiplayer, the stored values treat Player 1 wins as wins and Player 2
     * wins as losses so the statistics dialog can present a consistent layout.
     *
     * @param winner Winner code returned by the game logic.
     */
    void RecordMultiplayer(int winner);

    /**
     * @brief Provides read-only access to the current statistics snapshot.
     * @return const ConnectFourStats& Reference to the stored statistics.
     */
    const ConnectFourStats& Get() const { return m_stats; }

    /**
     * @brief Provides writable access to the current statistics snapshot.
     * @return ConnectFourStats& Reference to the stored statistics.
     */
    ConnectFourStats& Get() { return m_stats; }

    /**
     * @brief Builds the default file path used to store statistics locally.
     *
     * The path points to the user's local application data directory so the
     * statistics are specific to the current device.
     *
     * @return std::string Full path to the default statistics file.
     */
    static std::string GetDefaultFilePath();

private:
    /**
     * @brief Applies a winner code to one mode section and saves the result.
     *
     * The section always increments games played. A winner value of 0 records
     * a draw, 1 records a win, and any other value records a loss.
     *
     * @param stats Mode section that should be updated.
     * @param winner Winner code returned by the game logic.
     */
    void RecordModeResult(ConnectFourModeStats& stats, int winner);

    std::string m_path;   /**< File path used for future load and save operations. */
    ConnectFourStats m_stats; /**< In-memory statistics snapshot currently managed by the class. */
};

#endif
