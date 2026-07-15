#pragma once
#include <wx/wx.h>

/**
 * @file App.h
 * @brief Contains the declaration of the MyApp class, which is the main application class for
 * @date March 30th 2026
 * @author Manuelita Sowah
 */

/**
 * @class MyApp
 * @brief The main application class for the Anagrams game, responsible for initializing the application and
 */
class SoundManager; // forward declare

/**
 * @brief The main application class for the Anagrams game, responsible for initializing the application and providing access to shared resources like the SoundManager.
 */
class MyApp : public wxApp
{
public:
    /**
     * @brief Initializes the application, loads sound effects, and creates the main frame.
     * @return true if initialization was successful, false otherwise.
     */
    bool OnInit() override; // ✅ add this

    /**
     * @brief Gets the SoundManager instance.
     * @return Reference to the SoundManager instance.
     */
    SoundManager& GetSfx();
};

wxDECLARE_APP(MyApp);