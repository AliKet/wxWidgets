/////////////////////////////////////////////////////////////////////////////
// Name:        wx/qt/palette.h
// Author:      Peter Most
// Copyright:   (c) Peter Most
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_QT_PALETTE_H_
#define _WX_QT_PALETTE_H_

#include "wx/defs.h"
#include "wx/object.h"
#include "wx/gdiobj.h"
#include "wx/gdicmn.h"

//-----------------------------------------------------------------------------
// classes
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_FWD_CORE wxPalette;

//-----------------------------------------------------------------------------
// wxPalette
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxPalette: public wxPaletteBase
{
public:
    wxPalette();
    wxPalette( int n, const unsigned char *red, const unsigned char *green, const unsigned char *blue );
    virtual ~wxPalette();

    bool Create( int n, const unsigned char *red, const unsigned char *green, const unsigned char *blue);
    int GetPixel( unsigned char red, unsigned char green, unsigned char blue ) const;
    bool GetRGB( int pixel, unsigned char *red, unsigned char *green, unsigned char *blue ) const;

    virtual int GetColoursCount() const override;

protected:
    virtual wxGDIRefData *CreateGDIRefData() const override;
    virtual wxGDIRefData *CloneGDIRefData(const wxGDIRefData *data) const override;

private:
    wxDECLARE_DYNAMIC_CLASS(wxPalette);
};

#endif // _WX_QT_PALETTE_H_
