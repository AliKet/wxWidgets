/////////////////////////////////////////////////////////////////////////////
// Name:        src/qt/bitmap.cpp
// Author:      Peter Most, Javier Torres, Mariano Reingart, Sean D'Epagnier
// Copyright:   (c) 2010 wxWidgets dev team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"


#include <QtGui/QPixmap>
#include <QtGui/QBitmap>
#include <QtWidgets/QLabel>

#ifndef WX_PRECOMP
    #include "wx/dc.h"
    #include "wx/icon.h"
    #include "wx/image.h"
#endif // WX_PRECOMP

#include "wx/bitmap.h"
#include "wx/cursor.h"
#include "wx/rawbmp.h"
#include "wx/qt/colour.h"
#include "wx/qt/private/converter.h"
#include "wx/qt/private/utils.h"


static wxImage ConvertImage( QImage qtImage )
{
    const bool hasAlpha = qtImage.hasAlphaChannel();

    const int numPixels = qtImage.height() * qtImage.width();

    // For monochrome bitmaps, we just convert to RGB32 so we don't have to do any
    // bit twiddling to get at pixels and the same code below should work for any
    // image format we support.
    if ( qtImage.depth() == 1 )
        qtImage = qtImage.convertToFormat(QImage::Format_RGB32);

    unsigned char *data = (unsigned char *)malloc(sizeof(char) * 3 * numPixels);
    unsigned char *startData = data;

    unsigned char *alpha = nullptr;
    if (hasAlpha)
        alpha = (unsigned char *)malloc(sizeof(char) * numPixels);

    unsigned char *startAlpha = alpha;

    for (int y = 0; y < qtImage.height(); ++y)
    {
        QRgb *line = (QRgb*)qtImage.scanLine(y);

        for (int x = 0; x < qtImage.width(); ++x)
        {
            QRgb colour = line[x];

            if (hasAlpha)
            {
                colour = qUnpremultiply(colour);
                alpha[0] = qAlpha(colour);
                alpha++;
            }

            data[0] = qRed(colour);
            data[1] = qGreen(colour);
            data[2] = qBlue(colour);

            data += 3;
        }
    }
    if (hasAlpha)
        return wxImage(wxQtConvertSize(qtImage.size()), startData, startAlpha);
    else
        return wxImage(wxQtConvertSize(qtImage.size()), startData);
}

static QImage ConvertImage( const wxImage &image, wxMask** mask = nullptr  )
{
    const bool hasAlpha = image.HasAlpha();
    const bool hasMask = image.HasMask() && mask;
    QImage qtImage( wxQtConvertSize( image.GetSize() ),
                   ( hasAlpha ? QImage::Format_ARGB32_Premultiplied : QImage::Format_RGB32 ) );

    unsigned char *data = image.GetData();
    unsigned char *alpha = hasAlpha ? image.GetAlpha() : nullptr;
    QRgb colour;

    QImage qtMask;
    QRgb maskedColour;
    if ( hasMask )
    {
        unsigned char r, g, b;
        image.GetOrFindMaskColour( &r, &g, &b );
        maskedColour = qRgb(r, g, b);

        qtMask = QImage(image.GetWidth(), image.GetHeight(), QImage::Format_Mono);
        qtMask.fill(Qt::color0); // filled with 0s
    }

    for (int y = 0; y < image.GetHeight(); ++y)
    {
        for (int x = 0; x < image.GetWidth(); ++x)
        {
            const unsigned char a = hasAlpha ? alpha[0] : 255;
            const unsigned char r = data[0];
            const unsigned char g = data[1];
            const unsigned char b = data[2];

            colour = (a << 24) + (r << 16) + (g << 8) + b;

            if ( !qtMask.isNull() )
            {
                if ( qRgb(r, g, b) != maskedColour )
                    qtMask.setPixel(x, y, Qt::color1); // set to 1
            }

            if (hasAlpha)
            {
                colour = qPremultiply(colour);
                alpha++;
            }

            qtImage.setPixel(x, y, colour);

            data += 3;
        }
    }

    if ( hasMask )
    {
        *mask = new wxMask(new QBitmap{QBitmap::fromImage(qtMask)});
    }

    return qtImage;
}

//-----------------------------------------------------------------------------
// wxBitmapRefData
//-----------------------------------------------------------------------------

class wxBitmapRefData: public wxGDIRefData
{
    public:
        wxBitmapRefData() { m_mask = nullptr; }

        wxBitmapRefData( int width, int height, int depth )
        {
            if (depth == 1)
                m_qtPixmap = QBitmap( width, height );
            else
                m_qtPixmap = QPixmap( width, height );
            m_mask = nullptr;
        }

