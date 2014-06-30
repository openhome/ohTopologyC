#ifndef HEADER_UI_DISPLAY
#define HEADER_UI_DISPLAY

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/Functor.h>
#include <OpenHome/Private/Thread.h>
#include <vector>

#include <OpenHome/Ui/Otb.h>
#include <OpenHome/Ui/Glyph.h>
/*
#include <Linn/Ascii/Ascii.h>
#include <Linn/Unicode.h>


#include <Linn/Standard.h>
#include <Linn/Event.h>
#include <Linn/Unicode.h>
#include <Linn/Nvms/Store.h>
#include <Linn/Tags.h>
#include <Linn/Control/Tar.h>
*/
//EXCEPTION(LimitsExceeded);


///\note A strike is a digital font term used to describe a range of glyphs at a given point size
///
///\note A glyph is a pictorial representation of a character or symbol.


///\defgroup Geometry classes for expressing geometry
///\defgroup FileHandlingClasses classes for handling supported file types
///\defgroup Font classes relating to fonts


namespace OpenHome
{

namespace Ui
{


class FrameBuffer;
class FrameWriter;
class FrameReader;

///////////////////////////////////////////



///\ingroup Geometry
///\class Vector
///\brief A class which represents an offset from a coordinate expressed as (x,y).
class Vector
{
public:
    ///Creates a vector
    ///\param aX - the x offset
    ///\param aY - the y offset
    Vector(TInt aX = 0, TInt aY = 0);
    ///Creates a vector
    ///\param aX - the x offset
    ///\param aY - the y offset
    void Set(TInt aX, TInt aY);
    ///Set the Vector to a supplied Vector
    ///\param aVector - the supplied Vector
    void Set(const Vector& aVector);
    ///Set the Vector's X coordinate
    ///\param aX - the x offset
    void SetX(TInt aX) { iX = aX; }
    ///Set the Vector's Y coordinate
    ///\param aY - the y offset
    void SetY(TInt aY) { iY = aY; }
    ///Returns the Vector's X coordinate
    TInt X() const { return iX; }
    ///Returns the Vector's Y coordinate
    TInt Y() const { return iY; }

private:
    TInt iX;
    TInt iY;
};


////////////////////////////////////////////


///\ingroup Geometry
///\class Point
///\brief A class which represents a point (x,y).
class Point
{
public:
    ///Creates a point
    ///\note Default (empty arg list) Creates a point at origin (0,0)
    ///\param aX - the x coordinate
    ///\param aY - the y coordinate
    Point(TUint aX = 0, TUint aY = 0);
    ///Set the Point from an X and Y coordinate
    void Set(TUint aX, TUint aY) { iX = aX; iY = aY; }
    ///Set the Point from a supplied Point
    void Set(const Point& aPoint);
    ///Set the Point's X coordinate
    ///\param aX - the x coordinate
    void SetX(TUint aX) { iX = aX; }
    ///Set the Point's Y coordinate
    ///\param aY - the y coordinate
    void SetY(TUint aY) { iY = aY; }
    ///Adds a vector to create a new point
    ///\param aVector - the vector to add
    ///\retval Point& - a reference to a new point
    Point& Add(const Vector& aVector);
    ///Adds a vector to create a new point
    ///\param aX - the x coordinate
    ///\param aY - the y coordinate
    ///\retval Point& - a reference to a new point
    Point& Add(TInt aX, TInt aY);
    ///Returns the Point's X coordinate
    TUint X() const { return iX; }
    ///Returns the Point's Y coordinate
    TUint Y() const { return iY; }

private:
    TUint iX;
    TUint iY;

};


////////////////////////////////////////////

///\ingroup Geometry
///\class Rectangle
///\brief A class which represents a rectangle.
class Rectangle
{
public:
    ///Creates a rectangle from an origin (x,y) or (Point) and width and a length
    ///\note the origin in a Rectangle is considered to be the top left corner.
    ///\note Default (empty arg list) Creates a valid rectangle 1x1 at (0,0)
    ///\param aX - the x origin coordinate
    ///\param aY - the y origin coordinate
    ///\param aWidth - the width of the rectangle expressed from the point of origin
    ///\param aHeight - the height of the rectangle expressed from the point of origin
    Rectangle(TUint aX = 0, TUint aY = 0, TUint aWidth = 1, TUint aHeight = 1);
    Rectangle(const Point& aOrigin, TUint aWidth, TUint aHeight);

