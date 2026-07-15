#include "Dictionary.h"
#include <fstream>
#include <algorithm>
#include <cctype>
#include <random>

// Helper functions for loading and processing the dictionary. These are not part of the public interface.
static std::string LowerAndFilter(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (unsigned char ch : s) {
        if (ch == '\r' || ch == '\n') continue;
        out.push_back((char)std::tolower(ch));
    }
    return out;
}

static bool IsAllLowerLetters(const std::string& s) {
    if (s.empty()) return false;
    for (char c : s) {
        if (c < 'a' || c > 'z') return false;
    }
    return true;
}

static std::array<unsigned char, 26> MakeCounts(const std::string& w) {
    std::array<unsigned char, 26> c{};
    for (char ch : w) c[ch - 'a']++;
    return c;
}

static bool Fits(const std::array<unsigned char, 26>& need,
                 const std::array<unsigned char, 26>& have) {
    for (int i = 0; i < 26; i++) {
        if (need[i] > have[i]) return false;
    }
    return true;
}

bool Dictionary::LoadFromFile(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) return false;

    m_words.clear();
    m_wordsOfLen6.clear();
    m_subwords_3_to_6.clear();

    std::string line;
    while (std::getline(in, line)) {
        std::string w = LowerAndFilter(line);

        // Minimum length of 3
        if ((int)w.size() < 2) continue;

        if (!IsAllLowerLetters(w)) continue;

        m_words.insert(w);

        if ((int)w.size() == 6) {
            m_wordsOfLen6.push_back(w);
        }
        if ((int)w.size() >= 3 && (int)w.size() <= 6) {
            WordInfo info;
            info.word = w;
            info.counts = MakeCounts(w);
            m_subwords_3_to_6.push_back(std::move(info));
        }
    }

    return !m_words.empty();
}

bool Dictionary::IsWord(const std::string& word) const {
    return m_words.find(word) != m_words.end();
}

std::string Dictionary::RandomWordOfLength(int len) const {
    if (len == 6 && !m_wordsOfLen6.empty()) {
        static std::random_device rd;
        static std::mt19937 rng(rd());
        std::uniform_int_distribution<size_t> dist(0, m_wordsOfLen6.size() - 1);
        return m_wordsOfLen6[dist(rng)];
    }

    std::vector<std::string> candidates;
    candidates.reserve(5000);

    for (const auto& w : m_words) {
        if ((int)w.size() == len) candidates.push_back(w);
    }
    if (candidates.empty()) return "";

    static std::random_device rd;
    static std::mt19937 rng(rd());
    std::uniform_int_distribution<size_t> dist(0, candidates.size() - 1);
    return candidates[dist(rng)];
}

std::string Dictionary::PickGoodBaseWord(int len, int minSolutions, int attempts) const {
    if (len != 6) return "";
    if (m_wordsOfLen6.empty()) return "";

    static std::random_device rd;
    static std::mt19937 rng(rd());
    std::uniform_int_distribution<size_t> dist(0, m_wordsOfLen6.size() - 1);

    std::string best = m_wordsOfLen6[dist(rng)];
    int bestCount = -1;

    // Evaluate several random base words; choose the one with the most sub-words.
    for (int t = 0; t < attempts; t++) {
        const std::string& base = m_wordsOfLen6[dist(rng)];
        auto have = MakeCounts(base);

        int count = 0;
        for (const auto& wi : m_subwords_3_to_6) {
            if (Fits(wi.counts, have)) count++;
        }

        if (count > bestCount) {
            bestCount = count;
            best = base;
        }

        // Early exit once we found a rack with enough solutions
        if (bestCount >= minSolutions) break;
    }

    return best;
}

// This function is called every round to find all valid words from the rack.
std::vector<std::string> Dictionary::AllWordsFromRack(const std::string& rack, int minLen, int maxLen) const
{
    std::vector<std::string> out;
    auto have = MakeCounts(rack);

    for (const auto& wi : m_subwords_3_to_6) {
        int L = (int)wi.word.size();
        if (L < minLen || L > maxLen) continue;
        if (Fits(wi.counts, have)) out.push_back(wi.word);
    }

    std::sort(out.begin(), out.end());
    out.erase(std::unique(out.begin(), out.end()), out.end());
    return out;
}