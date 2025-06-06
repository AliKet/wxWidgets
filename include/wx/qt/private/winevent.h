/////////////////////////////////////////////////////////////////////////////
// Name:        include/wx/qt/private/winevent.h
// Purpose:     QWidget to wxWindow event handler
// Author:      Javier Torres, Peter Most
// Created:     21.06.10
// Copyright:   (c) Javier Torres
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_QT_EVENTSIGNALFORWARDER_H_
#define _WX_QT_EVENTSIGNALFORWARDER_H_

#include <QtCore/QEvent>
#include <QtGui/QCloseEvent>

#include "wx/log.h"
#include "wx/window.h"
#include "wx/qt/private/converter.h"
#include "wx/qt/private/utils.h"

#include <QtWidgets/QGestureEvent>
#include <QtGui/QCursor>

// redeclare wxEVT_TEXT_ENTER here instead of including "wx/textctrl.h"
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_TEXT_ENTER, wxCommandEvent);

// The parameter of QWidget::enterEvent() is changed to QEnterEvent in Qt6
#if QT_VERSION_MAJOR >= 6
using wxQtEnterEvent = QEnterEvent;
#else
using wxQtEnterEvent = QEvent;
#endif

class QPaintEvent;


class wxQtSignalHandler
{
protected:
    explicit wxQtSignalHandler( wxWindow *handler ) : m_handler(handler)
    {
    }

    bool EmitEvent( wxEvent &event ) const
    {
        event.SetEventObject( m_handler );
        return m_handler->HandleWindowEvent( event );
    }

    virtual wxWindow *GetHandler() const
    {
        return m_handler;
    }

    // A hack for wxQtEventSignalHandler<>::keyPressEvent() handler for the
    // convenience of wxTextCtrl-like controls to emit the wxEVT_TEXT_ENTER
    // event if the control has wxTE_PROCESS_ENTER flag.
    bool HandleKeyPressEvent(QWidget* widget, QKeyEvent* e)
    {
        if ( m_handler->HasFlag(wxTE_PROCESS_ENTER) )
        {
            if ( e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter )
            {
                wxCommandEvent event( wxEVT_TEXT_ENTER, m_handler->GetId() );
                event.SetString( GetValueForProcessEnter() );
                event.SetEventObject( m_handler );
                return m_handler->HandleWindowEvent( event );
            }
        }

        return m_handler->QtHandleKeyEvent(widget, e);
    }

    // Controls supporting wxTE_PROCESS_ENTER flag (e.g. wxTextCtrl, wxComboBox and wxSpinCtrl)
    // should override this to return the control value as string when enter is pressed.
    virtual wxString GetValueForProcessEnter() { return wxString(); }

private:
    wxWindow* const m_handler;
};

template < typename Widget, typename Handler >
class wxQtEventSignalHandler : public Widget, public wxQtSignalHandler
{
public:
    wxQtEventSignalHandler( wxWindow *parent, Handler *handler )
        : Widget( parent != nullptr ? parent->GetHandle() : nullptr )
        , wxQtSignalHandler( handler )
    {
        // Set immediately as it is used to check if wxWindow is alive
        wxWindow::QtStoreWindowPointer( this, handler );

        Widget::setMouseTracking(true);
    }

    virtual Handler *GetHandler() const override
    {
        // Only process the signal / event if the wxWindow is not destroyed
        if ( !wxWindow::QtRetrieveWindowPointer( this ) )
        {
            return nullptr;
        }
        else
            return static_cast<Handler*>(wxQtSignalHandler::GetHandler());
    }

protected:
    /* Not implemented here: wxHelpEvent, wxIdleEvent wxJoystickEvent,
     * wxMouseCaptureLostEvent, wxMouseCaptureChangedEvent,
     * wxPowerEvent, wxScrollWinEvent, wxSysColourChangedEvent */

    //wxActivateEvent
    virtual void changeEvent ( QEvent * event ) override
    {
        if ( !this->GetHandler() )
            return;

        if ( !this->GetHandler()->QtHandleChangeEvent(this, event) )
            Widget::changeEvent(event);
        else
            event->accept();
    }

    //wxCloseEvent
    virtual void closeEvent ( QCloseEvent * event ) override
    {
        if ( !this->GetHandler() )
            return;

        if ( !this->GetHandler()->QtHandleCloseEvent(this, event) )
            Widget::closeEvent(event);
        else
            event->ignore();
    }

