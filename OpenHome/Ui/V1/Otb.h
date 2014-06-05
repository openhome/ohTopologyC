#ifndef HEADER_OTB
#define HEADER_OTB

#include <Linn/Standard.h>
#include <Linn/Unicode.h>
#include <Linn/Ui/Drawing.h>

EXCEPTION(UnknownCharacter);

namespace Linn {
namespace Ui {

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

const Brn OtbTable(const Brx& aFile, const Brx& aName);
const Brn OtbCmapSubTable(const Brx& aCmapTable, EEncodingId aId);
TUint16 OtbCmapGlyphIndex(const Brn& aCmapSubTable, TUnicode aUnicode);
const Brn EblcBitmapTable(const Brx& aEblcTable, TUint aPointSize);
const Brn EblcSubTableArray(const Brx& aEblc, TUint aPointSize, TUint aGlyphIndex);

} // namespace Ui
} // namespace Linn

#endif // HEADER_OTB
