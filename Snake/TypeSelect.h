/**
 * @file TypeSelect.h
 * @brief Type selection dialog declaration for the snake game.
 *
 * This file declares the dialog helper used to let the player choose
 * a snake type before gameplay begins.
 *
 * @author Atabong Eyambe Mokom
 */
#pragma once
#include <wx/wx.h>
#include "Theme.h"
#include "Model.h"

/**
 * @brief Runs the snake type selection dialog.
 *
 * Displays a dialog allowing the player to choose a snake type.
 * If the player confirms the selection, the chosen type is written
 * to the output parameter.
 *
 * @param parent The parent window for the dialog.
 * @param skin The skin used to style the dialog.
 * @param initial The initially selected snake type.
 * @param outType Output parameter that receives the chosen snake type.
 * @return true if the user confirms a selection.
 * @return false if the user cancels the dialog.
 *
 * @author Atabong Eyambe Mokom
 */
bool RunTypeSelectDialog(wxWindow* parent, const Skin& skin, SnakeType initial, SnakeType* outType);