    /// Set rectangle dimensions at (x,y) or (point)
    void Set(TUint aX, TUint aY, TUint aWidth, TUint aHeight);
    void Set(const Point& aOrigin, TUint aWidth, TUint aHeight);
    ///Set the Rectangle's X coordinate
    void SetX(TUint aX) { iOrigin.SetX(aX); }
    ///Set the Rectangle's Y coordinate
    void SetY(TUint aY) { iOrigin.SetY(aY); }
    ///Set the Rectangle's Point as (x,y) or (point)
    void SetOrigin(TUint aX, TUint aY) { iOrigin.Set(aX, aY); }
    void SetOrigin(const Point& aOrigin) { iOrigin.Set(aOrigin); }
    ///Set the Rectangle's width
    void SetWidth(TUint aWidth);
    ///Set the Rectangle's height
    void SetHeight(TUint aHeight);

    ///Returns the Rectangle's X coordinate
    TUint X() const { return iOrigin.X(); }
    ///Returns the Rectangle's Y coordinate
    TUint Y() const { return iOrigin.Y(); }
    ///Returns the Rectangle's origin
    Point Origin() const { return iOrigin; }
    ///Returns the Rectangle's width
    TUint Width() const { return iWidth; }
    ///Returns the Rectangle's height
    TUint Height() const { return iHeight; }

    ///Returns the top left inner point of the rectangle
    Point TopLeft() const;
    ///Returns the top right inner point of the rectangle
    Point TopRight() const;
    ///Returns the bottom left inner point of the rectangle
    Point BottomLeft() const;
    ///Returns the bottom right inner point of the rectangle
    Point BottomRight() const;

    ///Returns the top left outer point(1x1 across and up/down - use for drawing adjacent to rectangle)
    Point OutsideTopLeft() const;
    ///Returns the top right outer point(1x1 across and up/down - use for drawing adjacent to rectangle)
    Point OutsideTopRight() const;
    ///Returns the bottom left outer point(1x1 across and up/down - use for drawing adjacent to rectangle)
    Point OutsideBottomLeft() const;
    ///Returns the bottom right outer point(1x1 across and up/down - use for drawing adjacent to rectangle)
    Point OutsideBottomRight() const;

    ///Returns the point one to the right of the top right inner point
    Point NextRight() const;
    ///Returns the point one below the bottom left point
    Point NextDown() const;

    ///Returns true if the given point is inside
    TBool IsInside(const Point& aPoint) const;
    ///Returns true if the given rectangle fits inside (including exactly adjacent) this rectangle
    TBool IsInside(const Rectangle& aRectangle) const;

private:
    Point iOrigin;
    TUint iWidth;
    TUint iHeight;
};

////////////////////////////////////////////

///\ingroup Font
///\class Strike
///\brief A container of all the characters available for use from a font for a given size
class Strike
{
public:
    static const TUnicode kUnknownCharacter = '*'; ///> character to substitute in for any unknown character
    static const TUint8 kJukeboxSymbol = 0x10;
    static const TUint8 kRadioSymbol = 0x11;
    static const TUint8 kVolumeSymbol = 0x12;
    static const TUint8 kMuteSymbol = 0x13;
    static const TUint8 kSourceSymbol = 0x14;
    static const TUint8 kSenderSymbol = 0x15;
    static const TUint8 kInputSymbol = 0x16;