    //wxContextMenuEvent
    virtual void contextMenuEvent ( QContextMenuEvent * event ) override
    {
        if ( !this->GetHandler() )
            return;

        this->GetHandler()->QtHandleContextMenuEvent(this, event);

        // Notice that we are simply accepting the event and deliberately not
        // calling Widget::contextMenuEvent(event); here because the context menu
        // is supposed to be shown from a wxEVT_CONTEXT_MENU handler and not from
        // QWidget::contextMenuEvent() overrides (and we are already in one of
        // these overrides to perform QContextMenuEvent --> wxContextMenuEvent
        // translation).
        // More importantly, the default implementation of contextMenuEvent() simply
        // ignores the context event, which means that the event will be propagated
        // to the parent widget again which is undesirable here because the event may
        // have already been propagated at the wxWidgets level.

        event->accept();
    }

    //wxDropFilesEvent
    //virtual void dropEvent ( QDropEvent * event ) { }

    //wxFocusEvent.
    virtual void focusInEvent ( QFocusEvent * event ) override
    {
        if ( !this->GetHandler() )
            return;

        if ( !this->GetHandler()->QtHandleFocusEvent(this, event) )
            Widget::focusInEvent(event);
        else
            event->accept();
    }

    //wxFocusEvent.
    virtual void focusOutEvent ( QFocusEvent * event ) override
    {
        if ( !this->GetHandler() )
            return;

        if ( !this->GetHandler()->QtHandleFocusEvent(this, event) )
            Widget::focusOutEvent(event);
        else
            event->accept();
    }

    //wxShowEvent
    virtual void hideEvent ( QHideEvent * event ) override
    {
        if ( !this->GetHandler() )
            return;

        if ( !this->GetHandler()->QtHandleShowEvent(this, event) )
            Widget::hideEvent(event);
        else
            event->accept();
    }

    //wxKeyEvent
    virtual void keyPressEvent ( QKeyEvent * event ) override
    {
        if ( !this->GetHandler() )
            return;

        if ( !this->HandleKeyPressEvent(this, event) )
            Widget::keyPressEvent(event);
        else
            event->accept();
    }

    //wxKeyEvent
    virtual void keyReleaseEvent ( QKeyEvent * event ) override
    {
        if ( !this->GetHandler() )
            return;

        if ( !this->GetHandler()->QtHandleKeyEvent(this, event) )
            Widget::keyReleaseEvent(event);
        else
            event->accept();
    }

    //wxMouseEvent
    virtual void enterEvent ( wxQtEnterEvent * event ) override
    {
        if ( !this->GetHandler() )
            return;

        if ( !this->GetHandler()->QtHandleEnterEvent(this, event) )
            Widget::enterEvent(event);
        else
            event->accept();
    }

    //wxMouseEvent
    virtual void leaveEvent ( QEvent * event ) override
    {
        if ( !this->GetHandler() )
            return;

        if ( !this->GetHandler()->QtHandleEnterEvent(this, event) )
            Widget::leaveEvent(event);
        else
            event->accept();
    }

    //wxMouseEvent
    virtual void mouseDoubleClickEvent ( QMouseEvent * event ) override
    {
        if ( !this->GetHandler() )
            return;

        if ( !this->GetHandler()->QtHandleMouseEvent(this, event) )
            Widget::mouseDoubleClickEvent(event);
        else
            event->accept();
    }

    //wxMouseEvent
    virtual void mouseMoveEvent ( QMouseEvent * event ) override
    {
        if ( !this->GetHandler() )
            return;

        if ( !this->GetHandler()->QtHandleMouseEvent(this, event) )
            Widget::mouseMoveEvent(event);
        else
            event->accept();
    }

    //wxMouseEvent
    virtual void mousePressEvent ( QMouseEvent * event ) override
    {
        if ( !this->GetHandler() )
            return;

        if ( !this->GetHandler()->QtHandleMouseEvent(this, event) )
            Widget::mousePressEvent(event);
        else
            event->accept();
    }

    //wxMouseEvent
    virtual void mouseReleaseEvent ( QMouseEvent * event ) override
    {
        if ( !this->GetHandler() )
            return;

        if ( !this->GetHandler()->QtHandleMouseEvent(this, event) )
            Widget::mouseReleaseEvent(event);
        else
            event->accept();
    }

