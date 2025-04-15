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
#include "wx/qt/private/utils.h"
#include "wx/qt/private/converter.h"
#include "wx/qt/private/winevent.h"

#include <QtWidgets/QMdiArea>
#include <QtWidgets/QMdiSubWindow>
#include <QtWidgets/QMainWindow>

// Main MDI window helper

class wxQtMDIParentFrame : public wxQtEventSignalHandler< QMainWindow, wxMDIParentFrame >
{
public:
    wxQtMDIParentFrame( wxWindow *parent, wxMDIParentFrame *handler );

private:
};

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
    (void)Create(parent, id, title, pos, size, style, name);
}

bool wxMDIParentFrame::Create(wxWindow *parent,
            wxWindowID id,
            const wxString& title,
            const wxPoint& pos,
            const wxSize& size,
            long style,
            const wxString& name)
{
    m_qtWindow = new wxQtMDIParentFrame( parent, this );

    if (!wxFrameBase::Create( parent, id, title, pos, size, style, name ))
        return false;

    wxMDIClientWindow *client = OnCreateClient();
    m_clientWindow = client;
    if ( !m_clientWindow->CreateClient(this, GetWindowStyleFlag()) )
        return false;

    GetQMainWindow()->setCentralWidget( client->GetHandle() );

    PostCreation();

    return true;
}

void wxMDIParentFrame::ActivateNext()
{
}

void wxMDIParentFrame::ActivatePrevious()
{
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

    return true;
}

void wxMDIChildFrame::Activate()
{
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

wxQtMDIParentFrame::wxQtMDIParentFrame( wxWindow *parent, wxMDIParentFrame *handler )
    : wxQtEventSignalHandler< QMainWindow, wxMDIParentFrame >( parent, handler )
{
}

wxQtMdiArea::wxQtMdiArea(wxWindow *parent, wxMDIClientWindow *handler )
    : wxQtEventSignalHandler< QMdiArea, wxMDIClientWindow >( parent, handler )
{
}

#endif // wxUSE_MDI