    /// various predefined unicode ranges
    enum EUnicodeRange
    {
        eNumbers, ///> 0x30 to 0x39, dot, symbols
        eUpperCaseLetters, ///> 0x41 to 0x5a
        eLowerCaseLetters, ///> 0x61 to 0x7a
        eAscii, ///> basic latin: standard ascii range 0x20 to 0x7e (no control characters), symbols
        eLatin1, ///> basic latin plus latin-1 supplement: extended range 0x20 to 0x7e and 0xa0 to 0xff (no control characters), symbols
                 ///> extra languages: Danish, Dutch, Faroese, Finnish, Flemish, German, Icelandic, Irish, Italian, Norwegian, Portuguese, Spanish and Swedish
        eLatin1French, ///> latin-1 plus necessary characters from Latin-A for French, metadata symbols
        eUnicodeBmpNonAscii, ///> Unicode: Basic MultilingualPlane (BMP 0x0000 to 0xffff) minus eAscii
        eUnicodeBmpNonLatin1, ///> Unicode: Basic MultilingualPlane (BMP 0x0000 to 0xffff) minus eLatin1
        eUnicodeBmpNonLatin1French, ///> Unicode: Basic MultilingualPlane (BMP 0x0000 to 0xffff) minus eLatin1French
        eUnicodeBmpNonLatin1FrenchTest, ///> Unicode: Basic MultilingualPlane (BMP 0x0000 to 0xffff) minus eLatin1French, minus all characters with a negative X bearing (mainly combining marks, signs, tones, etc: can't be tested as single characters)
    };

public:
    ///Constructed with a point size. Creates a blank strike
    ///\param aPointSize - the character size to be used
    Strike(TUint aPointSize);
    ///Adds a unicode range to the strike
    ///\param aOtb - the otb font to add from
    ///\param aUnicodeStart - the start code (unicode) for the range to be added to the strike
    ///\param aUnicodeEnd - the start code (unicode) for the range to be added to the strike
    void AddRange(const Otb& aOtb, TUnicode aUnicodeStart, TUnicode aUnicodeEnd);
    ///Adds a pre-defined unicode range to the strike
    ///\param aOtb - the otb font to add from
    ///\param aUnicodeRange - predefined range
    void AddRange(const Otb& aOtb, EUnicodeRange aUnicodeRange);
    ///Return the glyph data for the corresponding Unicode value
    const Glyph& FindGlyph(TUnicode aUnicode) const;
    /// Returns the rectangular boundaries of a utf-8 string given a starting offset (x,y) or (point)
    Rectangle Bounds(const Brx& aString, TUint aX, TUint aY) const;
    Rectangle Bounds(const Brx& aString, const Point& aPoint) const;
    /// Returns the portion of the given utf-8 string that fits within the given width
    ///\param TBool optional ellipsis added to the end of the string
    Brx& Fit(const Brx& aString, TUint aWidth, TBool aEllipsis);
    Brx& FitRtl(const Brx& aString, TUint aWidth, TBool aEllipsis);

