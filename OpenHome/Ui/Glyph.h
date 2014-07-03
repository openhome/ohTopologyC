#ifndef HEADER_UI_GLYPH
#define HEADER_UI_GLYPH

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/Functor.h>
#include <OpenHome/Private/Thread.h>
#include <vector>

#include <OpenHome/Ui/Otb.h>

//#include <Linn/Ascii/Ascii.h>
//#include <Linn/Unicode.h>

/*
#include <Linn/Standard.h>
#include <Linn/Event.h>
#include <Linn/Unicode.h>
#include <Linn/Nvms/Store.h>
#include <Linn/Tags.h>
#include <Linn/Control/Tar.h>
*/
EXCEPTION(LimitsExceeded);


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


//Fonts
struct GlyphMetrics
{
    TByte   iHeight;
    TByte   iWidth;
    TInt8   iBearingX;
    TInt8   iBearingY;
    TInt8   iAdvance; // all Combining Diacritical Marks (ie U+0300 to U+036F) will have negative advance
};

////////////////////////////////////////////

enum EGlyphAlignment
{
    eInvalid,
    eBitAligned,
    eByteAligned,
    eComponentArray,
    eLast               //ensure this is always the last entry
};

////////////////////////////////////////////


///\ingroup Font
///\class Glyph
///\brief A class which represents a bitmap of a character
///This class contains the bitmap for a character and information
///required to position it.
class Glyph
{
    friend class Otb;
public:
    ///The width of a glyph
    ///\retval TUint - a number of pixels
    TUint Width() const { return iMetrics->iWidth; }
    ///The height of a glyph
    ///\retval TUint - a number of pixels
    TUint Height() const { return iMetrics->iHeight; }
    ///The x offset from the origin to the start position
    ///\retval TInt - an offset in pixels
    TInt BearingX() const { return iMetrics->iBearingX; }
    ///The y offset from the origin to the start position
    ///\retval TInt - an offset in pixels
    TInt BearingY() const { return iMetrics->iBearingY; }
    ///The advance in pixels after the character to the start
    ///position for the next character
    ///\retval TUint - a number of pixels
    TUint Advance() const { return iMetrics->iAdvance; }
    ///The alignment type of the pixel data
    ///\retval EGlyphAlignment -eInvalid|eBitAligned|eByteAligned|eComponentArray
    EGlyphAlignment Alignment() const { return iAlignment; }
    ///A start address of the pixel data
    ///\retval const Brx& - the pixel data
    const Brx& Pixels() const { return iPixels; }
    ///Glyphs not initialised until requested for the first time
    ///\retval TBool - states if the glyph has been found and initialised
    TBool IsInitialised() { return (iPixels.Bytes() > 0); }
private:
    ///Set the glyph information
    void Set(const Brx& aPixels, EGlyphAlignment aGlyphAlignment, const GlyphMetrics* aMetrics);

private:
    Brn iPixels; // glyph pixel data - use to create bmp
    EGlyphAlignment iAlignment;
    const GlyphMetrics* iMetrics;
};


////////////////////////////////////////////

///\ingroup Font
///\class GlyphRange
///\brief A class which represents a range of glyphs of a particular point size
///\note check for class invariant - non of the ranges should overlap.
class GlyphRange : public INonCopyable
{
    friend class GlyphTable;
public:
    ///Construct a range of glyphs between the supplied unicode characters and point size
    //from the supplied Otb file.
    ///\param aUnicodeFirst - the first unicode character
    ///\param aUnicodeFirst - the last unicode character
    ///\param aPointSize - the point size of the glyph range
    ///\param aOtb - the font from which the range is to be located.
    GlyphRange(TUnicode aUnicodeFirst, TUnicode aUnicodeLast, TUint aPointSize, const Otb& aOtb);
    ///Destroy a GlyphRange
    ~GlyphRange();
    ///The glyph or bitmap representation of the supplied character
    ///\param aUnicode - a unicode character
    ///\retval Glyph& - a reference to a Glyph
    const Glyph& GlyphInRange(TUnicode aUnicode) const;
    ///Is the supplied character within the glyph range
    ///\param aUnicode - a unicode character
    ///\retval - TRUE or FALSE
    TBool IsInRange(TUnicode aUnicode) const;

private:
    TUnicode iUnicodeFirst;  //Unicode range supported
    TUnicode iUnicodeLast;
    TUint iPointSize;
    const Otb& iOtb;
    std::vector<Glyph*> iGlyph; // collection of glyphs (unicode first to unicode last) - each glyph has alignment/metrics/pixels)
};

////////////////////////////////////////////

///\ingroup Font
///\class GlyphTable
///\brief A class which represents a number of glyph ranges
///\note it is assumed that all ranges in a table are of the same point size
class GlyphTable
{
public:
    ///Construct a GlyphTable
    GlyphTable();
    ///Destroy a GlyphTable
    ~GlyphTable();
    ///Add a range of glyphs to a glyph table
    ///\param aGlyphRange
    void AddRange(const GlyphRange* aGlyphRange);
    ///Is the supplied character in the table
    ///\param aUnicode - the character to find
    ///\retval TRUE or FALSE
    TBool IsInTable(TUnicode aUnicode) const;
    ///Return the glyph data for the corresponding Unicode value
    const Glyph& GlyphInTable(TUnicode aUnicode) const;

private:
    std::vector<const GlyphRange*> iGlyphRange; // collection of glyph ranges
};

////////////////////////////////////////////////////////


} // namespace Ui

} // namespace OpenHome


#endif //HEADER_UI_GLYPH



























