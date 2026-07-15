/**
 * @file SnakeGame.h
 * @brief Factory declaration for creating the main snake game panel.
 *
 * This file declares the function responsible for creating the main
 * wxWidgets panel that hosts the snake game.
 *
 * @author Atabong Eyambe Mokom
 */
#pragma once
#include <wx/wx.h>

/**
 * @brief Creates the main snake game panel.
 *
 * This function constructs and returns the panel that contains the
 * main gameplay interface. The implementation is provided in
 * SnakeGame.cpp.
 *
 * @param parent The parent window that will contain the game panel.
 * @return A pointer to the created wxPanel.
 *
 * @author Atabong Eyambe Mokom
 */
wxPanel* CreateGamePanel(wxWindow* parent);