    /// Set the default pen location for a given strike (Optional)
    void SetDefaultPenLocation(TUint aX, TUint aY);
    /// Returns the default pen location for a given strike (will assert if not set)
    const Point& DefaultPenLocation() const;
    TUint PointSize() const;

private:
    TUint iPointSize;
    GlyphTable iGlyphTable;
    Point iDefaultPenLocation;
    Bws<256> iFittedString;
};

////////////////////////////////////////////

///\ingroup FileHandlingClasses
///\class Bmp
///\brief A class which represents a bitmap file of type *.bmp
class Bmp
{
public:
    ///Creates a bitmap from a buffer
    Bmp(const Brx& aFile);
    ///\retval width in pixels
    TUint Width() const { return iWidth; }
    ///\retval height in pixels
    TUint Height() const { return iHeight; }
    /// Returns the rectangular bounds for this bitmap given an origin point (x,y)
    /// Defaults to origin point of (0,0)
    /// \retaval the bounding rectangle
    Rectangle Bounds(TUint aX = 0, TUint aY = 0) const;
    /// Returns the rectangular bounds for this bitmap given an origin point
    /// \retaval the bounding rectangle
    Rectangle Bounds(const Point& aPoint) const;
    /// \retval number of bits per pixel
    TUint BitsPerPixel() const { return iBitsPerPixel; }
    /// \retval a reference to a buffer of pixel data
    const Brx& Pixels() const { return iPixels; }

private:
    TUint iWidth;
    TUint iHeight;
    TUint iBitsPerPixel;
    Brn iPixels;
};

////////////////////////////////////////////


///\ingroup FileHandlingClasses
///\class AnimatedBitmap
///\brief A class which represents an animated bitmap file
///\note Similar to a bmp file with multiple bmp pixel data at end
class AnimatedBitmap
{
public:
    ///Creates an AnimatedBitmap file class from a buffer
    AnimatedBitmap(const Brx& aFile);
    ///Returns the frame width
    ///\retval TUint - a frame width in pixels
    TUint Width() { return iWidth; }
    ///Returns the frame height
    ///\retval TUint - a frame height in pixels
    TUint Height() { return iHeight; }
    ///Returns the bit depth or number of bits per pixel
    ///\retval TUint - number of bits per pixel
    TUint BitsPerPixel() { return iBitsPerPixel; }
    ///Returns a buffer containing the pixel data
    ///\retval const Brx& - a buffer containing the pixel data
    const Brx& Pixels(TUint aFrame) { return iPixels; }
    ///Return the time interval between frames
    ///\retval TUint - the time between frames in microseconds
    TUint FrameInterval(TUint aFrame) { return iFrameInterval; }
    ///The number of frames in the animation
    ///\retval TUint - number of frames
    TUint FrameCount() { return iFrameCount; }

private:
    TUint iWidth;
    TUint iHeight;
    TUint iBitsPerPixel;
    TUint iFrameCount;
    TUint iFrameInterval;
    Brn iPixels;
};

////////////////////////////////////////////


///\class FrameBuffer
///\brief A container of all the pixels on all or part of a display
class FrameBuffer
{
    friend class FrameReader;
    friend class FrameWriter;

public:
    ///Construct a frame buffer of specified height, width and bits per pixel
    ///\param - aWidth in pixels
    ///\param - aHeight in pixels
    ///\param - aBitsPerPixel number of bits used per pixel (defaults to 1)
    FrameBuffer(TUint aWidth, TUint aHeight/*, TUint aBitsPerPixel = 1*/);

    void AddReaderCallback(Functor aCallback);
    FrameWriter* CreateWriter();

    // public data (read only)
    ///A width in pixels
    ///\retval TUint a width
    TUint Width() const; // { return iWidth; }
    ///A height in pixels
    ///\retval TUint a height
    TUint Height() const; // { return iHeight; }
    /// Returns the rectangular boundaries of a frame buffer given a starting offset (x,y) or (point)
    /// Defaults to full bounds (starting point 0,0)
    Rectangle Bounds(TUint aX = 0, TUint aY = 0) const;
    ///The number of bits to represent a pixel
    ///\retval TUint
    //TUint BitsPerPixel() const; //{ return iBitsPerPixel; }
    ///Return the pixel data of the frame buffer
    ///\retval const Brx& - a buffer containing the pixel data
    void Pixels(Bwx& aBuf) const; // { return iPixels; }
    /// Returns the producer for the frame buffer data changed event
    /// @param none
    /// @retval Producer& event producer reference
    //Producer& PixelsChanged() {return iPixelsChanged;}

    void Read(Bwx& aBuf);
    ///Returns the required number of bytes of pixel data
    ///\retval TUint - the number of bytes of pixel data required
    TUint PixelBytes() const;

private:
    void WriterLock();
    void WriterUnlock();


    void Write(const Brx& aBuf); // for unit test


    // Write Functions

