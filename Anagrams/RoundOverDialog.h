#pragma once
#include <wx/wx.h>
#include <vector>
#include <string>

/**
 * @file RoundOverDialog.h
 * @brief Contains the RoundOverDialog class which represents the dialog shown at the end of a round in the Anagrams game.
 * @date March 30th 2026
 * @author Manuelita Sowah
 */

 /**
  * @class RoundOverDialog
  * @brief A dialog that appears at the end of a round, showing the player's final
  */
class RoundOverDialog : public wxDialog {

public:
    /**
     * @brief Represents the result of the dialog, indicating whether the user wants to go back to the menu or start a new game.
     */
    enum Result { GoMenu = 1, PlayAgain = 2 };

    /**
     * @brief Constructs the RoundOverDialog.
     * @param parent The parent window of this dialog.
     * @param finalScore The final score achieved in the round, to be displayed in the dialog.
     * @param missedWords A list of valid words that were not found by the player, to be displayed in the dialog.
     */
    RoundOverDialog(wxWindow* parent, int finalScore, const std::vector<std::string>& missedWords);
};