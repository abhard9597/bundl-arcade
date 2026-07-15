#pragma once
#include <string>
#include <unordered_set>
#include <vector>
#include <array>

/**
 * @file Dictionary.h
 * @brief Contains the Dictionary class which loads and queries a word list for validating guesses in the Anagrams game.
 * @date March 6th 2026
 * @author Manuelita Sowah
 */

/**
 * @class Dictionary
 * @brief Loads and queries a word list for validating guesses.
 */
class Dictionary {
public:
    /**
     * @brief Loads the dictionary from a file. The file should contain one word per line.
     * @param filename The path to the dictionary file.
     * @return true if the dictionary was loaded successfully, false otherwise.
     */
    bool LoadFromFile(const std::string& filename);

    /**
     * @brief Checks if a given word exists in the dictionary.
     * @param word The word to check.
     * @return true if the word exists in the dictionary, false otherwise.
     */
    bool IsWord(const std::string& word) const;

    /**
     * @brief Gets a random word of the specified length from the dictionary.
     * @param len Desired word length.
     * @return A random word of the specified length, or an empty string if no such word exists.
     */
    std::string RandomWordOfLength(int len) const;

    /**
     * @brief Picks a "good" base word (rack source) that has many valid sub-words.
     *
     * This improves gameplay by avoiding racks with very few possible words.
     *
     * @param len Base word length (use 6 for now).
     * @param minSolutions Stop early if we find a base word with at least this many sub-words.
     * @param attempts Number of random base words to evaluate before choosing the best.
     * @return A base word of length len, or an empty string if none exist.
     */
    std::string PickGoodBaseWord(int len, int minSolutions, int attempts) const;

    /**
     * @brief Gets all valid sub-words that can be formed from the given rack.
     * @param rack The string representing the current rack (e.g., "AETRNS").
     * @param minLen Minimum length of sub-words to consider (default 3).
     * @param maxLen Maximum length of sub-words to consider (default 6).
     * @return A vector of valid sub-words that can be formed from the rack.
     */
    std::vector<std::string> AllWordsFromRack(const std::string& rack, int minLen = 3, int maxLen = 6) const;

private:
    /**
     * @struct WordInfo
     * @brief Structure to hold information about a word, including its letter counts.
     */
    struct WordInfo {
        std::string word;
        std::array<unsigned char, 26> counts{};
    };

    std::unordered_set<std::string> m_words; ///< Set of all valid words for quick lookup

    std::vector<std::string> m_wordsOfLen6;    ///< Vector of all 6-letter words
    std::vector<WordInfo> m_subwords_3_to_6;   ///< Vector of words with lengths 3..6 and precomputed letter counts
};