    ///Write a bitmap to this frame buffer at offset (x,y) or (point)
    ///\param aBitmap - the bitmap to write
    ///\param aX - the x offset
    ///\param aY - the y offset
    void Write(const Bmp& aBitmap, TUint aX, TUint aY);
    void Write(const Bmp& aBitmap, const Point& aPoint);
    ///Write a full frame buffer to this frame buffer at offset (x,y) or (point)
    ///\param aBitmap - the bitmap to write
    ///\param aX - the x offset
    ///\param aY - the y offset

//    void Write(const FrameBuffer& aFrameBuffer, TUint aX, TUint aY);
//    void Write(const FrameBuffer& aFrameBuffer, const Point& aPoint);
    ///Write a frame buffer rectangular portion to this frame buffer at offset (x,y) or (point)
    ///\param aFrameBuffer - the frame buffer to write
    ///\param aRectangle - the rectangular portion of aFrameBuffer to write
    ///\param aX - the x offset
    ///\param aY - the y offset
//    void Write(const FrameBuffer& aFrameBuffer, TUint aX, TUint aY, const Rectangle& aRectangle);
//    void Write(const FrameBuffer& aFrameBuffer, const Point& aPoint, const Rectangle& aRectangle);
    ///Write a utf-8 string to the frame buffer at position (x,y) or (point)
    ///\param aStrike - the strike to use for the string
    ///\param aString - the string of characters to write (utf-8 encoded: handles ascii (1byte) and unicode(4byte))
    ///\param aX - the x offset
    ///\param aY - the y offset
    void Write(const Strike& aStrike, const Brx& aString, TUint aX, TUint aY);
    void Write(const Strike& aStrike, const Brx& aString, const Point& aPoint);
    // Set a single pixel in the frame buffer at offset (x,y)
    ///\param aX - the x offset
    ///\param aY - the y offset
    void SetPixel(TUint aX, TUint aY);
    void SetPixel(const Point& aPoint);
    ///Fill frame buffer
    ///\param aFillByte - fill frame buffer pixels with this byte (default is 0xff - all on)
    void Fill(TByte aFillByte = 0xff);
    ///Fill frame buffer
    ///\param aFillWord - fill frame buffer pixels with this word
    void Fill(TUint32 aFillWord);
    ///Write Test Pattern to Frame Buffer
    ///\param Test Patter to write
    void TestPattern(TUint aTestPattern);
    ///Genereate a pixels changed event for the current frame buffer
    ///\param None
    //void Refresh();

    // Inverse Functions

    ///Inverse (reverse video) a rectangular portion of this frame buffer specified by param aRectangle
    ///Rectangular portion must already be written to (this function will only inverse the existing data)
    void Inverse(const Rectangle& aRectangle);

    // Clear Functions

    ///Clear a rectangular portion of this frame buffer specified by param aRectangle
    ///\param aRectangle to clear from frame buffer
    void Clear(const Rectangle& aRectangle);
    // Clear a single pixel from the frame buffer at offset (x,y) or (point)
    ///\param aX - the x offset
    ///\param aY - the y offset
    void ClearPixel(TUint aX, TUint aY);
    void ClearPixel(const Point& aPoint);
    ///Clear entire frame buffer contents
    void Clear();


    /// write a single glyph to the frame buffer (blitting) - write(strike,string) will write glyph by gyph
    void Write(const Glyph& aGlyph, TUint aX, TUint aY);
    void Write(const Glyph& aGlyph, const Point& aPoint);

private:
    TUint iWidth;
    TUint iHeight;
    //TUint iBitsPerPixel;
    TUint iPixelBytes;
    Bwh iWritePixels;
    Bwh iReadPixels;
    //Producer iPixelsChanged;

    static const TUint32 kMask[33];
    static const TUint32 kMaskInv[33];
    static const Brn kTestPatterns[7];

