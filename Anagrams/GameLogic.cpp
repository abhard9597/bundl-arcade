#include "GameLogic.h"
#include "Dictionary.h"
#include <array>
#include <algorithm>
#include <cctype>

GameLogic::GameLogic() {
    ResetRound();
}

void GameLogic::ResetRound() {
    m_found.clear();
    m_score = 0;
}

std::string GameLogic::Normalize(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    // Remove whitespace and convert to lowercase
    for (unsigned char ch : s) {
        if (std::isspace(ch)) continue;
        out.push_back((char)std::tolower(ch));
    }
    return out;
}

bool GameLogic::IsAllLetters(const std::string& s) {
    // Empty string is not valid
    for (char ch : s) {
        if (ch < 'a' || ch > 'z') return false;
    }
    return true;
}

bool GameLogic::CanFormFromRack(const std::string& word, const std::string& rack) {
    std::array<int, 26> avail{};
    for (char c : rack) avail[c - 'a']++;

    // Check if we can form the word with the available letters in the rack
    for (char c : word) {
        int idx = c - 'a';
        if (avail[idx] == 0) return false;
        avail[idx]--;
    }
    return true;
}

SubmitResult GameLogic::TrySubmit(const std::string& rawInput) {
    SubmitResult res;

    std::string w = Normalize(rawInput);

    // Validation checks
    if (w.empty()) {
        res.accepted = false;
        res.message = "Type a word first.";
        return res;
    }

    if (!IsAllLetters(w)) {
        res.accepted = false;
        res.message = "Only letters a-z allowed.";
        return res;
    }

    if ((int)w.size() < 3) {
        res.accepted = false;
        res.message = "Word too short (min 3).";
        return res;
    }

    if (m_found.find(w) != m_found.end()) {
        res.accepted = false;
        res.message = "Already found that word!";
        return res;
    }

    if (!CanFormFromRack(w, m_rackLetters)) {
        res.accepted = false;
        res.message = "Can't form that from the letters.";
        return res;
    }

    if (m_dict && !m_dict->IsWord(w)) {
        res.accepted = false;
        res.message = "Not in dictionary.";
        return res;
}

    // Accept
    int points = (int)w.size() * (int)w.size();
    m_score += points;
    m_found.insert(w);

    res.accepted = true;
    res.pointsGained = points;
    res.displayWord = w;
    res.message = "Nice! +" + std::to_string(points) + " points";
    return res;
}

std::string GameLogic::GetRackSpacedUpper() const {
    std::string out;
    out.reserve(m_rackLetters.size() * 2);

    for (size_t i = 0; i < m_rackLetters.size(); i++) {
        char up = (char)std::toupper((unsigned char)m_rackLetters[i]);
        out.push_back(up);
        if (i + 1 < m_rackLetters.size()) out.push_back(' ');
    }
    return out;
}

void GameLogic::SetRack(const std::string& rackLetters) {
    m_rackLetters = rackLetters;
}

std::vector<std::string> GameLogic::GetMissedWords(const std::vector<std::string>& allValid) const
{
    std::vector<std::string> missed;
    missed.reserve(allValid.size());

    for (const auto& w : allValid) {
        if (m_found.find(w) == m_found.end()) missed.push_back(w);
    }
    return missed;
}