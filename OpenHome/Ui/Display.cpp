#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/Private/Arch.h>
#include <OpenHome/Ui/Display.h>
#include <OpenHome/Ui/Unicode.h>
#include <OpenHome/Ui/Otb.h>
#include <vector>
#include <OpenHome/Private/Debug.h>
#include <OpenHome/Private/Converter.h>


using namespace OpenHome;
using namespace OpenHome::Ui;
using namespace std;




/*
FrameReader::FrameReader(FrameBuffer& aFrameBuffer)
    :iFrameBuffer(aFrameBuffer)
{
}


void FrameReader::Read(Bwx& aBuf)
{
    iFrameBuffer.Pixels(aBuf);
}
*/
//////////////////////////////////////////////////////////////////////////


FrameWriter::FrameWriter(FrameBuffer& aFrameBuffer)
    :iFrameBuffer(aFrameBuffer)
    ,iLocked(false)
{

}

void FrameWriter::Write(const Brx& aBuf) // for unit test
{
    ASSERT(iLocked);
    iFrameBuffer.Write(aBuf);
}

void FrameWriter::Write(const Bmp& aBitmap, TUint aX, TUint aY)
{
    ASSERT(iLocked);
    iFrameBuffer.Write(aBitmap, aX, aY);
}


void FrameWriter::Write(const Bmp& aBitmap, const Point& aPoint)
{
    ASSERT(iLocked);
    iFrameBuffer.Write(aBitmap, aPoint);
}

/*
void FrameWriter::Write(const FrameBuffer& aFrameBuffer, TUint aX, TUint aY)
{
    ASSERT(iLocked);
    iFrameBuffer.Write(aFrameBuffer, aX, aY);
}


void FrameWriter::Write(const FrameBuffer& aFrameBuffer, const Point& aPoint)
{
    ASSERT(iLocked);
    iFrameBuffer.Write(aFrameBuffer, aPoint);
}


void FrameWriter::Write(const FrameBuffer& aFrameBuffer, TUint aX, TUint aY, const Rectangle& aRectangle)
{
    ASSERT(iLocked);
    iFrameBuffer.Write(aFrameBuffer, aX, aY, aRectangle);
}


void FrameWriter::Write(const FrameBuffer& aFrameBuffer, const Point& aPoint, const Rectangle& aRectangle)
{
    ASSERT(iLocked);
    iFrameBuffer.Write(aFrameBuffer, aPoint, aRectangle);
}
*/

void FrameWriter::Write(const Strike& aStrike, const Brx& aString, TUint aX, TUint aY)
{
    ASSERT(iLocked);
    iFrameBuffer.Write(aStrike, aString, aX, aY);
}


void FrameWriter::Write(const Strike& aStrike, const Brx& aString, const Point& aPoint)
{
    ASSERT(iLocked);
    iFrameBuffer.Write(aStrike, aString, aPoint);
}


void FrameWriter::Lock()
{
    ASSERT(!iLocked);
    iFrameBuffer.WriterLock();
    iLocked = true;
}