    //wxMoveEvent
    virtual void moveEvent ( QMoveEvent * event ) override
    {
        if ( !this->GetHandler() )
            return;

        if ( !this->GetHandler()->QtHandleMoveEvent(this, event) )
            Widget::moveEvent(event);
        else
            event->accept();
    }

    //wxEraseEvent then wxPaintEvent
    virtual void paintEvent ( QPaintEvent * event ) override
    {
        if ( !this->GetHandler() )
            return;

        if ( !this->GetHandler()->QtHandlePaintEvent(this, event) )
            Widget::paintEvent(event);
        else
            event->accept();
    }

    //wxSizeEvent
    virtual void resizeEvent ( QResizeEvent * event ) override
    {
        if ( !this->GetHandler() )
            return;

        if ( !this->GetHandler()->QtHandleResizeEvent(this, event) )
            Widget::resizeEvent(event);
        else
            event->accept();
    }

    //wxShowEvent
    virtual void showEvent ( QShowEvent * event ) override
    {
        if ( !this->GetHandler() )
            return;

        if ( !this->GetHandler()->QtHandleShowEvent(this, event) )
            Widget::showEvent(event);
        else
            event->accept();
    }

    //wxMouseEvent
    virtual void wheelEvent ( QWheelEvent * event ) override
    {
        if ( !this->GetHandler() )
            return;

        if ( !this->GetHandler()->QtHandleWheelEvent(this, event) )
            Widget::wheelEvent(event);
        else
            event->accept();
    }

    /* Unused Qt events
    virtual void actionEvent ( QActionEvent * event ) { }
    virtual void dragEnterEvent ( QDragEnterEvent * event ) { }
    virtual void dragLeaveEvent ( QDragLeaveEvent * event ) { }
    virtual void dragMoveEvent ( QDragMoveEvent * event ) { }
    virtual void inputMethodEvent ( QInputMethodEvent * event ) { }
    virtual bool macEvent ( EventHandlerCallRef caller, EventRef event ) { }
    virtual bool qwsEvent ( QWSEvent * event ) { }
    virtual void tabletEvent ( QTabletEvent * event ) { }
    virtual bool winEvent ( MSG * message, long * result ) { }
    virtual bool x11Event ( XEvent * event ) { } */

    virtual bool event(QEvent *event) override
    {
        switch (event->type())
        {
            case QEvent::Gesture:
                return gestureEvent(static_cast<QGestureEvent*>(event), event);

            case QEvent::TouchBegin:
            case QEvent::TouchUpdate:
            case QEvent::TouchCancel:
            case QEvent::TouchEnd:
                return touchEvent(static_cast<QTouchEvent*>(event));
            default:;
        }

        return Widget::event(event);
    }

    bool touchEvent(QTouchEvent *touch)
    {
        bool handled = false;

        if ( wxWindow *win = wxWindow::QtRetrieveWindowPointer(this) )
        {
#if QT_VERSION_MAJOR >= 6
            for (const auto& tp : touch->points())
#else
            for (const auto& tp : touch->touchPoints())
#endif
            {
                wxEventType evtype = wxEVT_NULL;

                switch (tp.state())
                {
                    case Qt::TouchPointPressed:
                        evtype = wxEVT_TOUCH_BEGIN;
                        break;

                    case Qt::TouchPointMoved:
                        evtype = wxEVT_TOUCH_MOVE;
                        break;
                    case Qt::TouchPointReleased:
                        evtype = wxEVT_TOUCH_END;
                        break;

                    default:
                        continue;
                }

                wxMultiTouchEvent evt(win->GetId(), evtype);

                // Use screen position as the event might originate from a different
                // Qt window than this one.
#if QT_VERSION_MAJOR >= 6
                const auto screenPos = tp.globalPosition();
#else
                const auto screenPos = tp.screenPos();
#endif
                wxPoint2DDouble pt = wxQtConvertPointF(screenPos.toPoint());
                wxPoint ref = pt.GetFloor();

                evt.SetPosition(win->ScreenToClient(ref) + (pt - ref));
                evt.SetSequenceId(wxTouchSequenceId(wxUIntToPtr((unsigned)tp.id())));
                // Qt doesn't provide the primary point flag

                handled |= win->ProcessWindowEvent(evt);
            }
        }

        return handled;
    }

