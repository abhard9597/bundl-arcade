/**
 * @file ConnectFourStatsManager.cpp
 * @brief Implements local file storage for Connect Four statistics.
 *
 * This file contains the helper functions used to parse the statistics text
 * file and the member functions of ConnectFourStatsManager that update the
 * stored values after each finished game.
 *
 */

#include "ConnectFourStatsManager.h"

#include <fstream>
#include <string>

#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/filefn.h>

namespace
{
    /**
     * @brief Splits a key=value line into two strings.
     *
     * The statistics file uses a very small key=value format. This helper
     * validates that the separator exists before returning the parsed pieces.
     *
     * @param line Raw line read from the file.
     * @param key Output string that receives the parsed key.
     * @param value Output string that receives the parsed value text.
     * @return true if the line contains an equals sign.
     * @return false if the line does not match the expected format.
     */
    bool ParseKV(const std::string& line, std::string& key, std::string& value)
    {
        std::size_t pos = line.find('=');
        if (pos == std::string::npos) {
            return false;
        }

        key = line.substr(0, pos);
        value = line.substr(pos + 1);
        return true;
    }
}

/**
 * @brief Returns the default on-device path used for Connect Four statistics.
 *
 * The function creates the local application data directory if it does not
 * already exist, then appends the statistics file name.
 *
 * @return std::string Full path to the statistics text file.
 */
std::string ConnectFourStatsManager::GetDefaultFilePath()
{
    wxFileName dir(wxStandardPaths::Get().GetUserLocalDataDir(), "");

    if (!dir.DirExists()) {
        dir.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
    }

    wxFileName file(dir.GetFullPath(), "connect_four_stats.txt");
    return file.GetFullPath().ToStdString();
}

/**
 * @brief Loads statistics from the specified file.
 *
 * If the file cannot be opened, the function returns false and leaves the
 * existing in-memory totals unchanged except for storing the requested path.
 * Valid keys found in the file are copied into a temporary structure and then
 * assigned to the manager once parsing is complete.
 *
 * @param filepath Path to the statistics file.
 * @return true if the file was opened and parsed.
 * @return false if the file could not be opened.
 */
bool ConnectFourStatsManager::Load(const std::string& filepath)
{
    m_path = filepath;

    std::ifstream in(filepath);
    if (!in) {
        return false;
    }

    ConnectFourStats loadedStats;
    std::string line;

    while (std::getline(in, line)) {
        std::string key;
        std::string value;

        if (!ParseKV(line, key, value)) {
            continue; // Skip malformed lines instead of failing the entire load.
        }

        int number = 0;
        try {
            number = std::stoi(value);
        } catch (...) {
            continue; // Skip non-numeric values and keep reading the rest of the file.
        }

        if (key == "normal_gamesPlayed") loadedStats.normalSinglePlayer.gamesPlayed = number;
        else if (key == "normal_wins") loadedStats.normalSinglePlayer.wins = number;
        else if (key == "normal_losses") loadedStats.normalSinglePlayer.losses = number;
        else if (key == "normal_draws") loadedStats.normalSinglePlayer.draws = number;
        else if (key == "hard_gamesPlayed") loadedStats.hardSinglePlayer.gamesPlayed = number;
        else if (key == "hard_wins") loadedStats.hardSinglePlayer.wins = number;
        else if (key == "hard_losses") loadedStats.hardSinglePlayer.losses = number;
        else if (key == "hard_draws") loadedStats.hardSinglePlayer.draws = number;
        else if (key == "multi_gamesPlayed") loadedStats.multiplayer.gamesPlayed = number;
        else if (key == "multi_wins") loadedStats.multiplayer.wins = number;
        else if (key == "multi_losses") loadedStats.multiplayer.losses = number;
        else if (key == "multi_draws") loadedStats.multiplayer.draws = number;
    }

    m_stats = loadedStats;
    return true;
}

/**
 * @brief Saves the current statistics snapshot to disk.
 *
 * The file is written in a simple key=value format so it is easy to inspect,
 * reset, and reload.
 *
 * @return true if the file was written successfully.
 * @return false if the manager does not currently have a valid path or the file could not be opened.
 */
bool ConnectFourStatsManager::Save() const
{
    if (m_path.empty()) {
        return false;
    }

    std::ofstream out(m_path, std::ios::trunc);
    if (!out) {
        return false;
    }

    out << "normal_gamesPlayed=" << m_stats.normalSinglePlayer.gamesPlayed << "\n";
    out << "normal_wins=" << m_stats.normalSinglePlayer.wins << "\n";
    out << "normal_losses=" << m_stats.normalSinglePlayer.losses << "\n";
    out << "normal_draws=" << m_stats.normalSinglePlayer.draws << "\n";

    out << "hard_gamesPlayed=" << m_stats.hardSinglePlayer.gamesPlayed << "\n";
    out << "hard_wins=" << m_stats.hardSinglePlayer.wins << "\n";
    out << "hard_losses=" << m_stats.hardSinglePlayer.losses << "\n";
    out << "hard_draws=" << m_stats.hardSinglePlayer.draws << "\n";

    out << "multi_gamesPlayed=" << m_stats.multiplayer.gamesPlayed << "\n";
    out << "multi_wins=" << m_stats.multiplayer.wins << "\n";
    out << "multi_losses=" << m_stats.multiplayer.losses << "\n";
    out << "multi_draws=" << m_stats.multiplayer.draws << "\n";

    return true;
}

/**
 * @brief Clears all statistics and immediately saves the reset values.
 */
void ConnectFourStatsManager::Reset()
{
    m_stats = ConnectFourStats{};
    Save();
}

/**
 * @brief Updates one mode section using the game's winner code.
 *
 * The function increments games played every time it is called, then updates
 * the win, loss, or draw count based on the supplied winner value.
 *
 * @param stats Statistics section that should be modified.
 * @param winner Winner code produced by the game.
 */
void ConnectFourStatsManager::RecordModeResult(ConnectFourModeStats& stats, int winner)
{
    stats.gamesPlayed += 1;

    if (winner == 0) {
        stats.draws += 1;
    }
    else if (winner == 1) {
        stats.wins += 1;
    }
    else {
        stats.losses += 1;
    }

    Save();
}

/**
 * @brief Records a result in the normal single player section.
 * @param winner Winner code returned by the game logic.
 */
void ConnectFourStatsManager::RecordNormalSinglePlayer(int winner)
{
    RecordModeResult(m_stats.normalSinglePlayer, winner);
}

/**
 * @brief Records a result in the hard single player section.
 * @param winner Winner code returned by the game logic.
 */
void ConnectFourStatsManager::RecordHardSinglePlayer(int winner)
{
    RecordModeResult(m_stats.hardSinglePlayer, winner);
}

/**
 * @brief Records a result in the multiplayer section.
 * @param winner Winner code returned by the game logic.
 */
void ConnectFourStatsManager::RecordMultiplayer(int winner)
{
    RecordModeResult(m_stats.multiplayer, winner);
}