        wxBitmapRefData( QPixmap pix, wxMask* mask = nullptr )
            : m_qtPixmap(pix)
        {
            m_mask = mask;
        }

        virtual ~wxBitmapRefData() { delete m_mask; }

        QPixmap m_qtPixmap;
        QImage m_rawPixelSource;
        wxMask *m_mask;

private:
    wxBitmapRefData(const wxBitmapRefData&other);
    wxBitmapRefData& operator=(const wxBitmapRefData&other);
};

//-----------------------------------------------------------------------------
// wxBitmap
//-----------------------------------------------------------------------------

wxIMPLEMENT_DYNAMIC_CLASS(wxBitmap, wxObject);

#define M_PIXDATA ((wxBitmapRefData *)m_refData)->m_qtPixmap
#define M_MASK ((wxBitmapRefData *)m_refData)->m_mask

void wxBitmap::InitStandardHandlers()
{
}

wxBitmap::wxBitmap()
{
}

wxBitmap::wxBitmap(QPixmap pix)
{
    m_refData = new wxBitmapRefData(pix);
}

wxBitmap::wxBitmap(const char bits[], int width, int height, int depth )
{
    wxASSERT(depth == 1);

    if (width > 0 && height > 0 && depth == 1)
    {
        m_refData = new wxBitmapRefData();
        M_PIXDATA = QBitmap(QBitmap::fromData(QSize(width, height), (const uchar*)bits));
    }
}

wxBitmap::wxBitmap(int width, int height, int depth)
{
    Create(width, height, depth);
}

wxBitmap::wxBitmap(const wxSize& sz, int depth )
{
    Create(sz, depth);
}

wxBitmap::wxBitmap(int width, int height, const wxDC& dc)
{
    Create(width, height, dc);
}

// Create a wxBitmap from xpm data
wxBitmap::wxBitmap(const char* const* bits)
{
    m_refData = new wxBitmapRefData();
    M_PIXDATA = QPixmap( bits );
}

wxBitmap::wxBitmap(const wxString &filename, wxBitmapType type )
{
    LoadFile(filename, type);
}

void wxBitmap::InitFromImage(const wxImage& image, int depth, double WXUNUSED(scale) )
{
    wxMask* mask = nullptr;
    auto qtImage = depth == 1
                 ? QBitmap::fromImage(ConvertImage(image), Qt::ThresholdDither)
                 : QPixmap::fromImage(ConvertImage(image, &mask));

    m_refData = new wxBitmapRefData(qtImage, mask);
}

wxBitmap::wxBitmap(const wxImage& image, int depth, double scale)
{
    InitFromImage(image, depth, scale);
}

wxBitmap::wxBitmap(const wxImage& image, const wxDC& dc)
{
    InitFromImage(image, -1, dc.GetContentScaleFactor());
}

wxBitmap::wxBitmap(const wxCursor& cursor)
{
    // note that pixmap could be invalid if is not a pixmap cursor
    QPixmap pix = cursor.GetHandle().pixmap();
    m_refData = new wxBitmapRefData(pix);
}

bool wxBitmap::Create(int width, int height, int depth )
{
    UnRef();
    m_refData = new wxBitmapRefData(width, height, depth);

    return true;
}

bool wxBitmap::Create(const wxSize& sz, int depth )
{
    return Create(sz.GetWidth(), sz.GetHeight(), depth);
}

bool wxBitmap::Create(int width, int height, const wxDC& WXUNUSED(dc))
{
    return Create(width, height);
}

int wxBitmap::GetHeight() const
{
    return M_PIXDATA.height();
}

int wxBitmap::GetWidth() const
{
    return M_PIXDATA.width();
}

int wxBitmap::GetDepth() const
{
    return M_PIXDATA.depth();
}