    bool gestureEvent(QGestureEvent *gesture, QEvent *event)
    {
        if (QGesture *tah = gesture->gesture(Qt::TapAndHoldGesture))
        {
            //  Set the policy so that accepted gestures are taken by the first window that gets them
            tah->setGestureCancelPolicy ( QGesture::CancelAllInContext );
            tapandholdTriggered(static_cast<QTapAndHoldGesture *>(tah), event);
        }

        if (QGesture *pan = gesture->gesture(Qt::PanGesture))
        {
            panTriggered(static_cast<QPanGesture *>(pan), event);
        }

        if (QGesture *pinch = gesture->gesture(Qt::PinchGesture))
        {
            pinchTriggered(static_cast<QPinchGesture *>(pinch), event);
        }

        return true;
    }

    void tapandholdTriggered(QTapAndHoldGesture *gesture, QEvent *event)
    {
        if ( wxWindow *win = wxWindow::QtRetrieveWindowPointer( this ) )
        {
            if (gesture->state() == Qt::GestureFinished)
            {
                wxLongPressEvent ev(win->GetId());
                ev.SetPosition( wxQtConvertPoint( gesture->position().toPoint() ) );

                ev.SetGestureEnd();
                win->ProcessWindowEvent( ev );
            }
            event->accept();
        }
    }

    void panTriggered(QPanGesture *gesture, QEvent *event)
    {
        wxWindow *win = wxWindow::QtRetrieveWindowPointer( this );

        if (win)
        {
            wxPanGestureEvent evp(win->GetId());
            QPoint pos = QCursor::pos();
            evp.SetPosition( wxQtConvertPoint( pos ) );
            evp.SetDelta( wxQtConvertPoint( gesture->delta().toPoint() ) );

            switch(gesture->state())
            {
                case Qt::GestureStarted:
                    evp.SetGestureStart();
                    break;
                case Qt::GestureFinished:
                case Qt::GestureCanceled:
                    evp.SetGestureEnd();
                    break;
                default:
                    break;
            }

            win->ProcessWindowEvent( evp );

            event->accept();
        }
    }

    void pinchTriggered(QPinchGesture *gesture, QEvent *event)
    {
        wxWindow *win = wxWindow::QtRetrieveWindowPointer( this );
        if (win)
        {
            if (gesture->changeFlags() & QPinchGesture::ScaleFactorChanged)
            {
                wxZoomGestureEvent evp(win->GetId());
                evp.SetPosition(wxQtConvertPoint(gesture->centerPoint().toPoint()));
                evp.SetZoomFactor(gesture->totalScaleFactor());

                switch (gesture->state())
                {
                case Qt::GestureStarted:
                    evp.SetGestureStart();
                    break;
                case Qt::GestureFinished:
                case Qt::GestureCanceled:
                    evp.SetGestureEnd();
                    break;
                default:
                    break;
                }

                win->ProcessWindowEvent(evp);
            }

            if (gesture->changeFlags() & QPinchGesture::RotationAngleChanged)
            {
                wxRotateGestureEvent evp(win->GetId());
                evp.SetPosition(wxQtConvertPoint(gesture->centerPoint().toPoint()));
                evp.SetRotationAngle(wxDegToRad(gesture->totalRotationAngle()));

                switch (gesture->state())
                {
                case Qt::GestureStarted:
                    evp.SetGestureStart();
                    break;
                case Qt::GestureFinished:
                case Qt::GestureCanceled:
                    evp.SetGestureEnd();
                    break;
                default:
                    break;
                }

                win->ProcessWindowEvent(evp);
            }

            event->accept();
        }
    }
};

// RAII wrapper for blockSignals(). It blocks signals in its constructor and in
// the destructor it restores the state to what it was before the constructor ran.
class wxQtEnsureSignalsBlocked
{
public:
    // Use QObject instead of QWidget to avoid including <QWidget> from here.
    wxQtEnsureSignalsBlocked(QObject *widget) :
        m_widget(widget)
    {
        m_restore = m_widget->blockSignals(true);
    }

    ~wxQtEnsureSignalsBlocked()
    {
        m_widget->blockSignals(m_restore);
    }

private:
    QObject* const m_widget;
    bool m_restore;

    wxDECLARE_NO_COPY_CLASS(wxQtEnsureSignalsBlocked);
};

#endif
