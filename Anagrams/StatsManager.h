#pragma once
#include <string>

/**
 * @file StatsManager.h
 * @brief Contains the StatsManager class which manages the loading, saving, and updating of game statistics for the Anagrams game.
 * @date March 30th 2026
 * @author Manuelita Sowah
 */

/**
 * @struct GameStats
 * @brief A structure to hold various game statistics such as total games played, best score,
 */
struct GameStats {
    int gamesPlayed = 0; ///< Total number of games played
    int bestScore = 0; ///< Best score achieved
    int totalScore = 0; ///< Total score accumulated
    int totalWordsFound = 0; ///< Total words found
    int totalSecondsPlayed = 0; ///< Total time played in seconds

    /**
     * @brief Calculates the average score per game.
     * @return The average score, or 0 if no games have been played.
     */
    double AvgScore() const {
        return gamesPlayed == 0 ? 0.0 : (double)totalScore / (double)gamesPlayed;
    }

    /**
     * @brief Calculates the average number of words found per game.
     * @return The average words found, or 0 if no games have been played.
     */
    double AvgWords() const {
        return gamesPlayed == 0 ? 0.0 : (double)totalWordsFound / (double)gamesPlayed;
    }

    /**
     * @brief Calculates the average time played per game.
     * @return The average time played, or 0 if no games have been played.
     */
    double AvgSeconds() const {
        return gamesPlayed == 0 ? 0.0 : (double)totalSecondsPlayed / (double)gamesPlayed;
    }
};

/**
 * @class StatsManager
 * @brief A class responsible for managing game statistics, including loading from and saving to a file
 */
class StatsManager {
public:
    /**
     * @brief Loads game statistics from a file. If the file does not exist, it initializes with default stats.
     * @param filepath The path to the file where statistics are stored.
     * @return true if the statistics were loaded successfully, false otherwise.
     */
    bool Load(const std::string& filepath);

    /**
     * @brief Saves the current game statistics to a file.
     * @return true if the statistics were saved successfully, false otherwise.
     */
    bool Save() const;

    /**
     * @brief Records the results of a completed round, updating the statistics accordingly and saving to file.
     * @param score The score achieved in the round.
     * @param wordsFound The number of words found in the round.
     * @param secondsPlayed The time played in seconds for the round.
     */
    const GameStats& Get() const { return m_stats; }

    /**
     * @brief Gets a modifiable reference to the current game statistics, allowing for manual updates if needed.
     * @return A reference to the current GameStats object.
     */
    GameStats& Get() { return m_stats; }

    /**
     * @brief Records the results of a completed round, updating the statistics accordingly and saving to file.
     * @param score The score achieved in the round.
     * @param wordsFound The number of words found in the round.
     * @param secondsPlayed The time played in seconds for the round.
     */
    void RecordRound(int score, int wordsFound, int secondsPlayed);

    /**
     * @brief Resets all statistics to their default values and saves the reset state to file.
     */
    void Reset();

private:
    std::string m_path; ///< The file path where statistics are stored
    GameStats m_stats; ///< The current game statistics
};