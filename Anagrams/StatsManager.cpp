#include "StatsManager.h"
#include <fstream>
#include <string>

// Helper function to parse a line in the format "key=value" into its components. Returns true if parsing was successful.
static bool ParseKV(const std::string& line, std::string& k, std::string& v) {
    auto p = line.find('=');
    if (p == std::string::npos) return false;
    k = line.substr(0, p);
    v = line.substr(p + 1);
    return true;
}

// Implementation of the StatsManager class, responsible for managing game statistics including loading, saving, and updating stats based on completed rounds.
bool StatsManager::Load(const std::string& filepath)
{
    m_path = filepath;

    std::ifstream in(filepath);
    if (!in) return false; // first run is fine

    GameStats s;
    std::string line;

    while (std::getline(in, line)) {
        std::string k, v;
        if (!ParseKV(line, k, v)) continue;

        int n = 0;
        try { n = std::stoi(v); } catch (...) { continue; }

        if (k == "gamesPlayed") s.gamesPlayed = n;
        else if (k == "bestScore") s.bestScore = n;
        else if (k == "totalScore") s.totalScore = n;
        else if (k == "totalWordsFound") s.totalWordsFound = n;
        else if (k == "totalSecondsPlayed") s.totalSecondsPlayed = n;
    }

    m_stats = s;
    return true;
}

bool StatsManager::Save() const
{
    if (m_path.empty()) return false;

    std::ofstream out(m_path, std::ios::trunc);
    if (!out) return false;

    out << "gamesPlayed=" << m_stats.gamesPlayed << "\n";
    out << "bestScore=" << m_stats.bestScore << "\n";
    out << "totalScore=" << m_stats.totalScore << "\n";
    out << "totalWordsFound=" << m_stats.totalWordsFound << "\n";
    out << "totalSecondsPlayed=" << m_stats.totalSecondsPlayed << "\n";
    return true;
}

void StatsManager::RecordRound(int score, int wordsFound, int secondsPlayed)
{
    m_stats.gamesPlayed += 1;
    m_stats.totalScore += score;
    m_stats.totalWordsFound += wordsFound;
    m_stats.totalSecondsPlayed += secondsPlayed;

    if (score > m_stats.bestScore) m_stats.bestScore = score;

    Save();
}

void StatsManager::Reset()
{
    m_stats = GameStats{};
    Save();
}