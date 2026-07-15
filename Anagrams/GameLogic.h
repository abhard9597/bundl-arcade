#pragma once
#include <string>
#include <unordered_set>
#include <vector>

class Dictionary;

/**
 * @file GameLogic.h
 * @brief Contains the GameLogic class which manages the core game state and logic for the Anagrams game
 * @date March 6th 2026
 * @author Manuelita Sowah
 *
 */

/**
 * @struct SubmitResult
 * @brief Result of attempting to submit a word.
 *
 * Is returned by GameLogic::TrySubmit to indicate whether the submission was accepted, how many points were gained, and what message to show in the UI.
 */
struct SubmitResult
{
    bool accepted = false;   ///< True if the was accepted
    int pointsGained = 0;    ///< How many points the word is worth (0 if not accepted)
    std::string message;     ///< A message to show in the UI about the submission result
    std::string displayWord; ///< What to show in the UI list
};

/**
 * @class GameLogic
 * @brief Manages the game state and logic for the Anagrams game.
 *
 * This class handles the current rack of letters, the words found so far, the player's score, and the logic for validating submitted words.
 */
class GameLogic
{
public:
    /** @brief Constructs a new GameLogic instance.
     *
     * Initializes the game state. The rack will be set when ResetRound is called.
     */
    GameLogic();

    /**
     * @brief Resets the game state for a new round.
     *
     * This will generate a new rack of letters (right now is hard-coded), clear the found words, and reset the score to 0. It should be called at the start of each round.
     */
    void ResetRound();

    /**
     * @brief Sets the dictionary to use for validating words.
     * @param dict The dictionary to use.
     */
    void SetDictionary(const Dictionary *dict) { m_dict = dict; }

    /**
     * @brief Sets the current rack of letters.
     * @param rackLetters A string representing the letters in the rack (e.g. "
     */
    void SetRack(const std::string &rackLetters);

    /**
     * @brief Attempts to submit a word for scoring.
     *
     * This function takes the raw input from the user, normalizes it, checks if it's valid (only letters, can be formed from the rack, not already found),
     * and if valid, adds it to the found words and updates the score.
     *
     * @param rawInput The raw word input from the user (not case-normalized, may contain spaces).
     * @return It returns a SubmitResult indicating whether the submission was accepted and any relevant messages or points gained.
     */
    SubmitResult TrySubmit(const std::string &rawInput);

    /**
     * @brief Gets the current score.
     * @return The player's current score.
     */
    int GetScore() const { return m_score; }

    /**
     * @brief Gets the current rack of letters.
     * @return A string representing the current rack of letters (e.g. "aetrns").
     */
    std::string GetRack() const { return m_rackLetters; }

    /**
     * @brief Gets the current rack formatted for display in the UI.
     * @return A wxString representing the current rack of letters, spaced and in uppercase (e.g. "A E T R N S").
     */
    std::string GetRackSpacedUpper() const;

    /**
     * @brief Gets the list of words that were valid but not found by the player.
     * @param allValid The list of all valid words for the current rack.
     * @return A vector of missed words.
     */
    std::vector<std::string> GetMissedWords(const std::vector<std::string> &allValid) const;

    /**
     * @brief Gets the count of valid words found so far.
     * @return The number of valid words found.
     */
    int GetWordsFoundCount() const { return (int)m_found.size(); } // ✅ NEW

private:
    /**
     * @brief Normalizes a raw input word by removing whitespace and converting to lowercase.
     * @param s The raw input string to normalize.
     * @return A normalized version of the input string.
     */
    static std::string Normalize(const std::string &s);

    /**
     * @brief Checks if a string consists only of letters a-z or A-Z.
     * @param s The string to check.
     * @return True if the string contains only letters, false otherwise.
     */
    static bool IsAllLetters(const std::string &s);

    /**
     * @brief Checks if a word can be formed from the given rack of letters.
     * @param word The word to check (should be normalized).
     * @param rack The current rack of letters (should be normalized).
     * @return True if the word can be formed from the rack, false otherwise.
     */
    static bool CanFormFromRack(const std::string &word, const std::string &rack);

private:
    std::string m_rackLetters;               ///< The current rack of letters (normalized, e.g. "aetrns")
    std::unordered_set<std::string> m_found; ///< The set of words found so far (normalized)
    int m_score = 0;                         ///< The player's current score
    const Dictionary *m_dict = nullptr;      ///< Pointer to the dictionary for validating words (not owned by GameLogic)
};