#if wxUSE_IMAGE
wxImage wxBitmap::ConvertToImage() const
{
    wxImage image;

    wxCHECK_MSG(IsOk(), image, "invalid bitmap");

    image = ConvertImage(M_PIXDATA.toImage());

    // now handle the mask, if we have any
    if ( M_MASK && M_MASK->GetHandle() )
    {
        // we hard code the mask colour for now but we could also make an
        // effort (and waste time) to choose a colour not present in the
        // image already to avoid having to fudge the pixels below --
        // whether it's worth to do it is unclear however
        static const int MASK_RED = 1;
        static const int MASK_GREEN = 2;
        static const int MASK_BLUE = 3;
        static const int MASK_BLUE_REPLACEMENT = 2;

        wxBitmap bmpMask(M_MASK->GetBitmap());
        wxMonoPixelData dataMask(bmpMask);
        const int h = dataMask.GetHeight();
        const int w = dataMask.GetWidth();
        unsigned char* data = image.GetData();

        wxMonoPixelData::Iterator rowStart(dataMask);
        for ( int y = 0; y < h; ++y )
        {
            // traverse one mask line
            wxMonoPixelData::Iterator p = rowStart;
            for ( int x = 0; x < w; ++x )
            {
                // should this pixel be transparent?
                if ( p.Pixel() )
                {
                    // no, check that it isn't transparent by accident
                    if ( (data[0] == MASK_RED) &&
                            (data[1] == MASK_GREEN) &&
                                (data[2] == MASK_BLUE) )
                    {
                        // we have to fudge the colour a bit to prevent
                        // this pixel from appearing transparent
                        data[2] = MASK_BLUE_REPLACEMENT;
                    }

                    data += 3;
                }
                else // yes, transparent pixel
                {
                    *data++ = MASK_RED;
                    *data++ = MASK_GREEN;
                    *data++ = MASK_BLUE;
                }
                ++p;
            }
            rowStart.OffsetY(dataMask, 1);
        }

        image.SetMaskColour(MASK_RED, MASK_GREEN, MASK_BLUE);
    }

    return image;
}

#endif // wxUSE_IMAGE

wxMask *wxBitmap::GetMask() const
{
    return M_MASK;
}

void wxBitmap::SetMask(wxMask *mask)
{
    AllocExclusive();
    delete M_MASK;
    M_MASK = mask;
}

wxBitmap wxBitmap::GetSubBitmap(const wxRect& rect) const
{
    wxBitmap bmp = wxBitmap(M_PIXDATA.copy(wxQtConvertRect(rect)));

    if ( M_MASK && M_MASK->GetHandle() )
    {
        QBitmap* qtMask = M_MASK->GetHandle();
        bmp.SetMask(new wxMask(new QBitmap{qtMask->copy(wxQtConvertRect(rect))}));
    }

    return bmp;
}


bool wxBitmap::SaveFile(const wxString &name, wxBitmapType type,
              const wxPalette *WXUNUSED(palette) ) const
{
    wxCHECK_MSG( IsOk(), false, "invalid bitmap" );

    //Try to save using Qt
    const char* type_name = nullptr;
    switch (type)
    {
        case wxBITMAP_TYPE_BMP:  type_name = "bmp";  break;
        case wxBITMAP_TYPE_ICO:  type_name = "ico";  break;
        case wxBITMAP_TYPE_JPEG: type_name = "jpeg"; break;
        case wxBITMAP_TYPE_PNG:  type_name = "png";  break;
        case wxBITMAP_TYPE_GIF:  type_name = "gif";  break;
        case wxBITMAP_TYPE_CUR:  type_name = "cur";  break;
        case wxBITMAP_TYPE_TIFF: type_name = "tif";  break;
        case wxBITMAP_TYPE_XBM:  type_name = "xbm";  break;
        case wxBITMAP_TYPE_PCX:  type_name = "pcx";  break;
        case wxBITMAP_TYPE_BMP_RESOURCE:
        case wxBITMAP_TYPE_ICO_RESOURCE:
        case wxBITMAP_TYPE_CUR_RESOURCE:
        case wxBITMAP_TYPE_XBM_DATA:
        case wxBITMAP_TYPE_XPM:
        case wxBITMAP_TYPE_XPM_DATA:
        case wxBITMAP_TYPE_TIFF_RESOURCE:
        case wxBITMAP_TYPE_GIF_RESOURCE:
        case wxBITMAP_TYPE_PNG_RESOURCE:
        case wxBITMAP_TYPE_JPEG_RESOURCE:
        case wxBITMAP_TYPE_PNM:
        case wxBITMAP_TYPE_PNM_RESOURCE:
        case wxBITMAP_TYPE_PCX_RESOURCE:
        case wxBITMAP_TYPE_PICT:
        case wxBITMAP_TYPE_PICT_RESOURCE:
        case wxBITMAP_TYPE_ICON:
        case wxBITMAP_TYPE_ICON_RESOURCE:
        case wxBITMAP_TYPE_ANI:
        case wxBITMAP_TYPE_IFF:
        case wxBITMAP_TYPE_TGA:
        case wxBITMAP_TYPE_MACCURSOR:
        case wxBITMAP_TYPE_MACCURSOR_RESOURCE:
        case wxBITMAP_TYPE_MAX:
        case wxBITMAP_TYPE_ANY:
        default:
            break;
    }

    if ( M_PIXDATA.save(wxQtConvertString(name), type_name) )
    {
        return true;
    }

#if wxUSE_IMAGE
    //Try to save using wx
    return ConvertToImage().SaveFile(name, type);
#else
    return false;
#endif
}

