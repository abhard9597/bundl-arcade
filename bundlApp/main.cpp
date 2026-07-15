#include <wx/wx.h>
#include "MainFrame.h"
#include <wx/image.h>

/**
 * @brief Application entry class for bundl.
 *
 * This class initializes wxWidgets image support, creates the main launcher frame, and starts the application interface.
 *
 * @author Aditi Bhardwaj
 */
class BundlApp : public wxApp
{
public:
    /**
     * @brief Initializes the application on startup.
     *
     * Sets up image handlers, creates the main frame, and shows the launcher.
     *
     * @return true if initialization succeeds, otherwise false.
     */
    bool OnInit() override
    {
        wxInitAllImageHandlers();

        MainFrame *frame = new MainFrame();
        frame->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(BundlApp);