    TUint iWriterLockCount;
    std::vector<Functor> iReaderCallbacks;
    mutable Mutex iMutexRead;
    mutable Mutex iMutexWrite;
};

////////////////////////////////////////////


///\class Window
///\brief A class which subscribes to one frame buffer and writes the dynamically changing data to another frame buffer
///\note think of it as a 'picture in picture' concept
/*
class Window
{
public:

    // ** window priority should match the priority of the display being used (higher than calling thread)
    // ** user must close/lock window before writing to it for it to be thread safe **
    // ** order of operations: close window(locks), write to input FB, open (unlocks and signals output FB) **

    Window(FrameBuffer& aInput, FrameBuffer& aOutput, TUint aX, TUint aY);
    Window(FrameBuffer& aInput, FrameBuffer& aOutput, const Point& aPoint);
    Window(FrameBuffer& aInput, FrameBuffer& aOutput, TUint aX, TUint aY, const Rectangle& aRectangle);
    Window(FrameBuffer& aInput, FrameBuffer& aOutput, const Point& aPoint, const Rectangle& aRectangle);

    void Set(TUint aX, TUint aY);
    void Set(const Point& aPoint);
    void Set(TUint aX, TUint aY, const Rectangle& aRectangle);
    void Set(const Point& aPoint, const Rectangle& aRectangle);

    void Close(); // Lock window, Temporarily prevent changes to the input framebuffer from signaling the output framebuffer
    void Open(); // Unlock window, Re-enable connection from input to output framebuffers and update output if changed while closed

    FrameBuffer& FrameBufferIn() { return iFrameBufferIn; }
    FrameBuffer& FrameBufferOut() { return iFrameBufferOut; }
    Rectangle& Rect() { return iRectangle; }
    Point& Pnt() { return iPoint; }
    TUint X() { return iX; }
    TUint Y() { return iY; }
    TBool Opened();


private:
    void PixelsChanged();

private:
    //Consumer iPixelsChanged;
    FrameBuffer& iFrameBufferIn;
    FrameBuffer& iFrameBufferOut;
    Rectangle iRectangle;
    Point iPoint;
    TUint iX;
    TUint iY;
    TBool iChanged;
    TBool iOpen; // window will default to open
    Mutex iMutexOpen;
    Mutex iMutexLock;
    FrameWriter* iWriter;
};
*/

//////////////////////////////////////////

class FrameReader
{
public:
    FrameReader(FrameBuffer& aFrameBuffer);

    void Read(Bwx& aBuf);

private:
    FrameBuffer& iFrameBuffer;
};

///////////////////////////////////////////

class FrameWriter
{
public:
    FrameWriter(FrameBuffer& aFrameBuffer);

    void Lock();
    void Unlock();


    void Write(const Brx& aBuf); // for unit test

    ///Write a bitmap to this frame buffer at offset (x,y) or (point)
    ///\param aBitmap - the bitmap to write
    ///\param aX - the x offset
    ///\param aY - the y offset
    void Write(const Bmp& aBitmap, TUint aX, TUint aY);
    void Write(const Bmp& aBitmap, const Point& aPoint);
    ///Write a full frame buffer to this frame buffer at offset (x,y) or (point)
    ///\param aBitmap - the bitmap to write
    ///\param aX - the x offset
    ///\param aY - the y offset
/*
    void Write(const FrameBuffer& aFrameBuffer, TUint aX, TUint aY);
    void Write(const FrameBuffer& aFrameBuffer, const Point& aPoint);
    ///Write a frame buffer rectangular portion to this frame buffer at offset (x,y) or (point)
    ///\param aFrameBuffer - the frame buffer to write
    ///\param aRectangle - the rectangular portion of aFrameBuffer to write
    ///\param aX - the x offset
    ///\param aY - the y offset
    void Write(const FrameBuffer& aFrameBuffer, TUint aX, TUint aY, const Rectangle& aRectangle);
    void Write(const FrameBuffer& aFrameBuffer, const Point& aPoint, const Rectangle& aRectangle);
*/
    ///Write a utf-8 string to the frame buffer at position (x,y) or (point)
    ///\param aStrike - the strike to use for the string
    ///\param aString - the string of characters to write (utf-8 encoded: handles ascii (1byte) and unicode(4byte))
    ///\param aX - the x offset
    ///\param aY - the y offset
    void Write(const Strike& aStrike, const Brx& aString, TUint aX, TUint aY);
    void Write(const Strike& aStrike, const Brx& aString, const Point& aPoint);

private:
    FrameBuffer& iFrameBuffer;
    TBool iLocked;
};



} // namespace Ui

} // namespace OpenHome


#endif //HEADER_UI_DISPLAY



