bool wxBitmap::LoadFile(const wxString &name, wxBitmapType type)
{
    //Try to load using Qt
    AllocExclusive();

    const char* type_name = nullptr;
    switch (type)
    {
        case wxBITMAP_TYPE_BMP:  type_name = "bmp";  break;
        case wxBITMAP_TYPE_ICO:  type_name = "ico";  break;
        case wxBITMAP_TYPE_JPEG: type_name = "jpeg"; break;
        case wxBITMAP_TYPE_PNG:  type_name = "png";  break;
        case wxBITMAP_TYPE_GIF:  type_name = "gif";  break;
        case wxBITMAP_TYPE_CUR:  type_name = "cur";  break;
        case wxBITMAP_TYPE_TIFF: type_name = "tif";  break;
        case wxBITMAP_TYPE_XBM:  type_name = "xbm";  break;
        case wxBITMAP_TYPE_PCX:  type_name = "pcx";  break;
        case wxBITMAP_TYPE_BMP_RESOURCE:
        case wxBITMAP_TYPE_ICO_RESOURCE:
        case wxBITMAP_TYPE_CUR_RESOURCE:
        case wxBITMAP_TYPE_XBM_DATA:
        case wxBITMAP_TYPE_XPM:
        case wxBITMAP_TYPE_XPM_DATA:
        case wxBITMAP_TYPE_TIFF_RESOURCE:
        case wxBITMAP_TYPE_GIF_RESOURCE:
        case wxBITMAP_TYPE_PNG_RESOURCE:
        case wxBITMAP_TYPE_JPEG_RESOURCE:
        case wxBITMAP_TYPE_PNM:
        case wxBITMAP_TYPE_PNM_RESOURCE:
        case wxBITMAP_TYPE_PCX_RESOURCE:
        case wxBITMAP_TYPE_PICT:
        case wxBITMAP_TYPE_PICT_RESOURCE:
        case wxBITMAP_TYPE_ICON:
        case wxBITMAP_TYPE_ICON_RESOURCE:
        case wxBITMAP_TYPE_ANI:
        case wxBITMAP_TYPE_IFF:
        case wxBITMAP_TYPE_TGA:
        case wxBITMAP_TYPE_MACCURSOR:
        case wxBITMAP_TYPE_MACCURSOR_RESOURCE:
        case wxBITMAP_TYPE_MAX:
        case wxBITMAP_TYPE_ANY:
        default:
            break;
    }

    QImage img;
    if ( img.load(wxQtConvertString(name), type_name) )
    {
        M_PIXDATA = (img.colorCount() > 0 && img.colorCount() <= 2)
                  ? QBitmap::fromImage(img, Qt::ThresholdDither)
                  : QPixmap::fromImage(img);

        return true;
    }
#if wxUSE_IMAGE
    else
    {
        //Try to load using wx
        wxImage image;
        if (image.LoadFile(name, type) && image.IsOk())
        {
            *this = wxBitmap(image);
            return true;
        }
    }
#endif

    return false;
}


#if wxUSE_PALETTE
wxPalette *wxBitmap::GetPalette() const
{
    wxMISSING_IMPLEMENTATION( "wxBitmap palettes" );
    return nullptr;
}

void wxBitmap::SetPalette(const wxPalette& WXUNUSED(palette))
{
    wxMISSING_IMPLEMENTATION( "wxBitmap palettes" );
}

#endif // wxUSE_PALETTE

#if WXWIN_COMPATIBILITY_3_0
void wxBitmap::SetHeight(int height)
{
    M_PIXDATA = QPixmap(GetWidth(), height);
}

void wxBitmap::SetWidth(int width)
{
    M_PIXDATA = QPixmap(width, GetHeight());
}

void wxBitmap::SetDepth(int depth)
{
    if (depth == 1)
        M_PIXDATA = QBitmap(GetWidth(), GetHeight());
    else
        M_PIXDATA = QPixmap(GetWidth(), GetHeight());
}
#endif

