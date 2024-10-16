///////////////////////////////////////////////////////////////////////////////
// Name:        tests/events/enterleave.cpp
// Purpose:     Test wxEVT_ENTER_WINDOW and wxEVT_LEAVE_WINDOW events
// Author:      Ali Kettab
// Created:     2024-10-16
// Copyright:   (c) 2024 wxWidgets team
///////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#include "testprec.h"


#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/button.h"
    #include "wx/panel.h"
    #include "wx/textctrl.h"
    #include "wx/window.h"
#endif // WX_PRECOMP

#include "wx/uiaction.h"

#include "asserthelper.h"
#include "testableframe.h"
#include "waitfor.h"

// ----------------------------------------------------------------------------
// tests themselves
// ----------------------------------------------------------------------------

#if wxUSE_UIACTIONSIMULATOR

TEST_CASE("EnterLeaveEvents", "[wxEvent][enter-leave]")
{
    std::unique_ptr<wxWindow> panel(new wxPanel(wxTheApp->GetTopWindow(), wxID_ANY));

    auto button = new wxButton(panel.get(), wxID_ANY, "button", {50, 50});
    auto textctrl = new wxTextCtrl(panel.get(), wxID_ANY, "", {160, 50});

    EventCounter enter(panel.get(), wxEVT_ENTER_WINDOW);
    EventCounter leave(panel.get(), wxEVT_LEAVE_WINDOW);

    // Wait for the first paint event to be sure
    // that window really has its final size.
    WaitForPaint waitForPaint(panel.get());
    panel->Show();
    waitForPaint.YieldUntilPainted();

    wxUIActionSimulator sim;

    sim.MouseMove(panel->GetScreenPosition() + wxPoint(5, 5));
    wxYield();

    CHECK( enter.GetCount() == 1 );
    CHECK( leave.GetCount() == 0 );

    enter.Clear();

    sim.MouseMove(button->GetScreenPosition() + wxPoint(10, 5));
    wxYield();

    CHECK( enter.GetCount() == 0 );
    CHECK( leave.GetCount() == 1 );

    leave.Clear();

    EventCounter clicked(button, wxEVT_LEFT_DOWN);

    sim.MouseDown();
//    wxYield();
    // At least under wxMSW calling wxYield() just once doesn't always work, so
    // try for a while.
    WaitFor("button to be clicked", [&]() { return clicked.GetCount() != 0; });

    sim.MouseMove(panel->GetScreenPosition() + wxPoint(5, 5));
    wxYield();

    CHECK( enter.GetCount() == 0 );
    CHECK( leave.GetCount() == 0 );

    sim.MouseMove(textctrl->GetScreenPosition() + wxPoint(10, 5));
    wxYield();

    CHECK( enter.GetCount() == 0 );
    CHECK( leave.GetCount() == 0 );

    sim.MouseMove(panel->GetScreenPosition() + wxPoint(5, 5));
    wxYield();

    CHECK( enter.GetCount() == 0 );
    CHECK( leave.GetCount() == 0 );

    sim.MouseUp();
    wxYield();

#ifndef __WXGTK__
    CHECK( enter.GetCount() == 1 );
#else
    CHECK( enter.GetCount() == 0 );
#endif
    CHECK( leave.GetCount() == 0 );
}

#endif // wxUSE_UIACTIONSIMULATOR
