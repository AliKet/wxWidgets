/////////////////////////////////////////////////////////////////////////////
// Name:        wx/qt/mdi.h
// Author:      Peter Most, Mariano Reingart
// Copyright:   (c) 2010 wxWidgets dev team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_QT_MDI_H_
#define _WX_QT_MDI_H_

class QMdiArea;
class QMdiSubWindow;

class WXDLLIMPEXP_CORE wxMDIParentFrame : public wxMDIParentFrameBase
{
public:
    wxMDIParentFrame() = default;

    wxMDIParentFrame(wxWindow *parent,
                     wxWindowID id,
                     const wxString& title,
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& size = wxDefaultSize,
                     long style = wxDEFAULT_FRAME_STYLE | wxVSCROLL | wxHSCROLL,
                     const wxString& name = wxASCII_STR(wxFrameNameStr));

    ~wxMDIParentFrame();

    bool Create(wxWindow *parent,
                wxWindowID id,
                const wxString& title,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = wxDEFAULT_FRAME_STYLE | wxVSCROLL | wxHSCROLL,
                const wxString& name = wxASCII_STR(wxFrameNameStr));

    QMdiArea* GetQtMdiArea() const;

    // override/implement base class [pure] virtual methods
    // ----------------------------------------------------

    static bool IsTDI() { return false; }

    virtual void ActivateNext() override;
    virtual void ActivatePrevious() override;

private:
    wxDECLARE_DYNAMIC_CLASS(wxMDIParentFrame);
};



class WXDLLIMPEXP_CORE wxMDIChildFrame : public wxMDIChildFrameBase
{
public:
    wxMDIChildFrame() = default;

    wxMDIChildFrame(wxMDIParentFrame *parent,
                    wxWindowID id,
                    const wxString& title,
                    const wxPoint& pos = wxDefaultPosition,
                    const wxSize& size = wxDefaultSize,
                    long style = wxDEFAULT_FRAME_STYLE,
                    const wxString& name = wxASCII_STR(wxFrameNameStr));

    bool Create(wxMDIParentFrame *parent,
                wxWindowID id,
                const wxString& title,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = wxDEFAULT_FRAME_STYLE,
                const wxString& name = wxASCII_STR(wxFrameNameStr));

    virtual void Activate() override;

    // wxMDIChildFrame doesn't have toolbar nor statusbar
    // --------------------------------------------------

#if wxUSE_STATUSBAR
    virtual void SetStatusBar(wxStatusBar* WXUNUSED(statusBar)) override {}
#endif // wxUSE_STATUSBAR

#if wxUSE_TOOLBAR
    virtual void SetToolBar(wxToolBar* WXUNUSED(toolbar)) override {}
#endif // wxUSE_TOOLBAR

    virtual void SetWindowStyleFlag( long style ) override;

protected:
    virtual wxPoint GetClientAreaOrigin() const override
    {
        return wxWindow::GetClientAreaOrigin();
    }

    virtual QWidget* QtGetParentWidget() const override { return GetHandle(); }

private:

    QMdiSubWindow* m_qtSubWindow = nullptr;

    wxDECLARE_DYNAMIC_CLASS(wxMDIChildFrame);
};



class WXDLLIMPEXP_CORE wxMDIClientWindow : public wxMDIClientWindowBase
{
public:
    wxMDIClientWindow() = default;

    virtual bool CreateClient(wxMDIParentFrame *parent, long style = wxVSCROLL | wxHSCROLL) override;

    wxDECLARE_DYNAMIC_CLASS(wxMDIClientWindow);
};

#endif // _WX_QT_MDI_H_
