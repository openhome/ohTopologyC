#ifndef HEADER_OTB
#define HEADER_OTB

//#include <Linn/Standard.h>
//#include <Linn/Unicode.h>
//#include <OpenHome/Ui/Drawing.h>
#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Buffer.h>
//#include <OpenHome/Ui/Glyph.h>

EXCEPTION(UnknownCharacter);

namespace OpenHome {
namespace Ui {


typedef TUint32 TUnicode; // Big endian


class Glyph;
struct GlyphMetrics;


enum EEncodingId
{
    eSymbol      = 0,
    eUnicode     = 1,
    eShiftJis    = 2,
    ePrc         = 3,
    eBig5        = 4,
    eWansung     = 5,
    eJohab       = 6,
    eUcs4        = 10
};

////////////////////////////////////////////

///\ingroup FileHandlingClasses
///\class Otb
///\brief A class which represents an open type bitmap file of type *.otb
///An Otb file may contain multiple ranges of glyphs of a particular size.
class Otb
{
public:
    ///Creates an otb file class from a Buffer
    Otb(const Brx& aFile);
    ///Populates a Glyph object with information held on a particular glyph
    ///\param aGlyph - a reference to an empty Glyph object to be populated
    ///\param aUnicode - the unicode value of the glyph we would like to find in the otb
    ///\param aPointSize - the size of glyph we would like to find in the otb
    void InitialiseGlyph(Glyph& aGlyph, TUnicode aUnicode, TUint aPointSize) const;

    static const Brn OtbTable(const Brx& aFile, const Brx& aName);
    static const Brn OtbCmapSubTable(const Brx& aCmapTable, EEncodingId aId);
    static TUint16 OtbCmapGlyphIndex(const Brn& aCmapSubTable, TUnicode aUnicode);
    static const Brn EblcBitmapTable(const Brx& aEblcTable, TUint aPointSize);
    static const Brn EblcSubTableArray(const Brx& aEblc, TUint aPointSize, TUint aGlyphIndex);

private:
    Brn iFile;
};

} // namespace Ui
} // namespace OpenHome

#endif // HEADER_OTB
