/////////////////////////////////////////////////////////////////////////////
// Name:        wx/qt/mdi.h
// Author:      Peter Most, Mariano Reingart
// Copyright:   (c) 2010 wxWidgets dev team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_QT_MDI_H_
#define _WX_QT_MDI_H_

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

    // override/implement base class [pure] virtual methods
    // ----------------------------------------------------

    static bool IsTDI() { return false; }

    virtual void Cascade() override;
    virtual void Tile(wxOrientation orient = wxHORIZONTAL) override;
    virtual void ActivateNext() override;
    virtual void ActivatePrevious() override;

private:
    void OnMDICommand(wxCommandEvent& event);

    wxDECLARE_DYNAMIC_CLASS(wxMDIParentFrame);
};

class QMdiSubWindow;

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

    ~wxMDIChildFrame();

    bool Create(wxMDIParentFrame *parent,
                wxWindowID id,
                const wxString& title,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = wxDEFAULT_FRAME_STYLE,
                const wxString& name = wxASCII_STR(wxFrameNameStr));

    virtual void Activate() override;

    virtual void SetMenuBar(wxMenuBar* menubar) override;

    // This function is responsible of attaching this frame's menubar to m_mdiParent
    // when it becomes active and also restoring the m_mdiParent's menubar otherwise.
    void InternalSetMenuBar();

private:
    void UpdateWindowMenu(wxMenuBar* attachedMenuBar, wxMenuBar* detachedMenuBar);

    wxMenuBar* m_menuBar = nullptr;

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
