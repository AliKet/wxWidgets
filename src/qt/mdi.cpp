/////////////////////////////////////////////////////////////////////////////
// Name:        src/qt/mdi.cpp
// Author:      Mariano Reingart, Peter Most
// Copyright:   (c) 2010 wxWidgets dev team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#if wxUSE_MDI

#include "wx/mdi.h"
#include "wx/stockitem.h"
#include "wx/qt/private/utils.h"
#include "wx/qt/private/converter.h"
#include "wx/qt/private/winevent.h"

#include <QtWidgets/QMdiArea>
#include <QtWidgets/QMdiSubWindow>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>

namespace
{
static QMdiSubWindow* gs_qtActiveSubWindow = nullptr;
}

// Central widget helper (container to show scroll bars and receive events):

class wxQtMdiArea : public wxQtEventSignalHandler< QMdiArea, wxMDIClientWindow >
{
    public:
        wxQtMdiArea( wxWindow *parent, wxMDIClientWindow *handler );
};

wxIMPLEMENT_DYNAMIC_CLASS(wxMDIParentFrame, wxFrame);

wxMDIParentFrame::wxMDIParentFrame(wxWindow *parent,
                 wxWindowID id,
                 const wxString& title,
                 const wxPoint& pos,
                 const wxSize& size,
                 long style,
                 const wxString& name)
{
    Create(parent, id, title, pos, size, style, name);
}

wxMDIParentFrame::~wxMDIParentFrame()
{
    // m_windowMenu will be deleted by this frame's menubar, so
    // avoid a double deletion in the base class dtor.
    m_windowMenu = nullptr;

    delete m_clientWindow;
}

bool wxMDIParentFrame::Create(wxWindow *parent,
            wxWindowID id,
            const wxString& title,
            const wxPoint& pos,
            const wxSize& size,
            long style,
            const wxString& name)
{
    if ( !wxFrame::Create(parent, id, title, pos, size, style, name) )
        return false;

    wxMDIClientWindow* client = OnCreateClient();
    m_clientWindow = client;
    if ( !m_clientWindow->CreateClient(this, GetWindowStyleFlag()) )
        return false;

    // Get rid of the central widget set in wxFrame and set our own widget.
    GetQMainWindow()->setCentralWidget(client->GetHandle());

    // this style can be used to prevent a window from having the standard MDI
    // "Window" menu
    if ( !(style & wxFRAME_NO_WINDOW_MENU) )
    {
        // normal case: we have the window menu, so construct it
        m_windowMenu = new wxMenu;

        // Qt offers only "Tile" without specifying any direction, so just
        // reuse one of the predifined ids.
        const int wxID_MDI_WINDOW_TILE = wxID_MDI_WINDOW_TILE_HORZ;

        m_windowMenu->Append(wxID_MDI_WINDOW_CASCADE, _("&Cascade"));
        m_windowMenu->Append(wxID_MDI_WINDOW_TILE, _("&Tile"));
        m_windowMenu->AppendSeparator();
        m_windowMenu->Append(wxID_MDI_WINDOW_NEXT, _("&Next"));
        m_windowMenu->Append(wxID_MDI_WINDOW_PREV, _("&Previous"));
    }

    Bind(wxEVT_MENU, &wxMDIParentFrame::OnMDICommand, this, wxID_MDI_WINDOW_FIRST, wxID_MDI_WINDOW_LAST);

    return true;
}

void wxMDIParentFrame::Cascade()
{
    static_cast<QMdiArea*>(m_clientWindow->GetHandle())->cascadeSubWindows();
}

void wxMDIParentFrame::Tile(wxOrientation WXUNUSED(orient))
{
    static_cast<QMdiArea*>(m_clientWindow->GetHandle())->tileSubWindows();
}

void wxMDIParentFrame::ActivateNext()
{
    static_cast<QMdiArea*>(m_clientWindow->GetHandle())->activateNextSubWindow();
}

void wxMDIParentFrame::ActivatePrevious()
{
    static_cast<QMdiArea*>(m_clientWindow->GetHandle())->activatePreviousSubWindow();
}

void wxMDIParentFrame::OnMDICommand(wxCommandEvent& event)
{
    switch (event.GetId())
    {
    case wxID_MDI_WINDOW_CASCADE:
        Cascade();
        break;

    case wxID_MDI_WINDOW_TILE_HORZ:
        wxFALLTHROUGH;
    case wxID_MDI_WINDOW_TILE_VERT:
        Tile();
        break;

    case wxID_MDI_WINDOW_NEXT:
        ActivateNext();
        break;

    case wxID_MDI_WINDOW_PREV:
        ActivatePrevious();
        break;

    default:
        wxFAIL_MSG("unknown MDI command");
        break;
    }
}

//##############################################################################

wxIMPLEMENT_DYNAMIC_CLASS(wxMDIChildFrame, wxMDIChildFrameBase)

wxMDIChildFrame::wxMDIChildFrame(wxMDIParentFrame *parent,
                wxWindowID id,
                const wxString& title,
                const wxPoint& pos,
                const wxSize& size,
                long style,
                const wxString& name)
{
    Create(parent, id, title, pos, size, style, name);
}

wxMDIChildFrame::~wxMDIChildFrame()
{
    if ( gs_qtActiveSubWindow == m_qtSubWindow )
    {
        gs_qtActiveSubWindow = nullptr;
    }

    delete m_menuBar;
}