void *wxBitmap::GetRawData(wxPixelDataBase& data, int bpp)
{
    void* bits = nullptr;

    wxBitmapRefData *refData = static_cast<wxBitmapRefData *>(m_refData);

    // allow access if bpp is valid
    if ( !refData->m_qtPixmap.isNull() && (bpp == 32 || bpp == 24 || bpp == 1) )
    {
        refData->m_rawPixelSource = refData->m_qtPixmap.toImage().convertToFormat(
                                        bpp == 1 ? QImage::Format_Mono
                                                 : bpp == 32 ? QImage::Format_RGBA8888_Premultiplied
                                                             : QImage::Format_RGB888);
        data.m_height = refData->m_rawPixelSource.height();
        data.m_width = refData->m_rawPixelSource.width();
        data.m_stride = refData->m_rawPixelSource.bytesPerLine();
        bits = refData->m_rawPixelSource.bits();
    }

    return bits;
}

void wxBitmap::UngetRawData(wxPixelDataBase& WXUNUSED(data))
{
    wxBitmapRefData *refData = static_cast<wxBitmapRefData *>(m_refData);
    refData->m_qtPixmap = GetDepth() == 1
                        ? QBitmap::fromImage(refData->m_rawPixelSource)
                        : QPixmap::fromImage(refData->m_rawPixelSource);
    refData->m_rawPixelSource = QImage();
}

QPixmap *wxBitmap::GetHandle() const
{
    return ( m_refData != nullptr ) ? &M_PIXDATA : nullptr;
}

wxGDIRefData *wxBitmap::CreateGDIRefData() const
{
    return new wxBitmapRefData;
}

wxGDIRefData *wxBitmap::CloneGDIRefData(const wxGDIRefData *data) const
{
    const wxBitmapRefData* oldRef = static_cast<const wxBitmapRefData*>(data);
    wxBitmapRefData *d = new wxBitmapRefData;
    d->m_qtPixmap = oldRef->m_qtPixmap; //.copy();// copy not needed
    d->m_mask = oldRef->m_mask ? new wxMask(*oldRef->m_mask) : nullptr;
    return d;
}

bool wxBitmap::HasAlpha() const
{
    return M_PIXDATA.hasAlphaChannel();
}

//-----------------------------------------------------------------------------
// wxMask
//-----------------------------------------------------------------------------

wxIMPLEMENT_DYNAMIC_CLASS(wxMask, wxObject);

wxMask::wxMask()
{
    m_qtBitmap = nullptr;
}

wxMask::wxMask(const wxMask &mask)
{
    QBitmap *mask_bmp = mask.GetHandle();
    m_qtBitmap = mask_bmp ? new QBitmap(*mask_bmp) : nullptr;
}

wxMask& wxMask::operator=(const wxMask &mask)
{
    if (this != &mask)
    {
        delete m_qtBitmap;
        QBitmap *mask_bmp = mask.GetHandle();
        m_qtBitmap = mask_bmp ? new QBitmap(*mask_bmp) : nullptr;
    }
    return *this;
}

wxMask::wxMask(QBitmap* qtBitmap)
{
    m_qtBitmap = qtBitmap;
}

wxMask::wxMask(const wxBitmap& bitmap, const wxColour& colour)
{
    m_qtBitmap = nullptr;
    Create(bitmap, colour);
}

wxMask::wxMask(const wxBitmap& bitmap, int paletteIndex)
{
    m_qtBitmap = nullptr;
    Create(bitmap, paletteIndex);
}

wxMask::wxMask(const wxBitmap& bitmap)
{
    m_qtBitmap = nullptr;
    Create(bitmap);
}

wxMask::~wxMask()
{
    delete m_qtBitmap;
}

// this function is called from Create() to free the existing mask data
void wxMask::FreeData()
{
    delete m_qtBitmap;
    m_qtBitmap = nullptr;
}

bool wxMask::InitFromColour(const wxBitmap& bitmap, const wxColour& colour)
{
    if (!bitmap.IsOk())
        return false;

    delete m_qtBitmap;
    m_qtBitmap = new QBitmap(bitmap.GetHandle()->createMaskFromColor(colour.GetQColor(), Qt::MaskOutColor));

    return true;
}

bool wxMask::InitFromMonoBitmap(const wxBitmap& bitmap)
{
    //Only for mono bitmaps
    if (!bitmap.IsOk() || bitmap.GetDepth() != 1)
        return false;

    delete m_qtBitmap;
    m_qtBitmap = new QBitmap(*bitmap.GetHandle());

    return true;
}

wxBitmap wxMask::GetBitmap() const
{
    return wxBitmap(*m_qtBitmap);
}

QBitmap *wxMask::GetHandle() const
{
    return m_qtBitmap;
}
