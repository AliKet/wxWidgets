/////////////////////////////////////////////////////////////////////////////
// Name:        src/qt/uiaction.cpp
// Author:      Peter Most
// Copyright:   (c) Peter Most
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"


#if wxUSE_UIACTIONSIMULATOR

#include "wx/uiaction.h"
#include "wx/private/uiaction.h"

//#define QT_WIDGETS_LIB

#include <QtTest/QtTestGui> // define this -> QT_GUI_LIB
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#include <QtWidgets/QAbstractScrollArea>

#include "wx/qt/defs.h"
#include "wx/qt/private/utils.h"
#include "wx/qt/private/converter.h"


using namespace Qt;
using namespace QTest;

class wxUIActionSimulatorQtImpl : public wxUIActionSimulatorImpl
{
public:
    // Returns a pointer to the global simulator object: as it's stateless, we
    // can reuse the same one without having to allocate it on the heap all the
    // time.
    static wxUIActionSimulatorQtImpl* Get()
    {
        static wxUIActionSimulatorQtImpl s_impl;
        return &s_impl;
    }

    virtual bool MouseMove(long x, long y) override;
    virtual bool MouseDown(int button = wxMOUSE_BTN_LEFT) override;
    virtual bool MouseUp(int button = wxMOUSE_BTN_LEFT) override;

    virtual bool MouseClick(int button = wxMOUSE_BTN_LEFT) override;
    virtual bool MouseDblClick(int button = wxMOUSE_BTN_LEFT) override;

    virtual bool DoKey(int keycode, int modifiers, bool isDown) override;

private:
    // This class has no public ctors, use Get() instead.
    wxUIActionSimulatorQtImpl() { m_mousePosition = QCursor::pos(); }

    bool IsModifierKey(int keycode);

    int m_modifiers = wxMOD_NONE;
    QPoint m_mousePosition;

    wxDECLARE_NO_COPY_CLASS(wxUIActionSimulatorQtImpl);
};

static MouseButton ConvertMouseButton( int button )
{
    MouseButton qtButton;

    switch ( button )
    {
        case wxMOUSE_BTN_LEFT:
            qtButton = LeftButton;
            break;

        case wxMOUSE_BTN_MIDDLE:
            qtButton = MiddleButton;
            break;

        case wxMOUSE_BTN_RIGHT:
            qtButton = RightButton;
            break;

        case wxMOUSE_BTN_AUX1:
            qtButton = XButton1;
            break;

        case wxMOUSE_BTN_AUX2:
            qtButton = XButton2;
            break;

        default:
            wxFAIL_MSG( "Unsupported mouse button" );
            qtButton = NoButton;
            break;
    }
    return qtButton;
}

static Qt::KeyboardModifiers ConvertToQtModifiers(int modifiers)
{
    Qt::KeyboardModifiers qtmodifiers = Qt::NoModifier;

    if ( modifiers & wxMOD_SHIFT )
        qtmodifiers |= Qt::ShiftModifier;
    if ( modifiers & wxMOD_ALT )
        qtmodifiers |= Qt::AltModifier;
    if ( modifiers & wxMOD_CONTROL )
        qtmodifiers |= Qt::ControlModifier;

    return qtmodifiers;
}

static bool SimulateMouseButton( MouseAction mouseAction,
                                 MouseButton mouseButton,
                                 QPoint mousePosition,
                                 Qt::KeyboardModifiers modifiers = Qt::NoModifier )
{
    QWidget *widget = QApplication::widgetAt( mousePosition );

    if ( widget && !widget->windowHandle() )
    {
        widget = widget->nativeParentWidget();
    }

    if ( widget != nullptr )
    {
        QPoint pos = widget->mapFromGlobal(mousePosition);
        mouseEvent( mouseAction, widget->windowHandle(), mouseButton, modifiers, pos );
    }

    // If we found a widget then we successfully simulated an event:

    return widget != nullptr;
}

bool wxUIActionSimulatorQtImpl::IsModifierKey(int keycode)
{
    switch ( keycode )
    {
        case WXK_SHIFT:     m_modifiers |= wxMOD_SHIFT;     break;
        case WXK_ALT:       m_modifiers |= wxMOD_ALT;       break;
        case WXK_CONTROL:   m_modifiers |= wxMOD_CONTROL;   break;
        default:
            return false;
    }

    return true;
}

bool wxUIActionSimulatorQtImpl::MouseDown(int button)
{
    return SimulateMouseButton( MousePress, ConvertMouseButton( button ),
                                m_mousePosition, ConvertToQtModifiers( m_modifiers ) );
}

bool wxUIActionSimulatorQtImpl::MouseUp(int button)
{
    return SimulateMouseButton( MouseRelease, ConvertMouseButton( button ),
                                m_mousePosition, ConvertToQtModifiers( m_modifiers ) );
}

bool wxUIActionSimulatorQtImpl::MouseMove(long x, long y)
{
    m_mousePosition = QPoint( x, y );

    return SimulateMouseButton( QTest::MouseMove, NoButton, m_mousePosition );
}

bool wxUIActionSimulatorQtImpl::MouseClick(int button)
{
    return SimulateMouseButton( QTest::MouseClick, ConvertMouseButton( button ),
                                m_mousePosition, ConvertToQtModifiers( m_modifiers ) );
}

bool wxUIActionSimulatorQtImpl::MouseDblClick(int button)
{
    return SimulateMouseButton( QTest::MouseDClick, ConvertMouseButton( button ), m_mousePosition );
}

bool wxUIActionSimulatorQtImpl::DoKey(int keyCode, int modifiers, bool isDown)
{
    if ( IsModifierKey(keyCode) )
    {
        if ( !isDown )
            m_modifiers = wxMOD_NONE;

        return true;
    }

    QWidget *widget = QApplication::focusWidget();
    if ( !widget )
        widget = QApplication::activeWindow();
    if ( !widget )
        return false;

    Qt::KeyboardModifiers qtmodifiers;
    enum Key key;

    key = (enum Key) wxQtConvertKeyCode( keyCode, modifiers, qtmodifiers );

    wxCHECK_MSG(key, false, wxT("No current key conversion equivalent in Qt"));

    KeyAction keyAction = isDown ? Press : Release;
    keyEvent( keyAction, widget->windowHandle(), key, ConvertToQtModifiers(modifiers) );

    return true;
}

wxUIActionSimulator::wxUIActionSimulator()
                   : m_impl(wxUIActionSimulatorQtImpl::Get())
{
}

wxUIActionSimulator::~wxUIActionSimulator()
{
    // We can use a static wxUIActionSimulatorQtImpl object because it's
    // stateless, so no need to delete it.
}

#endif // wxUSE_UIACTIONSIMULATOR