bool wxMDIChildFrame::Create(wxMDIParentFrame *parent,
            wxWindowID id,
            const wxString& title,
            const wxPoint& pos,
            const wxSize& size,
            long style,
            const wxString& name)
{
    m_mdiParent = parent;

    if ( !wxFrameBase::Create(parent->GetClientWindow(), id,
                              title, pos, size, style, name) )
        return false;

    // Add the window to the internal MDI client area:
    auto qtMdiArea = static_cast<QMdiArea*>(GetParent()->GetHandle());
    m_qtSubWindow  = qtMdiArea->addSubWindow(GetHandle());

    if ( size != wxDefaultSize )
    {
        m_qtSubWindow->setMinimumSize(wxQtConvertSize(size));
    }

    QObject::connect(m_qtSubWindow, &QMdiSubWindow::aboutToActivate, [this, qtMdiArea]()
        {
            // m_qtSubWindow will be the active window, so we need to restore m_mdiParent's
            // menubar stored in the previouse active one gs_qtActiveSubWindow.
            if ( m_qtSubWindow != gs_qtActiveSubWindow && gs_qtActiveSubWindow )
            {
                auto win = wxWindowQt::QtRetrieveWindowPointer(gs_qtActiveSubWindow->widget());
                static_cast<wxMDIChildFrame*>(win)->InternalSetMenuBar();
            }
        });

    QObject::connect(qtMdiArea, &QMdiArea::subWindowActivated, [this, qtMdiArea](QMdiSubWindow* window)
        {
            QWidget* qtWidget = nullptr;

            if ( window )
            {
                if ( window != gs_qtActiveSubWindow )
                {
                    // window is the active window and it's menubar will be attached to m_mdiParent
                    // with InternalSetMenuBar() call below.
                    qtWidget = window->widget();
                }
            }
            else if ( gs_qtActiveSubWindow )
            {
                // the old active window "gs_qtActiveSubWindow" should restore the m_mdiParent's
                // menubar with InternalSetMenuBar() call below.
                qtWidget = gs_qtActiveSubWindow->widget();
            }

            if ( qtWidget )
            {
                auto win = wxWindowQt::QtRetrieveWindowPointer(qtWidget);
                static_cast<wxMDIChildFrame*>(win)->InternalSetMenuBar();
            }

            gs_qtActiveSubWindow = window;
        });

    return true;
}

void wxMDIChildFrame::SetWindowStyleFlag(long style)
{
    wxWindow::SetWindowStyleFlag(style);
}

void wxMDIChildFrame::Activate()
{
    auto qtMdiArea = static_cast<QMdiArea*>(GetParent()->GetHandle());

    qtMdiArea->setActiveSubWindow(m_qtSubWindow);
}

void wxMDIChildFrame::SetMenuBar(wxMenuBar* menuBar)
{
    m_menuBar = menuBar;

    // Don't call wxFrameBase::SetMenuBar() here because m_menuBar will be
    // attached to m_mdiParent later when this child frame becomes active.
}

void wxMDIChildFrame::InternalSetMenuBar()
{
    wxCHECK_RET(m_mdiParent, "Invalid MDI parent window");

    auto oldMenuBar = m_mdiParent->GetMenuBar();

    if ( m_menuBar != oldMenuBar )
    {
        m_mdiParent->SetMenuBar(m_menuBar);

        UpdateWindowMenu(m_menuBar, oldMenuBar);

        m_menuBar->Show(); // Show the attached menubar
        m_menuBar = oldMenuBar;
        m_menuBar->Hide(); // Hide the detached menubar
    }
}

void wxMDIChildFrame::UpdateWindowMenu(wxMenuBar* attachedMenuBar,
                                       wxMenuBar* detachedMenuBar)
{
    auto windowMenu = m_mdiParent->GetWindowMenu();

    if ( !windowMenu )
        return;

    const wxString windowMenuTitle = _("&Window");

    if ( detachedMenuBar )
    {
        auto pos = detachedMenuBar->FindMenu(windowMenuTitle);
        if ( pos != wxNOT_FOUND )
        {
            detachedMenuBar->Remove(pos);
        }
    }

    if ( attachedMenuBar )
    {

        auto pos = attachedMenuBar->FindMenu(wxGetStockLabel(wxID_HELP));
        if ( pos != wxNOT_FOUND )
        {
            attachedMenuBar->Insert(pos, windowMenu, windowMenuTitle);
        }
    }
}

//##############################################################################

wxIMPLEMENT_DYNAMIC_CLASS(wxMDIClientWindow,wxMDIClientWindowBase)

bool wxMDIClientWindow::CreateClient(wxMDIParentFrame *parent, long WXUNUSED(style))
{
    // create the MDI client area where the children window are displayed:
    m_qtWindow = new wxQtMdiArea( parent, this );
    m_qtContainer = dynamic_cast<QAbstractScrollArea*>(m_qtWindow);
    return true;
}

// Helper implementation:

wxQtMdiArea::wxQtMdiArea(wxWindow *parent, wxMDIClientWindow *handler )
    : wxQtEventSignalHandler< QMdiArea, wxMDIClientWindow >( parent, handler )
{
}

#endif // wxUSE_MDI