void FrameWriter::Unlock()
{
    ASSERT(iLocked);
    iFrameBuffer.WriterUnlock();
    iLocked = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////


//  BMP File Format
//
//  Name                Size            Offset      Description
//
//  FileHeader          14 bytes        0           Windows Structure: BITMAPFILEHEADER
//
//      Signature       2 bytes         0           'BM'
//      FileSize        4 bytes         2           File size in bytes
//      reserved        4 bytes         6           unused (=0)
//      DataOffset      4 bytes         10          File offset to Raster Data
//
//  InfoHeader          40 bytes        14          Windows Structure: BITMAPINFOHEADER
//
//      Size            4 bytes         14          Size of InfoHeader =40
//      Width           4 bytes         18          Bitmap Width (pixels)
//      Height          4 bytes         22          Bitmap Height (pixles)
//      Planes          2 bytes         24          Number of Planes (=1)
//      BitCount        2 bytes         26          Bits per Pixel
//                                                  1 = monochrome palette. NumColors = 1
//                                                  4 = 4bit palletized. NumColors = 16
//                                                  8 = 8bit palletized. NumColors = 256
//                                                  16 = 16bit RGB. NumColors = 65536 (?)
//                                                  24 = 24bit RGB. NumColors = 16M
//      Compression     4 bytes         30          Type of Compression
//                                                  0 = BI_RGB   no compression
//                                                  1 = BI_RLE8 8bit RLE encoding
//                                                  2 = BI_RLE4 4bit RLE encoding
//      ImageSize       4 bytes         34          (compressed) Size of Image
//                                                  It is valid to set this =0 if Compression = 0
//      XpixelsPerM     4 bytes         38          horizontal resolution: Pixels/meter
//      YpixelsPerM     4 bytes         42          vertical resolution: Pixels/meter
//      ColorsUsed      4 bytes         46          Number of actually used colors
//      ColorsImportant 4 bytes         50          Number of important colors
//                                                  0 = all
//
//  ColorTable          4 * NumColors               present only if Info.BitsPerPixel <= 8
//
//      Red             1 byte                      Red intensity
//      Green           1 byte                      Green intensity
//      Blue            1 byte                      Blue intensity
//      reserved        1 byte                      unused (=0)
//
//  Raster              n bytes                     The pixel data

Bmp::Bmp(const Brx& aFile)
{
    // get bmp data and metrics from file
    iWidth = Converter::LeUint32At(aFile, 18);
    iHeight = Converter::LeUint32At(aFile, 22);
    iBitsPerPixel = Converter::LeUint16At(aFile, 28);
    iPixels.Set(aFile.Ptr() + Converter::LeUint32At(aFile, 10), (aFile.Bytes() - Converter::LeUint32At(aFile, 10)));

    ASSERT(iWidth > 0); // insure bitmap dimensions are valid
    ASSERT(iHeight > 0); // insure bitmap dimensions are valid
    ASSERT(iBitsPerPixel == 1); // no greyscale or colors currently supported
    ASSERT(iPixels.Bytes() > 0); // insure bitmap data is valid

    //LOG(kUi,"Bmp Dimensions: %d(w), %d(h), %d(bpp)\n",iWidth, iHeight, iBitsPerPixel);
}


Rectangle Bmp::Bounds(TUint aX, TUint aY) const
{
    return Bounds(Point(aX, aY));
}

Rectangle Bmp::Bounds(const Point& aPoint) const
{
    return Rectangle(aPoint, iWidth, iHeight);
}

//////////////////////////////////////////////////////////////////////////

// Rectangle

Rectangle::Rectangle(TUint aX, TUint aY, TUint aWidth, TUint aHeight)
:iOrigin(aX,aY), iWidth(aWidth), iHeight(aHeight)
{
    ASSERT(iWidth > 0); // insure rectangle dimensions are valid
    ASSERT(iHeight > 0); // insure rectangle dimensions are valid
}


Rectangle::Rectangle(const Point& aOrigin, TUint aWidth, TUint aHeight)
:iOrigin(aOrigin), iWidth(aWidth), iHeight(aHeight)
{
    ASSERT(iWidth > 0); // insure rectangle dimensions are valid
    ASSERT(iHeight > 0); // insure rectangle dimensions are valid
}


void Rectangle::Set(TUint aX, TUint aY, TUint aWidth, TUint aHeight)
{
    iOrigin.Set(aX, aY);
    iWidth = aWidth;
    iHeight = aHeight;
    ASSERT(iWidth > 0); // insure rectangle dimensions are valid
    ASSERT(iHeight > 0); // insure rectangle dimensions are valid
}


void Rectangle::Set(const Point& aOrigin, TUint aWidth, TUint aHeight)
{
    iOrigin.Set(aOrigin);
    iWidth = aWidth;
    iHeight = aHeight;
    ASSERT(iWidth > 0); // insure rectangle dimensions are valid
    ASSERT(iHeight > 0); // insure rectangle dimensions are valid
}


void Rectangle::SetWidth(TUint aWidth)
{
    iWidth = aWidth;
    ASSERT(iWidth > 0); // insure rectangle dimensions are valid
}


void Rectangle::SetHeight(TUint aHeight)
{
    iHeight = aHeight;
    ASSERT(iHeight > 0); // insure rectangle dimensions are valid
}


Point Rectangle::TopLeft() const
{
    return iOrigin;
}


Point Rectangle::TopRight() const
{
    return Point(iOrigin.X() + iWidth - 1, iOrigin.Y());
}


Point Rectangle::BottomLeft() const
{
    return Point(iOrigin.X(), iOrigin.Y() + iHeight - 1);
}


Point Rectangle::BottomRight() const
{
    return Point(iOrigin.X() + iWidth - 1, iOrigin.Y() + iHeight - 1);
}


Point Rectangle::OutsideTopLeft() const
{
    TUint x = iOrigin.X();
    TUint y = iOrigin.Y();
    ASSERT(x > 0);
    ASSERT(y > 0);
    return Point(x - 1, y - 1);
}


Point Rectangle::OutsideTopRight() const
{
    TUint x = iOrigin.X();
    TUint y = iOrigin.Y();
    ASSERT(y > 0);
    return Point(x + iWidth, y - 1);
}


Point Rectangle::OutsideBottomLeft() const
{
    TUint x = iOrigin.X();
    TUint y = iOrigin.Y();
    ASSERT(x > 0);
    return Point(x - 1, y + iHeight);
}


Point Rectangle::OutsideBottomRight() const
{
    TUint x = iOrigin.X();
    TUint y = iOrigin.Y();
    return Point(x + iWidth, y + iHeight);
}


Point Rectangle::NextRight() const
{
    return Point(iOrigin.X() + iWidth, iOrigin.Y());
}

Point Rectangle::NextDown() const
{
    return Point(iOrigin.X(), iOrigin.Y() + iHeight);
}

TBool Rectangle::IsInside(const Point& aPoint) const
{
    TUint x = iOrigin.X();
    TUint y = iOrigin.Y();
    if ( (aPoint.X() < x) || (aPoint.X() > x + iWidth - 1) ||
         (aPoint.Y() < y) || (aPoint.Y() > y + iHeight - 1) )
    {
        return false;
    }
    return true;
}

TBool Rectangle::IsInside(const Rectangle& aRectangle) const
{
    if (IsInside(aRectangle.TopLeft())) {
        return (IsInside(aRectangle.BottomRight()));
    }
    return false;
}


//////////////////////////////////////////////////////////////////////////

// Vector

Vector::Vector(TInt aX, TInt aY) : iX(aX),iY(aY)
{
}

void Vector::Set(TInt aX, TInt aY)
{
    iX = aX;
    iY = aY;
}

void Vector::Set(const Vector& aVector)
{
    iX = aVector.iX;
    iY = aVector.iY;
}


//////////////////////////////////////////////////////////////////////////

// Point

Point::Point(TUint aX, TUint aY) :iX(aX), iY(aY)
{
}

void Point::Set(const Point& aPoint)
{
    iX = aPoint.iX;
    iY = aPoint.iY;
}


Point& Point::Add(const Vector& aVector)
{
    return(Add(aVector.X(), aVector.Y()));
}


Point& Point::Add(TInt aX, TInt aY)
{
    iX += aX;
    iY += aY;
    return(*this);
}



////////////////////////////////////////////////////////////////////////////////////////////////

// class Strike

Strike::Strike(TUint aPointSize)
{
    iPointSize = aPointSize;
}


void Strike::AddRange(const Otb& aOtb, TUnicode aUnicodeStart, TUnicode aUnicodeEnd)
{
    GlyphRange* glyphRange = new GlyphRange(aUnicodeStart, aUnicodeEnd, iPointSize, aOtb);
    iGlyphTable.AddRange(glyphRange); //Add the given unicode range for the given point size
}


void Strike::AddRange(const Otb& aOtb, EUnicodeRange aUnicodeRange)
{
    switch (aUnicodeRange)
    {
        case eNumbers:
        {
            AddRange(aOtb, kVolumeSymbol, kMuteSymbol);
            AddRange(aOtb, 0x2e, 0x2e); // dot
            AddRange(aOtb, 0x30, 0x39); // ascii numbers
            break;
        }
        case eUpperCaseLetters:
        {
            // used for test fonts
            AddRange(aOtb, 0x41, 0x5a);
            break;
        }
        case eLowerCaseLetters:
        {
            // used for test fonts
            AddRange(aOtb, 0x61, 0x7a);
            break;
        }
        case eAscii:
        {
            AddRange(aOtb, kJukeboxSymbol, kInputSymbol);
            AddRange(aOtb, 0x20, 0x7e);
            break;
        }
        case eLatin1:
        {
            // used for test fonts
            AddRange(aOtb, 0x20, 0x7e);
            AddRange(aOtb, 0xa0, 0xff);
            break;
        }
        case eLatin1French:
        {
            AddRange(aOtb, kJukeboxSymbol, kInputSymbol);
            AddRange(aOtb, 0x20, 0x7e); // Basic Latin (ascii)
            AddRange(aOtb, 0xa0, 0xff); // Latin-1 supplement
            AddRange(aOtb, 0x152, 0x153); // french chars only from Latin-A
            AddRange(aOtb, 0x178, 0x178); // french
            AddRange(aOtb, 0x20a3, 0x20a3); // franc currency symbol
            AddRange(aOtb, 0x20ac, 0x20ac); // euro currency symbol
            break;
        }
        case eUnicodeBmpNonAscii: // Unicode: Basic MultilingualPlane (BMP 0x0000 to 0xffff)
        {
            AddRange(aOtb, 0xa0, 0x115e);
            AddRange(aOtb, 0x1161, 0x205f); // unicode bmp does not support 115F to 1160
            AddRange(aOtb, 0x2065, 0x3163); // unicode bmp does not support 2060 to 2064
            AddRange(aOtb, 0x3165, 0xD7ff); // unicode bmp does not support 3164
            AddRange(aOtb, 0xf900, 0xfdcf); // ignore surrogate pairs (d800 to dfff) and private use (e000 to f8ff)
            AddRange(aOtb, 0xfdf0, 0xfffd); // unicode bmp does not support fdd0 to fdef, fffe to ffff
            break;
        }
        case eUnicodeBmpNonLatin1: // Unicode: Basic MultilingualPlane (BMP 0x0000 to 0xffff)
        {
            AddRange(aOtb, 0x100, 0x115e);
            AddRange(aOtb, 0x1161, 0x205f); // unicode bmp does not support 115F to 1160
            AddRange(aOtb, 0x2065, 0x3163); // unicode bmp does not support 2060 to 2064
            AddRange(aOtb, 0x3165, 0xD7ff); // unicode bmp does not support 3164
            AddRange(aOtb, 0xf900, 0xfdcf); // ignore surrogate pairs (d800 to dfff) and private use (e000 to f8ff)
            AddRange(aOtb, 0xfdf0, 0xfffd); // unicode bmp does not support fdd0 to fdef, fffe to ffff
            break;
        }
        case eUnicodeBmpNonLatin1French: // Unicode: Basic MultilingualPlane (BMP 0x0000 to 0xffff)
        {
            AddRange(aOtb, 0x100, 0x151);   // 0x00 to 0xff: Latin (covered by Linn font)
            AddRange(aOtb, 0x154, 0x177);   // French OE letter 152 capital, 153 lowercase (covered by Linn font)
            AddRange(aOtb, 0x179, 0x115e);  // French Ydiersis 178 (covered by linn font)
            AddRange(aOtb, 0x1161, 0x205f); // unicode bmp does not support 115F to 1160
            AddRange(aOtb, 0x2065, 0x20a2); // unicode bmp does not support 2060 to 2064
            AddRange(aOtb, 0x20a4, 0x20ab); // French Franc sign 20a3 (covered by Linn font)
            AddRange(aOtb, 0x20ad, 0x3163); // French Euro sign 20ac (covered by Linn font)
            AddRange(aOtb, 0x3165, 0xD7ff); // unicode bmp does not support 3164
            AddRange(aOtb, 0xf900, 0xfdcf); // ignore surrogate pairs (d800 to dfff) and private use (e000 to f8ff)
            AddRange(aOtb, 0xfdf0, 0xfffd); // unicode bmp does not support fdd0 to fdef, fffe to ffff
            break;
        }
        case eUnicodeBmpNonLatin1FrenchTest: // Unicode: Basic MultilingualPlane (BMP 0x0000 to 0xffff)
        {
            AddRange(aOtb, 0x100, 0x151); // 0x00 to 0xff: Latin (covered by Linn font)
            AddRange(aOtb, 0x154, 0x177); // French OE letter 152 capital, 153 lowercase (covered by Linn font)
            AddRange(aOtb, 0x179, 0x2ff); // French Ydiersis 178 (covered by linn font)
            AddRange(aOtb, 0x370, 0x482); // ignore combining diacritical marks 300 to 36f
            AddRange(aOtb, 0x48a, 0x590); // ignore combining cyrillic 483 to 489
            AddRange(aOtb, 0x5be, 0x5be); // ignore hebrew accents, points 591 to 5bd
            AddRange(aOtb, 0x5c0, 0x5c0); // ignore hebrew accents, points 5bf
            AddRange(aOtb, 0x5c3, 0x5c3); // ignore hebrew accents, points 5c1 to 5c2
            AddRange(aOtb, 0x5c6, 0x5c6); // ignore hebrew accents, points 5c4 to 5c5
            AddRange(aOtb, 0x5c8, 0x60f); // ignore hebrew accents, points 5c7
            AddRange(aOtb, 0x61b, 0x64a); // ignore arabic signs 610 to 61a
            AddRange(aOtb, 0x65f, 0x66f); // ignore arabic marks 64b to 65e
            AddRange(aOtb, 0x671, 0x6d5); // ignore arabic marks 670
            AddRange(aOtb, 0x6dd, 0x6dd); // ignore arabic marks 6d6 to 6dc
            AddRange(aOtb, 0x6e5, 0x6e6); // ignore arabic marks 6de to 6e4
            AddRange(aOtb, 0x6e9, 0x6e9); // ignore arabic marks 6e7 to 6e8
            AddRange(aOtb, 0x6ee, 0x710); // ignore arabic marks 6ea to 6ed
            AddRange(aOtb, 0x712, 0x72f); // ignore syriac mark 711
            AddRange(aOtb, 0x74c, 0x7a5); // ignore syriac marks 730 to 74b
            AddRange(aOtb, 0x7b1, 0x7ea); // ignore thana marks 7a6 to 7b0
            AddRange(aOtb, 0x7f4, 0x900); // ignore nko combining marks 7eb to 7f3
            AddRange(aOtb, 0x904, 0x93b); // ignore devanagari signs 901 to 903
            AddRange(aOtb, 0x93d, 0x93d); // ignore devanagari signs 93c
            AddRange(aOtb, 0x94e, 0x950); // ignore devanagari signs 93e to 94d
            AddRange(aOtb, 0x955, 0x961); // ignore devanagari signs 951 to 954
            AddRange(aOtb, 0x964, 0x980); // ignore devanagari signs 962 to 963
            AddRange(aOtb, 0x984, 0x9bb); // ignore bengali signs 981 to 983
            AddRange(aOtb, 0x9bd, 0x9bd); // ignore bengali signs 9bc
            AddRange(aOtb, 0x9d8, 0x9e1); // ignore bengali signs 9be to 9d7
            AddRange(aOtb, 0x9e4, 0xa00); // ignore bengali signs 9e2 to 9e3
            AddRange(aOtb, 0xa04, 0xa3b); // ignore gurmukhi signs a01 to a03
            AddRange(aOtb, 0xa52, 0xa6f); // ignore gurmukhi signs a3c to a51
            AddRange(aOtb, 0xa73, 0xa74); // ignore gurmukhi signs a70 to a72
            AddRange(aOtb, 0xa76, 0xa80); // ignore gurmukhi signs a75
            AddRange(aOtb, 0xa84, 0xabb); // ignore gujarati signs a81 to a83
            AddRange(aOtb, 0xabd, 0xabd); // ignore gujarati signs abc
            AddRange(aOtb, 0xace, 0xae1); // ignore gujarati signs abe to acd
            AddRange(aOtb, 0xae4, 0xb00); // ignore gujarati signs ae2 to ae3
            AddRange(aOtb, 0xb04, 0xb3b); // ignore oriya signs b01 to b03
            AddRange(aOtb, 0xb3d, 0xb3d); // ignore oriya signs b3c
            AddRange(aOtb, 0xb58, 0xb61); // ignore oriya signs b3e to b57
            AddRange(aOtb, 0xb64, 0xb81); // ignore oriya signs b62 to b63
            AddRange(aOtb, 0xb83, 0xbbd); // ignore tamil signs b82
            AddRange(aOtb, 0xbd8, 0xc00); // ignore tamil signs bbe to bd7
            AddRange(aOtb, 0xc04, 0xc3d); // ignore telugu signs c01 to c03
            AddRange(aOtb, 0xc57, 0xc61); // ignore telugu signs c3e to c56
            AddRange(aOtb, 0xc64, 0xc81); // ignore telugu signs c62 to c63
            AddRange(aOtb, 0xc84, 0xcbb); // ignore kannada signs c82 to c83
            AddRange(aOtb, 0xcbd, 0xcbd); // ignore kannada signs cbc
            AddRange(aOtb, 0xcd7, 0xce1); // ignore kannada signs cbe to cd6
            AddRange(aOtb, 0xce4, 0xd01); // ignore kannada signs ce2 to ce3
            AddRange(aOtb, 0xd04, 0xd3d); // ignore malayalam signs d02 to d03
            AddRange(aOtb, 0xd58, 0xd61); // ignore malayalam signs d3e to d57
            AddRange(aOtb, 0xd64, 0xd81); // ignore malayalam signs d62 to d63
            AddRange(aOtb, 0xd84, 0xdc9); // ignore malayalam signs d82 to d83
            AddRange(aOtb, 0xdf4, 0xe30); // ignore sinhala signs dca to df3
            AddRange(aOtb, 0xe32, 0xe33); // ignore thai signs e31
            AddRange(aOtb, 0xe3b, 0xe46); // ignore thai signs e34 to e3a
            AddRange(aOtb, 0xe4f, 0xeb0); // ignore thai signs e47 to e4e
            AddRange(aOtb, 0xeb2, 0xeb3); // ignore lao signs eb1
            AddRange(aOtb, 0xebd, 0xec7); // ignore lao signs eb4 to ebc
            AddRange(aOtb, 0xece, 0xf17); // ignore lao signs ec8 to ecd
            AddRange(aOtb, 0xf1a, 0xf34); // ignore tibetan signs f18 to f19
            AddRange(aOtb, 0xf36, 0xf36); // ignore tibetan signs f35
            AddRange(aOtb, 0xf38, 0xf38); // ignore tibetan signs f37
            AddRange(aOtb, 0xf3a, 0xf3d); // ignore tibetan signs f39
            AddRange(aOtb, 0xf40, 0xf70); // ignore tibetan signs f3e to f3f
            AddRange(aOtb, 0xf84, 0xf85); // ignore tibetan signs f71 to f83
            AddRange(aOtb, 0xf88, 0xf8f); // ignore tibetan signs f86 to f87
            AddRange(aOtb, 0xfbd, 0xfc5); // ignore tibetan signs f90 to fbc
            AddRange(aOtb, 0xfc7, 0x102a); // ignore tibetan signs fc6
            AddRange(aOtb, 0x103f, 0x1055); // ignore myanmar signs 102b to 103e
            AddRange(aOtb, 0x105a, 0x105d); // ignore myanmar signs 1056 to 1059
            AddRange(aOtb, 0x1061, 0x1061); // ignore myanmar signs 105e to 1060
            AddRange(aOtb, 0x1065, 0x1066); // ignore myanmar signs 1062 to 1064
            AddRange(aOtb, 0x106e, 0x1070); // ignore myanmar signs 1067 to 106d
            AddRange(aOtb, 0x1075, 0x1081); // ignore myanmar signs 1071 to 1074
            AddRange(aOtb, 0x108e, 0x108e); // ignore myanmar signs 1082 to 108d
            AddRange(aOtb, 0x1090, 0x115e); // ignore myanmar signs 108f
            AddRange(aOtb, 0x1161, 0x135e); // unicode bmp does not support 115F to 1160
            AddRange(aOtb, 0x1360, 0x1711); // ignore ethipoic combining mark 135f
            AddRange(aOtb, 0x1715, 0x1731); // ignore tagalog signs 1712 to 1714
            AddRange(aOtb, 0x1735, 0x1751); // ignore hanunoo signs 1732 to 1734
            AddRange(aOtb, 0x1754, 0x1771); // ignore buhid signs 1752 to 1753
            AddRange(aOtb, 0x1774, 0x17b5); // ignore tagbanwa signs 1772 to 1773
            AddRange(aOtb, 0x17d4, 0x17dc); // ignore khmer signs 17b6 to 17d3
            AddRange(aOtb, 0x17de, 0x18a8); // ignore khmer signs 17dd
            AddRange(aOtb, 0x18aa, 0x191f); // ignore mongolian sign 18a9
            AddRange(aOtb, 0x193c, 0x19af); // ignore limbu sign 1920 to 193b
            AddRange(aOtb, 0x19c1, 0x19c7); // ignore new tai lue sign 19bo to 19c0
            AddRange(aOtb, 0x19ca, 0x1a16); // ignore tai lue sign sign 19c8 to 19c9
            AddRange(aOtb, 0x1a1c, 0x1aff); // ignore buginese sign 1a17 to 1a1b
            AddRange(aOtb, 0x1b05, 0x1b33); // ignore balinese sign 1b00 to 1b04
            AddRange(aOtb, 0x1b45, 0x1b6a); // ignore balinese sign 1b34 to 1b44
            AddRange(aOtb, 0x1b74, 0x1b7f); // ignore balinese sign 1b6b to 1b73
            AddRange(aOtb, 0x1b83, 0x1ba0); // ignore sudanese sign 1b80 to 1b82
            AddRange(aOtb, 0x1bab, 0x1c23); // ignore sudanese sign 1ba1 to 1baa
            AddRange(aOtb, 0x1c38, 0x1dbf); // ignore lepcha sign 1c24 to 1c37
            AddRange(aOtb, 0x1e00, 0x205f); // ignore combining marks 1dc0 to 1dff
            AddRange(aOtb, 0x2065, 0x20a2); // unicode bmp does not support 2060 to 2064
            AddRange(aOtb, 0x20a4, 0x20ab); // French Franc sign 20a3 (covered by Linn font)
            AddRange(aOtb, 0x20ad, 0x20cf); // French Euro sign 20ac (covered by Linn font)
            AddRange(aOtb, 0x2100, 0x2ddf); // ignore combining marks 20d0 to 20ff
            AddRange(aOtb, 0x2e00, 0x3029); // ignore combining cyrillic marks 2de0 to 2dff
            AddRange(aOtb, 0x3030, 0x3098); // ignore ideographic tone marks 302a to 302f
            AddRange(aOtb, 0x309b, 0x3163); // ignore combining katakana-hiragana marks 3099 to 309a
            AddRange(aOtb, 0x3165, 0xa66e); // unicode bmp does not support 3164
            AddRange(aOtb, 0xa673, 0xa67b); // ignore combining cyrillic marks a66f to a672
            AddRange(aOtb, 0xa67e, 0xa801); // ignore ombining cyrillic marks a67c to a67d
            AddRange(aOtb, 0xa803, 0xa805); // ignore syloti sign a802
            AddRange(aOtb, 0xa807, 0xa80a); // ignore syloti sign a806
            AddRange(aOtb, 0xa80c, 0xa822); // ignore syloti sign a80b
            AddRange(aOtb, 0xa828, 0xa87f); // ignore syloti sign a823 to a827
            AddRange(aOtb, 0xa882, 0xa8b3); // ignore saurashtra sign a880 to a881
            AddRange(aOtb, 0xa8c5, 0xa925); // ignore saurashtra sign a8b4 to a8c4
            AddRange(aOtb, 0xa92e, 0xa946); // ignore kayahli sign a926 to a92d
            AddRange(aOtb, 0xa954, 0xaa28); // ignore rejang sign a947 to a953
            AddRange(aOtb, 0xaa37, 0xaa42); // ignore cham sign aa29 to aa36
            AddRange(aOtb, 0xaa44, 0xaa4b); // ignore cham sign aa43
            AddRange(aOtb, 0xaa4e, 0xd7ff); // ignore cham sign aa4c to aa4d
            AddRange(aOtb, 0xf900, 0xfb1d); // ignore surrogate pairs (d800 to dfff) and private use (e000 to f8ff)
            AddRange(aOtb, 0xfb1f, 0xfdcf); // ignore hebrew point fb1e
            AddRange(aOtb, 0xfdf0, 0xfe1f); // unicode bmp does not support fdd0 to fdef
            AddRange(aOtb, 0xfe30, 0xfffd); // ignore combining marks fe20 to fe2f, unicode bmp does not support fffe to ffff
            break;
        }
        default: { break; }
    }
}


const Glyph& Strike::FindGlyph(TUnicode aUnicode) const
{
    // insure Glyph imformation requested exists in the glyph table
    if (!iGlyphTable.IsInTable(aUnicode))
    {
        //LOG(kUi,"Missing Unicode Value in Glyph Table: %lx\n",aUnicode);
        THROW(UnknownCharacter);
    }

    return (iGlyphTable.GlyphInTable(aUnicode));
}


Rectangle Strike::Bounds(const Brx& aString, TUint aX, TUint aY) const
{
    // generate rectangle that covers the whole string exactly (ascension, decension, bearing, and advance)
    // change pen position to top left corner of drawing
    // initial x bearing and final advance including as part of the string bounds

    TInt x = aX, y = 0, width = 0, height = 0;
    TInt maxBearingY = 0, dropY = 0, maxDropY = 0;
    const Glyph* glyph;
    String unicodeString(aString);

    //build string rectangle from character data
    for(TUint i=0; i<unicodeString.Chars(); i++) // 4 bytes per character
    {
        if (unicodeString.At(i) == '\n') { continue; } // skip newline
        try { glyph = &(FindGlyph(unicodeString.At(i))); }
        catch (UnknownCharacter) { glyph = &(FindGlyph(kUnknownCharacter)); }
        // move origin to start of pixels in x direction (top-> bottom/left->right)
        if(maxBearingY < glyph->BearingY()) { maxBearingY = glyph->BearingY(); } // y values above origin
        dropY = glyph->Height() - glyph->BearingY();
        if( (maxDropY < dropY) && (dropY > 0) ) { maxDropY = dropY; } // y values below (and including) origin
        if ( (i==(unicodeString.Chars()-1)) && ((glyph->Width() + glyph->BearingX()) > glyph->Advance()) ) { width += (glyph->Width() + glyph->BearingX()); } // accounts for final width > advance
        else { width += glyph->Advance(); }
        if ( (i==0) && (glyph->BearingX() < 0) )  { x += glyph->BearingX(); width -= glyph->BearingX(); } // accounts for initial negative bearing
    }
    y = aY - (maxBearingY-1);
    height = maxBearingY + maxDropY;

    //LOG(Debug::kUi|Debug::kVerbose,"Unicode Bounds: %d(x), %d(y), %d(w), %d(h)\n",x,y,width,height);

    if(!(x >= 0)) { THROW(LimitsExceeded); }
    if(!(y >= 0)) { THROW(LimitsExceeded); }
    if(!(width > 0)) { THROW(LimitsExceeded); }
    if(!(height > 0)) { THROW(LimitsExceeded); }

    Rectangle bounds(x,y,width,height);
    return bounds;
}


Rectangle Strike::Bounds(const Brx& aString, const Point& aPoint) const
{
    return Bounds(aString, aPoint.X(), aPoint.Y());
}


Brx& Strike::Fit(const Brx& aString, TUint aWidth, TBool aEllipsis)
{
    // want to fit portion of a string into a given width
    // return the allowed portion of that string

    TInt width = 0;
    Brn ellipsis("...");

    if (aEllipsis)
    {
        Rectangle bounds = Bounds(aString,DefaultPenLocation());
        if (bounds.Width() > aWidth)
        {
            bounds = Bounds(ellipsis,DefaultPenLocation());
            aWidth -= bounds.Width();
        }
    }

    const Glyph* glyph;
    String unicodeString(aString);

    //build string rectangle from character data
    for(TUint i=0; i<unicodeString.Chars(); i++) // 4 bytes per character
    {
        try { glyph = &(FindGlyph(unicodeString.At(i))); }
        catch (UnknownCharacter) { glyph = &(FindGlyph(kUnknownCharacter)); }
        width += glyph->Advance();
        if (width > (TInt)aWidth) // reached the end of the line
        {
            unicodeString.Shrink(0,i); // splits the string down to the requested portion
            if (aEllipsis) { unicodeString.Append(ellipsis); }
            break;
        }
    }
    unicodeString.ToUtf8(iFittedString);

/*
    LOG(Debug::kUi|Debug::kVerbose,"Unicode Orgnal String: ");
    LOG(Debug::kUi|Debug::kVerbose,aString);
    LOG(Debug::kUi|Debug::kVerbose,"\n");
    LOG(Debug::kUi|Debug::kVerbose,"Unicode Fitted String: ");
    LOG(Debug::kUi|Debug::kVerbose,iFittedString);
    LOG(Debug::kUi|Debug::kVerbose,"\n");
*/
    return iFittedString;
}

Brx& Strike::FitRtl(const Brx& aString, TUint aWidth, TBool aEllipsis)
{
    // want to fit portion of a string into a given width
    // return the allowed portion of that string

    TInt width = 0;
    Brn ellipsis("...");

    if (aEllipsis)
    {
        Rectangle bounds = Bounds(aString,DefaultPenLocation());
        if (bounds.Width() > aWidth)
        {
            bounds = Bounds(ellipsis,DefaultPenLocation());
            aWidth -= bounds.Width();
        }
    }

    const Glyph* glyph;
    String unicodeString(aString);

    //build string rectangle from character data
    TInt start = unicodeString.Chars() - 1;
    for(TInt i=start; i>=0; i--) // 4 bytes per character
    {
        try { glyph = &(FindGlyph(unicodeString.At(i))); }
        catch (UnknownCharacter) { glyph = &(FindGlyph(kUnknownCharacter)); }
        width += glyph->Advance();
        if (width > (TInt)aWidth) // reached the end of the line
        {
            unicodeString.Shrink(i+1,start-i); // splits the string down to the requested portion
            if (aEllipsis) { unicodeString.Prepend(ellipsis); }
            break;
        }
    }
    unicodeString.ToUtf8(iFittedString);

/*
    LOG(Debug::kUi|Debug::kVerbose,"Unicode Orgnal String: ");
    LOG(Debug::kUi|Debug::kVerbose,aString);
    LOG(Debug::kUi|Debug::kVerbose,"\n");
    LOG(Debug::kUi|Debug::kVerbose,"Unicode Fitted String: ");
    LOG(Debug::kUi|Debug::kVerbose,iFittedString);
    LOG(Debug::kUi|Debug::kVerbose,"\n");
*/
    return iFittedString;
}

void Strike::SetDefaultPenLocation(TUint aX, TUint aY)
{
    iDefaultPenLocation.Set(aX,aY);
}


const Point& Strike::DefaultPenLocation() const
{
    ASSERT(iDefaultPenLocation.Y() != 0); //not set for given font (Y value must be > 0)
    return iDefaultPenLocation;
}


TUint Strike::PointSize() const
{
    ASSERT(iPointSize > 0);
    return iPointSize;
}

//////////////////////////////////////////////////////////////////////////

FrameBuffer::FrameBuffer(TUint aWidth, TUint aHeight)
    :iWidth(aWidth)
    ,iHeight(aHeight)
    ,iPixelBytes(PixelBytes())
    ,iWritePixels(iPixelBytes)
    ,iReadPixels(iPixelBytes)
    ,iWriterLockCount(0)
    ,iMutexRead("FBRX")
    ,iMutexWrite("FBWX")
{
    ASSERT((iWidth&0x1f) == 0); // frame buffer width has to be word aligned
    ASSERT(iWidth > 0); // ensure frame buffer dimensions are valid
    ASSERT(iHeight > 0); // ensure frame buffer dimensions are valid

    //LOG(Debug::kUi,"FrameBuffer(%d, %d, %d)\n", aWidth, aHeight, aBitsPerPixel);
    iWritePixels.SetBytes(iWritePixels.MaxBytes());
    iReadPixels.SetBytes(iReadPixels.MaxBytes());
    iWritePixels.Fill(0x00);
    iReadPixels.Fill(0x00);
}



FrameWriter* FrameBuffer::CreateWriter()
{
    return(new FrameWriter(*this));
}


void FrameBuffer::AddReaderCallback(Functor aCallback)
{
    iReaderCallbacks.push_back(aCallback);
    //return(new FrameReader(*this));
}


void FrameBuffer::WriterLock()
{
    iWriterLockCount++;
}


void FrameBuffer::WriterUnlock()
{
    OpenHome::Log::Print("FrameBuffer::WriterUnlock() iReaderCallbacks.size() = %d\n", iReaderCallbacks.size());
    ASSERT(iWriterLockCount>0);
    iWriterLockCount--;
    if(iWriterLockCount == 0)
    {
        // take snapshot of write buffer
        iMutexRead.Wait();
        iReadPixels.Replace(iWritePixels);
        iMutexRead.Signal();

        // execute reader callbacks
        for(TUint i=0; i<iReaderCallbacks.size(); i++)
        {
            iReaderCallbacks[i]();
        }
    }
}


void FrameBuffer::Write(const Brx& aBuf)
{
    AutoMutex mutex(iMutexWrite);
    ASSERT(iWriterLockCount>0);
    ASSERT(aBuf.Bytes()<=iPixelBytes);
    iWritePixels.Replace(aBuf);
}


void FrameBuffer::Read(Bwx& aBuf)
{
    AutoMutex mutex(iMutexRead);
    aBuf.Append(iReadPixels);
}



Rectangle FrameBuffer::Bounds(TUint aX, TUint aY) const
{
    Rectangle bounds(aX,aY,iWidth,iHeight);
    return bounds;
}


TUint FrameBuffer::PixelBytes() const
{
//    TUint bytes = (iWidth + 7) >> 3; // round up
//    bytes *= iHeight;

    return (((iWidth + 7) * iHeight)/8);
}


void FrameBuffer::Clear()
{
    iWritePixels.Fill(0x00);
}


void FrameBuffer::Fill(TByte aFillByte)
{
    iWritePixels.Fill(aFillByte);
}


void FrameBuffer::Fill(TUint32 aFillWord)
{
    TUint32* inPixels = (TUint32*)iWritePixels.Ptr();
    for (TUint i = 0; i < PixelBytes()>>2; i++) { *inPixels++ = Arch::BigEndian4(aFillWord); }
}


// standard bit mask (index 32 = index 0: saves calculation)
const TUint32 FrameBuffer::kMask[33] = {0x00,0x01,0x03,0x07,0x0f,0x1f,0x3f,0x7f,0xff,0x1ff,0x3ff,0x7ff,0xfff,0x1fff,0x3fff,0x7fff,0xffff,0x1ffff,0x3ffff,0x7ffff,0xfffff,0x1fffff,0x3fffff,0x7fffff,0xffffff,0x1ffffff,0x3ffffff,0x7ffffff,0xfffffff,0x1fffffff,0x3fffffff,0x7fffffff,0x00};
// standard bit mask inverted (ie ~ operator already invoked - ones complement) (index 32 = index 0: saves calculation)
const TUint32 FrameBuffer::kMaskInv[33] = {0xffffffff,0xfffffffe,0xfffffffc,0xfffffff8,0xfffffff0,0xffffffe0,0xffffffc0,0xffffff80,0xffffff00,0xfffffe00,0xfffffc00,0xfffff800,0xfffff000,0xffffe000,0xffffc000,0xffff8000,0xffff0000,0xfffe0000,0xfffc0000,0xfff80000,0xfff00000,0xffe00000,0xffc00000,0xff800000,0xff000000,0xfe000000,0xfc000000,0xf8000000,0xf0000000,0xe0000000,0xc0000000,0x80000000,0xffffffff};

void FrameBuffer::Write(const Bmp& aBitmap, TUint aX, TUint aY)
{
    ASSERT(iWriterLockCount>0);


    if (!(Bounds().IsInside(aBitmap.Bounds(aX,aY)))) { THROW(LimitsExceeded); } // test bmp rectangle stays inside buffer

    // bmp data is word aligned
    // bmp lines are stored in reverse (bottom line first, top line last - need to flip - decrement inPtr)
    // similar to framebuffer to framebuffer with rectangle(0,0,bmp width, bmp height) - inBitOffset always 0
    TUint widthWordMasked = aBitmap.Width()&0x1f;
    TUint inLineWidthInWords = (aBitmap.Width()>>5) + (widthWordMasked ? 1:0);
    TUint inLineWidthInBytes = inLineWidthInWords<<2;
    TUint32* inPtr = (TUint32*)(aBitmap.Pixels().Ptr() + (aBitmap.Height()*inLineWidthInBytes) - inLineWidthInBytes);

    TUint outLineWidthInWords = Width()>>5;
    TUint outYOffsetInBytes = aY * (Width()>>3); //Offset from start of frame buffer (Y Direction Only)
    TUint outXOffsetInWords = aX>>5; //Offset from start of frame buffer (X Direction Only)
    TUint32* outPtr = (TUint32*)(iWritePixels.Ptr() + outYOffsetInBytes + (outXOffsetInWords<<2));
    TUint outBitOffset = aX - (outXOffsetInWords<<5);
    TUint outBitOffset32 = 32 - outBitOffset;

    TUint height = aBitmap.Height();
    TUint rightEdgeBit = aX + aBitmap.Width();
    TUint rightHandMaskIndex = ((rightEdgeBit&0xffe0)+32) - rightEdgeBit;

    TUint inWidthWords = inLineWidthInWords; // inBitOffset always 0
    TUint outWidthWords = inLineWidthInWords + (((widthWordMasked ? widthWordMasked:32)+outBitOffset)>32 ? 1:0);
    TBool extraOutputWord = (outWidthWords > inWidthWords);
    TUint widthWords = extraOutputWord ? inWidthWords:outWidthWords; // width to use in inner loop
    inLineWidthInWords = inLineWidthInWords<<1; //use for inPtr decrement - go back 2 lines from end location

    TUint width;
    TUint32 inWord, inWordPrev, rightHandMask;

    // all three bit shift algorithms combined (blitting)
    // shift left algorithm: outWidthWords will always be <= inWidthWords (inBitOffset > outBitOffset)
    // shift right algorithm: inputWidthWords will always be <= outWidthWords (inBitOffset < outBitOffset)
    // no shift algorithm: inputWidthWords will always be = outWidthWords (inBitOffset = outBitOffset)

    while (height--) //rows
    {
        rightHandMask = Arch::BigEndian4(*(outPtr + (outWidthWords-1))) & kMask[rightHandMaskIndex];
        // keep left hand original output data in tact
        inWord = (TUint32)(((TUint64)(Arch::BigEndian4(*outPtr)))>>outBitOffset32);

        width = widthWords; //use smallest of in or out- left: out, right: in, none: in = out
        while (width--) //columns
        {
            inWordPrev = inWord;
            inWord = (TUint32)((Arch::BigEndian8(*(TUint64*)inPtr++))>>32); // inBitOffset32 always 32
            *outPtr++ = Arch::BigEndian4(((TUint32)(((TUint64)(inWordPrev))<<outBitOffset32)) | (inWord>>outBitOffset));
        }
        //spillover into extra output word (right only)
        if (extraOutputWord) { *outPtr++ = Arch::BigEndian4(((TUint32)(((TUint64)(inWord))<<outBitOffset32))); }

        // keep right hand original output data in tact
        *(outPtr-1) = Arch::BigEndian4((Arch::BigEndian4(*(outPtr-1)) & kMaskInv[rightHandMaskIndex]) | rightHandMask);

        inPtr -= inLineWidthInWords;
        outPtr += outLineWidthInWords - outWidthWords;
    }

}


void FrameBuffer::Write(const Bmp& aBitmap, const Point& aPoint)
{
    ASSERT(iWriterLockCount>0);
    Write(aBitmap, aPoint.X(), aPoint.Y());
}


void FrameBuffer::Write(const Glyph& aGlyph, TUint aX, TUint aY)
{
    ASSERT(iWriterLockCount>0);
    if (!((aX + aGlyph.BearingX()) >= 0)) { THROW(LimitsExceeded); } //left (glyph only: BearingX < 0)
    if (!((TInt)aY >= (aGlyph.BearingY()-1))) { THROW(LimitsExceeded); } //top (glyph only: character would extend past the buffer top)
    if (!(((TInt)iHeight - (TInt)aY) > ((TInt)aGlyph.Height() - (TInt)aGlyph.BearingY()))) { THROW(LimitsExceeded); } //bottom (given character would drop below buffer limit: height)
    if (!((aX + aGlyph.Advance()) <= iWidth)) { THROW(LimitsExceeded); } //right (character would extend past the buffer edge: width)

    aX += aGlyph.BearingX(); // move origin to start of pixels in x direction (top-> bottom/left->right)
    aY -= (aGlyph.BearingY()-1); // move origin to start of pixels in y direction (top-> bottom/left->right)

    if (!(aX < iWidth)) { THROW(LimitsExceeded); } // test x,y point starts in buffer
    if (!(aY < iHeight)) { THROW(LimitsExceeded); } // test x,y point starts in buffer

    // glyph pixel Data is bit aligned (not word aligned) - stored in correct orientation
    // similar to framebuffer to framebuffer with rectangle(0,0,glyph width, glyph height) - inBitOffset always 0

    TUint32* inPtr = (TUint32*)(aGlyph.Pixels().Ptr());
    TUint inBitOffset32 = 32; // glyph data always starts at 0,0 - inBitOffset always 0

    TUint outLineWidthInWords = Width()>>5;
    TUint outYOffsetInBytes = aY * (Width()>>3); //Offset from start of frame buffer (Y Direction Only)
    TUint outXOffsetInWords = aX>>5; //Offset from start of frame buffer (X Direction Only)
    TUint32* outPtr = (TUint32*)(iWritePixels.Ptr() + outYOffsetInBytes + (outXOffsetInWords<<2));
    TUint outBitOffset = aX - (outXOffsetInWords<<5);
    TUint outBitOffset32 = 32 - outBitOffset;

    TUint height = aGlyph.Height();
    TUint rightEdgeBit = aX + aGlyph.Width();
    TUint rightHandMaskIndex = ((rightEdgeBit&0xffe0)+32) - rightEdgeBit;

    TUint widthWordMasked = aGlyph.Width()&0x1f;
    TUint widthWords = (aGlyph.Width()>>5) + (widthWordMasked ? 1:0);
    TUint inWidthWords = widthWords; // inBitOffset always 0
    TUint outWidthWords = widthWords + (((widthWordMasked ? widthWordMasked:32)+outBitOffset)>32 ? 1:0);
    TBool extraOutputWord = (outWidthWords > inWidthWords);
    widthWords = extraOutputWord ? inWidthWords:outWidthWords; // width to use in inner loop

    TUint width;
    TUint32 inWord, inWordPrev, rightHandMask;

    // all three bit shift algorithms combined (blitting)
    // shift left algorithm: outWidthWords will always be <= inWidthWords (inBitOffset > outBitOffset)
    // shift right algorithm: inputWidthWords will always be <= outWidthWords (inBitOffset < outBitOffset)
    // no shift algorithm: inputWidthWords will always be = outWidthWords (inBitOffset = outBitOffset)

    while (height--) //rows
    {
        rightHandMask = Arch::BigEndian4(*(outPtr + (outWidthWords-1))) & kMask[rightHandMaskIndex];
        // keep left hand original output data in tact
        inWord = (TUint32)(((TUint64)(Arch::BigEndian4(*outPtr)))>>outBitOffset32);

        width = widthWords; //use smallest of in or out- left: out, right: in, none: in = out
        while (width--) //columns
        {
            inWordPrev = inWord;
            inWord = (TUint32)((Arch::BigEndian8(*(TUint64*)inPtr))>>inBitOffset32);
            if (aGlyph.Width() > inBitOffset32) { inPtr++; inBitOffset32 += 32; }
            inBitOffset32 -= aGlyph.Width();
            *outPtr++ = Arch::BigEndian4(((TUint32)(((TUint64)(inWordPrev))<<outBitOffset32)) | (inWord>>outBitOffset));
        }
        //spillover into extra output word (right only)
        if (extraOutputWord) { *outPtr++ = Arch::BigEndian4(((TUint32)(((TUint64)(inWord))<<outBitOffset32))); }

        // keep right hand original output data in tact
        *(outPtr-1) = Arch::BigEndian4((Arch::BigEndian4(*(outPtr-1)) & kMaskInv[rightHandMaskIndex]) | rightHandMask);

        outPtr += outLineWidthInWords - outWidthWords;
    }
    //LOG(Debug::kUi|Debug::kVerbose,"Glyph Write: (%dx,%dy) , (%dw,%dh)\n", aX, aY, aGlyph.Width(), aGlyph.Height());
}


void FrameBuffer::Write(const Glyph& aGlyph, const Point& aPoint)
{
    ASSERT(iWriterLockCount>0);
    Write(aGlyph, aPoint.X(), aPoint.Y());
}

/*
void FrameBuffer::Write(const FrameBuffer& aFrameBuffer, TUint aX, TUint aY, const Rectangle& aRectangle)
{
    if(!(aFrameBuffer.Bounds().IsInside(aRectangle))) { THROW(LimitsExceeded); } // test rectangle stays inside aFrameBuffer (input)
    if(!(Bounds().IsInside(Rectangle(aX,aY,aRectangle.Width(),aRectangle.Height())))) { THROW(LimitsExceeded); } // test rectangle stays inside this frame buffer at new X,Y (output)

    TUint inLineWidthInWords = (aFrameBuffer.Width()>>5); // frame buffer width always word aligned
    TUint inYOffsetInBytes = aRectangle.Y() * (aFrameBuffer.Width()>>3); //Offset from start of frame buffer (Y Direction Only)
    TUint inXOffsetInWords = aRectangle.X()>>5; //Offset from start of frame buffer (X Direction Only)
    TUint32* inPtr = (TUint32*)(aFrameBuffer.Pixels().Ptr() + inYOffsetInBytes + (inXOffsetInWords<<2));
    TUint inBitOffset = aRectangle.X() - (inXOffsetInWords<<5);
    TUint inBitOffset32 = 32 - inBitOffset;

    TUint outLineWidthInWords = Width()>>5;
    TUint outYOffsetInBytes = aY * (Width()>>3); //Offset from start of frame buffer (Y Direction Only)
    TUint outXOffsetInWords = aX>>5; //Offset from start of frame buffer (X Direction Only)
    TUint32* outPtr = (TUint32*)(iWritePixels.Ptr() + outYOffsetInBytes + (outXOffsetInWords<<2));
    TUint outBitOffset = aX - (outXOffsetInWords<<5);
    TUint outBitOffset32 = 32 - outBitOffset;

    TUint height = aRectangle.Height();
    TUint rightEdgeBit = aX + aRectangle.Width();
    TUint rightHandMaskIndex = ((rightEdgeBit&0xffe0)+32) - rightEdgeBit;

    TUint widthWordMasked = aRectangle.Width()&0x1f;
    TUint widthWords = (aRectangle.Width()>>5) + (widthWordMasked ? 1:0);
    TUint inWidthWords = widthWords + (((widthWordMasked ? widthWordMasked:32)+inBitOffset)>32 ? 1:0);
    TUint outWidthWords = widthWords + (((widthWordMasked ? widthWordMasked:32)+outBitOffset)>32 ? 1:0);
    TBool extraOutputWord = (outWidthWords > inWidthWords);
    widthWords = extraOutputWord ? inWidthWords:outWidthWords; // width to use in inner loop

    TUint width;
    TUint32 inWord, inWordPrev, rightHandMask;

    // all three bit shift algorithms combined
    // shift left algorithm: outWidthWords will always be <= inWidthWords (inBitOffset > outBitOffset)
    // shift right algorithm: inputWidthWords will always be <= outWidthWords (inBitOffset < outBitOffset)
    // no shift algorithm: inputWidthWords will always be = outWidthWords (inBitOffset = outBitOffset)

    while (height--) //rows
    {
        rightHandMask = Arch::BigEndian4(*(outPtr + (outWidthWords-1))) & kMask[rightHandMaskIndex];
        // keep left hand original output data in tact
        inWord = (TUint32)(((TUint64)(Arch::BigEndian4(*outPtr)))>>outBitOffset32);

        width = widthWords; //use smallest of in or out- left: out, right: in, none: in = out
        while (width--) //columns
        {
            inWordPrev = inWord;
            inWord = (TUint32)((Arch::BigEndian8(*(TUint64*)inPtr++))>>inBitOffset32);
            *outPtr++ = Arch::BigEndian4(((TUint32)(((TUint64)(inWordPrev))<<outBitOffset32)) | (inWord>>outBitOffset));
        }
        //spillover into extra output word (right only)
        if (extraOutputWord) { *outPtr++ = Arch::BigEndian4(((TUint32)(((TUint64)(inWord))<<outBitOffset32))); }

        // keep right hand original output data in tact
        *(outPtr-1) = Arch::BigEndian4((Arch::BigEndian4(*(outPtr-1)) & kMaskInv[rightHandMaskIndex]) | rightHandMask);

        inPtr += inLineWidthInWords - widthWords;
        outPtr += outLineWidthInWords - outWidthWords;
    }

}


void FrameBuffer::Write(const FrameBuffer& aFrameBuffer, const Point& aPoint, const Rectangle& aRectangle)
{
    Write(aFrameBuffer, aPoint.X(), aPoint.Y(), aRectangle);
}


void FrameBuffer::Write(const FrameBuffer& aFrameBuffer, TUint aX, TUint aY)
{
    // extra function that allows a full frame buffer to be written to another frame buffer
    Rectangle rectangle(0,0,aFrameBuffer.Width(),aFrameBuffer.Height());
    Write(aFrameBuffer, aX, aY, rectangle);
}


void FrameBuffer::Write(const FrameBuffer& aFrameBuffer, const Point& aPoint)
{
    Write(aFrameBuffer, aPoint.X(), aPoint.Y());
}
*/

void FrameBuffer::Write(const Strike& aStrike, const Brx& aString, TUint aX, TUint aY)
{
    ASSERT(iWriterLockCount>0);
    TUint xOffset = aX;
    const Glyph* glyph;
    String unicodeString(aString);

    for(TUint i=0; i<unicodeString.Chars(); i++) // 4 bytes per character
    {
        //write a character
        try { glyph = &(aStrike.FindGlyph(unicodeString.At(i))); }
        catch (UnknownCharacter) { glyph = &(aStrike.FindGlyph(Strike::kUnknownCharacter)); }
        Write(*glyph, xOffset, aY); // pass the current glyph and pen position
        xOffset += glyph->Advance();
        //LOG(Debug::kUi|Debug::kVerbose,"Glyph Metrics (0x%x): %d(h), %d(w), %d(BearX), %d(BearY), %d(adv), %d(align)\n", unicodeString.At(i), glyph->Height(), glyph->Width(), glyph->BearingX(), glyph->BearingY(), glyph->Advance(), glyph->Alignment());
    }

}


void FrameBuffer::Write(const Strike& aStrike, const Brx& aString, const Point& aPoint)
{
    ASSERT(iWriterLockCount>0);
    Write(aStrike, aString, aPoint.X(), aPoint.Y());
}


const Brn FrameBuffer::kTestPatterns[7] = {
    Brn("TestBmp10x10.bmp"),
    Brn("TestBmp64x16.bmp"),
    Brn("TestBmpBorder128x32.bmp"),
    Brn("TestBmpCourierText128x32.bmp"),
    Brn("TestBmpHorizontalStripes128x32.bmp"),
    Brn("TestBmpLetterE7x7.bmp"),
    Brn("TestBmpVerticalStripes128x32.bmp") };

void FrameBuffer::TestPattern(TUint /*aTestPattern*/)
{
/*
    ASSERT(aTestPattern <= 6);
    Tars bmpTar(uUiTar);
    Bmp bmp(bmpTar.Find(kTestPatterns[aTestPattern]));
    Write(bmp, 0, 0);
*/
}


void FrameBuffer::Clear(const Rectangle& aRectangle)
{
    ASSERT(iWriterLockCount>0);
    TUint aX, aY, aWidth, aHeight;
    aX = aRectangle.X(); aY = aRectangle.Y(); aWidth = aRectangle.Width(); aHeight = aRectangle.Height();

    if(!(Bounds().IsInside(aRectangle))) { THROW(LimitsExceeded); } // test rectangle stays inside buffer

    TUint lineWidthInWords = Width()>>5;
    TUint yOffsetInBytes = aY * (Width()>>3); //Offset from start of frame buffer (Y Direction Only)
    TUint xOffsetInWords = aX>>5; //Offset from start of frame buffer (X Direction Only)
    TUint32* wordPtr = (TUint32*)(iWritePixels.Ptr() + yOffsetInBytes + (xOffsetInWords<<2));
    TUint bitOffset = aX - (xOffsetInWords<<5);
    TUint bitOffset32 = 32 - bitOffset;

    TUint height = aHeight;
    TUint rightEdgeBit = aX + aWidth;
    TUint rightHandMaskIndex = ((rightEdgeBit&0xffe0)+32) - rightEdgeBit;

    TUint widthWordMasked = aWidth&0x1f;
    TUint widthWords = (aWidth>>5) + (widthWordMasked ? 1:0);
    widthWords += (((widthWordMasked ? widthWordMasked:32)+bitOffset)>32 ? 1:0);

    TUint width;
    TUint32 word, wordPrev, rightHandMask;

    while (height--) //rows
    {
        rightHandMask = Arch::BigEndian4(*(wordPtr + (widthWords-1))) & kMask[rightHandMaskIndex];
        word = (TUint32)(((TUint64)(Arch::BigEndian4(*wordPtr)))>>bitOffset32);

        width = widthWords;
        while (width--) //columns
        {
            wordPrev = word;
            word = 0x00;
            *wordPtr++ = Arch::BigEndian4((TUint32)(((TUint64)(wordPrev))<<bitOffset32));
        }

        // keep right hand original output data in tact
        *(wordPtr-1) = Arch::BigEndian4((Arch::BigEndian4(*(wordPtr-1)) & kMaskInv[rightHandMaskIndex]) | rightHandMask);

        wordPtr += lineWidthInWords - widthWords;
    }

}


void FrameBuffer::Inverse(const Rectangle& aRectangle)
{
    ASSERT(iWriterLockCount>0);
    TUint aX, aY, aWidth, aHeight;
    aX = aRectangle.X(); aY = aRectangle.Y(); aWidth = aRectangle.Width(); aHeight = aRectangle.Height();

    if(!(Bounds().IsInside(aRectangle))) { THROW(LimitsExceeded); } // test rectangle stays inside buffer

    TUint lineWidthInWords = Width()>>5;
    TUint yOffsetInBytes = aY * (Width()>>3); //Offset from start of frame buffer (Y Direction Only)
    TUint xOffsetInWords = aX>>5; //Offset from start of frame buffer (X Direction Only)
    TUint32* wordPtr = (TUint32*)(iWritePixels.Ptr() + yOffsetInBytes + (xOffsetInWords<<2));
    TUint bitOffset = aX - (xOffsetInWords<<5);
    TUint bitOffset32 = 32 - bitOffset;

    TUint height = aHeight;
    TUint rightEdgeBit = aX + aWidth;
    TUint rightHandMaskIndex = ((rightEdgeBit&0xffe0)+32) - rightEdgeBit;

    TUint widthWordMasked = aWidth&0x1f;
    TUint widthWords = (aWidth>>5) + (widthWordMasked ? 1:0);
    widthWords += (((widthWordMasked ? widthWordMasked:32)+bitOffset)>32 ? 1:0);

    TUint width;
    TUint32 word, wordPrev, rightHandMask;

    while (height--) //rows
    {
        rightHandMask = Arch::BigEndian4(*(wordPtr + (widthWords-1))) & kMask[rightHandMaskIndex];
        word = (TUint32)(((TUint64)(Arch::BigEndian4(*wordPtr)))>>bitOffset32);

        width = widthWords;
        while (width--) //columns
        {
            wordPrev = word;
            word = ~(Arch::BigEndian4(*wordPtr));
            *wordPtr++ = Arch::BigEndian4(((TUint32)(((TUint64)(wordPrev))<<bitOffset32)) | (word>>bitOffset));
        }

        // keep right hand original output data in tact
        *(wordPtr-1) = Arch::BigEndian4((Arch::BigEndian4(*(wordPtr-1)) & kMaskInv[rightHandMaskIndex]) | rightHandMask);

        wordPtr += lineWidthInWords - widthWords;
    }

}


void FrameBuffer::SetPixel(TUint aX, TUint aY)
{
    ASSERT(iWriterLockCount>0);
    if(!(aX < iWidth)) { THROW(LimitsExceeded); } // test x,y point starts in buffer
    if(!(aY < iHeight)) { THROW(LimitsExceeded); } // test x,y point starts in buffer

    static const TByte kBitMask[8] = {0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};

    //find correct byte and bit location in this frame buffer
    TUint yOffsetInBytes = aY * (Width()>>3); //Offset from start of frame buffer (Y Direction Only)
    TUint xOffsetInBytes = aX>>3; //Offset from start of frame buffer (X Direction Only)
    TUint bitOffset = aX - (xOffsetInBytes<<3);
    TByte* bytePtr = (TByte*)(iWritePixels.Ptr() + yOffsetInBytes + xOffsetInBytes);

    //set that bit only
    *bytePtr |= kBitMask[bitOffset];


}


void FrameBuffer::SetPixel(const Point& aPoint)
{
    ASSERT(iWriterLockCount>0);
    SetPixel(aPoint.X(), aPoint.Y());
}


void FrameBuffer::ClearPixel(TUint aX, TUint aY)
{
    ASSERT(iWriterLockCount>0);
    if(!(aX < iWidth)) { THROW(LimitsExceeded); } // test x,y point starts in buffer
    if(!(aY < iHeight)) { THROW(LimitsExceeded); } // test x,y point starts in buffer

    static const TByte kBitMaskInv[8] = {0x7f,0xbf,0xdf,0xef,0xf7,0xfb,0xfd,0xfe}; //~kBitMask

    //find correct byte and bit location in this frame buffer
    TUint yOffsetInBytes = aY * (Width()>>3); //Offset from start of frame buffer (Y Direction Only)
    TUint xOffsetInBytes = aX>>3; //Offset from start of frame buffer (X Direction Only)
    TUint bitOffset = aX - (xOffsetInBytes<<3);
    TByte* bytePtr = (TByte*)(iWritePixels.Ptr() + yOffsetInBytes + xOffsetInBytes);

    //set that bit only
    *bytePtr &= kBitMaskInv[bitOffset];


}


void FrameBuffer::ClearPixel(const Point& aPoint)
{
    ASSERT(iWriterLockCount>0);
    ClearPixel(aPoint.X(), aPoint.Y());
}


TUint FrameBuffer::Width() const
{
    return iWidth;
}


TUint FrameBuffer::Height() const
{
    return iHeight;
}


void FrameBuffer::Pixels(Bwx& aBuf) const
{
    AutoMutex mutex(iMutexRead);
    aBuf.Replace(iReadPixels);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

// window class
/*
Window::Window(FrameBuffer& aInput, FrameBuffer& aOutput, TUint aX, TUint aY) :
    //iPixelsChanged(MakeFunctor(*this, &Window::PixelsChanged)),
    iFrameBufferIn(aInput), iFrameBufferOut(aOutput), iX(aX), iY(aY),
    iChanged(false), iOpen(true), iMutexOpen("WOPN"), iMutexLock("WLCK")
{
    iPoint.Set(aX,aY);
    iRectangle.Set(0,0,iFrameBufferIn.Width(),iFrameBufferIn.Height());
    //iPixelsChanged.Subscribe(iFrameBufferIn.PixelsChanged(), aPriority);
}


Window::Window(FrameBuffer& aInput, FrameBuffer& aOutput, const Point& aPoint) :
    //iPixelsChanged(MakeFunctor(*this, &Window::PixelsChanged)),
    iFrameBufferIn(aInput), iFrameBufferOut(aOutput), iX(aPoint.X()), iY(aPoint.Y()),
    iChanged(false), iOpen(true), iMutexOpen("WOPN"), iMutexLock("WLCK")
{
    iPoint = aPoint;
    iRectangle.Set(0,0,iFrameBufferIn.Width(),iFrameBufferIn.Height());
    //iPixelsChanged.Subscribe(iFrameBufferIn.PixelsChanged(), aPriority);
}


Window::Window(FrameBuffer& aInput, FrameBuffer& aOutput, TUint aX, TUint aY, const Rectangle& aRectangle) :
    //iPixelsChanged(MakeFunctor(*this, &Window::PixelsChanged)),
    iFrameBufferIn(aInput), iFrameBufferOut(aOutput), iX(aX), iY(aY),
    iChanged(false), iOpen(true), iMutexOpen("WOPN"), iMutexLock("WLCK")
{
    iPoint.Set(aX,aY);
    iRectangle.Set(aRectangle.X(),aRectangle.Y(),aRectangle.Width(),aRectangle.Height());
    //iPixelsChanged.Subscribe(iFrameBufferIn.PixelsChanged(), aPriority);
}


Window::Window(FrameBuffer& aInput, FrameBuffer& aOutput, const Point& aPoint, const Rectangle& aRectangle) :
    //iPixelsChanged(MakeFunctor(*this, &Window::PixelsChanged)),
    iFrameBufferIn(aInput), iFrameBufferOut(aOutput), iX(aPoint.X()), iY(aPoint.Y()),
    iChanged(false), iOpen(true), iMutexOpen("WOPN"), iMutexLock("WLCK"),
    iWriter(iFrameBufferOut.CreateWriter())
{
    iPoint = aPoint;
    iRectangle.Set(aRectangle.X(),aRectangle.Y(),aRectangle.Width(),aRectangle.Height());
    //iPixelsChanged.Subscribe(iFrameBufferIn.PixelsChanged(), aPriority);


}


void Window::Close()
{
    iMutexLock.Wait(); // insure no other thread writes to this window until it is opened again

    iMutexOpen.Wait();
    ASSERT(iOpen); // window must always be closed then opened
    iOpen = false;
    iMutexOpen.Signal();
}


void Window::Open()
{
    iMutexOpen.Wait();
    ASSERT(!iOpen); // window must always be closed then opened
    iOpen = true;
    iMutexOpen.Signal();

    if (iChanged) // update display if changed while closed
    {
        iWriter->Lock();
        iWriter->Write(iFrameBufferIn, iX, iY, iRectangle);
        iWriter->Unlock();
        iChanged = false;
    }

    iMutexLock.Signal(); // allow other threads to write to this window
}


TBool Window::Opened()
{
    iMutexOpen.Wait();
    bool open = iOpen;
    iMutexOpen.Signal();
    return open;
}


void Window::PixelsChanged()
{
    if (iMutexLock.TryWait()) // gaurantees thread safety
    {
        // mutex obtained (window open)
        iFrameBufferOut.Write(iFrameBufferIn, iX, iY, iRectangle);
        iMutexLock.Signal();
    }
    else
    {
        // mutex not available (window closed)
        iChanged = true;
    }

}


void Window::Set(TUint aX, TUint aY)
{
    //ASSERT(iMutexLock.TryWait()); // mutex obtained (window open)
    iX = aX;
    iY = aY;
    iPoint.Set(aX,aY);
    iRectangle.Set(0,0,iFrameBufferIn.Width(),iFrameBufferIn.Height());
    iMutexLock.Signal();
}


void Window::Set(const Point& aPoint)
{
//    ASSERT(iMutexLock.TryWait()); // mutex obtained (window open)
    iX = aPoint.X();
    iY = aPoint.Y();
    iPoint = aPoint;
    iRectangle.Set(0,0,iFrameBufferIn.Width(),iFrameBufferIn.Height());
    iMutexLock.Signal();
}


void Window::Set(TUint aX, TUint aY, const Rectangle& aRectangle)
{
//    ASSERT(iMutexLock.TryWait()); // mutex obtained (window open)
    iX = aX;
    iY = aY;
    iPoint.Set(aX,aY);
    iRectangle.Set(aRectangle.X(),aRectangle.Y(),aRectangle.Width(),aRectangle.Height());
    iMutexLock.Signal();
}


void Window::Set(const Point& aPoint, const Rectangle& aRectangle)
{
//    ASSERT(iMutexLock.TryWait()); // mutex obtained (window open)
    iX = aPoint.X();
    iY = aPoint.Y();
    iPoint = aPoint;
    iRectangle.Set(aRectangle.X(),aRectangle.Y(),aRectangle.Width(),aRectangle.Height());
    iMutexLock.Signal();
}

*/

