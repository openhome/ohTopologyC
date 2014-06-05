#include <OpenHome/Ui/Unicode.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/Private/Converter.h>

using namespace OpenHome;
using namespace OpenHome::Unicode;

Utf8::Utf8()
:iBuffer(Brx::Empty()), iIndex(0)
{
}

Utf8::Utf8(const Brx& aBuffer)
:iBuffer(aBuffer), iIndex(0)
{
}

void Utf8::Set(const Brx& aBuffer)
{
    iBuffer.Set(aBuffer);
    iIndex = 0;
}

TUnicode Utf8::Next()
{
    TUnicode ucs4 = 0;
    TUint8 firstByte = FirstByte();

    if((firstByte & kFirstByteMaskWidth1) == eOneByte)    //0x00000000 - 0x0000007f
    {                                                     //0xxxxxxx
        ucs4 = (firstByte & ~kFirstByteMaskWidth1);       //mask out first bit of byte, it is not part of the data
    }
    else
    if((firstByte & kFirstByteMaskWidth2) == eTwoBytes)   //0x00000080 - 0x000007FF
    {                                                     //110xxxxx 10xxxxxx
        ucs4 = (firstByte & ~kFirstByteMaskWidth2) << 6;  //mask out first three bits of byte, it is not part of the data
        ucs4 |= (NextByte() & ~kFollowingBytesMask);      //mask out first two bits of byte, it is not part of the data
    }
    else
    if((firstByte & kFirstByteMaskWidth3) == eThreeBytes) //0x00000800 - 0x0000D7FF and 0x0000E000 - 0x0000FFFF
    {                                                     //1110xxxx 10xxxxxx 10xxxxxx
        ucs4 = (firstByte & ~kFirstByteMaskWidth3) << 12; //mask out first three bits of byte, it is not part of the data
        ucs4 |= (NextByte() & ~kFollowingBytesMask) << 6; //mask out first two bits of byte, it is not part of the data
        ucs4 |= (NextByte() & ~kFollowingBytesMask);      //mask out first two bits of byte, it is not part of the data
    }
    else
    if((firstByte & kFirstByteMaskWidth4) == eFourBytes)   //0x00010000 - 0x0010FFFF
    {                                                      //11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        ucs4 = (firstByte & ~kFirstByteMaskWidth4) << 18;  //mask out first three bits of byte, it is not part of the data
        ucs4 |= (NextByte() & ~kFollowingBytesMask) << 12; //mask out first two bits of byte, it is not part of the data
        ucs4 |= (NextByte() & ~kFollowingBytesMask) << 6;  //mask out first two bits of byte, it is not part of the dat
        ucs4 |= (NextByte() & ~kFollowingBytesMask);       //mask out first two bits of byte, it is not part of the data
    }
    else { ASSERT(0); } // programmer error - invalid first byte should be caught and thrown

    return(ucs4);
}

TBool Utf8::End()
{
    return (iIndex == iBuffer.Bytes());
}

TByte Utf8::FirstByte()
{
    if (End()) { THROW(InvalidString); }
    TByte byte = iBuffer.At(iIndex++); // get first byte
    if (!IsValidFirst(byte)) { THROW(InvalidString); } // validate first byte

    return byte;
}

TByte Utf8::NextByte()
{
    if (End()) { THROW(InvalidString); }
    TByte byte = iBuffer.At(iIndex++); // get next byte
    if (!IsValidFollowing(byte)) { THROW(InvalidString); } // validate next byte

    return byte;
}

TBool Utf8::IsValidFirst(TByte aByte)
{
    //Binary              Hex       Width
    //00000000-01111111   0-7F      1 byte
    //11000010-11011111   C2-DF     2 bytes
    //11100000-11101111   E0-EF     3 bytes
    //11110000-11110100   F0-F4     4 bytes
    return ( (aByte <= 0x7f) || ((aByte >= 0xc2) && (aByte <= 0xf4)) );
}

TBool Utf8::IsValidFollowing(TByte aByte)
{
    // following bytes always take the form 10xx xxxx (80 - bf)
    return ( (aByte >= 0x80) && (aByte <= 0xbf) );
}

TUint Utf8::FollowingBytesExpected(TByte aByte)
{
    ASSERT(IsValidFirst(aByte));
    //Binary              Hex       Width
    //00000000-01111111   0-7F      1 byte
    //11000010-11011111   C2-DF     2 bytes
    //11100000-11101111   E0-EF     3 bytes
    //11110000-11110100   F0-F4     4 bytes
    if (aByte >= 0xf0 && aByte <= 0xf4) {
        return 3;
    }
    else if (aByte >= 0xe0 && aByte <= 0xef) {
        return 2;
    }
    else if (aByte >= 0xc2 && aByte <= 0xdf) {
        return 1;
    }
    else {
        return 0;
    }
}

TUint Utf8::MaxBufferSize(const Brx& aBuffer, TUint aMaxSizeAllowed)
{
    if (aBuffer.Bytes() <= aMaxSizeAllowed) {
        for (TUint i = (aBuffer.Bytes()-1); i >= 0; i--) {
            if (IsValidFirst(aBuffer.At(i))) {
                if ((FollowingBytesExpected(aBuffer.At(i)) + i) == (aBuffer.Bytes()-1)) {
                    return aBuffer.Bytes();
                }
                else {
                    return i;
                }
            }
        }
    }
    else {
        for (TUint i = aMaxSizeAllowed; i > 0; i--) {
            if (IsValidFirst(aBuffer.At(i))) {
                return i;
            }
        }
    }
    return 0; // probably an invalid utf8 string
}

TBool Unicode::OrderedString(const Brx& aOriginal, Bwx& aOrdered, String::Encoding aOriginalEncoding) {
    String unicodeString(aOriginal, aOriginalEncoding);
    if (unicodeString.IsRightToLeft()) {
        unicodeString.ReorderRightToLeft();
        if (aOriginalEncoding != String::eRawUnicode) {
            unicodeString.ToUtf8(aOrdered);
        }
        else {
            aOrdered.Replace(unicodeString.Buffer());
        }
        return true; // RTL
    }
    else {
        ////LOG(kSysLib, "Unicode::OrderedString bypassed --> string is LTR\n");
        aOrdered.Replace(aOriginal);
        return false; // LTR
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

String::String(Encoding aOriginalEncoding) : iBuffer(Brx::Empty()), iOriginalEncoding(aOriginalEncoding)
{
    iBuffer.SetBytes(0);
}

String::String(const Brx& aBuffer, Encoding aOriginalEncoding) : iBuffer(Brx::Empty()), iOriginalEncoding(aOriginalEncoding)
{
    iBuffer.SetBytes(0);
    Replace(aBuffer);
}

void String::Replace(const Brx& aBuffer)
{
    if (iOriginalEncoding == eRawUnicode)
    {
        iBuffer.Grow(aBuffer.Bytes()); // 4 bytes per character
        iBuffer.Replace(aBuffer);
    }
    else if (iOriginalEncoding == eLatin1)
    {
        iBuffer.Grow(aBuffer.Bytes()*4); // 1 byte per charcter -> 4 bytes per character
        iBuffer.SetBytes(0);

        for (TUint i = 0; i < aBuffer.Bytes(); i++) {
            iBuffer.Append(static_cast<TUint32>(aBuffer.At(i)));
        }
    }
    else // utf8 (default)
    {
        iBuffer.Grow(aBuffer.Bytes()*4); // max bytes is bytes * 4 (assuming each byte represented a single character)
        Utf8 utf8(aBuffer);
        iBuffer.SetBytes(0);

        while (!utf8.End())
        {
            iBuffer.Append(static_cast<TUint32>(utf8.Next()));
        }
    }
}

void String::Append(const Brx& aBuffer)
{
    if (iOriginalEncoding == eRawUnicode)
    {
        iBuffer.Grow(aBuffer.Bytes()+iBuffer.Bytes());
        iBuffer.Append(aBuffer);
    }
    else if (iOriginalEncoding == eLatin1)
    {
        iBuffer.Grow(iBuffer.Bytes()+(aBuffer.Bytes()*4));

        for (TUint i = 0; i < aBuffer.Bytes(); i++) {
            iBuffer.Append(static_cast<TUint32>(aBuffer.At(i)));
        }
    }
    else // utf8 (default)
    {
        Utf8 utf8(aBuffer);
        iBuffer.Grow(iBuffer.Bytes()+(aBuffer.Bytes()*4));
        while (!utf8.End())
        {
            iBuffer.Append(static_cast<TUint32>(utf8.Next()));
        }
    }
}

void String::Prepend(const Brx& aBuffer)
{
    Bwh bufferOriginal(iBuffer);
    if (iOriginalEncoding == eRawUnicode)
    {
        iBuffer.Grow(aBuffer.Bytes()+iBuffer.Bytes());
        iBuffer.Replace(Brx::Empty());
        iBuffer.Append(aBuffer);
    }
    else if (iOriginalEncoding == eLatin1)
    {
        iBuffer.Grow(iBuffer.Bytes()+(aBuffer.Bytes()*4));
        iBuffer.Replace(Brx::Empty());
        for (TUint i = 0; i < aBuffer.Bytes(); i++) {
            iBuffer.Append(static_cast<TUint32>(aBuffer.At(i)));
        }
    }
    else // utf8 (default)
    {
        Utf8 utf8(aBuffer);
        iBuffer.Grow(iBuffer.Bytes()+(aBuffer.Bytes()*4));
        iBuffer.Replace(Brx::Empty());
        while (!utf8.End())
        {
            iBuffer.Append(static_cast<TUint32>(utf8.Next()));
        }
    }
    iBuffer.Append(bufferOriginal);
}

void String::Shrink(TUint aCharIndex, TUint aChars)
{
    iBuffer.Replace(iBuffer.Split(aCharIndex*4, aChars*4));
}

TUnicode String::At(TUint aCharIndex) const
{
    return(Converter::BeUint32At(iBuffer, aCharIndex*4));
}

TUint String::Chars() const
{
    return(iBuffer.Bytes()/4);
}

void String::ToUtf8(Bwx& aBuffer)
{
    aBuffer.SetBytes(0);

    TUnicode unicode = 0;
    TByte byte, /*byte1,*/ byte2, byte3, byte4;
    for (TUint i = 0; i < Chars(); i++)
    {
        unicode = At(i); // four byte unicode value for each character
        //byte1 = (unicode & 0xff000000) >> 24;
        byte2 = (unicode & 0x00ff0000) >> 16;
        byte3 = (unicode & 0x0000ff00) >> 8;
        byte4 = (unicode & 0x000000ff);

        if (unicode <= 0x7f) // width = 1 (utf8 encoded)
        {
            // 00000000 00000000 00000000 0zzzzzzz -> 0zzzzzzz(00-7F)
            if (!aBuffer.TryAppend(byte4))
            {
                // if supplied buffer size not large enough for new utf8 string, return an empty string instead
                aBuffer.Replace(Brx::Empty());
            }
        }
        else if ( (unicode >= 0x80) && (unicode <= 0x7ff) ) // width = 2 (utf8 encoded)
        {
            // 00000000 00000000 00000yyy yyzzzzzz -> 110yyyyy(C2-DF) 10zzzzzz(80-BF)
            byte = 0xc0 | ( ((byte3 & 0x07) << 2) | ((byte4 & 0xc0) >> 6) );
            if (!aBuffer.TryAppend(byte))
            {
                // if supplied buffer size not large enough for new utf8 string, return an empty string instead
                aBuffer.Replace(Brx::Empty());
            }
            byte = 0x80 | ( (byte4 & 0x3f) );
            if (!aBuffer.TryAppend(byte))
            {
                // if supplied buffer size not large enough for new utf8 string, return an empty string instead
                aBuffer.Replace(Brx::Empty());
            }
        }
        else if ( ((unicode >= 0x800) && (unicode <= 0xd7ff)) || ((unicode >= 0xe000) && (unicode <= 0xffff)) ) // width = 3 (utf8 encoded)
        {
            // 00000000 00000000 xxxxyyyy yyzzzzzz -> 1110xxxx(E0-EF) 10yyyyyy 10zzzzzz
            byte = 0xe0 | ( ((byte3 & 0xf0) >> 4) );
            if (!aBuffer.TryAppend(byte))
            {
                // if supplied buffer size not large enough for new utf8 string, return an empty string instead
                aBuffer.Replace(Brx::Empty());
            }
            byte = 0x80 | ( ((byte3 & 0x0f) << 2) | ((byte4 & 0xc0) >> 6) );
            if (!aBuffer.TryAppend(byte))
            {
                // if supplied buffer size not large enough for new utf8 string, return an empty string instead
                aBuffer.Replace(Brx::Empty());
            }
            byte = 0x80 | ( (byte4 & 0x3f) );
            if (!aBuffer.TryAppend(byte))
            {
                // if supplied buffer size not large enough for new utf8 string, return an empty string instead
                aBuffer.Replace(Brx::Empty());
            }
        }
        else if ( (unicode >= 0x10000) && (unicode <= 0x10ffff) ) // width = 4 (utf8 encoded)
        {
            // 00000000 000wwwxx xxxxyyyy yyzzzzzz -> 11110www(F0-F4) 10xxxxxx 10yyyyyy 10zzzzzz
            byte = 0xf0 | ( ((byte2 & 0x1C) >> 2) );
            if (!aBuffer.TryAppend(byte))
            {
                // if supplied buffer size not large enough for new utf8 string, return an empty string instead
                aBuffer.Replace(Brx::Empty());
            }
            byte = 0x80 | ( ((byte2 & 0x03) << 4) | ((byte3 & 0xf0) >> 4) );
            if (!aBuffer.TryAppend(byte))
            {
                // if supplied buffer size not large enough for new utf8 string, return an empty string instead
                aBuffer.Replace(Brx::Empty());
            }
            byte = 0x80 | ( ((byte3 & 0x0f) << 2) | ((byte4 & 0xc0) >> 6) );
            if (!aBuffer.TryAppend(byte))
            {
                // if supplied buffer size not large enough for new utf8 string, return an empty string instead
                aBuffer.Replace(Brx::Empty());
            }
            byte = 0x80 | ( (byte4 & 0x3f) );
            if (!aBuffer.TryAppend(byte))
            {
                // if supplied buffer size not large enough for new utf8 string, return an empty string instead
                aBuffer.Replace(Brx::Empty());
            }
        }
        else { ASSERT(0); } // programmer error - construction of string should eliminate this possibility
    }
}

const Brx& String::Buffer() const
{
    return iBuffer;
}

TBool String::IsRightToLeft() {
    TUnicode unicode = 0;
    for (TUint i = 0; i < Chars(); i++)
    {
        unicode = At(i);
        if (BidiCharType(unicode) == eR || BidiCharType(unicode) == eAL) {
            ////LOG(kSysLib, "Unicode value = 0x%lx, string is treated as RTL\n", unicode);
            return true;
        }
    }
    return false;
}

void String::ReorderRightToLeft() {
    // implementation of the Unicode Bidirectional Algorithm (USA #9): http://www.unicode.org/reports/tr9/
    // bidi test tool: http://unicode.org/cldr/utility/bidi.jsp
    // Should only be called if IsRightToLeft() returns true --> Assumes RTL explicitly as base direction
    // Will only ever have two levels: L1 (RTL text) and L2 (embedded LTR text)
    // Implicit levels ignored (LRM: 0x200E, RLM: 0x200F) --> treated as ON
    // Explicit Embedding and overrides ignored (LRE: 0x202A, RLE: 0x202B, PDF: 0x202C, LRO: 0x202D, RLO: 0x202E) --> treated as ON

    ASSERT(IsRightToLeft());

    ////LOG(Debug::kSysLib|Debug::kVerbose, "Original string (HEX) = ");
    //LOG_HEX(Debug::kSysLib|Debug::kVerbose, iBuffer);
    ////LOG(Debug::kSysLib|Debug::kVerbose, "\n");

    // Parse string, get Bidi character type for each char
    Bwh typeBuffer(Brx::Empty());
    typeBuffer.Grow(Chars());
    typeBuffer.SetBytes(0);
    for (TUint i = 0; i < Chars(); i++) {
        typeBuffer.Append((TUint8)BidiCharType(At(i)));
    }

    ////LOG(Debug::kSysLib|Debug::kVerbose, "char types (HEX) = ");
    //LOG_HEX(Debug::kSysLib|Debug::kVerbose, typeBuffer);
    ////LOG(Debug::kSysLib|Debug::kVerbose, "\n");

    // Resolve weak types (3.3.3)
    // W1: Change any NSM to type of previous char, use base direction R (see above) if at start
    EBidiCharType prevType = eR;
    for (TUint i = 0; i < typeBuffer.Bytes(); i++) {
        if(typeBuffer[i] == eNSM) {
            ////LOG(Debug::kSysLib|Debug::kVerbose, "W1: NSM found (char index = %d), changed to type: %x\n", i, prevType);
            typeBuffer[i] = prevType;
        }
        prevType = (EBidiCharType)typeBuffer[i];
    }

    // W2: search backwards from any EN for R/L/AL, if AL then change EN to AN
    for (TUint i = 0; i < typeBuffer.Bytes(); i++) {
        if(typeBuffer[i] == eEN) {
            for (TInt j = (i-1); j >= 0; j--) {
                if (typeBuffer[j] == eR || typeBuffer[j] == eL) {
                    ////LOG(Debug::kSysLib|Debug::kVerbose, "W2: EN found (char index = %d), backwards search found R or L, type unchanged\n", i);
                    break;
                }
                else if (typeBuffer[j] == eAL) {
                    ////LOG(Debug::kSysLib|Debug::kVerbose, "W2: EN found (char index = %d), backwards search found AL, type changed to AN\n", i);
                    typeBuffer[i] = eAN;
                    break;
                }
            }
        }
    }

    // W3: Change all AL to R
    for (TUint i = 0; i < typeBuffer.Bytes(); i++) {
        if(typeBuffer[i] == eAL) {
            ////LOG(Debug::kSysLib|Debug::kVerbose, "W3: AL found (char index = %d), changed to R\n", i);
            typeBuffer[i] = eR;
        }
    }

    // W4: CS/ES between two EN's changes to EN, CS between two AN's changes to AN
    for (TUint i = 0; i < typeBuffer.Bytes(); i++) {
        if(typeBuffer[i] == eEN) {
            if (typeBuffer.Bytes() >= (i+3)) {
                if ((typeBuffer[i+2] == eEN) && (typeBuffer[i+1] == eCS || typeBuffer[i+1] == eES)) {
                    ////LOG(Debug::kSysLib|Debug::kVerbose, "W4: CS/ES found btw 2 EN's (char index = %d), changed to EN\n", i+1);
                    typeBuffer[i+1] = eEN;
                }
            }
        }
        else if(typeBuffer[i] == eAN) {
            if (typeBuffer.Bytes() >= (i+3)) {
                if ((typeBuffer[i+2] == eAN) && (typeBuffer[i+1] == eCS)) {
                    ////LOG(Debug::kSysLib|Debug::kVerbose, "W4: CS found btw 2 AN's (char index = %d), changed to AN\n", i+1);
                    typeBuffer[i+1] = eAN;
                }
            }
        }
    }

    // W5: any sequence of ET's adjacent to an EN changes all ET's to EN
    for (TUint i = 0; i < typeBuffer.Bytes(); i++) {
        if(typeBuffer[i] == eEN) {
            // ET's to the right
            for (TUint j = (i+1); j < typeBuffer.Bytes(); j++) {
                if (typeBuffer[j] == eET) {
                    ////LOG(Debug::kSysLib|Debug::kVerbose, "W5: ET adjacent (right) to EN (char index = %d), changed to EN\n", j);
                    typeBuffer[j] = eEN;
                }
                else {
                    break;
                }
            }
            // ET's to the left
            for (TInt k = (i-1); k >= 0; k--) {
                if (typeBuffer[k] == eET) {
                    ////LOG(Debug::kSysLib|Debug::kVerbose, "W5: ET adjacent (left) to EN (char index = %d), changed to EN\n", k);
                    typeBuffer[k] = eEN;
                }
                else {
                    break;
                }
            }
        }
    }

    // W6: Any remaining ET/ES/CS changes to ON
    for (TUint i = 0; i < typeBuffer.Bytes(); i++) {
        if (typeBuffer[i] == eET || typeBuffer[i] == eES || typeBuffer[i] == eCS) {
            ////LOG(Debug::kSysLib|Debug::kVerbose, "W6: Found remaining ET/ES/CS (char index = %d), changed to ON\n", i);
            typeBuffer[i] = eON;
        }
    }

    // W7: search backwards from any EN for R/L, if L then change EN to L
    for (TUint i = 0; i < typeBuffer.Bytes(); i++) {
        if (typeBuffer[i] == eEN) {
            for (TInt j = (i-1); j >= 0; j--) {
                if (typeBuffer[j] == eR) {
                    ////LOG(Debug::kSysLib|Debug::kVerbose, "W7: EN found (char index = %d), backwards search found R, type unchanged\n", i);
                    break;
                }
                else if (typeBuffer[j] == eL) {
                    ////LOG(Debug::kSysLib|Debug::kVerbose, "W7: EN found (char index = %d), backwards search found L, type changed to L\n", i);
                    typeBuffer[i] = eL;
                    break;
                }
            }
        }
    }

    // Resolve neutral types (3.3.4)
    // N1: Sequence of neutrals (B/S/WS/ON) surrounded by strong types change to strong (R or L) type (EN/AN behave as R for this rule)
    for (TUint i = 0; i < typeBuffer.Bytes(); i++) {
        if (typeBuffer[i] == eB || typeBuffer[i] == eS || typeBuffer[i] == eWS || typeBuffer[i] == eON) {
            // left side type (neutral(s) start index = i)
            EBidiCharType leftType = eR;
            EBidiCharType rightType = eR;
            if (i > 0) {
                leftType = (EBidiCharType)typeBuffer[i-1];
            }
            // right side type (neutral(s) end index = j)
            TUint j;
            for (j = (i+1); j < typeBuffer.Bytes(); j++) {
                // pass over other neutrals in sequence
                if (!(typeBuffer[j] == eB || typeBuffer[j] == eS || typeBuffer[j] == eWS || typeBuffer[j] == eON)) {
                    rightType = (EBidiCharType)typeBuffer[j];
                    break;
                }
            }
            // replace neutrals with strong type (L or R) if left and right match
            if (leftType == eL && rightType == eL) {
                for (TUint k = i; k < j; k++) {
                    ////LOG(Debug::kSysLib|Debug::kVerbose, "N1: neutral found (char index = %d), type changed to L\n", k);
                    typeBuffer[k] = eL;
                }
            }
            else if ((leftType == eR || leftType == eEN || leftType == eAN) && (rightType == eR || rightType == eEN || rightType == eAN)) {
                for (TUint k = i; k < j; k++) {
                    ////LOG(Debug::kSysLib|Debug::kVerbose, "N1: neutral found (char index = %d), type changed to R\n", k);
                    typeBuffer[k] = eR;
                }
            }
        }
    }

    // N2: Remaining neutrals take on embedding direction (use base direction)
    for (TUint i = 0; i < typeBuffer.Bytes(); i++) {
        if (typeBuffer[i] == eB || typeBuffer[i] == eS || typeBuffer[i] == eWS || typeBuffer[i] == eON || typeBuffer[i] == eNL) {
            ////LOG(Debug::kSysLib|Debug::kVerbose, "N2: Remaining neutral found (char index = %d), type changed to R\n", i);
            typeBuffer[i] = eR;
        }
    }

    // Reordering Resolved Levels (3.4)
    // will only have two levels, as implict and explicit markers are ignored, base direction is explicitly defined as R
    // L1 - all sequences of R chars, L2 - all sequences of L/EN/AN chars

    // split into levels for convenience
    for (TUint i = 0; i < typeBuffer.Bytes(); i++) {
        if (typeBuffer[i] == eL || typeBuffer[i] == eEN || typeBuffer[i] == eAN) {
            typeBuffer[i] = 2;
        }
        else {
            typeBuffer[i] = 1;
        }
    }

    ////LOG(Debug::kSysLib|Debug::kVerbose, "char levels = ");
    //LOG_HEX(Debug::kSysLib|Debug::kVerbose, typeBuffer);
    ////LOG(Debug::kSysLib|Debug::kVerbose, "\n");

    Bwh bufferReorder(Brx::Empty());
    bufferReorder.Grow(Chars());
    bufferReorder.SetBytes(0);

    // Step 1 - reverse each L2 block independently, L1 blocks unchanged
    for (TUint i = 0; i < typeBuffer.Bytes(); i++) {
        TUint l2Start = 0;
        TUint l2End = 0;
        if (typeBuffer[i] == 2) {
            // found start of L2 block
            ////LOG(Debug::kSysLib|Debug::kVerbose, "L2 Start found: %d\n", i);
            l2Start = i;
            l2End = i;
            for (TUint j = (i+1); j < typeBuffer.Bytes(); j++) {
                if (typeBuffer[j] == 1) {
                    // found end of L2 block
                    l2End = j-1;
                    ////LOG(Debug::kSysLib|Debug::kVerbose, "L1 Start found: %d\n", j);
                    break;
                }
                else {
                    l2End = j;
                    ////LOG(Debug::kSysLib|Debug::kVerbose, "L2 continuation found: %d\n", j);
                }
            }
            ////LOG(Debug::kSysLib|Debug::kVerbose, "L2 End found: %d\n", l2End);
            // reorder L2 block
            for (TInt k = (TInt)l2End; k >= (TInt)l2Start; k--) {
                bufferReorder.Append((TUint8)k);
                //LOG(Debug::kSysLib|Debug::kVerbose, "Reorder L2: %d\n", k);
            }
            // skip past L2 for further processing
            //LOG(Debug::kSysLib|Debug::kVerbose, "skip past L2 for further processing, i = %d\n", l2End);
            i = l2End;
        }
        else {
            // L1 block unchanged
            bufferReorder.Append((TUint8)i);
            //LOG(Debug::kSysLib|Debug::kVerbose, "L1 Unchanged: %d\n", i);
        }
    }
    //LOG(Debug::kSysLib|Debug::kVerbose, "After Step 1: order buffer (HEX) = ");
    //LOG_HEX(Debug::kSysLib|Debug::kVerbose, bufferReorder);
    //LOG(Debug::kSysLib|Debug::kVerbose, "\n");

    // Step 2 - reverse L 1-2 (complete buffer from start to finish)

    typeBuffer.Replace(Brx::Empty());
    typeBuffer.Grow(Chars());
    typeBuffer.SetBytes(0);
    for (TInt i = ((TInt)bufferReorder.Bytes() - 1);  i >= 0; i--) {
        typeBuffer.Append((TUint8)bufferReorder[i]);
    }

    //LOG(Debug::kSysLib|Debug::kVerbose, "After Step 2: order buffer (HEX) = ");
    //LOG_HEX(Debug::kSysLib|Debug::kVerbose, typeBuffer);
    //LOG(Debug::kSysLib|Debug::kVerbose, "\n");

    // Final stage - reorder the actual string

    bufferReorder.Replace(Brx::Empty());
    bufferReorder.Grow(iBuffer.Bytes());
    bufferReorder.SetBytes(0);

    for (TUint i = 0; i < typeBuffer.Bytes(); i++) {
        bufferReorder.Append(At(typeBuffer[i]));
    }

    iBuffer.Replace(bufferReorder);

    //LOG(Debug::kSysLib|Debug::kVerbose, "Final string (HEX) = ");
    //LOG_HEX(Debug::kSysLib|Debug::kVerbose, iBuffer);
    //LOG(Debug::kSysLib|Debug::kVerbose, "\n");
}

String::EBidiCharType String::BidiCharType(TUnicode aUnicode) {
    switch (aUnicode) {
        case (0x0000): { return eBN; }
        case (0x0001): { return eBN; }
        case (0x0002): { return eBN; }
        case (0x0003): { return eBN; }
        case (0x0004): { return eBN; }
        case (0x0005): { return eBN; }
        case (0x0006): { return eBN; }
        case (0x0007): { return eBN; }
        case (0x0008): { return eBN; }
        case (0x0009): { return eS; }
        case (0x000A): { return eNL; }
        case (0x000B): { return eS; }
        case (0x000C): { return eWS; }
        case (0x000D): { return eB; }
        case (0x000E): { return eBN; }
        case (0x000F): { return eBN; }
        case (0x0010): { return eBN; }
        case (0x0011): { return eBN; }
        case (0x0012): { return eBN; }
        case (0x0013): { return eBN; }
        case (0x0014): { return eBN; }
        case (0x0015): { return eBN; }
        case (0x0016): { return eBN; }
        case (0x0017): { return eBN; }
        case (0x0018): { return eBN; }
        case (0x0019): { return eBN; }
        case (0x001A): { return eBN; }
        case (0x001B): { return eBN; }
        case (0x001C): { return eB; }
        case (0x001D): { return eB; }
        case (0x001E): { return eB; }
        case (0x001F): { return eS; }
        case (0x0020): { return eWS; }
        case (0x0021): { return eON; }
        case (0x0022): { return eON; }
        case (0x0023): { return eET; }
        case (0x0024): { return eET; }
        case (0x0025): { return eET; }
        case (0x0026): { return eON; }
        case (0x0027): { return eON; }
        case (0x0028): { return eON; }
        case (0x0029): { return eON; }
        case (0x002A): { return eON; }
        case (0x002B): { return eES; }
        case (0x002C): { return eCS; }
        case (0x002D): { return eES; }
        case (0x002E): { return eCS; }
        case (0x002F): { return eCS; }
        case (0x0030): { return eEN; }
        case (0x0031): { return eEN; }
        case (0x0032): { return eEN; }
        case (0x0033): { return eEN; }
        case (0x0034): { return eEN; }
        case (0x0035): { return eEN; }
        case (0x0036): { return eEN; }
        case (0x0037): { return eEN; }
        case (0x0038): { return eEN; }
        case (0x0039): { return eEN; }
        case (0x003A): { return eCS; }
        case (0x003B): { return eON; }
        case (0x003C): { return eON; }
        case (0x003D): { return eON; }
        case (0x003E): { return eON; }
        case (0x003F): { return eON; }
        case (0x0040): { return eON; }
        case (0x005B): { return eON; }
        case (0x005C): { return eON; }
        case (0x005D): { return eON; }
        case (0x005E): { return eON; }
        case (0x005F): { return eON; }
        case (0x0060): { return eON; }
        case (0x007B): { return eON; }
        case (0x007C): { return eON; }
        case (0x007D): { return eON; }
        case (0x007E): { return eON; }
        case (0x007F): { return eBN; }
        case (0x0080): { return eBN; }
        case (0x0081): { return eBN; }
        case (0x0082): { return eBN; }
        case (0x0083): { return eBN; }
        case (0x0084): { return eBN; }
        case (0x0085): { return eB; }
        case (0x0086): { return eBN; }
        case (0x0087): { return eBN; }
        case (0x0088): { return eBN; }
        case (0x0089): { return eBN; }
        case (0x008A): { return eBN; }
        case (0x008B): { return eBN; }
        case (0x008C): { return eBN; }
        case (0x008D): { return eBN; }
        case (0x008E): { return eBN; }
        case (0x008F): { return eBN; }
        case (0x0090): { return eBN; }
        case (0x0091): { return eBN; }
        case (0x0092): { return eBN; }
        case (0x0093): { return eBN; }
        case (0x0094): { return eBN; }
        case (0x0095): { return eBN; }
        case (0x0096): { return eBN; }
        case (0x0097): { return eBN; }
        case (0x0098): { return eBN; }
        case (0x0099): { return eBN; }
        case (0x009A): { return eBN; }
        case (0x009B): { return eBN; }
        case (0x009C): { return eBN; }
        case (0x009D): { return eBN; }
        case (0x009E): { return eBN; }
        case (0x009F): { return eBN; }
        case (0x00A0): { return eCS; }
        case (0x00A1): { return eON; }
        case (0x00A2): { return eET; }
        case (0x00A3): { return eET; }
        case (0x00A4): { return eET; }
        case (0x00A5): { return eET; }
        case (0x00A6): { return eON; }
        case (0x00A7): { return eON; }
        case (0x00A8): { return eON; }
        case (0x00A9): { return eON; }
        case (0x00AB): { return eON; }
        case (0x00AC): { return eON; }
        case (0x00AD): { return eBN; }
        case (0x00AE): { return eON; }
        case (0x00AF): { return eON; }
        case (0x00B0): { return eET; }
        case (0x00B1): { return eET; }
        case (0x00B2): { return eEN; }
        case (0x00B3): { return eEN; }
        case (0x00B4): { return eON; }
        case (0x00B6): { return eON; }
        case (0x00B7): { return eON; }
        case (0x00B8): { return eON; }
        case (0x00B9): { return eEN; }
        case (0x00BB): { return eON; }
        case (0x00BC): { return eON; }
        case (0x00BD): { return eON; }
        case (0x00BE): { return eON; }
        case (0x00BF): { return eON; }
        case (0x00D7): { return eON; }
        case (0x00F7): { return eON; }
        case (0x02B9): { return eON; }
        case (0x02BA): { return eON; }
        case (0x02C2): { return eON; }
        case (0x02C3): { return eON; }
        case (0x02C4): { return eON; }
        case (0x02C5): { return eON; }
        case (0x02C6): { return eON; }
        case (0x02C7): { return eON; }
        case (0x02C8): { return eON; }
        case (0x02C9): { return eON; }
        case (0x02CA): { return eON; }
        case (0x02CB): { return eON; }
        case (0x02CC): { return eON; }
        case (0x02CD): { return eON; }
        case (0x02CE): { return eON; }
        case (0x02CF): { return eON; }
        case (0x02D2): { return eON; }
        case (0x02D3): { return eON; }
        case (0x02D4): { return eON; }
        case (0x02D5): { return eON; }
        case (0x02D6): { return eON; }
        case (0x02D7): { return eON; }
        case (0x02D8): { return eON; }
        case (0x02D9): { return eON; }
        case (0x02DA): { return eON; }
        case (0x02DB): { return eON; }
        case (0x02DC): { return eON; }
        case (0x02DD): { return eON; }
        case (0x02DE): { return eON; }
        case (0x02DF): { return eON; }
        case (0x02E5): { return eON; }
        case (0x02E6): { return eON; }
        case (0x02E7): { return eON; }
        case (0x02E8): { return eON; }
        case (0x02E9): { return eON; }
        case (0x02EA): { return eON; }
        case (0x02EB): { return eON; }
        case (0x02EC): { return eON; }
        case (0x02ED): { return eON; }
        case (0x02EF): { return eON; }
        case (0x02F0): { return eON; }
        case (0x02F1): { return eON; }
        case (0x02F2): { return eON; }
        case (0x02F3): { return eON; }
        case (0x02F4): { return eON; }
        case (0x02F5): { return eON; }
        case (0x02F6): { return eON; }
        case (0x02F7): { return eON; }
        case (0x02F8): { return eON; }
        case (0x02F9): { return eON; }
        case (0x02FA): { return eON; }
        case (0x02FB): { return eON; }
        case (0x02FC): { return eON; }
        case (0x02FD): { return eON; }
        case (0x02FE): { return eON; }
        case (0x02FF): { return eON; }
        case (0x0300): { return eNSM; }
        case (0x0301): { return eNSM; }
        case (0x0302): { return eNSM; }
        case (0x0303): { return eNSM; }
        case (0x0304): { return eNSM; }
        case (0x0305): { return eNSM; }
        case (0x0306): { return eNSM; }
        case (0x0307): { return eNSM; }
        case (0x0308): { return eNSM; }
        case (0x0309): { return eNSM; }
        case (0x030A): { return eNSM; }
        case (0x030B): { return eNSM; }
        case (0x030C): { return eNSM; }
        case (0x030D): { return eNSM; }
        case (0x030E): { return eNSM; }
        case (0x030F): { return eNSM; }
        case (0x0310): { return eNSM; }
        case (0x0311): { return eNSM; }
        case (0x0312): { return eNSM; }
        case (0x0313): { return eNSM; }
        case (0x0314): { return eNSM; }
        case (0x0315): { return eNSM; }
        case (0x0316): { return eNSM; }
        case (0x0317): { return eNSM; }
        case (0x0318): { return eNSM; }
        case (0x0319): { return eNSM; }
        case (0x031A): { return eNSM; }
        case (0x031B): { return eNSM; }
        case (0x031C): { return eNSM; }
        case (0x031D): { return eNSM; }
        case (0x031E): { return eNSM; }
        case (0x031F): { return eNSM; }
        case (0x0320): { return eNSM; }
        case (0x0321): { return eNSM; }
        case (0x0322): { return eNSM; }
        case (0x0323): { return eNSM; }
        case (0x0324): { return eNSM; }
        case (0x0325): { return eNSM; }
        case (0x0326): { return eNSM; }
        case (0x0327): { return eNSM; }
        case (0x0328): { return eNSM; }
        case (0x0329): { return eNSM; }
        case (0x032A): { return eNSM; }
        case (0x032B): { return eNSM; }
        case (0x032C): { return eNSM; }
        case (0x032D): { return eNSM; }
        case (0x032E): { return eNSM; }
        case (0x032F): { return eNSM; }
        case (0x0330): { return eNSM; }
        case (0x0331): { return eNSM; }
        case (0x0332): { return eNSM; }
        case (0x0333): { return eNSM; }
        case (0x0334): { return eNSM; }
        case (0x0335): { return eNSM; }
        case (0x0336): { return eNSM; }
        case (0x0337): { return eNSM; }
        case (0x0338): { return eNSM; }
        case (0x0339): { return eNSM; }
        case (0x033A): { return eNSM; }
        case (0x033B): { return eNSM; }
        case (0x033C): { return eNSM; }
        case (0x033D): { return eNSM; }
        case (0x033E): { return eNSM; }
        case (0x033F): { return eNSM; }
        case (0x0340): { return eNSM; }
        case (0x0341): { return eNSM; }
        case (0x0342): { return eNSM; }
        case (0x0343): { return eNSM; }
        case (0x0344): { return eNSM; }
        case (0x0345): { return eNSM; }
        case (0x0346): { return eNSM; }
        case (0x0347): { return eNSM; }
        case (0x0348): { return eNSM; }
        case (0x0349): { return eNSM; }
        case (0x034A): { return eNSM; }
        case (0x034B): { return eNSM; }
        case (0x034C): { return eNSM; }
        case (0x034D): { return eNSM; }
        case (0x034E): { return eNSM; }
        case (0x034F): { return eNSM; }
        case (0x0350): { return eNSM; }
        case (0x0351): { return eNSM; }
        case (0x0352): { return eNSM; }
        case (0x0353): { return eNSM; }
        case (0x0354): { return eNSM; }
        case (0x0355): { return eNSM; }
        case (0x0356): { return eNSM; }
        case (0x0357): { return eNSM; }
        case (0x0358): { return eNSM; }
        case (0x0359): { return eNSM; }
        case (0x035A): { return eNSM; }
        case (0x035B): { return eNSM; }
        case (0x035C): { return eNSM; }
        case (0x035D): { return eNSM; }
        case (0x035E): { return eNSM; }
        case (0x035F): { return eNSM; }
        case (0x0360): { return eNSM; }
        case (0x0361): { return eNSM; }
        case (0x0362): { return eNSM; }
        case (0x0363): { return eNSM; }
        case (0x0364): { return eNSM; }
        case (0x0365): { return eNSM; }
        case (0x0366): { return eNSM; }
        case (0x0367): { return eNSM; }
        case (0x0368): { return eNSM; }
        case (0x0369): { return eNSM; }
        case (0x036A): { return eNSM; }
        case (0x036B): { return eNSM; }
        case (0x036C): { return eNSM; }
        case (0x036D): { return eNSM; }
        case (0x036E): { return eNSM; }
        case (0x036F): { return eNSM; }
        case (0x0374): { return eON; }
        case (0x0375): { return eON; }
        case (0x037E): { return eON; }
        case (0x0384): { return eON; }
        case (0x0385): { return eON; }
        case (0x0387): { return eON; }
        case (0x03F6): { return eON; }
        case (0x0483): { return eNSM; }
        case (0x0484): { return eNSM; }
        case (0x0485): { return eNSM; }
        case (0x0486): { return eNSM; }
        case (0x0487): { return eNSM; }
        case (0x0488): { return eNSM; }
        case (0x0489): { return eNSM; }
        case (0x058A): { return eON; }
        case (0x0591): { return eNSM; }
        case (0x0592): { return eNSM; }
        case (0x0593): { return eNSM; }
        case (0x0594): { return eNSM; }
        case (0x0595): { return eNSM; }
        case (0x0596): { return eNSM; }
        case (0x0597): { return eNSM; }
        case (0x0598): { return eNSM; }
        case (0x0599): { return eNSM; }
        case (0x059A): { return eNSM; }
        case (0x059B): { return eNSM; }
        case (0x059C): { return eNSM; }
        case (0x059D): { return eNSM; }
        case (0x059E): { return eNSM; }
        case (0x059F): { return eNSM; }
        case (0x05A0): { return eNSM; }
        case (0x05A1): { return eNSM; }
        case (0x05A2): { return eNSM; }
        case (0x05A3): { return eNSM; }
        case (0x05A4): { return eNSM; }
        case (0x05A5): { return eNSM; }
        case (0x05A6): { return eNSM; }
        case (0x05A7): { return eNSM; }
        case (0x05A8): { return eNSM; }
        case (0x05A9): { return eNSM; }
        case (0x05AA): { return eNSM; }
        case (0x05AB): { return eNSM; }
        case (0x05AC): { return eNSM; }
        case (0x05AD): { return eNSM; }
        case (0x05AE): { return eNSM; }
        case (0x05AF): { return eNSM; }
        case (0x05B0): { return eNSM; }
        case (0x05B1): { return eNSM; }
        case (0x05B2): { return eNSM; }
        case (0x05B3): { return eNSM; }
        case (0x05B4): { return eNSM; }
        case (0x05B5): { return eNSM; }
        case (0x05B6): { return eNSM; }
        case (0x05B7): { return eNSM; }
        case (0x05B8): { return eNSM; }
        case (0x05B9): { return eNSM; }
        case (0x05BA): { return eNSM; }
        case (0x05BB): { return eNSM; }
        case (0x05BC): { return eNSM; }
        case (0x05BD): { return eNSM; }
        case (0x05BE): { return eR; }
        case (0x05BF): { return eNSM; }
        case (0x05C0): { return eR; }
        case (0x05C1): { return eNSM; }
        case (0x05C2): { return eNSM; }
        case (0x05C3): { return eR; }
        case (0x05C4): { return eNSM; }
        case (0x05C5): { return eNSM; }
        case (0x05C6): { return eR; }
        case (0x05C7): { return eNSM; }
        case (0x05D0): { return eR; }
        case (0x05D1): { return eR; }
        case (0x05D2): { return eR; }
        case (0x05D3): { return eR; }
        case (0x05D4): { return eR; }
        case (0x05D5): { return eR; }
        case (0x05D6): { return eR; }
        case (0x05D7): { return eR; }
        case (0x05D8): { return eR; }
        case (0x05D9): { return eR; }
        case (0x05DA): { return eR; }
        case (0x05DB): { return eR; }
        case (0x05DC): { return eR; }
        case (0x05DD): { return eR; }
        case (0x05DE): { return eR; }
        case (0x05DF): { return eR; }
        case (0x05E0): { return eR; }
        case (0x05E1): { return eR; }
        case (0x05E2): { return eR; }
        case (0x05E3): { return eR; }
        case (0x05E4): { return eR; }
        case (0x05E5): { return eR; }
        case (0x05E6): { return eR; }
        case (0x05E7): { return eR; }
        case (0x05E8): { return eR; }
        case (0x05E9): { return eR; }
        case (0x05EA): { return eR; }
        case (0x05F0): { return eR; }
        case (0x05F1): { return eR; }
        case (0x05F2): { return eR; }
        case (0x05F3): { return eR; }
        case (0x05F4): { return eR; }
        case (0x0600): { return eAN; }
        case (0x0601): { return eAN; }
        case (0x0602): { return eAN; }
        case (0x0603): { return eAN; }
        case (0x0606): { return eON; }
        case (0x0607): { return eON; }
        case (0x0608): { return eAL; }
        case (0x0609): { return eET; }
        case (0x060A): { return eET; }
        case (0x060B): { return eAL; }
        case (0x060C): { return eCS; }
        case (0x060D): { return eAL; }
        case (0x060E): { return eON; }
        case (0x060F): { return eON; }
        case (0x0610): { return eNSM; }
        case (0x0611): { return eNSM; }
        case (0x0612): { return eNSM; }
        case (0x0613): { return eNSM; }
        case (0x0614): { return eNSM; }
        case (0x0615): { return eNSM; }
        case (0x0616): { return eNSM; }
        case (0x0617): { return eNSM; }
        case (0x0618): { return eNSM; }
        case (0x0619): { return eNSM; }
        case (0x061A): { return eNSM; }
        case (0x061B): { return eAL; }
        case (0x061E): { return eAL; }
        case (0x061F): { return eAL; }
        case (0x0620): { return eAL; }
        case (0x0621): { return eAL; }
        case (0x0622): { return eAL; }
        case (0x0623): { return eAL; }
        case (0x0624): { return eAL; }
        case (0x0625): { return eAL; }
        case (0x0626): { return eAL; }
        case (0x0627): { return eAL; }
        case (0x0628): { return eAL; }
        case (0x0629): { return eAL; }
        case (0x062A): { return eAL; }
        case (0x062B): { return eAL; }
        case (0x062C): { return eAL; }
        case (0x062D): { return eAL; }
        case (0x062E): { return eAL; }
        case (0x062F): { return eAL; }
        case (0x0630): { return eAL; }
        case (0x0631): { return eAL; }
        case (0x0632): { return eAL; }
        case (0x0633): { return eAL; }
        case (0x0634): { return eAL; }
        case (0x0635): { return eAL; }
        case (0x0636): { return eAL; }
        case (0x0637): { return eAL; }
        case (0x0638): { return eAL; }
        case (0x0639): { return eAL; }
        case (0x063A): { return eAL; }
        case (0x063B): { return eAL; }
        case (0x063C): { return eAL; }
        case (0x063D): { return eAL; }
        case (0x063E): { return eAL; }
        case (0x063F): { return eAL; }
        case (0x0640): { return eAL; }
        case (0x0641): { return eAL; }
        case (0x0642): { return eAL; }
        case (0x0643): { return eAL; }
        case (0x0644): { return eAL; }
        case (0x0645): { return eAL; }
        case (0x0646): { return eAL; }
        case (0x0647): { return eAL; }
        case (0x0648): { return eAL; }
        case (0x0649): { return eAL; }
        case (0x064A): { return eAL; }
        case (0x064B): { return eNSM; }
        case (0x064C): { return eNSM; }
        case (0x064D): { return eNSM; }
        case (0x064E): { return eNSM; }
        case (0x064F): { return eNSM; }
        case (0x0650): { return eNSM; }
        case (0x0651): { return eNSM; }
        case (0x0652): { return eNSM; }
        case (0x0653): { return eNSM; }
        case (0x0654): { return eNSM; }
        case (0x0655): { return eNSM; }
        case (0x0656): { return eNSM; }
        case (0x0657): { return eNSM; }
        case (0x0658): { return eNSM; }
        case (0x0659): { return eNSM; }
        case (0x065A): { return eNSM; }
        case (0x065B): { return eNSM; }
        case (0x065C): { return eNSM; }
        case (0x065D): { return eNSM; }
        case (0x065E): { return eNSM; }
        case (0x065F): { return eNSM; }
        case (0x0660): { return eAN; }
        case (0x0661): { return eAN; }
        case (0x0662): { return eAN; }
        case (0x0663): { return eAN; }
        case (0x0664): { return eAN; }
        case (0x0665): { return eAN; }
        case (0x0666): { return eAN; }
        case (0x0667): { return eAN; }
        case (0x0668): { return eAN; }
        case (0x0669): { return eAN; }
        case (0x066A): { return eET; }
        case (0x066B): { return eAN; }
        case (0x066C): { return eAN; }
        case (0x066D): { return eAL; }
        case (0x066E): { return eAL; }
        case (0x066F): { return eAL; }
        case (0x0670): { return eNSM; }
        case (0x0671): { return eAL; }
        case (0x0672): { return eAL; }
        case (0x0673): { return eAL; }
        case (0x0674): { return eAL; }
        case (0x0675): { return eAL; }
        case (0x0676): { return eAL; }
        case (0x0677): { return eAL; }
        case (0x0678): { return eAL; }
        case (0x0679): { return eAL; }
        case (0x067A): { return eAL; }
        case (0x067B): { return eAL; }
        case (0x067C): { return eAL; }
        case (0x067D): { return eAL; }
        case (0x067E): { return eAL; }
        case (0x067F): { return eAL; }
        case (0x0680): { return eAL; }
        case (0x0681): { return eAL; }
        case (0x0682): { return eAL; }
        case (0x0683): { return eAL; }
        case (0x0684): { return eAL; }
        case (0x0685): { return eAL; }
        case (0x0686): { return eAL; }
        case (0x0687): { return eAL; }
        case (0x0688): { return eAL; }
        case (0x0689): { return eAL; }
        case (0x068A): { return eAL; }
        case (0x068B): { return eAL; }
        case (0x068C): { return eAL; }
        case (0x068D): { return eAL; }
        case (0x068E): { return eAL; }
        case (0x068F): { return eAL; }
        case (0x0690): { return eAL; }
        case (0x0691): { return eAL; }
        case (0x0692): { return eAL; }
        case (0x0693): { return eAL; }
        case (0x0694): { return eAL; }
        case (0x0695): { return eAL; }
        case (0x0696): { return eAL; }
        case (0x0697): { return eAL; }
        case (0x0698): { return eAL; }
        case (0x0699): { return eAL; }
        case (0x069A): { return eAL; }
        case (0x069B): { return eAL; }
        case (0x069C): { return eAL; }
        case (0x069D): { return eAL; }
        case (0x069E): { return eAL; }
        case (0x069F): { return eAL; }
        case (0x06A0): { return eAL; }
        case (0x06A1): { return eAL; }
        case (0x06A2): { return eAL; }
        case (0x06A3): { return eAL; }
        case (0x06A4): { return eAL; }
        case (0x06A5): { return eAL; }
        case (0x06A6): { return eAL; }
        case (0x06A7): { return eAL; }
        case (0x06A8): { return eAL; }
        case (0x06A9): { return eAL; }
        case (0x06AA): { return eAL; }
        case (0x06AB): { return eAL; }
        case (0x06AC): { return eAL; }
        case (0x06AD): { return eAL; }
        case (0x06AE): { return eAL; }
        case (0x06AF): { return eAL; }
        case (0x06B0): { return eAL; }
        case (0x06B1): { return eAL; }
        case (0x06B2): { return eAL; }
        case (0x06B3): { return eAL; }
        case (0x06B4): { return eAL; }
        case (0x06B5): { return eAL; }
        case (0x06B6): { return eAL; }
        case (0x06B7): { return eAL; }
        case (0x06B8): { return eAL; }
        case (0x06B9): { return eAL; }
        case (0x06BA): { return eAL; }
        case (0x06BB): { return eAL; }
        case (0x06BC): { return eAL; }
        case (0x06BD): { return eAL; }
        case (0x06BE): { return eAL; }
        case (0x06BF): { return eAL; }
        case (0x06C0): { return eAL; }
        case (0x06C1): { return eAL; }
        case (0x06C2): { return eAL; }
        case (0x06C3): { return eAL; }
        case (0x06C4): { return eAL; }
        case (0x06C5): { return eAL; }
        case (0x06C6): { return eAL; }
        case (0x06C7): { return eAL; }
        case (0x06C8): { return eAL; }
        case (0x06C9): { return eAL; }
        case (0x06CA): { return eAL; }
        case (0x06CB): { return eAL; }
        case (0x06CC): { return eAL; }
        case (0x06CD): { return eAL; }
        case (0x06CE): { return eAL; }
        case (0x06CF): { return eAL; }
        case (0x06D0): { return eAL; }
        case (0x06D1): { return eAL; }
        case (0x06D2): { return eAL; }
        case (0x06D3): { return eAL; }
        case (0x06D4): { return eAL; }
        case (0x06D5): { return eAL; }
        case (0x06D6): { return eNSM; }
        case (0x06D7): { return eNSM; }
        case (0x06D8): { return eNSM; }
        case (0x06D9): { return eNSM; }
        case (0x06DA): { return eNSM; }
        case (0x06DB): { return eNSM; }
        case (0x06DC): { return eNSM; }
        case (0x06DD): { return eAN; }
        case (0x06DE): { return eON; }
        case (0x06DF): { return eNSM; }
        case (0x06E0): { return eNSM; }
        case (0x06E1): { return eNSM; }
        case (0x06E2): { return eNSM; }
        case (0x06E3): { return eNSM; }
        case (0x06E4): { return eNSM; }
        case (0x06E5): { return eAL; }
        case (0x06E6): { return eAL; }
        case (0x06E7): { return eNSM; }
        case (0x06E8): { return eNSM; }
        case (0x06E9): { return eON; }
        case (0x06EA): { return eNSM; }
        case (0x06EB): { return eNSM; }
        case (0x06EC): { return eNSM; }
        case (0x06ED): { return eNSM; }
        case (0x06EE): { return eAL; }
        case (0x06EF): { return eAL; }
        case (0x06F0): { return eEN; }
        case (0x06F1): { return eEN; }
        case (0x06F2): { return eEN; }
        case (0x06F3): { return eEN; }
        case (0x06F4): { return eEN; }
        case (0x06F5): { return eEN; }
        case (0x06F6): { return eEN; }
        case (0x06F7): { return eEN; }
        case (0x06F8): { return eEN; }
        case (0x06F9): { return eEN; }
        case (0x06FA): { return eAL; }
        case (0x06FB): { return eAL; }
        case (0x06FC): { return eAL; }
        case (0x06FD): { return eAL; }
        case (0x06FE): { return eAL; }
        case (0x06FF): { return eAL; }
        case (0x0700): { return eAL; }
        case (0x0701): { return eAL; }
        case (0x0702): { return eAL; }
        case (0x0703): { return eAL; }
        case (0x0704): { return eAL; }
        case (0x0705): { return eAL; }
        case (0x0706): { return eAL; }
        case (0x0707): { return eAL; }
        case (0x0708): { return eAL; }
        case (0x0709): { return eAL; }
        case (0x070A): { return eAL; }
        case (0x070B): { return eAL; }
        case (0x070C): { return eAL; }
        case (0x070D): { return eAL; }
        case (0x070F): { return eAN; }
        case (0x0710): { return eAL; }
        case (0x0711): { return eNSM; }
        case (0x0712): { return eAL; }
        case (0x0713): { return eAL; }
        case (0x0714): { return eAL; }
        case (0x0715): { return eAL; }
        case (0x0716): { return eAL; }
        case (0x0717): { return eAL; }
        case (0x0718): { return eAL; }
        case (0x0719): { return eAL; }
        case (0x071A): { return eAL; }
        case (0x071B): { return eAL; }
        case (0x071C): { return eAL; }
        case (0x071D): { return eAL; }
        case (0x071E): { return eAL; }
        case (0x071F): { return eAL; }
        case (0x0720): { return eAL; }
        case (0x0721): { return eAL; }
        case (0x0722): { return eAL; }
        case (0x0723): { return eAL; }
        case (0x0724): { return eAL; }
        case (0x0725): { return eAL; }
        case (0x0726): { return eAL; }
        case (0x0727): { return eAL; }
        case (0x0728): { return eAL; }
        case (0x0729): { return eAL; }
        case (0x072A): { return eAL; }
        case (0x072B): { return eAL; }
        case (0x072C): { return eAL; }
        case (0x072D): { return eAL; }
        case (0x072E): { return eAL; }
        case (0x072F): { return eAL; }
        case (0x0730): { return eNSM; }
        case (0x0731): { return eNSM; }
        case (0x0732): { return eNSM; }
        case (0x0733): { return eNSM; }
        case (0x0734): { return eNSM; }
        case (0x0735): { return eNSM; }
        case (0x0736): { return eNSM; }
        case (0x0737): { return eNSM; }
        case (0x0738): { return eNSM; }
        case (0x0739): { return eNSM; }
        case (0x073A): { return eNSM; }
        case (0x073B): { return eNSM; }
        case (0x073C): { return eNSM; }
        case (0x073D): { return eNSM; }
        case (0x073E): { return eNSM; }
        case (0x073F): { return eNSM; }
        case (0x0740): { return eNSM; }
        case (0x0741): { return eNSM; }
        case (0x0742): { return eNSM; }
        case (0x0743): { return eNSM; }
        case (0x0744): { return eNSM; }
        case (0x0745): { return eNSM; }
        case (0x0746): { return eNSM; }
        case (0x0747): { return eNSM; }
        case (0x0748): { return eNSM; }
        case (0x0749): { return eNSM; }
        case (0x074A): { return eNSM; }
        case (0x074D): { return eAL; }
        case (0x074E): { return eAL; }
        case (0x074F): { return eAL; }
        case (0x0750): { return eAL; }
        case (0x0751): { return eAL; }
        case (0x0752): { return eAL; }
        case (0x0753): { return eAL; }
        case (0x0754): { return eAL; }
        case (0x0755): { return eAL; }
        case (0x0756): { return eAL; }
        case (0x0757): { return eAL; }
        case (0x0758): { return eAL; }
        case (0x0759): { return eAL; }
        case (0x075A): { return eAL; }
        case (0x075B): { return eAL; }
        case (0x075C): { return eAL; }
        case (0x075D): { return eAL; }
        case (0x075E): { return eAL; }
        case (0x075F): { return eAL; }
        case (0x0760): { return eAL; }
        case (0x0761): { return eAL; }
        case (0x0762): { return eAL; }
        case (0x0763): { return eAL; }
        case (0x0764): { return eAL; }
        case (0x0765): { return eAL; }
        case (0x0766): { return eAL; }
        case (0x0767): { return eAL; }
        case (0x0768): { return eAL; }
        case (0x0769): { return eAL; }
        case (0x076A): { return eAL; }
        case (0x076B): { return eAL; }
        case (0x076C): { return eAL; }
        case (0x076D): { return eAL; }
        case (0x076E): { return eAL; }
        case (0x076F): { return eAL; }
        case (0x0770): { return eAL; }
        case (0x0771): { return eAL; }
        case (0x0772): { return eAL; }
        case (0x0773): { return eAL; }
        case (0x0774): { return eAL; }
        case (0x0775): { return eAL; }
        case (0x0776): { return eAL; }
        case (0x0777): { return eAL; }
        case (0x0778): { return eAL; }
        case (0x0779): { return eAL; }
        case (0x077A): { return eAL; }
        case (0x077B): { return eAL; }
        case (0x077C): { return eAL; }
        case (0x077D): { return eAL; }
        case (0x077E): { return eAL; }
        case (0x077F): { return eAL; }
        case (0x0780): { return eAL; }
        case (0x0781): { return eAL; }
        case (0x0782): { return eAL; }
        case (0x0783): { return eAL; }
        case (0x0784): { return eAL; }
        case (0x0785): { return eAL; }
        case (0x0786): { return eAL; }
        case (0x0787): { return eAL; }
        case (0x0788): { return eAL; }
        case (0x0789): { return eAL; }
        case (0x078A): { return eAL; }
        case (0x078B): { return eAL; }
        case (0x078C): { return eAL; }
        case (0x078D): { return eAL; }
        case (0x078E): { return eAL; }
        case (0x078F): { return eAL; }
        case (0x0790): { return eAL; }
        case (0x0791): { return eAL; }
        case (0x0792): { return eAL; }
        case (0x0793): { return eAL; }
        case (0x0794): { return eAL; }
        case (0x0795): { return eAL; }
        case (0x0796): { return eAL; }
        case (0x0797): { return eAL; }
        case (0x0798): { return eAL; }
        case (0x0799): { return eAL; }
        case (0x079A): { return eAL; }
        case (0x079B): { return eAL; }
        case (0x079C): { return eAL; }
        case (0x079D): { return eAL; }
        case (0x079E): { return eAL; }
        case (0x079F): { return eAL; }
        case (0x07A0): { return eAL; }
        case (0x07A1): { return eAL; }
        case (0x07A2): { return eAL; }
        case (0x07A3): { return eAL; }
        case (0x07A4): { return eAL; }
        case (0x07A5): { return eAL; }
        case (0x07A6): { return eNSM; }
        case (0x07A7): { return eNSM; }
        case (0x07A8): { return eNSM; }
        case (0x07A9): { return eNSM; }
        case (0x07AA): { return eNSM; }
        case (0x07AB): { return eNSM; }
        case (0x07AC): { return eNSM; }
        case (0x07AD): { return eNSM; }
        case (0x07AE): { return eNSM; }
        case (0x07AF): { return eNSM; }
        case (0x07B0): { return eNSM; }
        case (0x07B1): { return eAL; }
        case (0x07C0): { return eR; }
        case (0x07C1): { return eR; }
        case (0x07C2): { return eR; }
        case (0x07C3): { return eR; }
        case (0x07C4): { return eR; }
        case (0x07C5): { return eR; }
        case (0x07C6): { return eR; }
        case (0x07C7): { return eR; }
        case (0x07C8): { return eR; }
        case (0x07C9): { return eR; }
        case (0x07CA): { return eR; }
        case (0x07CB): { return eR; }
        case (0x07CC): { return eR; }
        case (0x07CD): { return eR; }
        case (0x07CE): { return eR; }
        case (0x07CF): { return eR; }
        case (0x07D0): { return eR; }
        case (0x07D1): { return eR; }
        case (0x07D2): { return eR; }
        case (0x07D3): { return eR; }
        case (0x07D4): { return eR; }
        case (0x07D5): { return eR; }
        case (0x07D6): { return eR; }
        case (0x07D7): { return eR; }
        case (0x07D8): { return eR; }
        case (0x07D9): { return eR; }
        case (0x07DA): { return eR; }
        case (0x07DB): { return eR; }
        case (0x07DC): { return eR; }
        case (0x07DD): { return eR; }
        case (0x07DE): { return eR; }
        case (0x07DF): { return eR; }
        case (0x07E0): { return eR; }
        case (0x07E1): { return eR; }
        case (0x07E2): { return eR; }
        case (0x07E3): { return eR; }
        case (0x07E4): { return eR; }
        case (0x07E5): { return eR; }
        case (0x07E6): { return eR; }
        case (0x07E7): { return eR; }
        case (0x07E8): { return eR; }
        case (0x07E9): { return eR; }
        case (0x07EA): { return eR; }
        case (0x07EB): { return eNSM; }
        case (0x07EC): { return eNSM; }
        case (0x07ED): { return eNSM; }
        case (0x07EE): { return eNSM; }
        case (0x07EF): { return eNSM; }
        case (0x07F0): { return eNSM; }
        case (0x07F1): { return eNSM; }
        case (0x07F2): { return eNSM; }
        case (0x07F3): { return eNSM; }
        case (0x07F4): { return eR; }
        case (0x07F5): { return eR; }
        case (0x07F6): { return eON; }
        case (0x07F7): { return eON; }
        case (0x07F8): { return eON; }
        case (0x07F9): { return eON; }
        case (0x07FA): { return eR; }
        case (0x0800): { return eR; }
        case (0x0801): { return eR; }
        case (0x0802): { return eR; }
        case (0x0803): { return eR; }
        case (0x0804): { return eR; }
        case (0x0805): { return eR; }
        case (0x0806): { return eR; }
        case (0x0807): { return eR; }
        case (0x0808): { return eR; }
        case (0x0809): { return eR; }
        case (0x080A): { return eR; }
        case (0x080B): { return eR; }
        case (0x080C): { return eR; }
        case (0x080D): { return eR; }
        case (0x080E): { return eR; }
        case (0x080F): { return eR; }
        case (0x0810): { return eR; }
        case (0x0811): { return eR; }
        case (0x0812): { return eR; }
        case (0x0813): { return eR; }
        case (0x0814): { return eR; }
        case (0x0815): { return eR; }
        case (0x0816): { return eNSM; }
        case (0x0817): { return eNSM; }
        case (0x0818): { return eNSM; }
        case (0x0819): { return eNSM; }
        case (0x081A): { return eR; }
        case (0x081B): { return eNSM; }
        case (0x081C): { return eNSM; }
        case (0x081D): { return eNSM; }
        case (0x081E): { return eNSM; }
        case (0x081F): { return eNSM; }
        case (0x0820): { return eNSM; }
        case (0x0821): { return eNSM; }
        case (0x0822): { return eNSM; }
        case (0x0823): { return eNSM; }
        case (0x0824): { return eR; }
        case (0x0825): { return eNSM; }
        case (0x0826): { return eNSM; }
        case (0x0827): { return eNSM; }
        case (0x0828): { return eR; }
        case (0x0829): { return eNSM; }
        case (0x082A): { return eNSM; }
        case (0x082B): { return eNSM; }
        case (0x082C): { return eNSM; }
        case (0x082D): { return eNSM; }
        case (0x0830): { return eR; }
        case (0x0831): { return eR; }
        case (0x0832): { return eR; }
        case (0x0833): { return eR; }
        case (0x0834): { return eR; }
        case (0x0835): { return eR; }
        case (0x0836): { return eR; }
        case (0x0837): { return eR; }
        case (0x0838): { return eR; }
        case (0x0839): { return eR; }
        case (0x083A): { return eR; }
        case (0x083B): { return eR; }
        case (0x083C): { return eR; }
        case (0x083D): { return eR; }
        case (0x083E): { return eR; }
        case (0x0840): { return eR; }
        case (0x0841): { return eR; }
        case (0x0842): { return eR; }
        case (0x0843): { return eR; }
        case (0x0844): { return eR; }
        case (0x0845): { return eR; }
        case (0x0846): { return eR; }
        case (0x0847): { return eR; }
        case (0x0848): { return eR; }
        case (0x0849): { return eR; }
        case (0x084A): { return eR; }
        case (0x084B): { return eR; }
        case (0x084C): { return eR; }
        case (0x084D): { return eR; }
        case (0x084E): { return eR; }
        case (0x084F): { return eR; }
        case (0x0850): { return eR; }
        case (0x0851): { return eR; }
        case (0x0852): { return eR; }
        case (0x0853): { return eR; }
        case (0x0854): { return eR; }
        case (0x0855): { return eR; }
        case (0x0856): { return eR; }
        case (0x0857): { return eR; }
        case (0x0858): { return eR; }
        case (0x0859): { return eNSM; }
        case (0x085A): { return eNSM; }
        case (0x085B): { return eNSM; }
        case (0x085E): { return eR; }
        case (0x0900): { return eNSM; }
        case (0x0901): { return eNSM; }
        case (0x0902): { return eNSM; }
        case (0x093A): { return eNSM; }
        case (0x093C): { return eNSM; }
        case (0x0941): { return eNSM; }
        case (0x0942): { return eNSM; }
        case (0x0943): { return eNSM; }
        case (0x0944): { return eNSM; }
        case (0x0945): { return eNSM; }
        case (0x0946): { return eNSM; }
        case (0x0947): { return eNSM; }
        case (0x0948): { return eNSM; }
        case (0x094D): { return eNSM; }
        case (0x0951): { return eNSM; }
        case (0x0952): { return eNSM; }
        case (0x0953): { return eNSM; }
        case (0x0954): { return eNSM; }
        case (0x0955): { return eNSM; }
        case (0x0956): { return eNSM; }
        case (0x0957): { return eNSM; }
        case (0x0962): { return eNSM; }
        case (0x0963): { return eNSM; }
        case (0x0981): { return eNSM; }
        case (0x09BC): { return eNSM; }
        case (0x09C1): { return eNSM; }
        case (0x09C2): { return eNSM; }
        case (0x09C3): { return eNSM; }
        case (0x09C4): { return eNSM; }
        case (0x09CD): { return eNSM; }
        case (0x09E2): { return eNSM; }
        case (0x09E3): { return eNSM; }
        case (0x09F2): { return eET; }
        case (0x09F3): { return eET; }
        case (0x09FB): { return eET; }
        case (0x0A01): { return eNSM; }
        case (0x0A02): { return eNSM; }
        case (0x0A3C): { return eNSM; }
        case (0x0A41): { return eNSM; }
        case (0x0A42): { return eNSM; }
        case (0x0A47): { return eNSM; }
        case (0x0A48): { return eNSM; }
        case (0x0A4B): { return eNSM; }
        case (0x0A4C): { return eNSM; }
        case (0x0A4D): { return eNSM; }
        case (0x0A51): { return eNSM; }
        case (0x0A70): { return eNSM; }
        case (0x0A71): { return eNSM; }
        case (0x0A75): { return eNSM; }
        case (0x0A81): { return eNSM; }
        case (0x0A82): { return eNSM; }
        case (0x0ABC): { return eNSM; }
        case (0x0AC1): { return eNSM; }
        case (0x0AC2): { return eNSM; }
        case (0x0AC3): { return eNSM; }
        case (0x0AC4): { return eNSM; }
        case (0x0AC5): { return eNSM; }
        case (0x0AC7): { return eNSM; }
        case (0x0AC8): { return eNSM; }
        case (0x0ACD): { return eNSM; }
        case (0x0AE2): { return eNSM; }
        case (0x0AE3): { return eNSM; }
        case (0x0AF1): { return eET; }
        case (0x0B01): { return eNSM; }
        case (0x0B3C): { return eNSM; }
        case (0x0B3F): { return eNSM; }
        case (0x0B41): { return eNSM; }
        case (0x0B42): { return eNSM; }
        case (0x0B43): { return eNSM; }
        case (0x0B44): { return eNSM; }
        case (0x0B4D): { return eNSM; }
        case (0x0B56): { return eNSM; }
        case (0x0B62): { return eNSM; }
        case (0x0B63): { return eNSM; }
        case (0x0B82): { return eNSM; }
        case (0x0BC0): { return eNSM; }
        case (0x0BCD): { return eNSM; }
        case (0x0BF3): { return eON; }
        case (0x0BF4): { return eON; }
        case (0x0BF5): { return eON; }
        case (0x0BF6): { return eON; }
        case (0x0BF7): { return eON; }
        case (0x0BF8): { return eON; }
        case (0x0BF9): { return eET; }
        case (0x0BFA): { return eON; }
        case (0x0C3E): { return eNSM; }
        case (0x0C3F): { return eNSM; }
        case (0x0C40): { return eNSM; }
        case (0x0C46): { return eNSM; }
        case (0x0C47): { return eNSM; }
        case (0x0C48): { return eNSM; }
        case (0x0C4A): { return eNSM; }
        case (0x0C4B): { return eNSM; }
        case (0x0C4C): { return eNSM; }
        case (0x0C4D): { return eNSM; }
        case (0x0C55): { return eNSM; }
        case (0x0C56): { return eNSM; }
        case (0x0C62): { return eNSM; }
        case (0x0C63): { return eNSM; }
        case (0x0C78): { return eON; }
        case (0x0C79): { return eON; }
        case (0x0C7A): { return eON; }
        case (0x0C7B): { return eON; }
        case (0x0C7C): { return eON; }
        case (0x0C7D): { return eON; }
        case (0x0C7E): { return eON; }
        case (0x0CBC): { return eNSM; }
        case (0x0CCC): { return eNSM; }
        case (0x0CCD): { return eNSM; }
        case (0x0CE2): { return eNSM; }
        case (0x0CE3): { return eNSM; }
        case (0x0D41): { return eNSM; }
        case (0x0D42): { return eNSM; }
        case (0x0D43): { return eNSM; }
        case (0x0D44): { return eNSM; }
        case (0x0D4D): { return eNSM; }
        case (0x0D62): { return eNSM; }
        case (0x0D63): { return eNSM; }
        case (0x0DCA): { return eNSM; }
        case (0x0DD2): { return eNSM; }
        case (0x0DD3): { return eNSM; }
        case (0x0DD4): { return eNSM; }
        case (0x0DD6): { return eNSM; }
        case (0x0E31): { return eNSM; }
        case (0x0E34): { return eNSM; }
        case (0x0E35): { return eNSM; }
        case (0x0E36): { return eNSM; }
        case (0x0E37): { return eNSM; }
        case (0x0E38): { return eNSM; }
        case (0x0E39): { return eNSM; }
        case (0x0E3A): { return eNSM; }
        case (0x0E3F): { return eET; }
        case (0x0E47): { return eNSM; }
        case (0x0E48): { return eNSM; }
        case (0x0E49): { return eNSM; }
        case (0x0E4A): { return eNSM; }
        case (0x0E4B): { return eNSM; }
        case (0x0E4C): { return eNSM; }
        case (0x0E4D): { return eNSM; }
        case (0x0E4E): { return eNSM; }
        case (0x0EB1): { return eNSM; }
        case (0x0EB4): { return eNSM; }
        case (0x0EB5): { return eNSM; }
        case (0x0EB6): { return eNSM; }
        case (0x0EB7): { return eNSM; }
        case (0x0EB8): { return eNSM; }
        case (0x0EB9): { return eNSM; }
        case (0x0EBB): { return eNSM; }
        case (0x0EBC): { return eNSM; }
        case (0x0EC8): { return eNSM; }
        case (0x0EC9): { return eNSM; }
        case (0x0ECA): { return eNSM; }
        case (0x0ECB): { return eNSM; }
        case (0x0ECC): { return eNSM; }
        case (0x0ECD): { return eNSM; }
        case (0x0F18): { return eNSM; }
        case (0x0F19): { return eNSM; }
        case (0x0F35): { return eNSM; }
        case (0x0F37): { return eNSM; }
        case (0x0F39): { return eNSM; }
        case (0x0F3A): { return eON; }
        case (0x0F3B): { return eON; }
        case (0x0F3C): { return eON; }
        case (0x0F3D): { return eON; }
        case (0x0F71): { return eNSM; }
        case (0x0F72): { return eNSM; }
        case (0x0F73): { return eNSM; }
        case (0x0F74): { return eNSM; }
        case (0x0F75): { return eNSM; }
        case (0x0F76): { return eNSM; }
        case (0x0F77): { return eNSM; }
        case (0x0F78): { return eNSM; }
        case (0x0F79): { return eNSM; }
        case (0x0F7A): { return eNSM; }
        case (0x0F7B): { return eNSM; }
        case (0x0F7C): { return eNSM; }
        case (0x0F7D): { return eNSM; }
        case (0x0F7E): { return eNSM; }
        case (0x0F80): { return eNSM; }
        case (0x0F81): { return eNSM; }
        case (0x0F82): { return eNSM; }
        case (0x0F83): { return eNSM; }
        case (0x0F84): { return eNSM; }
        case (0x0F86): { return eNSM; }
        case (0x0F87): { return eNSM; }
        case (0x0F8D): { return eNSM; }
        case (0x0F8E): { return eNSM; }
        case (0x0F8F): { return eNSM; }
        case (0x0F90): { return eNSM; }
        case (0x0F91): { return eNSM; }
        case (0x0F92): { return eNSM; }
        case (0x0F93): { return eNSM; }
        case (0x0F94): { return eNSM; }
        case (0x0F95): { return eNSM; }
        case (0x0F96): { return eNSM; }
        case (0x0F97): { return eNSM; }
        case (0x0F99): { return eNSM; }
        case (0x0F9A): { return eNSM; }
        case (0x0F9B): { return eNSM; }
        case (0x0F9C): { return eNSM; }
        case (0x0F9D): { return eNSM; }
        case (0x0F9E): { return eNSM; }
        case (0x0F9F): { return eNSM; }
        case (0x0FA0): { return eNSM; }
        case (0x0FA1): { return eNSM; }
        case (0x0FA2): { return eNSM; }
        case (0x0FA3): { return eNSM; }
        case (0x0FA4): { return eNSM; }
        case (0x0FA5): { return eNSM; }
        case (0x0FA6): { return eNSM; }
        case (0x0FA7): { return eNSM; }
        case (0x0FA8): { return eNSM; }
        case (0x0FA9): { return eNSM; }
        case (0x0FAA): { return eNSM; }
        case (0x0FAB): { return eNSM; }
        case (0x0FAC): { return eNSM; }
        case (0x0FAD): { return eNSM; }
        case (0x0FAE): { return eNSM; }
        case (0x0FAF): { return eNSM; }
        case (0x0FB0): { return eNSM; }
        case (0x0FB1): { return eNSM; }
        case (0x0FB2): { return eNSM; }
        case (0x0FB3): { return eNSM; }
        case (0x0FB4): { return eNSM; }
        case (0x0FB5): { return eNSM; }
        case (0x0FB6): { return eNSM; }
        case (0x0FB7): { return eNSM; }
        case (0x0FB8): { return eNSM; }
        case (0x0FB9): { return eNSM; }
        case (0x0FBA): { return eNSM; }
        case (0x0FBB): { return eNSM; }
        case (0x0FBC): { return eNSM; }
        case (0x0FC6): { return eNSM; }
        case (0x102D): { return eNSM; }
        case (0x102E): { return eNSM; }
        case (0x102F): { return eNSM; }
        case (0x1030): { return eNSM; }
        case (0x1032): { return eNSM; }
        case (0x1033): { return eNSM; }
        case (0x1034): { return eNSM; }
        case (0x1035): { return eNSM; }
        case (0x1036): { return eNSM; }
        case (0x1037): { return eNSM; }
        case (0x1039): { return eNSM; }
        case (0x103A): { return eNSM; }
        case (0x103D): { return eNSM; }
        case (0x103E): { return eNSM; }
        case (0x1058): { return eNSM; }
        case (0x1059): { return eNSM; }
        case (0x105E): { return eNSM; }
        case (0x105F): { return eNSM; }
        case (0x1060): { return eNSM; }
        case (0x1071): { return eNSM; }
        case (0x1072): { return eNSM; }
        case (0x1073): { return eNSM; }
        case (0x1074): { return eNSM; }
        case (0x1082): { return eNSM; }
        case (0x1085): { return eNSM; }
        case (0x1086): { return eNSM; }
        case (0x108D): { return eNSM; }
        case (0x109D): { return eNSM; }
        case (0x135D): { return eNSM; }
        case (0x135E): { return eNSM; }
        case (0x135F): { return eNSM; }
        case (0x1390): { return eON; }
        case (0x1391): { return eON; }
        case (0x1392): { return eON; }
        case (0x1393): { return eON; }
        case (0x1394): { return eON; }
        case (0x1395): { return eON; }
        case (0x1396): { return eON; }
        case (0x1397): { return eON; }
        case (0x1398): { return eON; }
        case (0x1399): { return eON; }
        case (0x1400): { return eON; }
        case (0x1680): { return eWS; }
        case (0x169B): { return eON; }
        case (0x169C): { return eON; }
        case (0x1712): { return eNSM; }
        case (0x1713): { return eNSM; }
        case (0x1714): { return eNSM; }
        case (0x1732): { return eNSM; }
        case (0x1733): { return eNSM; }
        case (0x1734): { return eNSM; }
        case (0x1752): { return eNSM; }
        case (0x1753): { return eNSM; }
        case (0x1772): { return eNSM; }
        case (0x1773): { return eNSM; }
        case (0x17B7): { return eNSM; }
        case (0x17B8): { return eNSM; }
        case (0x17B9): { return eNSM; }
        case (0x17BA): { return eNSM; }
        case (0x17BB): { return eNSM; }
        case (0x17BC): { return eNSM; }
        case (0x17BD): { return eNSM; }
        case (0x17C6): { return eNSM; }
        case (0x17C9): { return eNSM; }
        case (0x17CA): { return eNSM; }
        case (0x17CB): { return eNSM; }
        case (0x17CC): { return eNSM; }
        case (0x17CD): { return eNSM; }
        case (0x17CE): { return eNSM; }
        case (0x17CF): { return eNSM; }
        case (0x17D0): { return eNSM; }
        case (0x17D1): { return eNSM; }
        case (0x17D2): { return eNSM; }
        case (0x17D3): { return eNSM; }
        case (0x17DB): { return eET; }
        case (0x17DD): { return eNSM; }
        case (0x17F0): { return eON; }
        case (0x17F1): { return eON; }
        case (0x17F2): { return eON; }
        case (0x17F3): { return eON; }
        case (0x17F4): { return eON; }
        case (0x17F5): { return eON; }
        case (0x17F6): { return eON; }
        case (0x17F7): { return eON; }
        case (0x17F8): { return eON; }
        case (0x17F9): { return eON; }
        case (0x1800): { return eON; }
        case (0x1801): { return eON; }
        case (0x1802): { return eON; }
        case (0x1803): { return eON; }
        case (0x1804): { return eON; }
        case (0x1805): { return eON; }
        case (0x1806): { return eON; }
        case (0x1807): { return eON; }
        case (0x1808): { return eON; }
        case (0x1809): { return eON; }
        case (0x180A): { return eON; }
        case (0x180B): { return eNSM; }
        case (0x180C): { return eNSM; }
        case (0x180D): { return eNSM; }
        case (0x180E): { return eWS; }
        case (0x18A9): { return eNSM; }
        case (0x1920): { return eNSM; }
        case (0x1921): { return eNSM; }
        case (0x1922): { return eNSM; }
        case (0x1927): { return eNSM; }
        case (0x1928): { return eNSM; }
        case (0x1932): { return eNSM; }
        case (0x1939): { return eNSM; }
        case (0x193A): { return eNSM; }
        case (0x193B): { return eNSM; }
        case (0x1940): { return eON; }
        case (0x1944): { return eON; }
        case (0x1945): { return eON; }
        case (0x19DE): { return eON; }
        case (0x19DF): { return eON; }
        case (0x19E0): { return eON; }
        case (0x19E1): { return eON; }
        case (0x19E2): { return eON; }
        case (0x19E3): { return eON; }
        case (0x19E4): { return eON; }
        case (0x19E5): { return eON; }
        case (0x19E6): { return eON; }
        case (0x19E7): { return eON; }
        case (0x19E8): { return eON; }
        case (0x19E9): { return eON; }
        case (0x19EA): { return eON; }
        case (0x19EB): { return eON; }
        case (0x19EC): { return eON; }
        case (0x19ED): { return eON; }
        case (0x19EE): { return eON; }
        case (0x19EF): { return eON; }
        case (0x19F0): { return eON; }
        case (0x19F1): { return eON; }
        case (0x19F2): { return eON; }
        case (0x19F3): { return eON; }
        case (0x19F4): { return eON; }
        case (0x19F5): { return eON; }
        case (0x19F6): { return eON; }
        case (0x19F7): { return eON; }
        case (0x19F8): { return eON; }
        case (0x19F9): { return eON; }
        case (0x19FA): { return eON; }
        case (0x19FB): { return eON; }
        case (0x19FC): { return eON; }
        case (0x19FD): { return eON; }
        case (0x19FE): { return eON; }
        case (0x19FF): { return eON; }
        case (0x1A17): { return eNSM; }
        case (0x1A18): { return eNSM; }
        case (0x1A56): { return eNSM; }
        case (0x1A58): { return eNSM; }
        case (0x1A59): { return eNSM; }
        case (0x1A5A): { return eNSM; }
        case (0x1A5B): { return eNSM; }
        case (0x1A5C): { return eNSM; }
        case (0x1A5D): { return eNSM; }
        case (0x1A5E): { return eNSM; }
        case (0x1A60): { return eNSM; }
        case (0x1A62): { return eNSM; }
        case (0x1A65): { return eNSM; }
        case (0x1A66): { return eNSM; }
        case (0x1A67): { return eNSM; }
        case (0x1A68): { return eNSM; }
        case (0x1A69): { return eNSM; }
        case (0x1A6A): { return eNSM; }
        case (0x1A6B): { return eNSM; }
        case (0x1A6C): { return eNSM; }
        case (0x1A73): { return eNSM; }
        case (0x1A74): { return eNSM; }
        case (0x1A75): { return eNSM; }
        case (0x1A76): { return eNSM; }
        case (0x1A77): { return eNSM; }
        case (0x1A78): { return eNSM; }
        case (0x1A79): { return eNSM; }
        case (0x1A7A): { return eNSM; }
        case (0x1A7B): { return eNSM; }
        case (0x1A7C): { return eNSM; }
        case (0x1A7F): { return eNSM; }
        case (0x1B00): { return eNSM; }
        case (0x1B01): { return eNSM; }
        case (0x1B02): { return eNSM; }
        case (0x1B03): { return eNSM; }
        case (0x1B34): { return eNSM; }
        case (0x1B36): { return eNSM; }
        case (0x1B37): { return eNSM; }
        case (0x1B38): { return eNSM; }
        case (0x1B39): { return eNSM; }
        case (0x1B3A): { return eNSM; }
        case (0x1B3C): { return eNSM; }
        case (0x1B42): { return eNSM; }
        case (0x1B6B): { return eNSM; }
        case (0x1B6C): { return eNSM; }
        case (0x1B6D): { return eNSM; }
        case (0x1B6E): { return eNSM; }
        case (0x1B6F): { return eNSM; }
        case (0x1B70): { return eNSM; }
        case (0x1B71): { return eNSM; }
        case (0x1B72): { return eNSM; }
        case (0x1B73): { return eNSM; }
        case (0x1B80): { return eNSM; }
        case (0x1B81): { return eNSM; }
        case (0x1BA2): { return eNSM; }
        case (0x1BA3): { return eNSM; }
        case (0x1BA4): { return eNSM; }
        case (0x1BA5): { return eNSM; }
        case (0x1BA8): { return eNSM; }
        case (0x1BA9): { return eNSM; }
        case (0x1BE6): { return eNSM; }
        case (0x1BE8): { return eNSM; }
        case (0x1BE9): { return eNSM; }
        case (0x1BED): { return eNSM; }
        case (0x1BEF): { return eNSM; }
        case (0x1BF0): { return eNSM; }
        case (0x1BF1): { return eNSM; }
        case (0x1C2C): { return eNSM; }
        case (0x1C2D): { return eNSM; }
        case (0x1C2E): { return eNSM; }
        case (0x1C2F): { return eNSM; }
        case (0x1C30): { return eNSM; }
        case (0x1C31): { return eNSM; }
        case (0x1C32): { return eNSM; }
        case (0x1C33): { return eNSM; }
        case (0x1C36): { return eNSM; }
        case (0x1C37): { return eNSM; }
        case (0x1CD0): { return eNSM; }
        case (0x1CD1): { return eNSM; }
        case (0x1CD2): { return eNSM; }
        case (0x1CD4): { return eNSM; }
        case (0x1CD5): { return eNSM; }
        case (0x1CD6): { return eNSM; }
        case (0x1CD7): { return eNSM; }
        case (0x1CD8): { return eNSM; }
        case (0x1CD9): { return eNSM; }
        case (0x1CDA): { return eNSM; }
        case (0x1CDB): { return eNSM; }
        case (0x1CDC): { return eNSM; }
        case (0x1CDD): { return eNSM; }
        case (0x1CDE): { return eNSM; }
        case (0x1CDF): { return eNSM; }
        case (0x1CE0): { return eNSM; }
        case (0x1CE2): { return eNSM; }
        case (0x1CE3): { return eNSM; }
        case (0x1CE4): { return eNSM; }
        case (0x1CE5): { return eNSM; }
        case (0x1CE6): { return eNSM; }
        case (0x1CE7): { return eNSM; }
        case (0x1CE8): { return eNSM; }
        case (0x1CED): { return eNSM; }
        case (0x1DC0): { return eNSM; }
        case (0x1DC1): { return eNSM; }
        case (0x1DC2): { return eNSM; }
        case (0x1DC3): { return eNSM; }
        case (0x1DC4): { return eNSM; }
        case (0x1DC5): { return eNSM; }
        case (0x1DC6): { return eNSM; }
        case (0x1DC7): { return eNSM; }
        case (0x1DC8): { return eNSM; }
        case (0x1DC9): { return eNSM; }
        case (0x1DCA): { return eNSM; }
        case (0x1DCB): { return eNSM; }
        case (0x1DCC): { return eNSM; }
        case (0x1DCD): { return eNSM; }
        case (0x1DCE): { return eNSM; }
        case (0x1DCF): { return eNSM; }
        case (0x1DD0): { return eNSM; }
        case (0x1DD1): { return eNSM; }
        case (0x1DD2): { return eNSM; }
        case (0x1DD3): { return eNSM; }
        case (0x1DD4): { return eNSM; }
        case (0x1DD5): { return eNSM; }
        case (0x1DD6): { return eNSM; }
        case (0x1DD7): { return eNSM; }
        case (0x1DD8): { return eNSM; }
        case (0x1DD9): { return eNSM; }
        case (0x1DDA): { return eNSM; }
        case (0x1DDB): { return eNSM; }
        case (0x1DDC): { return eNSM; }
        case (0x1DDD): { return eNSM; }
        case (0x1DDE): { return eNSM; }
        case (0x1DDF): { return eNSM; }
        case (0x1DE0): { return eNSM; }
        case (0x1DE1): { return eNSM; }
        case (0x1DE2): { return eNSM; }
        case (0x1DE3): { return eNSM; }
        case (0x1DE4): { return eNSM; }
        case (0x1DE5): { return eNSM; }
        case (0x1DE6): { return eNSM; }
        case (0x1DFC): { return eNSM; }
        case (0x1DFD): { return eNSM; }
        case (0x1DFE): { return eNSM; }
        case (0x1DFF): { return eNSM; }
        case (0x1FBD): { return eON; }
        case (0x1FBF): { return eON; }
        case (0x1FC0): { return eON; }
        case (0x1FC1): { return eON; }
        case (0x1FCD): { return eON; }
        case (0x1FCE): { return eON; }
        case (0x1FCF): { return eON; }
        case (0x1FDD): { return eON; }
        case (0x1FDE): { return eON; }
        case (0x1FDF): { return eON; }
        case (0x1FED): { return eON; }
        case (0x1FEE): { return eON; }
        case (0x1FEF): { return eON; }
        case (0x1FFD): { return eON; }
        case (0x1FFE): { return eON; }
        case (0x2000): { return eWS; }
        case (0x2001): { return eWS; }
        case (0x2002): { return eWS; }
        case (0x2003): { return eWS; }
        case (0x2004): { return eWS; }
        case (0x2005): { return eWS; }
        case (0x2006): { return eWS; }
        case (0x2007): { return eWS; }
        case (0x2008): { return eWS; }
        case (0x2009): { return eWS; }
        case (0x200A): { return eWS; }
        case (0x200B): { return eBN; }
        case (0x200C): { return eBN; }
        case (0x200D): { return eBN; }
        case (0x200E): { return eON; } // LRM ignored, treated as ON
        case (0x200F): { return eON; } // RLM ignored, treated as ON
        case (0x2010): { return eON; }
        case (0x2011): { return eON; }
        case (0x2012): { return eON; }
        case (0x2013): { return eON; }
        case (0x2014): { return eON; }
        case (0x2015): { return eON; }
        case (0x2016): { return eON; }
        case (0x2017): { return eON; }
        case (0x2018): { return eON; }
        case (0x2019): { return eON; }
        case (0x201A): { return eON; }
        case (0x201B): { return eON; }
        case (0x201C): { return eON; }
        case (0x201D): { return eON; }
        case (0x201E): { return eON; }
        case (0x201F): { return eON; }
        case (0x2020): { return eON; }
        case (0x2021): { return eON; }
        case (0x2022): { return eON; }
        case (0x2023): { return eON; }
        case (0x2024): { return eON; }
        case (0x2025): { return eON; }
        case (0x2026): { return eON; }
        case (0x2027): { return eON; }
        case (0x2028): { return eWS; }
        case (0x2029): { return eB; }
        case (0x202A): { return eON; } // LRE ignored, treated as ON
        case (0x202B): { return eON; } // RLE ignored, treated as ON
        case (0x202C): { return eON; } // PDF ignored, treated as ON
        case (0x202D): { return eON; } // LRO ignored, treated as ON
        case (0x202E): { return eON; } // RLO ignored, treated as ON
        case (0x202F): { return eCS; }
        case (0x2030): { return eET; }
        case (0x2031): { return eET; }
        case (0x2032): { return eET; }
        case (0x2033): { return eET; }
        case (0x2034): { return eET; }
        case (0x2035): { return eON; }
        case (0x2036): { return eON; }
        case (0x2037): { return eON; }
        case (0x2038): { return eON; }
        case (0x2039): { return eON; }
        case (0x203A): { return eON; }
        case (0x203B): { return eON; }
        case (0x203C): { return eON; }
        case (0x203D): { return eON; }
        case (0x203E): { return eON; }
        case (0x203F): { return eON; }
        case (0x2040): { return eON; }
        case (0x2041): { return eON; }
        case (0x2042): { return eON; }
        case (0x2043): { return eON; }
        case (0x2044): { return eCS; }
        case (0x2045): { return eON; }
        case (0x2046): { return eON; }
        case (0x2047): { return eON; }
        case (0x2048): { return eON; }
        case (0x2049): { return eON; }
        case (0x204A): { return eON; }
        case (0x204B): { return eON; }
        case (0x204C): { return eON; }
        case (0x204D): { return eON; }
        case (0x204E): { return eON; }
        case (0x204F): { return eON; }
        case (0x2050): { return eON; }
        case (0x2051): { return eON; }
        case (0x2052): { return eON; }
        case (0x2053): { return eON; }
        case (0x2054): { return eON; }
        case (0x2055): { return eON; }
        case (0x2056): { return eON; }
        case (0x2057): { return eON; }
        case (0x2058): { return eON; }
        case (0x2059): { return eON; }
        case (0x205A): { return eON; }
        case (0x205B): { return eON; }
        case (0x205C): { return eON; }
        case (0x205D): { return eON; }
        case (0x205E): { return eON; }
        case (0x205F): { return eWS; }
        case (0x2060): { return eBN; }
        case (0x2061): { return eBN; }
        case (0x2062): { return eBN; }
        case (0x2063): { return eBN; }
        case (0x2064): { return eBN; }
        case (0x206A): { return eBN; }
        case (0x206B): { return eBN; }
        case (0x206C): { return eBN; }
        case (0x206D): { return eBN; }
        case (0x206E): { return eBN; }
        case (0x206F): { return eBN; }
        case (0x2070): { return eEN; }
        case (0x2074): { return eEN; }
        case (0x2075): { return eEN; }
        case (0x2076): { return eEN; }
        case (0x2077): { return eEN; }
        case (0x2078): { return eEN; }
        case (0x2079): { return eEN; }
        case (0x207A): { return eES; }
        case (0x207B): { return eES; }
        case (0x207C): { return eON; }
        case (0x207D): { return eON; }
        case (0x207E): { return eON; }
        case (0x2080): { return eEN; }
        case (0x2081): { return eEN; }
        case (0x2082): { return eEN; }
        case (0x2083): { return eEN; }
        case (0x2084): { return eEN; }
        case (0x2085): { return eEN; }
        case (0x2086): { return eEN; }
        case (0x2087): { return eEN; }
        case (0x2088): { return eEN; }
        case (0x2089): { return eEN; }
        case (0x208A): { return eES; }
        case (0x208B): { return eES; }
        case (0x208C): { return eON; }
        case (0x208D): { return eON; }
        case (0x208E): { return eON; }
        case (0x20A0): { return eET; }
        case (0x20A1): { return eET; }
        case (0x20A2): { return eET; }
        case (0x20A3): { return eET; }
        case (0x20A4): { return eET; }
        case (0x20A5): { return eET; }
        case (0x20A6): { return eET; }
        case (0x20A7): { return eET; }
        case (0x20A8): { return eET; }
        case (0x20A9): { return eET; }
        case (0x20AA): { return eET; }
        case (0x20AB): { return eET; }
        case (0x20AC): { return eET; }
        case (0x20AD): { return eET; }
        case (0x20AE): { return eET; }
        case (0x20AF): { return eET; }
        case (0x20B0): { return eET; }
        case (0x20B1): { return eET; }
        case (0x20B2): { return eET; }
        case (0x20B3): { return eET; }
        case (0x20B4): { return eET; }
        case (0x20B5): { return eET; }
        case (0x20B6): { return eET; }
        case (0x20B7): { return eET; }
        case (0x20B8): { return eET; }
        case (0x20B9): { return eET; }
        case (0x20D0): { return eNSM; }
        case (0x20D1): { return eNSM; }
        case (0x20D2): { return eNSM; }
        case (0x20D3): { return eNSM; }
        case (0x20D4): { return eNSM; }
        case (0x20D5): { return eNSM; }
        case (0x20D6): { return eNSM; }
        case (0x20D7): { return eNSM; }
        case (0x20D8): { return eNSM; }
        case (0x20D9): { return eNSM; }
        case (0x20DA): { return eNSM; }
        case (0x20DB): { return eNSM; }
        case (0x20DC): { return eNSM; }
        case (0x20DD): { return eNSM; }
        case (0x20DE): { return eNSM; }
        case (0x20DF): { return eNSM; }
        case (0x20E0): { return eNSM; }
        case (0x20E1): { return eNSM; }
        case (0x20E2): { return eNSM; }
        case (0x20E3): { return eNSM; }
        case (0x20E4): { return eNSM; }
        case (0x20E5): { return eNSM; }
        case (0x20E6): { return eNSM; }
        case (0x20E7): { return eNSM; }
        case (0x20E8): { return eNSM; }
        case (0x20E9): { return eNSM; }
        case (0x20EA): { return eNSM; }
        case (0x20EB): { return eNSM; }
        case (0x20EC): { return eNSM; }
        case (0x20ED): { return eNSM; }
        case (0x20EE): { return eNSM; }
        case (0x20EF): { return eNSM; }
        case (0x20F0): { return eNSM; }
        case (0x2100): { return eON; }
        case (0x2101): { return eON; }
        case (0x2103): { return eON; }
        case (0x2104): { return eON; }
        case (0x2105): { return eON; }
        case (0x2106): { return eON; }
        case (0x2108): { return eON; }
        case (0x2109): { return eON; }
        case (0x2114): { return eON; }
        case (0x2116): { return eON; }
        case (0x2117): { return eON; }
        case (0x2118): { return eON; }
        case (0x211E): { return eON; }
        case (0x211F): { return eON; }
        case (0x2120): { return eON; }
        case (0x2121): { return eON; }
        case (0x2122): { return eON; }
        case (0x2123): { return eON; }
        case (0x2125): { return eON; }
        case (0x2127): { return eON; }
        case (0x2129): { return eON; }
        case (0x212E): { return eET; }
        case (0x213A): { return eON; }
        case (0x213B): { return eON; }
        case (0x2140): { return eON; }
        case (0x2141): { return eON; }
        case (0x2142): { return eON; }
        case (0x2143): { return eON; }
        case (0x2144): { return eON; }
        case (0x214A): { return eON; }
        case (0x214B): { return eON; }
        case (0x214C): { return eON; }
        case (0x214D): { return eON; }
        case (0x2150): { return eON; }
        case (0x2151): { return eON; }
        case (0x2152): { return eON; }
        case (0x2153): { return eON; }
        case (0x2154): { return eON; }
        case (0x2155): { return eON; }
        case (0x2156): { return eON; }
        case (0x2157): { return eON; }
        case (0x2158): { return eON; }
        case (0x2159): { return eON; }
        case (0x215A): { return eON; }
        case (0x215B): { return eON; }
        case (0x215C): { return eON; }
        case (0x215D): { return eON; }
        case (0x215E): { return eON; }
        case (0x215F): { return eON; }
        case (0x2189): { return eON; }
        case (0x2190): { return eON; }
        case (0x2191): { return eON; }
        case (0x2192): { return eON; }
        case (0x2193): { return eON; }
        case (0x2194): { return eON; }
        case (0x2195): { return eON; }
        case (0x2196): { return eON; }
        case (0x2197): { return eON; }
        case (0x2198): { return eON; }
        case (0x2199): { return eON; }
        case (0x219A): { return eON; }
        case (0x219B): { return eON; }
        case (0x219C): { return eON; }
        case (0x219D): { return eON; }
        case (0x219E): { return eON; }
        case (0x219F): { return eON; }
        case (0x21A0): { return eON; }
        case (0x21A1): { return eON; }
        case (0x21A2): { return eON; }
        case (0x21A3): { return eON; }
        case (0x21A4): { return eON; }
        case (0x21A5): { return eON; }
        case (0x21A6): { return eON; }
        case (0x21A7): { return eON; }
        case (0x21A8): { return eON; }
        case (0x21A9): { return eON; }
        case (0x21AA): { return eON; }
        case (0x21AB): { return eON; }
        case (0x21AC): { return eON; }
        case (0x21AD): { return eON; }
        case (0x21AE): { return eON; }
        case (0x21AF): { return eON; }
        case (0x21B0): { return eON; }
        case (0x21B1): { return eON; }
        case (0x21B2): { return eON; }
        case (0x21B3): { return eON; }
        case (0x21B4): { return eON; }
        case (0x21B5): { return eON; }
        case (0x21B6): { return eON; }
        case (0x21B7): { return eON; }
        case (0x21B8): { return eON; }
        case (0x21B9): { return eON; }
        case (0x21BA): { return eON; }
        case (0x21BB): { return eON; }
        case (0x21BC): { return eON; }
        case (0x21BD): { return eON; }
        case (0x21BE): { return eON; }
        case (0x21BF): { return eON; }
        case (0x21C0): { return eON; }
        case (0x21C1): { return eON; }
        case (0x21C2): { return eON; }
        case (0x21C3): { return eON; }
        case (0x21C4): { return eON; }
        case (0x21C5): { return eON; }
        case (0x21C6): { return eON; }
        case (0x21C7): { return eON; }
        case (0x21C8): { return eON; }
        case (0x21C9): { return eON; }
        case (0x21CA): { return eON; }
        case (0x21CB): { return eON; }
        case (0x21CC): { return eON; }
        case (0x21CD): { return eON; }
        case (0x21CE): { return eON; }
        case (0x21CF): { return eON; }
        case (0x21D0): { return eON; }
        case (0x21D1): { return eON; }
        case (0x21D2): { return eON; }
        case (0x21D3): { return eON; }
        case (0x21D4): { return eON; }
        case (0x21D5): { return eON; }
        case (0x21D6): { return eON; }
        case (0x21D7): { return eON; }
        case (0x21D8): { return eON; }
        case (0x21D9): { return eON; }
        case (0x21DA): { return eON; }
        case (0x21DB): { return eON; }
        case (0x21DC): { return eON; }
        case (0x21DD): { return eON; }
        case (0x21DE): { return eON; }
        case (0x21DF): { return eON; }
        case (0x21E0): { return eON; }
        case (0x21E1): { return eON; }
        case (0x21E2): { return eON; }
        case (0x21E3): { return eON; }
        case (0x21E4): { return eON; }
        case (0x21E5): { return eON; }
        case (0x21E6): { return eON; }
        case (0x21E7): { return eON; }
        case (0x21E8): { return eON; }
        case (0x21E9): { return eON; }
        case (0x21EA): { return eON; }
        case (0x21EB): { return eON; }
        case (0x21EC): { return eON; }
        case (0x21ED): { return eON; }
        case (0x21EE): { return eON; }
        case (0x21EF): { return eON; }
        case (0x21F0): { return eON; }
        case (0x21F1): { return eON; }
        case (0x21F2): { return eON; }
        case (0x21F3): { return eON; }
        case (0x21F4): { return eON; }
        case (0x21F5): { return eON; }
        case (0x21F6): { return eON; }
        case (0x21F7): { return eON; }
        case (0x21F8): { return eON; }
        case (0x21F9): { return eON; }
        case (0x21FA): { return eON; }
        case (0x21FB): { return eON; }
        case (0x21FC): { return eON; }
        case (0x21FD): { return eON; }
        case (0x21FE): { return eON; }
        case (0x21FF): { return eON; }
        case (0x2200): { return eON; }
        case (0x2201): { return eON; }
        case (0x2202): { return eON; }
        case (0x2203): { return eON; }
        case (0x2204): { return eON; }
        case (0x2205): { return eON; }
        case (0x2206): { return eON; }
        case (0x2207): { return eON; }
        case (0x2208): { return eON; }
        case (0x2209): { return eON; }
        case (0x220A): { return eON; }
        case (0x220B): { return eON; }
        case (0x220C): { return eON; }
        case (0x220D): { return eON; }
        case (0x220E): { return eON; }
        case (0x220F): { return eON; }
        case (0x2210): { return eON; }
        case (0x2211): { return eON; }
        case (0x2212): { return eES; }
        case (0x2213): { return eET; }
        case (0x2214): { return eON; }
        case (0x2215): { return eON; }
        case (0x2216): { return eON; }
        case (0x2217): { return eON; }
        case (0x2218): { return eON; }
        case (0x2219): { return eON; }
        case (0x221A): { return eON; }
        case (0x221B): { return eON; }
        case (0x221C): { return eON; }
        case (0x221D): { return eON; }
        case (0x221E): { return eON; }
        case (0x221F): { return eON; }
        case (0x2220): { return eON; }
        case (0x2221): { return eON; }
        case (0x2222): { return eON; }
        case (0x2223): { return eON; }
        case (0x2224): { return eON; }
        case (0x2225): { return eON; }
        case (0x2226): { return eON; }
        case (0x2227): { return eON; }
        case (0x2228): { return eON; }
        case (0x2229): { return eON; }
        case (0x222A): { return eON; }
        case (0x222B): { return eON; }
        case (0x222C): { return eON; }
        case (0x222D): { return eON; }
        case (0x222E): { return eON; }
        case (0x222F): { return eON; }
        case (0x2230): { return eON; }
        case (0x2231): { return eON; }
        case (0x2232): { return eON; }
        case (0x2233): { return eON; }
        case (0x2234): { return eON; }
        case (0x2235): { return eON; }
        case (0x2236): { return eON; }
        case (0x2237): { return eON; }
        case (0x2238): { return eON; }
        case (0x2239): { return eON; }
        case (0x223A): { return eON; }
        case (0x223B): { return eON; }
        case (0x223C): { return eON; }
        case (0x223D): { return eON; }
        case (0x223E): { return eON; }
        case (0x223F): { return eON; }
        case (0x2240): { return eON; }
        case (0x2241): { return eON; }
        case (0x2242): { return eON; }
        case (0x2243): { return eON; }
        case (0x2244): { return eON; }
        case (0x2245): { return eON; }
        case (0x2246): { return eON; }
        case (0x2247): { return eON; }
        case (0x2248): { return eON; }
        case (0x2249): { return eON; }
        case (0x224A): { return eON; }
        case (0x224B): { return eON; }
        case (0x224C): { return eON; }
        case (0x224D): { return eON; }
        case (0x224E): { return eON; }
        case (0x224F): { return eON; }
        case (0x2250): { return eON; }
        case (0x2251): { return eON; }
        case (0x2252): { return eON; }
        case (0x2253): { return eON; }
        case (0x2254): { return eON; }
        case (0x2255): { return eON; }
        case (0x2256): { return eON; }
        case (0x2257): { return eON; }
        case (0x2258): { return eON; }
        case (0x2259): { return eON; }
        case (0x225A): { return eON; }
        case (0x225B): { return eON; }
        case (0x225C): { return eON; }
        case (0x225D): { return eON; }
        case (0x225E): { return eON; }
        case (0x225F): { return eON; }
        case (0x2260): { return eON; }
        case (0x2261): { return eON; }
        case (0x2262): { return eON; }
        case (0x2263): { return eON; }
        case (0x2264): { return eON; }
        case (0x2265): { return eON; }
        case (0x2266): { return eON; }
        case (0x2267): { return eON; }
        case (0x2268): { return eON; }
        case (0x2269): { return eON; }
        case (0x226A): { return eON; }
        case (0x226B): { return eON; }
        case (0x226C): { return eON; }
        case (0x226D): { return eON; }
        case (0x226E): { return eON; }
        case (0x226F): { return eON; }
        case (0x2270): { return eON; }
        case (0x2271): { return eON; }
        case (0x2272): { return eON; }
        case (0x2273): { return eON; }
        case (0x2274): { return eON; }
        case (0x2275): { return eON; }
        case (0x2276): { return eON; }
        case (0x2277): { return eON; }
        case (0x2278): { return eON; }
        case (0x2279): { return eON; }
        case (0x227A): { return eON; }
        case (0x227B): { return eON; }
        case (0x227C): { return eON; }
        case (0x227D): { return eON; }
        case (0x227E): { return eON; }
        case (0x227F): { return eON; }
        case (0x2280): { return eON; }
        case (0x2281): { return eON; }
        case (0x2282): { return eON; }
        case (0x2283): { return eON; }
        case (0x2284): { return eON; }
        case (0x2285): { return eON; }
        case (0x2286): { return eON; }
        case (0x2287): { return eON; }
        case (0x2288): { return eON; }
        case (0x2289): { return eON; }
        case (0x228A): { return eON; }
        case (0x228B): { return eON; }
        case (0x228C): { return eON; }
        case (0x228D): { return eON; }
        case (0x228E): { return eON; }
        case (0x228F): { return eON; }
        case (0x2290): { return eON; }
        case (0x2291): { return eON; }
        case (0x2292): { return eON; }
        case (0x2293): { return eON; }
        case (0x2294): { return eON; }
        case (0x2295): { return eON; }
        case (0x2296): { return eON; }
        case (0x2297): { return eON; }
        case (0x2298): { return eON; }
        case (0x2299): { return eON; }
        case (0x229A): { return eON; }
        case (0x229B): { return eON; }
        case (0x229C): { return eON; }
        case (0x229D): { return eON; }
        case (0x229E): { return eON; }
        case (0x229F): { return eON; }
        case (0x22A0): { return eON; }
        case (0x22A1): { return eON; }
        case (0x22A2): { return eON; }
        case (0x22A3): { return eON; }
        case (0x22A4): { return eON; }
        case (0x22A5): { return eON; }
        case (0x22A6): { return eON; }
        case (0x22A7): { return eON; }
        case (0x22A8): { return eON; }
        case (0x22A9): { return eON; }
        case (0x22AA): { return eON; }
        case (0x22AB): { return eON; }
        case (0x22AC): { return eON; }
        case (0x22AD): { return eON; }
        case (0x22AE): { return eON; }
        case (0x22AF): { return eON; }
        case (0x22B0): { return eON; }
        case (0x22B1): { return eON; }
        case (0x22B2): { return eON; }
        case (0x22B3): { return eON; }
        case (0x22B4): { return eON; }
        case (0x22B5): { return eON; }
        case (0x22B6): { return eON; }
        case (0x22B7): { return eON; }
        case (0x22B8): { return eON; }
        case (0x22B9): { return eON; }
        case (0x22BA): { return eON; }
        case (0x22BB): { return eON; }
        case (0x22BC): { return eON; }
        case (0x22BD): { return eON; }
        case (0x22BE): { return eON; }
        case (0x22BF): { return eON; }
        case (0x22C0): { return eON; }
        case (0x22C1): { return eON; }
        case (0x22C2): { return eON; }
        case (0x22C3): { return eON; }
        case (0x22C4): { return eON; }
        case (0x22C5): { return eON; }
        case (0x22C6): { return eON; }
        case (0x22C7): { return eON; }
        case (0x22C8): { return eON; }
        case (0x22C9): { return eON; }
        case (0x22CA): { return eON; }
        case (0x22CB): { return eON; }
        case (0x22CC): { return eON; }
        case (0x22CD): { return eON; }
        case (0x22CE): { return eON; }
        case (0x22CF): { return eON; }
        case (0x22D0): { return eON; }
        case (0x22D1): { return eON; }
        case (0x22D2): { return eON; }
        case (0x22D3): { return eON; }
        case (0x22D4): { return eON; }
        case (0x22D5): { return eON; }
        case (0x22D6): { return eON; }
        case (0x22D7): { return eON; }
        case (0x22D8): { return eON; }
        case (0x22D9): { return eON; }
        case (0x22DA): { return eON; }
        case (0x22DB): { return eON; }
        case (0x22DC): { return eON; }
        case (0x22DD): { return eON; }
        case (0x22DE): { return eON; }
        case (0x22DF): { return eON; }
        case (0x22E0): { return eON; }
        case (0x22E1): { return eON; }
        case (0x22E2): { return eON; }
        case (0x22E3): { return eON; }
        case (0x22E4): { return eON; }
        case (0x22E5): { return eON; }
        case (0x22E6): { return eON; }
        case (0x22E7): { return eON; }
        case (0x22E8): { return eON; }
        case (0x22E9): { return eON; }
        case (0x22EA): { return eON; }
        case (0x22EB): { return eON; }
        case (0x22EC): { return eON; }
        case (0x22ED): { return eON; }
        case (0x22EE): { return eON; }
        case (0x22EF): { return eON; }
        case (0x22F0): { return eON; }
        case (0x22F1): { return eON; }
        case (0x22F2): { return eON; }
        case (0x22F3): { return eON; }
        case (0x22F4): { return eON; }
        case (0x22F5): { return eON; }
        case (0x22F6): { return eON; }
        case (0x22F7): { return eON; }
        case (0x22F8): { return eON; }
        case (0x22F9): { return eON; }
        case (0x22FA): { return eON; }
        case (0x22FB): { return eON; }
        case (0x22FC): { return eON; }
        case (0x22FD): { return eON; }
        case (0x22FE): { return eON; }
        case (0x22FF): { return eON; }
        case (0x2300): { return eON; }
        case (0x2301): { return eON; }
        case (0x2302): { return eON; }
        case (0x2303): { return eON; }
        case (0x2304): { return eON; }
        case (0x2305): { return eON; }
        case (0x2306): { return eON; }
        case (0x2307): { return eON; }
        case (0x2308): { return eON; }
        case (0x2309): { return eON; }
        case (0x230A): { return eON; }
        case (0x230B): { return eON; }
        case (0x230C): { return eON; }
        case (0x230D): { return eON; }
        case (0x230E): { return eON; }
        case (0x230F): { return eON; }
        case (0x2310): { return eON; }
        case (0x2311): { return eON; }
        case (0x2312): { return eON; }
        case (0x2313): { return eON; }
        case (0x2314): { return eON; }
        case (0x2315): { return eON; }
        case (0x2316): { return eON; }
        case (0x2317): { return eON; }
        case (0x2318): { return eON; }
        case (0x2319): { return eON; }
        case (0x231A): { return eON; }
        case (0x231B): { return eON; }
        case (0x231C): { return eON; }
        case (0x231D): { return eON; }
        case (0x231E): { return eON; }
        case (0x231F): { return eON; }
        case (0x2320): { return eON; }
        case (0x2321): { return eON; }
        case (0x2322): { return eON; }
        case (0x2323): { return eON; }
        case (0x2324): { return eON; }
        case (0x2325): { return eON; }
        case (0x2326): { return eON; }
        case (0x2327): { return eON; }
        case (0x2328): { return eON; }
        case (0x2329): { return eON; }
        case (0x232A): { return eON; }
        case (0x232B): { return eON; }
        case (0x232C): { return eON; }
        case (0x232D): { return eON; }
        case (0x232E): { return eON; }
        case (0x232F): { return eON; }
        case (0x2330): { return eON; }
        case (0x2331): { return eON; }
        case (0x2332): { return eON; }
        case (0x2333): { return eON; }
        case (0x2334): { return eON; }
        case (0x2335): { return eON; }
        case (0x237B): { return eON; }
        case (0x237C): { return eON; }
        case (0x237D): { return eON; }
        case (0x237E): { return eON; }
        case (0x237F): { return eON; }
        case (0x2380): { return eON; }
        case (0x2381): { return eON; }
        case (0x2382): { return eON; }
        case (0x2383): { return eON; }
        case (0x2384): { return eON; }
        case (0x2385): { return eON; }
        case (0x2386): { return eON; }
        case (0x2387): { return eON; }
        case (0x2388): { return eON; }
        case (0x2389): { return eON; }
        case (0x238A): { return eON; }
        case (0x238B): { return eON; }
        case (0x238C): { return eON; }
        case (0x238D): { return eON; }
        case (0x238E): { return eON; }
        case (0x238F): { return eON; }
        case (0x2390): { return eON; }
        case (0x2391): { return eON; }
        case (0x2392): { return eON; }
        case (0x2393): { return eON; }
        case (0x2394): { return eON; }
        case (0x2396): { return eON; }
        case (0x2397): { return eON; }
        case (0x2398): { return eON; }
        case (0x2399): { return eON; }
        case (0x239A): { return eON; }
        case (0x239B): { return eON; }
        case (0x239C): { return eON; }
        case (0x239D): { return eON; }
        case (0x239E): { return eON; }
        case (0x239F): { return eON; }
        case (0x23A0): { return eON; }
        case (0x23A1): { return eON; }
        case (0x23A2): { return eON; }
        case (0x23A3): { return eON; }
        case (0x23A4): { return eON; }
        case (0x23A5): { return eON; }
        case (0x23A6): { return eON; }
        case (0x23A7): { return eON; }
        case (0x23A8): { return eON; }
        case (0x23A9): { return eON; }
        case (0x23AA): { return eON; }
        case (0x23AB): { return eON; }
        case (0x23AC): { return eON; }
        case (0x23AD): { return eON; }
        case (0x23AE): { return eON; }
        case (0x23AF): { return eON; }
        case (0x23B0): { return eON; }
        case (0x23B1): { return eON; }
        case (0x23B2): { return eON; }
        case (0x23B3): { return eON; }
        case (0x23B4): { return eON; }
        case (0x23B5): { return eON; }
        case (0x23B6): { return eON; }
        case (0x23B7): { return eON; }
        case (0x23B8): { return eON; }
        case (0x23B9): { return eON; }
        case (0x23BA): { return eON; }
        case (0x23BB): { return eON; }
        case (0x23BC): { return eON; }
        case (0x23BD): { return eON; }
        case (0x23BE): { return eON; }
        case (0x23BF): { return eON; }
        case (0x23C0): { return eON; }
        case (0x23C1): { return eON; }
        case (0x23C2): { return eON; }
        case (0x23C3): { return eON; }
        case (0x23C4): { return eON; }
        case (0x23C5): { return eON; }
        case (0x23C6): { return eON; }
        case (0x23C7): { return eON; }
        case (0x23C8): { return eON; }
        case (0x23C9): { return eON; }
        case (0x23CA): { return eON; }
        case (0x23CB): { return eON; }
        case (0x23CC): { return eON; }
        case (0x23CD): { return eON; }
        case (0x23CE): { return eON; }
        case (0x23CF): { return eON; }
        case (0x23D0): { return eON; }
        case (0x23D1): { return eON; }
        case (0x23D2): { return eON; }
        case (0x23D3): { return eON; }
        case (0x23D4): { return eON; }
        case (0x23D5): { return eON; }
        case (0x23D6): { return eON; }
        case (0x23D7): { return eON; }
        case (0x23D8): { return eON; }
        case (0x23D9): { return eON; }
        case (0x23DA): { return eON; }
        case (0x23DB): { return eON; }
        case (0x23DC): { return eON; }
        case (0x23DD): { return eON; }
        case (0x23DE): { return eON; }
        case (0x23DF): { return eON; }
        case (0x23E0): { return eON; }
        case (0x23E1): { return eON; }
        case (0x23E2): { return eON; }
        case (0x23E3): { return eON; }
        case (0x23E4): { return eON; }
        case (0x23E5): { return eON; }
        case (0x23E6): { return eON; }
        case (0x23E7): { return eON; }
        case (0x23E8): { return eON; }
        case (0x23E9): { return eON; }
        case (0x23EA): { return eON; }
        case (0x23EB): { return eON; }
        case (0x23EC): { return eON; }
        case (0x23ED): { return eON; }
        case (0x23EE): { return eON; }
        case (0x23EF): { return eON; }
        case (0x23F0): { return eON; }
        case (0x23F1): { return eON; }
        case (0x23F2): { return eON; }
        case (0x23F3): { return eON; }
        case (0x2400): { return eON; }
        case (0x2401): { return eON; }
        case (0x2402): { return eON; }
        case (0x2403): { return eON; }
        case (0x2404): { return eON; }
        case (0x2405): { return eON; }
        case (0x2406): { return eON; }
        case (0x2407): { return eON; }
        case (0x2408): { return eON; }
        case (0x2409): { return eON; }
        case (0x240A): { return eON; }
        case (0x240B): { return eON; }
        case (0x240C): { return eON; }
        case (0x240D): { return eON; }
        case (0x240E): { return eON; }
        case (0x240F): { return eON; }
        case (0x2410): { return eON; }
        case (0x2411): { return eON; }
        case (0x2412): { return eON; }
        case (0x2413): { return eON; }
        case (0x2414): { return eON; }
        case (0x2415): { return eON; }
        case (0x2416): { return eON; }
        case (0x2417): { return eON; }
        case (0x2418): { return eON; }
        case (0x2419): { return eON; }
        case (0x241A): { return eON; }
        case (0x241B): { return eON; }
        case (0x241C): { return eON; }
        case (0x241D): { return eON; }
        case (0x241E): { return eON; }
        case (0x241F): { return eON; }
        case (0x2420): { return eON; }
        case (0x2421): { return eON; }
        case (0x2422): { return eON; }
        case (0x2423): { return eON; }
        case (0x2424): { return eON; }
        case (0x2425): { return eON; }
        case (0x2426): { return eON; }
        case (0x2440): { return eON; }
        case (0x2441): { return eON; }
        case (0x2442): { return eON; }
        case (0x2443): { return eON; }
        case (0x2444): { return eON; }
        case (0x2445): { return eON; }
        case (0x2446): { return eON; }
        case (0x2447): { return eON; }
        case (0x2448): { return eON; }
        case (0x2449): { return eON; }
        case (0x244A): { return eON; }
        case (0x2460): { return eON; }
        case (0x2461): { return eON; }
        case (0x2462): { return eON; }
        case (0x2463): { return eON; }
        case (0x2464): { return eON; }
        case (0x2465): { return eON; }
        case (0x2466): { return eON; }
        case (0x2467): { return eON; }
        case (0x2468): { return eON; }
        case (0x2469): { return eON; }
        case (0x246A): { return eON; }
        case (0x246B): { return eON; }
        case (0x246C): { return eON; }
        case (0x246D): { return eON; }
        case (0x246E): { return eON; }
        case (0x246F): { return eON; }
        case (0x2470): { return eON; }
        case (0x2471): { return eON; }
        case (0x2472): { return eON; }
        case (0x2473): { return eON; }
        case (0x2474): { return eON; }
        case (0x2475): { return eON; }
        case (0x2476): { return eON; }
        case (0x2477): { return eON; }
        case (0x2478): { return eON; }
        case (0x2479): { return eON; }
        case (0x247A): { return eON; }
        case (0x247B): { return eON; }
        case (0x247C): { return eON; }
        case (0x247D): { return eON; }
        case (0x247E): { return eON; }
        case (0x247F): { return eON; }
        case (0x2480): { return eON; }
        case (0x2481): { return eON; }
        case (0x2482): { return eON; }
        case (0x2483): { return eON; }
        case (0x2484): { return eON; }
        case (0x2485): { return eON; }
        case (0x2486): { return eON; }
        case (0x2487): { return eON; }
        case (0x2488): { return eEN; }
        case (0x2489): { return eEN; }
        case (0x248A): { return eEN; }
        case (0x248B): { return eEN; }
        case (0x248C): { return eEN; }
        case (0x248D): { return eEN; }
        case (0x248E): { return eEN; }
        case (0x248F): { return eEN; }
        case (0x2490): { return eEN; }
        case (0x2491): { return eEN; }
        case (0x2492): { return eEN; }
        case (0x2493): { return eEN; }
        case (0x2494): { return eEN; }
        case (0x2495): { return eEN; }
        case (0x2496): { return eEN; }
        case (0x2497): { return eEN; }
        case (0x2498): { return eEN; }
        case (0x2499): { return eEN; }
        case (0x249A): { return eEN; }
        case (0x249B): { return eEN; }
        case (0x24EA): { return eON; }
        case (0x24EB): { return eON; }
        case (0x24EC): { return eON; }
        case (0x24ED): { return eON; }
        case (0x24EE): { return eON; }
        case (0x24EF): { return eON; }
        case (0x24F0): { return eON; }
        case (0x24F1): { return eON; }
        case (0x24F2): { return eON; }
        case (0x24F3): { return eON; }
        case (0x24F4): { return eON; }
        case (0x24F5): { return eON; }
        case (0x24F6): { return eON; }
        case (0x24F7): { return eON; }
        case (0x24F8): { return eON; }
        case (0x24F9): { return eON; }
        case (0x24FA): { return eON; }
        case (0x24FB): { return eON; }
        case (0x24FC): { return eON; }
        case (0x24FD): { return eON; }
        case (0x24FE): { return eON; }
        case (0x24FF): { return eON; }
        case (0x2500): { return eON; }
        case (0x2501): { return eON; }
        case (0x2502): { return eON; }
        case (0x2503): { return eON; }
        case (0x2504): { return eON; }
        case (0x2505): { return eON; }
        case (0x2506): { return eON; }
        case (0x2507): { return eON; }
        case (0x2508): { return eON; }
        case (0x2509): { return eON; }
        case (0x250A): { return eON; }
        case (0x250B): { return eON; }
        case (0x250C): { return eON; }
        case (0x250D): { return eON; }
        case (0x250E): { return eON; }
        case (0x250F): { return eON; }
        case (0x2510): { return eON; }
        case (0x2511): { return eON; }
        case (0x2512): { return eON; }
        case (0x2513): { return eON; }
        case (0x2514): { return eON; }
        case (0x2515): { return eON; }
        case (0x2516): { return eON; }
        case (0x2517): { return eON; }
        case (0x2518): { return eON; }
        case (0x2519): { return eON; }
        case (0x251A): { return eON; }
        case (0x251B): { return eON; }
        case (0x251C): { return eON; }
        case (0x251D): { return eON; }
        case (0x251E): { return eON; }
        case (0x251F): { return eON; }
        case (0x2520): { return eON; }
        case (0x2521): { return eON; }
        case (0x2522): { return eON; }
        case (0x2523): { return eON; }
        case (0x2524): { return eON; }
        case (0x2525): { return eON; }
        case (0x2526): { return eON; }
        case (0x2527): { return eON; }
        case (0x2528): { return eON; }
        case (0x2529): { return eON; }
        case (0x252A): { return eON; }
        case (0x252B): { return eON; }
        case (0x252C): { return eON; }
        case (0x252D): { return eON; }
        case (0x252E): { return eON; }
        case (0x252F): { return eON; }
        case (0x2530): { return eON; }
        case (0x2531): { return eON; }
        case (0x2532): { return eON; }
        case (0x2533): { return eON; }
        case (0x2534): { return eON; }
        case (0x2535): { return eON; }
        case (0x2536): { return eON; }
        case (0x2537): { return eON; }
        case (0x2538): { return eON; }
        case (0x2539): { return eON; }
        case (0x253A): { return eON; }
        case (0x253B): { return eON; }
        case (0x253C): { return eON; }
        case (0x253D): { return eON; }
        case (0x253E): { return eON; }
        case (0x253F): { return eON; }
        case (0x2540): { return eON; }
        case (0x2541): { return eON; }
        case (0x2542): { return eON; }
        case (0x2543): { return eON; }
        case (0x2544): { return eON; }
        case (0x2545): { return eON; }
        case (0x2546): { return eON; }
        case (0x2547): { return eON; }
        case (0x2548): { return eON; }
        case (0x2549): { return eON; }
        case (0x254A): { return eON; }
        case (0x254B): { return eON; }
        case (0x254C): { return eON; }
        case (0x254D): { return eON; }
        case (0x254E): { return eON; }
        case (0x254F): { return eON; }
        case (0x2550): { return eON; }
        case (0x2551): { return eON; }
        case (0x2552): { return eON; }
        case (0x2553): { return eON; }
        case (0x2554): { return eON; }
        case (0x2555): { return eON; }
        case (0x2556): { return eON; }
        case (0x2557): { return eON; }
        case (0x2558): { return eON; }
        case (0x2559): { return eON; }
        case (0x255A): { return eON; }
        case (0x255B): { return eON; }
        case (0x255C): { return eON; }
        case (0x255D): { return eON; }
        case (0x255E): { return eON; }
        case (0x255F): { return eON; }
        case (0x2560): { return eON; }
        case (0x2561): { return eON; }
        case (0x2562): { return eON; }
        case (0x2563): { return eON; }
        case (0x2564): { return eON; }
        case (0x2565): { return eON; }
        case (0x2566): { return eON; }
        case (0x2567): { return eON; }
        case (0x2568): { return eON; }
        case (0x2569): { return eON; }
        case (0x256A): { return eON; }
        case (0x256B): { return eON; }
        case (0x256C): { return eON; }
        case (0x256D): { return eON; }
        case (0x256E): { return eON; }
        case (0x256F): { return eON; }
        case (0x2570): { return eON; }
        case (0x2571): { return eON; }
        case (0x2572): { return eON; }
        case (0x2573): { return eON; }
        case (0x2574): { return eON; }
        case (0x2575): { return eON; }
        case (0x2576): { return eON; }
        case (0x2577): { return eON; }
        case (0x2578): { return eON; }
        case (0x2579): { return eON; }
        case (0x257A): { return eON; }
        case (0x257B): { return eON; }
        case (0x257C): { return eON; }
        case (0x257D): { return eON; }
        case (0x257E): { return eON; }
        case (0x257F): { return eON; }
        case (0x2580): { return eON; }
        case (0x2581): { return eON; }
        case (0x2582): { return eON; }
        case (0x2583): { return eON; }
        case (0x2584): { return eON; }
        case (0x2585): { return eON; }
        case (0x2586): { return eON; }
        case (0x2587): { return eON; }
        case (0x2588): { return eON; }
        case (0x2589): { return eON; }
        case (0x258A): { return eON; }
        case (0x258B): { return eON; }
        case (0x258C): { return eON; }
        case (0x258D): { return eON; }
        case (0x258E): { return eON; }
        case (0x258F): { return eON; }
        case (0x2590): { return eON; }
        case (0x2591): { return eON; }
        case (0x2592): { return eON; }
        case (0x2593): { return eON; }
        case (0x2594): { return eON; }
        case (0x2595): { return eON; }
        case (0x2596): { return eON; }
        case (0x2597): { return eON; }
        case (0x2598): { return eON; }
        case (0x2599): { return eON; }
        case (0x259A): { return eON; }
        case (0x259B): { return eON; }
        case (0x259C): { return eON; }
        case (0x259D): { return eON; }
        case (0x259E): { return eON; }
        case (0x259F): { return eON; }
        case (0x25A0): { return eON; }
        case (0x25A1): { return eON; }
        case (0x25A2): { return eON; }
        case (0x25A3): { return eON; }
        case (0x25A4): { return eON; }
        case (0x25A5): { return eON; }
        case (0x25A6): { return eON; }
        case (0x25A7): { return eON; }
        case (0x25A8): { return eON; }
        case (0x25A9): { return eON; }
        case (0x25AA): { return eON; }
        case (0x25AB): { return eON; }
        case (0x25AC): { return eON; }
        case (0x25AD): { return eON; }
        case (0x25AE): { return eON; }
        case (0x25AF): { return eON; }
        case (0x25B0): { return eON; }
        case (0x25B1): { return eON; }
        case (0x25B2): { return eON; }
        case (0x25B3): { return eON; }
        case (0x25B4): { return eON; }
        case (0x25B5): { return eON; }
        case (0x25B6): { return eON; }
        case (0x25B7): { return eON; }
        case (0x25B8): { return eON; }
        case (0x25B9): { return eON; }
        case (0x25BA): { return eON; }
        case (0x25BB): { return eON; }
        case (0x25BC): { return eON; }
        case (0x25BD): { return eON; }
        case (0x25BE): { return eON; }
        case (0x25BF): { return eON; }
        case (0x25C0): { return eON; }
        case (0x25C1): { return eON; }
        case (0x25C2): { return eON; }
        case (0x25C3): { return eON; }
        case (0x25C4): { return eON; }
        case (0x25C5): { return eON; }
        case (0x25C6): { return eON; }
        case (0x25C7): { return eON; }
        case (0x25C8): { return eON; }
        case (0x25C9): { return eON; }
        case (0x25CA): { return eON; }
        case (0x25CB): { return eON; }
        case (0x25CC): { return eON; }
        case (0x25CD): { return eON; }
        case (0x25CE): { return eON; }
        case (0x25CF): { return eON; }
        case (0x25D0): { return eON; }
        case (0x25D1): { return eON; }
        case (0x25D2): { return eON; }
        case (0x25D3): { return eON; }
        case (0x25D4): { return eON; }
        case (0x25D5): { return eON; }
        case (0x25D6): { return eON; }
        case (0x25D7): { return eON; }
        case (0x25D8): { return eON; }
        case (0x25D9): { return eON; }
        case (0x25DA): { return eON; }
        case (0x25DB): { return eON; }
        case (0x25DC): { return eON; }
        case (0x25DD): { return eON; }
        case (0x25DE): { return eON; }
        case (0x25DF): { return eON; }
        case (0x25E0): { return eON; }
        case (0x25E1): { return eON; }
        case (0x25E2): { return eON; }
        case (0x25E3): { return eON; }
        case (0x25E4): { return eON; }
        case (0x25E5): { return eON; }
        case (0x25E6): { return eON; }
        case (0x25E7): { return eON; }
        case (0x25E8): { return eON; }
        case (0x25E9): { return eON; }
        case (0x25EA): { return eON; }
        case (0x25EB): { return eON; }
        case (0x25EC): { return eON; }
        case (0x25ED): { return eON; }
        case (0x25EE): { return eON; }
        case (0x25EF): { return eON; }
        case (0x25F0): { return eON; }
        case (0x25F1): { return eON; }
        case (0x25F2): { return eON; }
        case (0x25F3): { return eON; }
        case (0x25F4): { return eON; }
        case (0x25F5): { return eON; }
        case (0x25F6): { return eON; }
        case (0x25F7): { return eON; }
        case (0x25F8): { return eON; }
        case (0x25F9): { return eON; }
        case (0x25FA): { return eON; }
        case (0x25FB): { return eON; }
        case (0x25FC): { return eON; }
        case (0x25FD): { return eON; }
        case (0x25FE): { return eON; }
        case (0x25FF): { return eON; }
        case (0x2600): { return eON; }
        case (0x2601): { return eON; }
        case (0x2602): { return eON; }
        case (0x2603): { return eON; }
        case (0x2604): { return eON; }
        case (0x2605): { return eON; }
        case (0x2606): { return eON; }
        case (0x2607): { return eON; }
        case (0x2608): { return eON; }
        case (0x2609): { return eON; }
        case (0x260A): { return eON; }
        case (0x260B): { return eON; }
        case (0x260C): { return eON; }
        case (0x260D): { return eON; }
        case (0x260E): { return eON; }
        case (0x260F): { return eON; }
        case (0x2610): { return eON; }
        case (0x2611): { return eON; }
        case (0x2612): { return eON; }
        case (0x2613): { return eON; }
        case (0x2614): { return eON; }
        case (0x2615): { return eON; }
        case (0x2616): { return eON; }
        case (0x2617): { return eON; }
        case (0x2618): { return eON; }
        case (0x2619): { return eON; }
        case (0x261A): { return eON; }
        case (0x261B): { return eON; }
        case (0x261C): { return eON; }
        case (0x261D): { return eON; }
        case (0x261E): { return eON; }
        case (0x261F): { return eON; }
        case (0x2620): { return eON; }
        case (0x2621): { return eON; }
        case (0x2622): { return eON; }
        case (0x2623): { return eON; }
        case (0x2624): { return eON; }
        case (0x2625): { return eON; }
        case (0x2626): { return eON; }
        case (0x2627): { return eON; }
        case (0x2628): { return eON; }
        case (0x2629): { return eON; }
        case (0x262A): { return eON; }
        case (0x262B): { return eON; }
        case (0x262C): { return eON; }
        case (0x262D): { return eON; }
        case (0x262E): { return eON; }
        case (0x262F): { return eON; }
        case (0x2630): { return eON; }
        case (0x2631): { return eON; }
        case (0x2632): { return eON; }
        case (0x2633): { return eON; }
        case (0x2634): { return eON; }
        case (0x2635): { return eON; }
        case (0x2636): { return eON; }
        case (0x2637): { return eON; }
        case (0x2638): { return eON; }
        case (0x2639): { return eON; }
        case (0x263A): { return eON; }
        case (0x263B): { return eON; }
        case (0x263C): { return eON; }
        case (0x263D): { return eON; }
        case (0x263E): { return eON; }
        case (0x263F): { return eON; }
        case (0x2640): { return eON; }
        case (0x2641): { return eON; }
        case (0x2642): { return eON; }
        case (0x2643): { return eON; }
        case (0x2644): { return eON; }
        case (0x2645): { return eON; }
        case (0x2646): { return eON; }
        case (0x2647): { return eON; }
        case (0x2648): { return eON; }
        case (0x2649): { return eON; }
        case (0x264A): { return eON; }
        case (0x264B): { return eON; }
        case (0x264C): { return eON; }
        case (0x264D): { return eON; }
        case (0x264E): { return eON; }
        case (0x264F): { return eON; }
        case (0x2650): { return eON; }
        case (0x2651): { return eON; }
        case (0x2652): { return eON; }
        case (0x2653): { return eON; }
        case (0x2654): { return eON; }
        case (0x2655): { return eON; }
        case (0x2656): { return eON; }
        case (0x2657): { return eON; }
        case (0x2658): { return eON; }
        case (0x2659): { return eON; }
        case (0x265A): { return eON; }
        case (0x265B): { return eON; }
        case (0x265C): { return eON; }
        case (0x265D): { return eON; }
        case (0x265E): { return eON; }
        case (0x265F): { return eON; }
        case (0x2660): { return eON; }
        case (0x2661): { return eON; }
        case (0x2662): { return eON; }
        case (0x2663): { return eON; }
        case (0x2664): { return eON; }
        case (0x2665): { return eON; }
        case (0x2666): { return eON; }
        case (0x2667): { return eON; }
        case (0x2668): { return eON; }
        case (0x2669): { return eON; }
        case (0x266A): { return eON; }
        case (0x266B): { return eON; }
        case (0x266C): { return eON; }
        case (0x266D): { return eON; }
        case (0x266E): { return eON; }
        case (0x266F): { return eON; }
        case (0x2670): { return eON; }
        case (0x2671): { return eON; }
        case (0x2672): { return eON; }
        case (0x2673): { return eON; }
        case (0x2674): { return eON; }
        case (0x2675): { return eON; }
        case (0x2676): { return eON; }
        case (0x2677): { return eON; }
        case (0x2678): { return eON; }
        case (0x2679): { return eON; }
        case (0x267A): { return eON; }
        case (0x267B): { return eON; }
        case (0x267C): { return eON; }
        case (0x267D): { return eON; }
        case (0x267E): { return eON; }
        case (0x267F): { return eON; }
        case (0x2680): { return eON; }
        case (0x2681): { return eON; }
        case (0x2682): { return eON; }
        case (0x2683): { return eON; }
        case (0x2684): { return eON; }
        case (0x2685): { return eON; }
        case (0x2686): { return eON; }
        case (0x2687): { return eON; }
        case (0x2688): { return eON; }
        case (0x2689): { return eON; }
        case (0x268A): { return eON; }
        case (0x268B): { return eON; }
        case (0x268C): { return eON; }
        case (0x268D): { return eON; }
        case (0x268E): { return eON; }
        case (0x268F): { return eON; }
        case (0x2690): { return eON; }
        case (0x2691): { return eON; }
        case (0x2692): { return eON; }
        case (0x2693): { return eON; }
        case (0x2694): { return eON; }
        case (0x2695): { return eON; }
        case (0x2696): { return eON; }
        case (0x2697): { return eON; }
        case (0x2698): { return eON; }
        case (0x2699): { return eON; }
        case (0x269A): { return eON; }
        case (0x269B): { return eON; }
        case (0x269C): { return eON; }
        case (0x269D): { return eON; }
        case (0x269E): { return eON; }
        case (0x269F): { return eON; }
        case (0x26A0): { return eON; }
        case (0x26A1): { return eON; }
        case (0x26A2): { return eON; }
        case (0x26A3): { return eON; }
        case (0x26A4): { return eON; }
        case (0x26A5): { return eON; }
        case (0x26A6): { return eON; }
        case (0x26A7): { return eON; }
        case (0x26A8): { return eON; }
        case (0x26A9): { return eON; }
        case (0x26AA): { return eON; }
        case (0x26AB): { return eON; }
        case (0x26AD): { return eON; }
        case (0x26AE): { return eON; }
        case (0x26AF): { return eON; }
        case (0x26B0): { return eON; }
        case (0x26B1): { return eON; }
        case (0x26B2): { return eON; }
        case (0x26B3): { return eON; }
        case (0x26B4): { return eON; }
        case (0x26B5): { return eON; }
        case (0x26B6): { return eON; }
        case (0x26B7): { return eON; }
        case (0x26B8): { return eON; }
        case (0x26B9): { return eON; }
        case (0x26BA): { return eON; }
        case (0x26BB): { return eON; }
        case (0x26BC): { return eON; }
        case (0x26BD): { return eON; }
        case (0x26BE): { return eON; }
        case (0x26BF): { return eON; }
        case (0x26C0): { return eON; }
        case (0x26C1): { return eON; }
        case (0x26C2): { return eON; }
        case (0x26C3): { return eON; }
        case (0x26C4): { return eON; }
        case (0x26C5): { return eON; }
        case (0x26C6): { return eON; }
        case (0x26C7): { return eON; }
        case (0x26C8): { return eON; }
        case (0x26C9): { return eON; }
        case (0x26CA): { return eON; }
        case (0x26CB): { return eON; }
        case (0x26CC): { return eON; }
        case (0x26CD): { return eON; }
        case (0x26CE): { return eON; }
        case (0x26CF): { return eON; }
        case (0x26D0): { return eON; }
        case (0x26D1): { return eON; }
        case (0x26D2): { return eON; }
        case (0x26D3): { return eON; }
        case (0x26D4): { return eON; }
        case (0x26D5): { return eON; }
        case (0x26D6): { return eON; }
        case (0x26D7): { return eON; }
        case (0x26D8): { return eON; }
        case (0x26D9): { return eON; }
        case (0x26DA): { return eON; }
        case (0x26DB): { return eON; }
        case (0x26DC): { return eON; }
        case (0x26DD): { return eON; }
        case (0x26DE): { return eON; }
        case (0x26DF): { return eON; }
        case (0x26E0): { return eON; }
        case (0x26E1): { return eON; }
        case (0x26E2): { return eON; }
        case (0x26E3): { return eON; }
        case (0x26E4): { return eON; }
        case (0x26E5): { return eON; }
        case (0x26E6): { return eON; }
        case (0x26E7): { return eON; }
        case (0x26E8): { return eON; }
        case (0x26E9): { return eON; }
        case (0x26EA): { return eON; }
        case (0x26EB): { return eON; }
        case (0x26EC): { return eON; }
        case (0x26ED): { return eON; }
        case (0x26EE): { return eON; }
        case (0x26EF): { return eON; }
        case (0x26F0): { return eON; }
        case (0x26F1): { return eON; }
        case (0x26F2): { return eON; }
        case (0x26F3): { return eON; }
        case (0x26F4): { return eON; }
        case (0x26F5): { return eON; }
        case (0x26F6): { return eON; }
        case (0x26F7): { return eON; }
        case (0x26F8): { return eON; }
        case (0x26F9): { return eON; }
        case (0x26FA): { return eON; }
        case (0x26FB): { return eON; }
        case (0x26FC): { return eON; }
        case (0x26FD): { return eON; }
        case (0x26FE): { return eON; }
        case (0x26FF): { return eON; }
        case (0x2701): { return eON; }
        case (0x2702): { return eON; }
        case (0x2703): { return eON; }
        case (0x2704): { return eON; }
        case (0x2705): { return eON; }
        case (0x2706): { return eON; }
        case (0x2707): { return eON; }
        case (0x2708): { return eON; }
        case (0x2709): { return eON; }
        case (0x270A): { return eON; }
        case (0x270B): { return eON; }
        case (0x270C): { return eON; }
        case (0x270D): { return eON; }
        case (0x270E): { return eON; }
        case (0x270F): { return eON; }
        case (0x2710): { return eON; }
        case (0x2711): { return eON; }
        case (0x2712): { return eON; }
        case (0x2713): { return eON; }
        case (0x2714): { return eON; }
        case (0x2715): { return eON; }
        case (0x2716): { return eON; }
        case (0x2717): { return eON; }
        case (0x2718): { return eON; }
        case (0x2719): { return eON; }
        case (0x271A): { return eON; }
        case (0x271B): { return eON; }
        case (0x271C): { return eON; }
        case (0x271D): { return eON; }
        case (0x271E): { return eON; }
        case (0x271F): { return eON; }
        case (0x2720): { return eON; }
        case (0x2721): { return eON; }
        case (0x2722): { return eON; }
        case (0x2723): { return eON; }
        case (0x2724): { return eON; }
        case (0x2725): { return eON; }
        case (0x2726): { return eON; }
        case (0x2727): { return eON; }
        case (0x2728): { return eON; }
        case (0x2729): { return eON; }
        case (0x272A): { return eON; }
        case (0x272B): { return eON; }
        case (0x272C): { return eON; }
        case (0x272D): { return eON; }
        case (0x272E): { return eON; }
        case (0x272F): { return eON; }
        case (0x2730): { return eON; }
        case (0x2731): { return eON; }
        case (0x2732): { return eON; }
        case (0x2733): { return eON; }
        case (0x2734): { return eON; }
        case (0x2735): { return eON; }
        case (0x2736): { return eON; }
        case (0x2737): { return eON; }
        case (0x2738): { return eON; }
        case (0x2739): { return eON; }
        case (0x273A): { return eON; }
        case (0x273B): { return eON; }
        case (0x273C): { return eON; }
        case (0x273D): { return eON; }
        case (0x273E): { return eON; }
        case (0x273F): { return eON; }
        case (0x2740): { return eON; }
        case (0x2741): { return eON; }
        case (0x2742): { return eON; }
        case (0x2743): { return eON; }
        case (0x2744): { return eON; }
        case (0x2745): { return eON; }
        case (0x2746): { return eON; }
        case (0x2747): { return eON; }
        case (0x2748): { return eON; }
        case (0x2749): { return eON; }
        case (0x274A): { return eON; }
        case (0x274B): { return eON; }
        case (0x274C): { return eON; }
        case (0x274D): { return eON; }
        case (0x274E): { return eON; }
        case (0x274F): { return eON; }
        case (0x2750): { return eON; }
        case (0x2751): { return eON; }
        case (0x2752): { return eON; }
        case (0x2753): { return eON; }
        case (0x2754): { return eON; }
        case (0x2755): { return eON; }
        case (0x2756): { return eON; }
        case (0x2757): { return eON; }
        case (0x2758): { return eON; }
        case (0x2759): { return eON; }
        case (0x275A): { return eON; }
        case (0x275B): { return eON; }
        case (0x275C): { return eON; }
        case (0x275D): { return eON; }
        case (0x275E): { return eON; }
        case (0x275F): { return eON; }
        case (0x2760): { return eON; }
        case (0x2761): { return eON; }
        case (0x2762): { return eON; }
        case (0x2763): { return eON; }
        case (0x2764): { return eON; }
        case (0x2765): { return eON; }
        case (0x2766): { return eON; }
        case (0x2767): { return eON; }
        case (0x2768): { return eON; }
        case (0x2769): { return eON; }
        case (0x276A): { return eON; }
        case (0x276B): { return eON; }
        case (0x276C): { return eON; }
        case (0x276D): { return eON; }
        case (0x276E): { return eON; }
        case (0x276F): { return eON; }
        case (0x2770): { return eON; }
        case (0x2771): { return eON; }
        case (0x2772): { return eON; }
        case (0x2773): { return eON; }
        case (0x2774): { return eON; }
        case (0x2775): { return eON; }
        case (0x2776): { return eON; }
        case (0x2777): { return eON; }
        case (0x2778): { return eON; }
        case (0x2779): { return eON; }
        case (0x277A): { return eON; }
        case (0x277B): { return eON; }
        case (0x277C): { return eON; }
        case (0x277D): { return eON; }
        case (0x277E): { return eON; }
        case (0x277F): { return eON; }
        case (0x2780): { return eON; }
        case (0x2781): { return eON; }
        case (0x2782): { return eON; }
        case (0x2783): { return eON; }
        case (0x2784): { return eON; }
        case (0x2785): { return eON; }
        case (0x2786): { return eON; }
        case (0x2787): { return eON; }
        case (0x2788): { return eON; }
        case (0x2789): { return eON; }
        case (0x278A): { return eON; }
        case (0x278B): { return eON; }
        case (0x278C): { return eON; }
        case (0x278D): { return eON; }
        case (0x278E): { return eON; }
        case (0x278F): { return eON; }
        case (0x2790): { return eON; }
        case (0x2791): { return eON; }
        case (0x2792): { return eON; }
        case (0x2793): { return eON; }
        case (0x2794): { return eON; }
        case (0x2795): { return eON; }
        case (0x2796): { return eON; }
        case (0x2797): { return eON; }
        case (0x2798): { return eON; }
        case (0x2799): { return eON; }
        case (0x279A): { return eON; }
        case (0x279B): { return eON; }
        case (0x279C): { return eON; }
        case (0x279D): { return eON; }
        case (0x279E): { return eON; }
        case (0x279F): { return eON; }
        case (0x27A0): { return eON; }
        case (0x27A1): { return eON; }
        case (0x27A2): { return eON; }
        case (0x27A3): { return eON; }
        case (0x27A4): { return eON; }
        case (0x27A5): { return eON; }
        case (0x27A6): { return eON; }
        case (0x27A7): { return eON; }
        case (0x27A8): { return eON; }
        case (0x27A9): { return eON; }
        case (0x27AA): { return eON; }
        case (0x27AB): { return eON; }
        case (0x27AC): { return eON; }
        case (0x27AD): { return eON; }
        case (0x27AE): { return eON; }
        case (0x27AF): { return eON; }
        case (0x27B0): { return eON; }
        case (0x27B1): { return eON; }
        case (0x27B2): { return eON; }
        case (0x27B3): { return eON; }
        case (0x27B4): { return eON; }
        case (0x27B5): { return eON; }
        case (0x27B6): { return eON; }
        case (0x27B7): { return eON; }
        case (0x27B8): { return eON; }
        case (0x27B9): { return eON; }
        case (0x27BA): { return eON; }
        case (0x27BB): { return eON; }
        case (0x27BC): { return eON; }
        case (0x27BD): { return eON; }
        case (0x27BE): { return eON; }
        case (0x27BF): { return eON; }
        case (0x27C0): { return eON; }
        case (0x27C1): { return eON; }
        case (0x27C2): { return eON; }
        case (0x27C3): { return eON; }
        case (0x27C4): { return eON; }
        case (0x27C5): { return eON; }
        case (0x27C6): { return eON; }
        case (0x27C7): { return eON; }
        case (0x27C8): { return eON; }
        case (0x27C9): { return eON; }
        case (0x27CA): { return eON; }
        case (0x27CC): { return eON; }
        case (0x27CE): { return eON; }
        case (0x27CF): { return eON; }
        case (0x27D0): { return eON; }
        case (0x27D1): { return eON; }
        case (0x27D2): { return eON; }
        case (0x27D3): { return eON; }
        case (0x27D4): { return eON; }
        case (0x27D5): { return eON; }
        case (0x27D6): { return eON; }
        case (0x27D7): { return eON; }
        case (0x27D8): { return eON; }
        case (0x27D9): { return eON; }
        case (0x27DA): { return eON; }
        case (0x27DB): { return eON; }
        case (0x27DC): { return eON; }
        case (0x27DD): { return eON; }
        case (0x27DE): { return eON; }
        case (0x27DF): { return eON; }
        case (0x27E0): { return eON; }
        case (0x27E1): { return eON; }
        case (0x27E2): { return eON; }
        case (0x27E3): { return eON; }
        case (0x27E4): { return eON; }
        case (0x27E5): { return eON; }
        case (0x27E6): { return eON; }
        case (0x27E7): { return eON; }
        case (0x27E8): { return eON; }
        case (0x27E9): { return eON; }
        case (0x27EA): { return eON; }
        case (0x27EB): { return eON; }
        case (0x27EC): { return eON; }
        case (0x27ED): { return eON; }
        case (0x27EE): { return eON; }
        case (0x27EF): { return eON; }
        case (0x27F0): { return eON; }
        case (0x27F1): { return eON; }
        case (0x27F2): { return eON; }
        case (0x27F3): { return eON; }
        case (0x27F4): { return eON; }
        case (0x27F5): { return eON; }
        case (0x27F6): { return eON; }
        case (0x27F7): { return eON; }
        case (0x27F8): { return eON; }
        case (0x27F9): { return eON; }
        case (0x27FA): { return eON; }
        case (0x27FB): { return eON; }
        case (0x27FC): { return eON; }
        case (0x27FD): { return eON; }
        case (0x27FE): { return eON; }
        case (0x27FF): { return eON; }
        case (0x2900): { return eON; }
        case (0x2901): { return eON; }
        case (0x2902): { return eON; }
        case (0x2903): { return eON; }
        case (0x2904): { return eON; }
        case (0x2905): { return eON; }
        case (0x2906): { return eON; }
        case (0x2907): { return eON; }
        case (0x2908): { return eON; }
        case (0x2909): { return eON; }
        case (0x290A): { return eON; }
        case (0x290B): { return eON; }
        case (0x290C): { return eON; }
        case (0x290D): { return eON; }
        case (0x290E): { return eON; }
        case (0x290F): { return eON; }
        case (0x2910): { return eON; }
        case (0x2911): { return eON; }
        case (0x2912): { return eON; }
        case (0x2913): { return eON; }
        case (0x2914): { return eON; }
        case (0x2915): { return eON; }
        case (0x2916): { return eON; }
        case (0x2917): { return eON; }
        case (0x2918): { return eON; }
        case (0x2919): { return eON; }
        case (0x291A): { return eON; }
        case (0x291B): { return eON; }
        case (0x291C): { return eON; }
        case (0x291D): { return eON; }
        case (0x291E): { return eON; }
        case (0x291F): { return eON; }
        case (0x2920): { return eON; }
        case (0x2921): { return eON; }
        case (0x2922): { return eON; }
        case (0x2923): { return eON; }
        case (0x2924): { return eON; }
        case (0x2925): { return eON; }
        case (0x2926): { return eON; }
        case (0x2927): { return eON; }
        case (0x2928): { return eON; }
        case (0x2929): { return eON; }
        case (0x292A): { return eON; }
        case (0x292B): { return eON; }
        case (0x292C): { return eON; }
        case (0x292D): { return eON; }
        case (0x292E): { return eON; }
        case (0x292F): { return eON; }
        case (0x2930): { return eON; }
        case (0x2931): { return eON; }
        case (0x2932): { return eON; }
        case (0x2933): { return eON; }
        case (0x2934): { return eON; }
        case (0x2935): { return eON; }
        case (0x2936): { return eON; }
        case (0x2937): { return eON; }
        case (0x2938): { return eON; }
        case (0x2939): { return eON; }
        case (0x293A): { return eON; }
        case (0x293B): { return eON; }
        case (0x293C): { return eON; }
        case (0x293D): { return eON; }
        case (0x293E): { return eON; }
        case (0x293F): { return eON; }
        case (0x2940): { return eON; }
        case (0x2941): { return eON; }
        case (0x2942): { return eON; }
        case (0x2943): { return eON; }
        case (0x2944): { return eON; }
        case (0x2945): { return eON; }
        case (0x2946): { return eON; }
        case (0x2947): { return eON; }
        case (0x2948): { return eON; }
        case (0x2949): { return eON; }
        case (0x294A): { return eON; }
        case (0x294B): { return eON; }
        case (0x294C): { return eON; }
        case (0x294D): { return eON; }
        case (0x294E): { return eON; }
        case (0x294F): { return eON; }
        case (0x2950): { return eON; }
        case (0x2951): { return eON; }
        case (0x2952): { return eON; }
        case (0x2953): { return eON; }
        case (0x2954): { return eON; }
        case (0x2955): { return eON; }
        case (0x2956): { return eON; }
        case (0x2957): { return eON; }
        case (0x2958): { return eON; }
        case (0x2959): { return eON; }
        case (0x295A): { return eON; }
        case (0x295B): { return eON; }
        case (0x295C): { return eON; }
        case (0x295D): { return eON; }
        case (0x295E): { return eON; }
        case (0x295F): { return eON; }
        case (0x2960): { return eON; }
        case (0x2961): { return eON; }
        case (0x2962): { return eON; }
        case (0x2963): { return eON; }
        case (0x2964): { return eON; }
        case (0x2965): { return eON; }
        case (0x2966): { return eON; }
        case (0x2967): { return eON; }
        case (0x2968): { return eON; }
        case (0x2969): { return eON; }
        case (0x296A): { return eON; }
        case (0x296B): { return eON; }
        case (0x296C): { return eON; }
        case (0x296D): { return eON; }
        case (0x296E): { return eON; }
        case (0x296F): { return eON; }
        case (0x2970): { return eON; }
        case (0x2971): { return eON; }
        case (0x2972): { return eON; }
        case (0x2973): { return eON; }
        case (0x2974): { return eON; }
        case (0x2975): { return eON; }
        case (0x2976): { return eON; }
        case (0x2977): { return eON; }
        case (0x2978): { return eON; }
        case (0x2979): { return eON; }
        case (0x297A): { return eON; }
        case (0x297B): { return eON; }
        case (0x297C): { return eON; }
        case (0x297D): { return eON; }
        case (0x297E): { return eON; }
        case (0x297F): { return eON; }
        case (0x2980): { return eON; }
        case (0x2981): { return eON; }
        case (0x2982): { return eON; }
        case (0x2983): { return eON; }
        case (0x2984): { return eON; }
        case (0x2985): { return eON; }
        case (0x2986): { return eON; }
        case (0x2987): { return eON; }
        case (0x2988): { return eON; }
        case (0x2989): { return eON; }
        case (0x298A): { return eON; }
        case (0x298B): { return eON; }
        case (0x298C): { return eON; }
        case (0x298D): { return eON; }
        case (0x298E): { return eON; }
        case (0x298F): { return eON; }
        case (0x2990): { return eON; }
        case (0x2991): { return eON; }
        case (0x2992): { return eON; }
        case (0x2993): { return eON; }
        case (0x2994): { return eON; }
        case (0x2995): { return eON; }
        case (0x2996): { return eON; }
        case (0x2997): { return eON; }
        case (0x2998): { return eON; }
        case (0x2999): { return eON; }
        case (0x299A): { return eON; }
        case (0x299B): { return eON; }
        case (0x299C): { return eON; }
        case (0x299D): { return eON; }
        case (0x299E): { return eON; }
        case (0x299F): { return eON; }
        case (0x29A0): { return eON; }
        case (0x29A1): { return eON; }
        case (0x29A2): { return eON; }
        case (0x29A3): { return eON; }
        case (0x29A4): { return eON; }
        case (0x29A5): { return eON; }
        case (0x29A6): { return eON; }
        case (0x29A7): { return eON; }
        case (0x29A8): { return eON; }
        case (0x29A9): { return eON; }
        case (0x29AA): { return eON; }
        case (0x29AB): { return eON; }
        case (0x29AC): { return eON; }
        case (0x29AD): { return eON; }
        case (0x29AE): { return eON; }
        case (0x29AF): { return eON; }
        case (0x29B0): { return eON; }
        case (0x29B1): { return eON; }
        case (0x29B2): { return eON; }
        case (0x29B3): { return eON; }
        case (0x29B4): { return eON; }
        case (0x29B5): { return eON; }
        case (0x29B6): { return eON; }
        case (0x29B7): { return eON; }
        case (0x29B8): { return eON; }
        case (0x29B9): { return eON; }
        case (0x29BA): { return eON; }
        case (0x29BB): { return eON; }
        case (0x29BC): { return eON; }
        case (0x29BD): { return eON; }
        case (0x29BE): { return eON; }
        case (0x29BF): { return eON; }
        case (0x29C0): { return eON; }
        case (0x29C1): { return eON; }
        case (0x29C2): { return eON; }
        case (0x29C3): { return eON; }
        case (0x29C4): { return eON; }
        case (0x29C5): { return eON; }
        case (0x29C6): { return eON; }
        case (0x29C7): { return eON; }
        case (0x29C8): { return eON; }
        case (0x29C9): { return eON; }
        case (0x29CA): { return eON; }
        case (0x29CB): { return eON; }
        case (0x29CC): { return eON; }
        case (0x29CD): { return eON; }
        case (0x29CE): { return eON; }
        case (0x29CF): { return eON; }
        case (0x29D0): { return eON; }
        case (0x29D1): { return eON; }
        case (0x29D2): { return eON; }
        case (0x29D3): { return eON; }
        case (0x29D4): { return eON; }
        case (0x29D5): { return eON; }
        case (0x29D6): { return eON; }
        case (0x29D7): { return eON; }
        case (0x29D8): { return eON; }
        case (0x29D9): { return eON; }
        case (0x29DA): { return eON; }
        case (0x29DB): { return eON; }
        case (0x29DC): { return eON; }
        case (0x29DD): { return eON; }
        case (0x29DE): { return eON; }
        case (0x29DF): { return eON; }
        case (0x29E0): { return eON; }
        case (0x29E1): { return eON; }
        case (0x29E2): { return eON; }
        case (0x29E3): { return eON; }
        case (0x29E4): { return eON; }
        case (0x29E5): { return eON; }
        case (0x29E6): { return eON; }
        case (0x29E7): { return eON; }
        case (0x29E8): { return eON; }
        case (0x29E9): { return eON; }
        case (0x29EA): { return eON; }
        case (0x29EB): { return eON; }
        case (0x29EC): { return eON; }
        case (0x29ED): { return eON; }
        case (0x29EE): { return eON; }
        case (0x29EF): { return eON; }
        case (0x29F0): { return eON; }
        case (0x29F1): { return eON; }
        case (0x29F2): { return eON; }
        case (0x29F3): { return eON; }
        case (0x29F4): { return eON; }
        case (0x29F5): { return eON; }
        case (0x29F6): { return eON; }
        case (0x29F7): { return eON; }
        case (0x29F8): { return eON; }
        case (0x29F9): { return eON; }
        case (0x29FA): { return eON; }
        case (0x29FB): { return eON; }
        case (0x29FC): { return eON; }
        case (0x29FD): { return eON; }
        case (0x29FE): { return eON; }
        case (0x29FF): { return eON; }
        case (0x2A00): { return eON; }
        case (0x2A01): { return eON; }
        case (0x2A02): { return eON; }
        case (0x2A03): { return eON; }
        case (0x2A04): { return eON; }
        case (0x2A05): { return eON; }
        case (0x2A06): { return eON; }
        case (0x2A07): { return eON; }
        case (0x2A08): { return eON; }
        case (0x2A09): { return eON; }
        case (0x2A0A): { return eON; }
        case (0x2A0B): { return eON; }
        case (0x2A0C): { return eON; }
        case (0x2A0D): { return eON; }
        case (0x2A0E): { return eON; }
        case (0x2A0F): { return eON; }
        case (0x2A10): { return eON; }
        case (0x2A11): { return eON; }
        case (0x2A12): { return eON; }
        case (0x2A13): { return eON; }
        case (0x2A14): { return eON; }
        case (0x2A15): { return eON; }
        case (0x2A16): { return eON; }
        case (0x2A17): { return eON; }
        case (0x2A18): { return eON; }
        case (0x2A19): { return eON; }
        case (0x2A1A): { return eON; }
        case (0x2A1B): { return eON; }
        case (0x2A1C): { return eON; }
        case (0x2A1D): { return eON; }
        case (0x2A1E): { return eON; }
        case (0x2A1F): { return eON; }
        case (0x2A20): { return eON; }
        case (0x2A21): { return eON; }
        case (0x2A22): { return eON; }
        case (0x2A23): { return eON; }
        case (0x2A24): { return eON; }
        case (0x2A25): { return eON; }
        case (0x2A26): { return eON; }
        case (0x2A27): { return eON; }
        case (0x2A28): { return eON; }
        case (0x2A29): { return eON; }
        case (0x2A2A): { return eON; }
        case (0x2A2B): { return eON; }
        case (0x2A2C): { return eON; }
        case (0x2A2D): { return eON; }
        case (0x2A2E): { return eON; }
        case (0x2A2F): { return eON; }
        case (0x2A30): { return eON; }
        case (0x2A31): { return eON; }
        case (0x2A32): { return eON; }
        case (0x2A33): { return eON; }
        case (0x2A34): { return eON; }
        case (0x2A35): { return eON; }
        case (0x2A36): { return eON; }
        case (0x2A37): { return eON; }
        case (0x2A38): { return eON; }
        case (0x2A39): { return eON; }
        case (0x2A3A): { return eON; }
        case (0x2A3B): { return eON; }
        case (0x2A3C): { return eON; }
        case (0x2A3D): { return eON; }
        case (0x2A3E): { return eON; }
        case (0x2A3F): { return eON; }
        case (0x2A40): { return eON; }
        case (0x2A41): { return eON; }
        case (0x2A42): { return eON; }
        case (0x2A43): { return eON; }
        case (0x2A44): { return eON; }
        case (0x2A45): { return eON; }
        case (0x2A46): { return eON; }
        case (0x2A47): { return eON; }
        case (0x2A48): { return eON; }
        case (0x2A49): { return eON; }
        case (0x2A4A): { return eON; }
        case (0x2A4B): { return eON; }
        case (0x2A4C): { return eON; }
        case (0x2A4D): { return eON; }
        case (0x2A4E): { return eON; }
        case (0x2A4F): { return eON; }
        case (0x2A50): { return eON; }
        case (0x2A51): { return eON; }
        case (0x2A52): { return eON; }
        case (0x2A53): { return eON; }
        case (0x2A54): { return eON; }
        case (0x2A55): { return eON; }
        case (0x2A56): { return eON; }
        case (0x2A57): { return eON; }
        case (0x2A58): { return eON; }
        case (0x2A59): { return eON; }
        case (0x2A5A): { return eON; }
        case (0x2A5B): { return eON; }
        case (0x2A5C): { return eON; }
        case (0x2A5D): { return eON; }
        case (0x2A5E): { return eON; }
        case (0x2A5F): { return eON; }
        case (0x2A60): { return eON; }
        case (0x2A61): { return eON; }
        case (0x2A62): { return eON; }
        case (0x2A63): { return eON; }
        case (0x2A64): { return eON; }
        case (0x2A65): { return eON; }
        case (0x2A66): { return eON; }
        case (0x2A67): { return eON; }
        case (0x2A68): { return eON; }
        case (0x2A69): { return eON; }
        case (0x2A6A): { return eON; }
        case (0x2A6B): { return eON; }
        case (0x2A6C): { return eON; }
        case (0x2A6D): { return eON; }
        case (0x2A6E): { return eON; }
        case (0x2A6F): { return eON; }
        case (0x2A70): { return eON; }
        case (0x2A71): { return eON; }
        case (0x2A72): { return eON; }
        case (0x2A73): { return eON; }
        case (0x2A74): { return eON; }
        case (0x2A75): { return eON; }
        case (0x2A76): { return eON; }
        case (0x2A77): { return eON; }
        case (0x2A78): { return eON; }
        case (0x2A79): { return eON; }
        case (0x2A7A): { return eON; }
        case (0x2A7B): { return eON; }
        case (0x2A7C): { return eON; }
        case (0x2A7D): { return eON; }
        case (0x2A7E): { return eON; }
        case (0x2A7F): { return eON; }
        case (0x2A80): { return eON; }
        case (0x2A81): { return eON; }
        case (0x2A82): { return eON; }
        case (0x2A83): { return eON; }
        case (0x2A84): { return eON; }
        case (0x2A85): { return eON; }
        case (0x2A86): { return eON; }
        case (0x2A87): { return eON; }
        case (0x2A88): { return eON; }
        case (0x2A89): { return eON; }
        case (0x2A8A): { return eON; }
        case (0x2A8B): { return eON; }
        case (0x2A8C): { return eON; }
        case (0x2A8D): { return eON; }
        case (0x2A8E): { return eON; }
        case (0x2A8F): { return eON; }
        case (0x2A90): { return eON; }
        case (0x2A91): { return eON; }
        case (0x2A92): { return eON; }
        case (0x2A93): { return eON; }
        case (0x2A94): { return eON; }
        case (0x2A95): { return eON; }
        case (0x2A96): { return eON; }
        case (0x2A97): { return eON; }
        case (0x2A98): { return eON; }
        case (0x2A99): { return eON; }
        case (0x2A9A): { return eON; }
        case (0x2A9B): { return eON; }
        case (0x2A9C): { return eON; }
        case (0x2A9D): { return eON; }
        case (0x2A9E): { return eON; }
        case (0x2A9F): { return eON; }
        case (0x2AA0): { return eON; }
        case (0x2AA1): { return eON; }
        case (0x2AA2): { return eON; }
        case (0x2AA3): { return eON; }
        case (0x2AA4): { return eON; }
        case (0x2AA5): { return eON; }
        case (0x2AA6): { return eON; }
        case (0x2AA7): { return eON; }
        case (0x2AA8): { return eON; }
        case (0x2AA9): { return eON; }
        case (0x2AAA): { return eON; }
        case (0x2AAB): { return eON; }
        case (0x2AAC): { return eON; }
        case (0x2AAD): { return eON; }
        case (0x2AAE): { return eON; }
        case (0x2AAF): { return eON; }
        case (0x2AB0): { return eON; }
        case (0x2AB1): { return eON; }
        case (0x2AB2): { return eON; }
        case (0x2AB3): { return eON; }
        case (0x2AB4): { return eON; }
        case (0x2AB5): { return eON; }
        case (0x2AB6): { return eON; }
        case (0x2AB7): { return eON; }
        case (0x2AB8): { return eON; }
        case (0x2AB9): { return eON; }
        case (0x2ABA): { return eON; }
        case (0x2ABB): { return eON; }
        case (0x2ABC): { return eON; }
        case (0x2ABD): { return eON; }
        case (0x2ABE): { return eON; }
        case (0x2ABF): { return eON; }
        case (0x2AC0): { return eON; }
        case (0x2AC1): { return eON; }
        case (0x2AC2): { return eON; }
        case (0x2AC3): { return eON; }
        case (0x2AC4): { return eON; }
        case (0x2AC5): { return eON; }
        case (0x2AC6): { return eON; }
        case (0x2AC7): { return eON; }
        case (0x2AC8): { return eON; }
        case (0x2AC9): { return eON; }
        case (0x2ACA): { return eON; }
        case (0x2ACB): { return eON; }
        case (0x2ACC): { return eON; }
        case (0x2ACD): { return eON; }
        case (0x2ACE): { return eON; }
        case (0x2ACF): { return eON; }
        case (0x2AD0): { return eON; }
        case (0x2AD1): { return eON; }
        case (0x2AD2): { return eON; }
        case (0x2AD3): { return eON; }
        case (0x2AD4): { return eON; }
        case (0x2AD5): { return eON; }
        case (0x2AD6): { return eON; }
        case (0x2AD7): { return eON; }
        case (0x2AD8): { return eON; }
        case (0x2AD9): { return eON; }
        case (0x2ADA): { return eON; }
        case (0x2ADB): { return eON; }
        case (0x2ADC): { return eON; }
        case (0x2ADD): { return eON; }
        case (0x2ADE): { return eON; }
        case (0x2ADF): { return eON; }
        case (0x2AE0): { return eON; }
        case (0x2AE1): { return eON; }
        case (0x2AE2): { return eON; }
        case (0x2AE3): { return eON; }
        case (0x2AE4): { return eON; }
        case (0x2AE5): { return eON; }
        case (0x2AE6): { return eON; }
        case (0x2AE7): { return eON; }
        case (0x2AE8): { return eON; }
        case (0x2AE9): { return eON; }
        case (0x2AEA): { return eON; }
        case (0x2AEB): { return eON; }
        case (0x2AEC): { return eON; }
        case (0x2AED): { return eON; }
        case (0x2AEE): { return eON; }
        case (0x2AEF): { return eON; }
        case (0x2AF0): { return eON; }
        case (0x2AF1): { return eON; }
        case (0x2AF2): { return eON; }
        case (0x2AF3): { return eON; }
        case (0x2AF4): { return eON; }
        case (0x2AF5): { return eON; }
        case (0x2AF6): { return eON; }
        case (0x2AF7): { return eON; }
        case (0x2AF8): { return eON; }
        case (0x2AF9): { return eON; }
        case (0x2AFA): { return eON; }
        case (0x2AFB): { return eON; }
        case (0x2AFC): { return eON; }
        case (0x2AFD): { return eON; }
        case (0x2AFE): { return eON; }
        case (0x2AFF): { return eON; }
        case (0x2B00): { return eON; }
        case (0x2B01): { return eON; }
        case (0x2B02): { return eON; }
        case (0x2B03): { return eON; }
        case (0x2B04): { return eON; }
        case (0x2B05): { return eON; }
        case (0x2B06): { return eON; }
        case (0x2B07): { return eON; }
        case (0x2B08): { return eON; }
        case (0x2B09): { return eON; }
        case (0x2B0A): { return eON; }
        case (0x2B0B): { return eON; }
        case (0x2B0C): { return eON; }
        case (0x2B0D): { return eON; }
        case (0x2B0E): { return eON; }
        case (0x2B0F): { return eON; }
        case (0x2B10): { return eON; }
        case (0x2B11): { return eON; }
        case (0x2B12): { return eON; }
        case (0x2B13): { return eON; }
        case (0x2B14): { return eON; }
        case (0x2B15): { return eON; }
        case (0x2B16): { return eON; }
        case (0x2B17): { return eON; }
        case (0x2B18): { return eON; }
        case (0x2B19): { return eON; }
        case (0x2B1A): { return eON; }
        case (0x2B1B): { return eON; }
        case (0x2B1C): { return eON; }
        case (0x2B1D): { return eON; }
        case (0x2B1E): { return eON; }
        case (0x2B1F): { return eON; }
        case (0x2B20): { return eON; }
        case (0x2B21): { return eON; }
        case (0x2B22): { return eON; }
        case (0x2B23): { return eON; }
        case (0x2B24): { return eON; }
        case (0x2B25): { return eON; }
        case (0x2B26): { return eON; }
        case (0x2B27): { return eON; }
        case (0x2B28): { return eON; }
        case (0x2B29): { return eON; }
        case (0x2B2A): { return eON; }
        case (0x2B2B): { return eON; }
        case (0x2B2C): { return eON; }
        case (0x2B2D): { return eON; }
        case (0x2B2E): { return eON; }
        case (0x2B2F): { return eON; }
        case (0x2B30): { return eON; }
        case (0x2B31): { return eON; }
        case (0x2B32): { return eON; }
        case (0x2B33): { return eON; }
        case (0x2B34): { return eON; }
        case (0x2B35): { return eON; }
        case (0x2B36): { return eON; }
        case (0x2B37): { return eON; }
        case (0x2B38): { return eON; }
        case (0x2B39): { return eON; }
        case (0x2B3A): { return eON; }
        case (0x2B3B): { return eON; }
        case (0x2B3C): { return eON; }
        case (0x2B3D): { return eON; }
        case (0x2B3E): { return eON; }
        case (0x2B3F): { return eON; }
        case (0x2B40): { return eON; }
        case (0x2B41): { return eON; }
        case (0x2B42): { return eON; }
        case (0x2B43): { return eON; }
        case (0x2B44): { return eON; }
        case (0x2B45): { return eON; }
        case (0x2B46): { return eON; }
        case (0x2B47): { return eON; }
        case (0x2B48): { return eON; }
        case (0x2B49): { return eON; }
        case (0x2B4A): { return eON; }
        case (0x2B4B): { return eON; }
        case (0x2B4C): { return eON; }
        case (0x2B50): { return eON; }
        case (0x2B51): { return eON; }
        case (0x2B52): { return eON; }
        case (0x2B53): { return eON; }
        case (0x2B54): { return eON; }
        case (0x2B55): { return eON; }
        case (0x2B56): { return eON; }
        case (0x2B57): { return eON; }
        case (0x2B58): { return eON; }
        case (0x2B59): { return eON; }
        case (0x2CE5): { return eON; }
        case (0x2CE6): { return eON; }
        case (0x2CE7): { return eON; }
        case (0x2CE8): { return eON; }
        case (0x2CE9): { return eON; }
        case (0x2CEA): { return eON; }
        case (0x2CEF): { return eNSM; }
        case (0x2CF0): { return eNSM; }
        case (0x2CF1): { return eNSM; }
        case (0x2CF9): { return eON; }
        case (0x2CFA): { return eON; }
        case (0x2CFB): { return eON; }
        case (0x2CFC): { return eON; }
        case (0x2CFD): { return eON; }
        case (0x2CFE): { return eON; }
        case (0x2CFF): { return eON; }
        case (0x2D7F): { return eNSM; }
        case (0x2DE0): { return eNSM; }
        case (0x2DE1): { return eNSM; }
        case (0x2DE2): { return eNSM; }
        case (0x2DE3): { return eNSM; }
        case (0x2DE4): { return eNSM; }
        case (0x2DE5): { return eNSM; }
        case (0x2DE6): { return eNSM; }
        case (0x2DE7): { return eNSM; }
        case (0x2DE8): { return eNSM; }
        case (0x2DE9): { return eNSM; }
        case (0x2DEA): { return eNSM; }
        case (0x2DEB): { return eNSM; }
        case (0x2DEC): { return eNSM; }
        case (0x2DED): { return eNSM; }
        case (0x2DEE): { return eNSM; }
        case (0x2DEF): { return eNSM; }
        case (0x2DF0): { return eNSM; }
        case (0x2DF1): { return eNSM; }
        case (0x2DF2): { return eNSM; }
        case (0x2DF3): { return eNSM; }
        case (0x2DF4): { return eNSM; }
        case (0x2DF5): { return eNSM; }
        case (0x2DF6): { return eNSM; }
        case (0x2DF7): { return eNSM; }
        case (0x2DF8): { return eNSM; }
        case (0x2DF9): { return eNSM; }
        case (0x2DFA): { return eNSM; }
        case (0x2DFB): { return eNSM; }
        case (0x2DFC): { return eNSM; }
        case (0x2DFD): { return eNSM; }
        case (0x2DFE): { return eNSM; }
        case (0x2DFF): { return eNSM; }
        case (0x2E00): { return eON; }
        case (0x2E01): { return eON; }
        case (0x2E02): { return eON; }
        case (0x2E03): { return eON; }
        case (0x2E04): { return eON; }
        case (0x2E05): { return eON; }
        case (0x2E06): { return eON; }
        case (0x2E07): { return eON; }
        case (0x2E08): { return eON; }
        case (0x2E09): { return eON; }
        case (0x2E0A): { return eON; }
        case (0x2E0B): { return eON; }
        case (0x2E0C): { return eON; }
        case (0x2E0D): { return eON; }
        case (0x2E0E): { return eON; }
        case (0x2E0F): { return eON; }
        case (0x2E10): { return eON; }
        case (0x2E11): { return eON; }
        case (0x2E12): { return eON; }
        case (0x2E13): { return eON; }
        case (0x2E14): { return eON; }
        case (0x2E15): { return eON; }
        case (0x2E16): { return eON; }
        case (0x2E17): { return eON; }
        case (0x2E18): { return eON; }
        case (0x2E19): { return eON; }
        case (0x2E1A): { return eON; }
        case (0x2E1B): { return eON; }
        case (0x2E1C): { return eON; }
        case (0x2E1D): { return eON; }
        case (0x2E1E): { return eON; }
        case (0x2E1F): { return eON; }
        case (0x2E20): { return eON; }
        case (0x2E21): { return eON; }
        case (0x2E22): { return eON; }
        case (0x2E23): { return eON; }
        case (0x2E24): { return eON; }
        case (0x2E25): { return eON; }
        case (0x2E26): { return eON; }
        case (0x2E27): { return eON; }
        case (0x2E28): { return eON; }
        case (0x2E29): { return eON; }
        case (0x2E2A): { return eON; }
        case (0x2E2B): { return eON; }
        case (0x2E2C): { return eON; }
        case (0x2E2D): { return eON; }
        case (0x2E2E): { return eON; }
        case (0x2E2F): { return eON; }
        case (0x2E30): { return eON; }
        case (0x2E31): { return eON; }
        case (0x2E80): { return eON; }
        case (0x2E81): { return eON; }
        case (0x2E82): { return eON; }
        case (0x2E83): { return eON; }
        case (0x2E84): { return eON; }
        case (0x2E85): { return eON; }
        case (0x2E86): { return eON; }
        case (0x2E87): { return eON; }
        case (0x2E88): { return eON; }
        case (0x2E89): { return eON; }
        case (0x2E8A): { return eON; }
        case (0x2E8B): { return eON; }
        case (0x2E8C): { return eON; }
        case (0x2E8D): { return eON; }
        case (0x2E8E): { return eON; }
        case (0x2E8F): { return eON; }
        case (0x2E90): { return eON; }
        case (0x2E91): { return eON; }
        case (0x2E92): { return eON; }
        case (0x2E93): { return eON; }
        case (0x2E94): { return eON; }
        case (0x2E95): { return eON; }
        case (0x2E96): { return eON; }
        case (0x2E97): { return eON; }
        case (0x2E98): { return eON; }
        case (0x2E99): { return eON; }
        case (0x2E9B): { return eON; }
        case (0x2E9C): { return eON; }
        case (0x2E9D): { return eON; }
        case (0x2E9E): { return eON; }
        case (0x2E9F): { return eON; }
        case (0x2EA0): { return eON; }
        case (0x2EA1): { return eON; }
        case (0x2EA2): { return eON; }
        case (0x2EA3): { return eON; }
        case (0x2EA4): { return eON; }
        case (0x2EA5): { return eON; }
        case (0x2EA6): { return eON; }
        case (0x2EA7): { return eON; }
        case (0x2EA8): { return eON; }
        case (0x2EA9): { return eON; }
        case (0x2EAA): { return eON; }
        case (0x2EAB): { return eON; }
        case (0x2EAC): { return eON; }
        case (0x2EAD): { return eON; }
        case (0x2EAE): { return eON; }
        case (0x2EAF): { return eON; }
        case (0x2EB0): { return eON; }
        case (0x2EB1): { return eON; }
        case (0x2EB2): { return eON; }
        case (0x2EB3): { return eON; }
        case (0x2EB4): { return eON; }
        case (0x2EB5): { return eON; }
        case (0x2EB6): { return eON; }
        case (0x2EB7): { return eON; }
        case (0x2EB8): { return eON; }
        case (0x2EB9): { return eON; }
        case (0x2EBA): { return eON; }
        case (0x2EBB): { return eON; }
        case (0x2EBC): { return eON; }
        case (0x2EBD): { return eON; }
        case (0x2EBE): { return eON; }
        case (0x2EBF): { return eON; }
        case (0x2EC0): { return eON; }
        case (0x2EC1): { return eON; }
        case (0x2EC2): { return eON; }
        case (0x2EC3): { return eON; }
        case (0x2EC4): { return eON; }
        case (0x2EC5): { return eON; }
        case (0x2EC6): { return eON; }
        case (0x2EC7): { return eON; }
        case (0x2EC8): { return eON; }
        case (0x2EC9): { return eON; }
        case (0x2ECA): { return eON; }
        case (0x2ECB): { return eON; }
        case (0x2ECC): { return eON; }
        case (0x2ECD): { return eON; }
        case (0x2ECE): { return eON; }
        case (0x2ECF): { return eON; }
        case (0x2ED0): { return eON; }
        case (0x2ED1): { return eON; }
        case (0x2ED2): { return eON; }
        case (0x2ED3): { return eON; }
        case (0x2ED4): { return eON; }
        case (0x2ED5): { return eON; }
        case (0x2ED6): { return eON; }
        case (0x2ED7): { return eON; }
        case (0x2ED8): { return eON; }
        case (0x2ED9): { return eON; }
        case (0x2EDA): { return eON; }
        case (0x2EDB): { return eON; }
        case (0x2EDC): { return eON; }
        case (0x2EDD): { return eON; }
        case (0x2EDE): { return eON; }
        case (0x2EDF): { return eON; }
        case (0x2EE0): { return eON; }
        case (0x2EE1): { return eON; }
        case (0x2EE2): { return eON; }
        case (0x2EE3): { return eON; }
        case (0x2EE4): { return eON; }
        case (0x2EE5): { return eON; }
        case (0x2EE6): { return eON; }
        case (0x2EE7): { return eON; }
        case (0x2EE8): { return eON; }
        case (0x2EE9): { return eON; }
        case (0x2EEA): { return eON; }
        case (0x2EEB): { return eON; }
        case (0x2EEC): { return eON; }
        case (0x2EED): { return eON; }
        case (0x2EEE): { return eON; }
        case (0x2EEF): { return eON; }
        case (0x2EF0): { return eON; }
        case (0x2EF1): { return eON; }
        case (0x2EF2): { return eON; }
        case (0x2EF3): { return eON; }
        case (0x2F00): { return eON; }
        case (0x2F01): { return eON; }
        case (0x2F02): { return eON; }
        case (0x2F03): { return eON; }
        case (0x2F04): { return eON; }
        case (0x2F05): { return eON; }
        case (0x2F06): { return eON; }
        case (0x2F07): { return eON; }
        case (0x2F08): { return eON; }
        case (0x2F09): { return eON; }
        case (0x2F0A): { return eON; }
        case (0x2F0B): { return eON; }
        case (0x2F0C): { return eON; }
        case (0x2F0D): { return eON; }
        case (0x2F0E): { return eON; }
        case (0x2F0F): { return eON; }
        case (0x2F10): { return eON; }
        case (0x2F11): { return eON; }
        case (0x2F12): { return eON; }
        case (0x2F13): { return eON; }
        case (0x2F14): { return eON; }
        case (0x2F15): { return eON; }
        case (0x2F16): { return eON; }
        case (0x2F17): { return eON; }
        case (0x2F18): { return eON; }
        case (0x2F19): { return eON; }
        case (0x2F1A): { return eON; }
        case (0x2F1B): { return eON; }
        case (0x2F1C): { return eON; }
        case (0x2F1D): { return eON; }
        case (0x2F1E): { return eON; }
        case (0x2F1F): { return eON; }
        case (0x2F20): { return eON; }
        case (0x2F21): { return eON; }
        case (0x2F22): { return eON; }
        case (0x2F23): { return eON; }
        case (0x2F24): { return eON; }
        case (0x2F25): { return eON; }
        case (0x2F26): { return eON; }
        case (0x2F27): { return eON; }
        case (0x2F28): { return eON; }
        case (0x2F29): { return eON; }
        case (0x2F2A): { return eON; }
        case (0x2F2B): { return eON; }
        case (0x2F2C): { return eON; }
        case (0x2F2D): { return eON; }
        case (0x2F2E): { return eON; }
        case (0x2F2F): { return eON; }
        case (0x2F30): { return eON; }
        case (0x2F31): { return eON; }
        case (0x2F32): { return eON; }
        case (0x2F33): { return eON; }
        case (0x2F34): { return eON; }
        case (0x2F35): { return eON; }
        case (0x2F36): { return eON; }
        case (0x2F37): { return eON; }
        case (0x2F38): { return eON; }
        case (0x2F39): { return eON; }
        case (0x2F3A): { return eON; }
        case (0x2F3B): { return eON; }
        case (0x2F3C): { return eON; }
        case (0x2F3D): { return eON; }
        case (0x2F3E): { return eON; }
        case (0x2F3F): { return eON; }
        case (0x2F40): { return eON; }
        case (0x2F41): { return eON; }
        case (0x2F42): { return eON; }
        case (0x2F43): { return eON; }
        case (0x2F44): { return eON; }
        case (0x2F45): { return eON; }
        case (0x2F46): { return eON; }
        case (0x2F47): { return eON; }
        case (0x2F48): { return eON; }
        case (0x2F49): { return eON; }
        case (0x2F4A): { return eON; }
        case (0x2F4B): { return eON; }
        case (0x2F4C): { return eON; }
        case (0x2F4D): { return eON; }
        case (0x2F4E): { return eON; }
        case (0x2F4F): { return eON; }
        case (0x2F50): { return eON; }
        case (0x2F51): { return eON; }
        case (0x2F52): { return eON; }
        case (0x2F53): { return eON; }
        case (0x2F54): { return eON; }
        case (0x2F55): { return eON; }
        case (0x2F56): { return eON; }
        case (0x2F57): { return eON; }
        case (0x2F58): { return eON; }
        case (0x2F59): { return eON; }
        case (0x2F5A): { return eON; }
        case (0x2F5B): { return eON; }
        case (0x2F5C): { return eON; }
        case (0x2F5D): { return eON; }
        case (0x2F5E): { return eON; }
        case (0x2F5F): { return eON; }
        case (0x2F60): { return eON; }
        case (0x2F61): { return eON; }
        case (0x2F62): { return eON; }
        case (0x2F63): { return eON; }
        case (0x2F64): { return eON; }
        case (0x2F65): { return eON; }
        case (0x2F66): { return eON; }
        case (0x2F67): { return eON; }
        case (0x2F68): { return eON; }
        case (0x2F69): { return eON; }
        case (0x2F6A): { return eON; }
        case (0x2F6B): { return eON; }
        case (0x2F6C): { return eON; }
        case (0x2F6D): { return eON; }
        case (0x2F6E): { return eON; }
        case (0x2F6F): { return eON; }
        case (0x2F70): { return eON; }
        case (0x2F71): { return eON; }
        case (0x2F72): { return eON; }
        case (0x2F73): { return eON; }
        case (0x2F74): { return eON; }
        case (0x2F75): { return eON; }
        case (0x2F76): { return eON; }
        case (0x2F77): { return eON; }
        case (0x2F78): { return eON; }
        case (0x2F79): { return eON; }
        case (0x2F7A): { return eON; }
        case (0x2F7B): { return eON; }
        case (0x2F7C): { return eON; }
        case (0x2F7D): { return eON; }
        case (0x2F7E): { return eON; }
        case (0x2F7F): { return eON; }
        case (0x2F80): { return eON; }
        case (0x2F81): { return eON; }
        case (0x2F82): { return eON; }
        case (0x2F83): { return eON; }
        case (0x2F84): { return eON; }
        case (0x2F85): { return eON; }
        case (0x2F86): { return eON; }
        case (0x2F87): { return eON; }
        case (0x2F88): { return eON; }
        case (0x2F89): { return eON; }
        case (0x2F8A): { return eON; }
        case (0x2F8B): { return eON; }
        case (0x2F8C): { return eON; }
        case (0x2F8D): { return eON; }
        case (0x2F8E): { return eON; }
        case (0x2F8F): { return eON; }
        case (0x2F90): { return eON; }
        case (0x2F91): { return eON; }
        case (0x2F92): { return eON; }
        case (0x2F93): { return eON; }
        case (0x2F94): { return eON; }
        case (0x2F95): { return eON; }
        case (0x2F96): { return eON; }
        case (0x2F97): { return eON; }
        case (0x2F98): { return eON; }
        case (0x2F99): { return eON; }
        case (0x2F9A): { return eON; }
        case (0x2F9B): { return eON; }
        case (0x2F9C): { return eON; }
        case (0x2F9D): { return eON; }
        case (0x2F9E): { return eON; }
        case (0x2F9F): { return eON; }
        case (0x2FA0): { return eON; }
        case (0x2FA1): { return eON; }
        case (0x2FA2): { return eON; }
        case (0x2FA3): { return eON; }
        case (0x2FA4): { return eON; }
        case (0x2FA5): { return eON; }
        case (0x2FA6): { return eON; }
        case (0x2FA7): { return eON; }
        case (0x2FA8): { return eON; }
        case (0x2FA9): { return eON; }
        case (0x2FAA): { return eON; }
        case (0x2FAB): { return eON; }
        case (0x2FAC): { return eON; }
        case (0x2FAD): { return eON; }
        case (0x2FAE): { return eON; }
        case (0x2FAF): { return eON; }
        case (0x2FB0): { return eON; }
        case (0x2FB1): { return eON; }
        case (0x2FB2): { return eON; }
        case (0x2FB3): { return eON; }
        case (0x2FB4): { return eON; }
        case (0x2FB5): { return eON; }
        case (0x2FB6): { return eON; }
        case (0x2FB7): { return eON; }
        case (0x2FB8): { return eON; }
        case (0x2FB9): { return eON; }
        case (0x2FBA): { return eON; }
        case (0x2FBB): { return eON; }
        case (0x2FBC): { return eON; }
        case (0x2FBD): { return eON; }
        case (0x2FBE): { return eON; }
        case (0x2FBF): { return eON; }
        case (0x2FC0): { return eON; }
        case (0x2FC1): { return eON; }
        case (0x2FC2): { return eON; }
        case (0x2FC3): { return eON; }
        case (0x2FC4): { return eON; }
        case (0x2FC5): { return eON; }
        case (0x2FC6): { return eON; }
        case (0x2FC7): { return eON; }
        case (0x2FC8): { return eON; }
        case (0x2FC9): { return eON; }
        case (0x2FCA): { return eON; }
        case (0x2FCB): { return eON; }
        case (0x2FCC): { return eON; }
        case (0x2FCD): { return eON; }
        case (0x2FCE): { return eON; }
        case (0x2FCF): { return eON; }
        case (0x2FD0): { return eON; }
        case (0x2FD1): { return eON; }
        case (0x2FD2): { return eON; }
        case (0x2FD3): { return eON; }
        case (0x2FD4): { return eON; }
        case (0x2FD5): { return eON; }
        case (0x2FF0): { return eON; }
        case (0x2FF1): { return eON; }
        case (0x2FF2): { return eON; }
        case (0x2FF3): { return eON; }
        case (0x2FF4): { return eON; }
        case (0x2FF5): { return eON; }
        case (0x2FF6): { return eON; }
        case (0x2FF7): { return eON; }
        case (0x2FF8): { return eON; }
        case (0x2FF9): { return eON; }
        case (0x2FFA): { return eON; }
        case (0x2FFB): { return eON; }
        case (0x3000): { return eWS; }
        case (0x3001): { return eON; }
        case (0x3002): { return eON; }
        case (0x3003): { return eON; }
        case (0x3004): { return eON; }
        case (0x3008): { return eON; }
        case (0x3009): { return eON; }
        case (0x300A): { return eON; }
        case (0x300B): { return eON; }
        case (0x300C): { return eON; }
        case (0x300D): { return eON; }
        case (0x300E): { return eON; }
        case (0x300F): { return eON; }
        case (0x3010): { return eON; }
        case (0x3011): { return eON; }
        case (0x3012): { return eON; }
        case (0x3013): { return eON; }
        case (0x3014): { return eON; }
        case (0x3015): { return eON; }
        case (0x3016): { return eON; }
        case (0x3017): { return eON; }
        case (0x3018): { return eON; }
        case (0x3019): { return eON; }
        case (0x301A): { return eON; }
        case (0x301B): { return eON; }
        case (0x301C): { return eON; }
        case (0x301D): { return eON; }
        case (0x301E): { return eON; }
        case (0x301F): { return eON; }
        case (0x3020): { return eON; }
        case (0x302A): { return eNSM; }
        case (0x302B): { return eNSM; }
        case (0x302C): { return eNSM; }
        case (0x302D): { return eNSM; }
        case (0x302E): { return eNSM; }
        case (0x302F): { return eNSM; }
        case (0x3030): { return eON; }
        case (0x3036): { return eON; }
        case (0x3037): { return eON; }
        case (0x303D): { return eON; }
        case (0x303E): { return eON; }
        case (0x303F): { return eON; }
        case (0x3099): { return eNSM; }
        case (0x309A): { return eNSM; }
        case (0x309B): { return eON; }
        case (0x309C): { return eON; }
        case (0x30A0): { return eON; }
        case (0x30FB): { return eON; }
        case (0x31C0): { return eON; }
        case (0x31C1): { return eON; }
        case (0x31C2): { return eON; }
        case (0x31C3): { return eON; }
        case (0x31C4): { return eON; }
        case (0x31C5): { return eON; }
        case (0x31C6): { return eON; }
        case (0x31C7): { return eON; }
        case (0x31C8): { return eON; }
        case (0x31C9): { return eON; }
        case (0x31CA): { return eON; }
        case (0x31CB): { return eON; }
        case (0x31CC): { return eON; }
        case (0x31CD): { return eON; }
        case (0x31CE): { return eON; }
        case (0x31CF): { return eON; }
        case (0x31D0): { return eON; }
        case (0x31D1): { return eON; }
        case (0x31D2): { return eON; }
        case (0x31D3): { return eON; }
        case (0x31D4): { return eON; }
        case (0x31D5): { return eON; }
        case (0x31D6): { return eON; }
        case (0x31D7): { return eON; }
        case (0x31D8): { return eON; }
        case (0x31D9): { return eON; }
        case (0x31DA): { return eON; }
        case (0x31DB): { return eON; }
        case (0x31DC): { return eON; }
        case (0x31DD): { return eON; }
        case (0x31DE): { return eON; }
        case (0x31DF): { return eON; }
        case (0x31E0): { return eON; }
        case (0x31E1): { return eON; }
        case (0x31E2): { return eON; }
        case (0x31E3): { return eON; }
        case (0x321D): { return eON; }
        case (0x321E): { return eON; }
        case (0x3250): { return eON; }
        case (0x3251): { return eON; }
        case (0x3252): { return eON; }
        case (0x3253): { return eON; }
        case (0x3254): { return eON; }
        case (0x3255): { return eON; }
        case (0x3256): { return eON; }
        case (0x3257): { return eON; }
        case (0x3258): { return eON; }
        case (0x3259): { return eON; }
        case (0x325A): { return eON; }
        case (0x325B): { return eON; }
        case (0x325C): { return eON; }
        case (0x325D): { return eON; }
        case (0x325E): { return eON; }
        case (0x325F): { return eON; }
        case (0x327C): { return eON; }
        case (0x327D): { return eON; }
        case (0x327E): { return eON; }
        case (0x32B1): { return eON; }
        case (0x32B2): { return eON; }
        case (0x32B3): { return eON; }
        case (0x32B4): { return eON; }
        case (0x32B5): { return eON; }
        case (0x32B6): { return eON; }
        case (0x32B7): { return eON; }
        case (0x32B8): { return eON; }
        case (0x32B9): { return eON; }
        case (0x32BA): { return eON; }
        case (0x32BB): { return eON; }
        case (0x32BC): { return eON; }
        case (0x32BD): { return eON; }
        case (0x32BE): { return eON; }
        case (0x32BF): { return eON; }
        case (0x32CC): { return eON; }
        case (0x32CD): { return eON; }
        case (0x32CE): { return eON; }
        case (0x32CF): { return eON; }
        case (0x3377): { return eON; }
        case (0x3378): { return eON; }
        case (0x3379): { return eON; }
        case (0x337A): { return eON; }
        case (0x33DE): { return eON; }
        case (0x33DF): { return eON; }
        case (0x33FF): { return eON; }
        case (0x4DC0): { return eON; }
        case (0x4DC1): { return eON; }
        case (0x4DC2): { return eON; }
        case (0x4DC3): { return eON; }
        case (0x4DC4): { return eON; }
        case (0x4DC5): { return eON; }
        case (0x4DC6): { return eON; }
        case (0x4DC7): { return eON; }
        case (0x4DC8): { return eON; }
        case (0x4DC9): { return eON; }
        case (0x4DCA): { return eON; }
        case (0x4DCB): { return eON; }
        case (0x4DCC): { return eON; }
        case (0x4DCD): { return eON; }
        case (0x4DCE): { return eON; }
        case (0x4DCF): { return eON; }
        case (0x4DD0): { return eON; }
        case (0x4DD1): { return eON; }
        case (0x4DD2): { return eON; }
        case (0x4DD3): { return eON; }
        case (0x4DD4): { return eON; }
        case (0x4DD5): { return eON; }
        case (0x4DD6): { return eON; }
        case (0x4DD7): { return eON; }
        case (0x4DD8): { return eON; }
        case (0x4DD9): { return eON; }
        case (0x4DDA): { return eON; }
        case (0x4DDB): { return eON; }
        case (0x4DDC): { return eON; }
        case (0x4DDD): { return eON; }
        case (0x4DDE): { return eON; }
        case (0x4DDF): { return eON; }
        case (0x4DE0): { return eON; }
        case (0x4DE1): { return eON; }
        case (0x4DE2): { return eON; }
        case (0x4DE3): { return eON; }
        case (0x4DE4): { return eON; }
        case (0x4DE5): { return eON; }
        case (0x4DE6): { return eON; }
        case (0x4DE7): { return eON; }
        case (0x4DE8): { return eON; }
        case (0x4DE9): { return eON; }
        case (0x4DEA): { return eON; }
        case (0x4DEB): { return eON; }
        case (0x4DEC): { return eON; }
        case (0x4DED): { return eON; }
        case (0x4DEE): { return eON; }
        case (0x4DEF): { return eON; }
        case (0x4DF0): { return eON; }
        case (0x4DF1): { return eON; }
        case (0x4DF2): { return eON; }
        case (0x4DF3): { return eON; }
        case (0x4DF4): { return eON; }
        case (0x4DF5): { return eON; }
        case (0x4DF6): { return eON; }
        case (0x4DF7): { return eON; }
        case (0x4DF8): { return eON; }
        case (0x4DF9): { return eON; }
        case (0x4DFA): { return eON; }
        case (0x4DFB): { return eON; }
        case (0x4DFC): { return eON; }
        case (0x4DFD): { return eON; }
        case (0x4DFE): { return eON; }
        case (0x4DFF): { return eON; }
        case (0xA490): { return eON; }
        case (0xA491): { return eON; }
        case (0xA492): { return eON; }
        case (0xA493): { return eON; }
        case (0xA494): { return eON; }
        case (0xA495): { return eON; }
        case (0xA496): { return eON; }
        case (0xA497): { return eON; }
        case (0xA498): { return eON; }
        case (0xA499): { return eON; }
        case (0xA49A): { return eON; }
        case (0xA49B): { return eON; }
        case (0xA49C): { return eON; }
        case (0xA49D): { return eON; }
        case (0xA49E): { return eON; }
        case (0xA49F): { return eON; }
        case (0xA4A0): { return eON; }
        case (0xA4A1): { return eON; }
        case (0xA4A2): { return eON; }
        case (0xA4A3): { return eON; }
        case (0xA4A4): { return eON; }
        case (0xA4A5): { return eON; }
        case (0xA4A6): { return eON; }
        case (0xA4A7): { return eON; }
        case (0xA4A8): { return eON; }
        case (0xA4A9): { return eON; }
        case (0xA4AA): { return eON; }
        case (0xA4AB): { return eON; }
        case (0xA4AC): { return eON; }
        case (0xA4AD): { return eON; }
        case (0xA4AE): { return eON; }
        case (0xA4AF): { return eON; }
        case (0xA4B0): { return eON; }
        case (0xA4B1): { return eON; }
        case (0xA4B2): { return eON; }
        case (0xA4B3): { return eON; }
        case (0xA4B4): { return eON; }
        case (0xA4B5): { return eON; }
        case (0xA4B6): { return eON; }
        case (0xA4B7): { return eON; }
        case (0xA4B8): { return eON; }
        case (0xA4B9): { return eON; }
        case (0xA4BA): { return eON; }
        case (0xA4BB): { return eON; }
        case (0xA4BC): { return eON; }
        case (0xA4BD): { return eON; }
        case (0xA4BE): { return eON; }
        case (0xA4BF): { return eON; }
        case (0xA4C0): { return eON; }
        case (0xA4C1): { return eON; }
        case (0xA4C2): { return eON; }
        case (0xA4C3): { return eON; }
        case (0xA4C4): { return eON; }
        case (0xA4C5): { return eON; }
        case (0xA4C6): { return eON; }
        case (0xA60D): { return eON; }
        case (0xA60E): { return eON; }
        case (0xA60F): { return eON; }
        case (0xA66F): { return eNSM; }
        case (0xA670): { return eNSM; }
        case (0xA671): { return eNSM; }
        case (0xA672): { return eNSM; }
        case (0xA673): { return eON; }
        case (0xA67C): { return eNSM; }
        case (0xA67D): { return eNSM; }
        case (0xA67E): { return eON; }
        case (0xA67F): { return eON; }
        case (0xA6F0): { return eNSM; }
        case (0xA6F1): { return eNSM; }
        case (0xA700): { return eON; }
        case (0xA701): { return eON; }
        case (0xA702): { return eON; }
        case (0xA703): { return eON; }
        case (0xA704): { return eON; }
        case (0xA705): { return eON; }
        case (0xA706): { return eON; }
        case (0xA707): { return eON; }
        case (0xA708): { return eON; }
        case (0xA709): { return eON; }
        case (0xA70A): { return eON; }
        case (0xA70B): { return eON; }
        case (0xA70C): { return eON; }
        case (0xA70D): { return eON; }
        case (0xA70E): { return eON; }
        case (0xA70F): { return eON; }
        case (0xA710): { return eON; }
        case (0xA711): { return eON; }
        case (0xA712): { return eON; }
        case (0xA713): { return eON; }
        case (0xA714): { return eON; }
        case (0xA715): { return eON; }
        case (0xA716): { return eON; }
        case (0xA717): { return eON; }
        case (0xA718): { return eON; }
        case (0xA719): { return eON; }
        case (0xA71A): { return eON; }
        case (0xA71B): { return eON; }
        case (0xA71C): { return eON; }
        case (0xA71D): { return eON; }
        case (0xA71E): { return eON; }
        case (0xA71F): { return eON; }
        case (0xA720): { return eON; }
        case (0xA721): { return eON; }
        case (0xA788): { return eON; }
        case (0xA802): { return eNSM; }
        case (0xA806): { return eNSM; }
        case (0xA80B): { return eNSM; }
        case (0xA825): { return eNSM; }
        case (0xA826): { return eNSM; }
        case (0xA828): { return eON; }
        case (0xA829): { return eON; }
        case (0xA82A): { return eON; }
        case (0xA82B): { return eON; }
        case (0xA838): { return eET; }
        case (0xA839): { return eET; }
        case (0xA874): { return eON; }
        case (0xA875): { return eON; }
        case (0xA876): { return eON; }
        case (0xA877): { return eON; }
        case (0xA8C4): { return eNSM; }
        case (0xA8E0): { return eNSM; }
        case (0xA8E1): { return eNSM; }
        case (0xA8E2): { return eNSM; }
        case (0xA8E3): { return eNSM; }
        case (0xA8E4): { return eNSM; }
        case (0xA8E5): { return eNSM; }
        case (0xA8E6): { return eNSM; }
        case (0xA8E7): { return eNSM; }
        case (0xA8E8): { return eNSM; }
        case (0xA8E9): { return eNSM; }
        case (0xA8EA): { return eNSM; }
        case (0xA8EB): { return eNSM; }
        case (0xA8EC): { return eNSM; }
        case (0xA8ED): { return eNSM; }
        case (0xA8EE): { return eNSM; }
        case (0xA8EF): { return eNSM; }
        case (0xA8F0): { return eNSM; }
        case (0xA8F1): { return eNSM; }
        case (0xA926): { return eNSM; }
        case (0xA927): { return eNSM; }
        case (0xA928): { return eNSM; }
        case (0xA929): { return eNSM; }
        case (0xA92A): { return eNSM; }
        case (0xA92B): { return eNSM; }
        case (0xA92C): { return eNSM; }
        case (0xA92D): { return eNSM; }
        case (0xA947): { return eNSM; }
        case (0xA948): { return eNSM; }
        case (0xA949): { return eNSM; }
        case (0xA94A): { return eNSM; }
        case (0xA94B): { return eNSM; }
        case (0xA94C): { return eNSM; }
        case (0xA94D): { return eNSM; }
        case (0xA94E): { return eNSM; }
        case (0xA94F): { return eNSM; }
        case (0xA950): { return eNSM; }
        case (0xA951): { return eNSM; }
        case (0xA980): { return eNSM; }
        case (0xA981): { return eNSM; }
        case (0xA982): { return eNSM; }
        case (0xA9B3): { return eNSM; }
        case (0xA9B6): { return eNSM; }
        case (0xA9B7): { return eNSM; }
        case (0xA9B8): { return eNSM; }
        case (0xA9B9): { return eNSM; }
        case (0xA9BC): { return eNSM; }
        case (0xAA29): { return eNSM; }
        case (0xAA2A): { return eNSM; }
        case (0xAA2B): { return eNSM; }
        case (0xAA2C): { return eNSM; }
        case (0xAA2D): { return eNSM; }
        case (0xAA2E): { return eNSM; }
        case (0xAA31): { return eNSM; }
        case (0xAA32): { return eNSM; }
        case (0xAA35): { return eNSM; }
        case (0xAA36): { return eNSM; }
        case (0xAA43): { return eNSM; }
        case (0xAA4C): { return eNSM; }
        case (0xAAB0): { return eNSM; }
        case (0xAAB2): { return eNSM; }
        case (0xAAB3): { return eNSM; }
        case (0xAAB4): { return eNSM; }
        case (0xAAB7): { return eNSM; }
        case (0xAAB8): { return eNSM; }
        case (0xAABE): { return eNSM; }
        case (0xAABF): { return eNSM; }
        case (0xAAC1): { return eNSM; }
        case (0xABE5): { return eNSM; }
        case (0xABE8): { return eNSM; }
        case (0xABED): { return eNSM; }
        case (0xFB1D): { return eR; }
        case (0xFB1E): { return eNSM; }
        case (0xFB1F): { return eR; }
        case (0xFB20): { return eR; }
        case (0xFB21): { return eR; }
        case (0xFB22): { return eR; }
        case (0xFB23): { return eR; }
        case (0xFB24): { return eR; }
        case (0xFB25): { return eR; }
        case (0xFB26): { return eR; }
        case (0xFB27): { return eR; }
        case (0xFB28): { return eR; }
        case (0xFB29): { return eES; }
        case (0xFB2A): { return eR; }
        case (0xFB2B): { return eR; }
        case (0xFB2C): { return eR; }
        case (0xFB2D): { return eR; }
        case (0xFB2E): { return eR; }
        case (0xFB2F): { return eR; }
        case (0xFB30): { return eR; }
        case (0xFB31): { return eR; }
        case (0xFB32): { return eR; }
        case (0xFB33): { return eR; }
        case (0xFB34): { return eR; }
        case (0xFB35): { return eR; }
        case (0xFB36): { return eR; }
        case (0xFB38): { return eR; }
        case (0xFB39): { return eR; }
        case (0xFB3A): { return eR; }
        case (0xFB3B): { return eR; }
        case (0xFB3C): { return eR; }
        case (0xFB3E): { return eR; }
        case (0xFB40): { return eR; }
        case (0xFB41): { return eR; }
        case (0xFB43): { return eR; }
        case (0xFB44): { return eR; }
        case (0xFB46): { return eR; }
        case (0xFB47): { return eR; }
        case (0xFB48): { return eR; }
        case (0xFB49): { return eR; }
        case (0xFB4A): { return eR; }
        case (0xFB4B): { return eR; }
        case (0xFB4C): { return eR; }
        case (0xFB4D): { return eR; }
        case (0xFB4E): { return eR; }
        case (0xFB4F): { return eR; }
        case (0xFB50): { return eAL; }
        case (0xFB51): { return eAL; }
        case (0xFB52): { return eAL; }
        case (0xFB53): { return eAL; }
        case (0xFB54): { return eAL; }
        case (0xFB55): { return eAL; }
        case (0xFB56): { return eAL; }
        case (0xFB57): { return eAL; }
        case (0xFB58): { return eAL; }
        case (0xFB59): { return eAL; }
        case (0xFB5A): { return eAL; }
        case (0xFB5B): { return eAL; }
        case (0xFB5C): { return eAL; }
        case (0xFB5D): { return eAL; }
        case (0xFB5E): { return eAL; }
        case (0xFB5F): { return eAL; }
        case (0xFB60): { return eAL; }
        case (0xFB61): { return eAL; }
        case (0xFB62): { return eAL; }
        case (0xFB63): { return eAL; }
        case (0xFB64): { return eAL; }
        case (0xFB65): { return eAL; }
        case (0xFB66): { return eAL; }
        case (0xFB67): { return eAL; }
        case (0xFB68): { return eAL; }
        case (0xFB69): { return eAL; }
        case (0xFB6A): { return eAL; }
        case (0xFB6B): { return eAL; }
        case (0xFB6C): { return eAL; }
        case (0xFB6D): { return eAL; }
        case (0xFB6E): { return eAL; }
        case (0xFB6F): { return eAL; }
        case (0xFB70): { return eAL; }
        case (0xFB71): { return eAL; }
        case (0xFB72): { return eAL; }
        case (0xFB73): { return eAL; }
        case (0xFB74): { return eAL; }
        case (0xFB75): { return eAL; }
        case (0xFB76): { return eAL; }
        case (0xFB77): { return eAL; }
        case (0xFB78): { return eAL; }
        case (0xFB79): { return eAL; }
        case (0xFB7A): { return eAL; }
        case (0xFB7B): { return eAL; }
        case (0xFB7C): { return eAL; }
        case (0xFB7D): { return eAL; }
        case (0xFB7E): { return eAL; }
        case (0xFB7F): { return eAL; }
        case (0xFB80): { return eAL; }
        case (0xFB81): { return eAL; }
        case (0xFB82): { return eAL; }
        case (0xFB83): { return eAL; }
        case (0xFB84): { return eAL; }
        case (0xFB85): { return eAL; }
        case (0xFB86): { return eAL; }
        case (0xFB87): { return eAL; }
        case (0xFB88): { return eAL; }
        case (0xFB89): { return eAL; }
        case (0xFB8A): { return eAL; }
        case (0xFB8B): { return eAL; }
        case (0xFB8C): { return eAL; }
        case (0xFB8D): { return eAL; }
        case (0xFB8E): { return eAL; }
        case (0xFB8F): { return eAL; }
        case (0xFB90): { return eAL; }
        case (0xFB91): { return eAL; }
        case (0xFB92): { return eAL; }
        case (0xFB93): { return eAL; }
        case (0xFB94): { return eAL; }
        case (0xFB95): { return eAL; }
        case (0xFB96): { return eAL; }
        case (0xFB97): { return eAL; }
        case (0xFB98): { return eAL; }
        case (0xFB99): { return eAL; }
        case (0xFB9A): { return eAL; }
        case (0xFB9B): { return eAL; }
        case (0xFB9C): { return eAL; }
        case (0xFB9D): { return eAL; }
        case (0xFB9E): { return eAL; }
        case (0xFB9F): { return eAL; }
        case (0xFBA0): { return eAL; }
        case (0xFBA1): { return eAL; }
        case (0xFBA2): { return eAL; }
        case (0xFBA3): { return eAL; }
        case (0xFBA4): { return eAL; }
        case (0xFBA5): { return eAL; }
        case (0xFBA6): { return eAL; }
        case (0xFBA7): { return eAL; }
        case (0xFBA8): { return eAL; }
        case (0xFBA9): { return eAL; }
        case (0xFBAA): { return eAL; }
        case (0xFBAB): { return eAL; }
        case (0xFBAC): { return eAL; }
        case (0xFBAD): { return eAL; }
        case (0xFBAE): { return eAL; }
        case (0xFBAF): { return eAL; }
        case (0xFBB0): { return eAL; }
        case (0xFBB1): { return eAL; }
        case (0xFBB2): { return eAL; }
        case (0xFBB3): { return eAL; }
        case (0xFBB4): { return eAL; }
        case (0xFBB5): { return eAL; }
        case (0xFBB6): { return eAL; }
        case (0xFBB7): { return eAL; }
        case (0xFBB8): { return eAL; }
        case (0xFBB9): { return eAL; }
        case (0xFBBA): { return eAL; }
        case (0xFBBB): { return eAL; }
        case (0xFBBC): { return eAL; }
        case (0xFBBD): { return eAL; }
        case (0xFBBE): { return eAL; }
        case (0xFBBF): { return eAL; }
        case (0xFBC0): { return eAL; }
        case (0xFBC1): { return eAL; }
        case (0xFBD3): { return eAL; }
        case (0xFBD4): { return eAL; }
        case (0xFBD5): { return eAL; }
        case (0xFBD6): { return eAL; }
        case (0xFBD7): { return eAL; }
        case (0xFBD8): { return eAL; }
        case (0xFBD9): { return eAL; }
        case (0xFBDA): { return eAL; }
        case (0xFBDB): { return eAL; }
        case (0xFBDC): { return eAL; }
        case (0xFBDD): { return eAL; }
        case (0xFBDE): { return eAL; }
        case (0xFBDF): { return eAL; }
        case (0xFBE0): { return eAL; }
        case (0xFBE1): { return eAL; }
        case (0xFBE2): { return eAL; }
        case (0xFBE3): { return eAL; }
        case (0xFBE4): { return eAL; }
        case (0xFBE5): { return eAL; }
        case (0xFBE6): { return eAL; }
        case (0xFBE7): { return eAL; }
        case (0xFBE8): { return eAL; }
        case (0xFBE9): { return eAL; }
        case (0xFBEA): { return eAL; }
        case (0xFBEB): { return eAL; }
        case (0xFBEC): { return eAL; }
        case (0xFBED): { return eAL; }
        case (0xFBEE): { return eAL; }
        case (0xFBEF): { return eAL; }
        case (0xFBF0): { return eAL; }
        case (0xFBF1): { return eAL; }
        case (0xFBF2): { return eAL; }
        case (0xFBF3): { return eAL; }
        case (0xFBF4): { return eAL; }
        case (0xFBF5): { return eAL; }
        case (0xFBF6): { return eAL; }
        case (0xFBF7): { return eAL; }
        case (0xFBF8): { return eAL; }
        case (0xFBF9): { return eAL; }
        case (0xFBFA): { return eAL; }
        case (0xFBFB): { return eAL; }
        case (0xFBFC): { return eAL; }
        case (0xFBFD): { return eAL; }
        case (0xFBFE): { return eAL; }
        case (0xFBFF): { return eAL; }
        case (0xFC00): { return eAL; }
        case (0xFC01): { return eAL; }
        case (0xFC02): { return eAL; }
        case (0xFC03): { return eAL; }
        case (0xFC04): { return eAL; }
        case (0xFC05): { return eAL; }
        case (0xFC06): { return eAL; }
        case (0xFC07): { return eAL; }
        case (0xFC08): { return eAL; }
        case (0xFC09): { return eAL; }
        case (0xFC0A): { return eAL; }
        case (0xFC0B): { return eAL; }
        case (0xFC0C): { return eAL; }
        case (0xFC0D): { return eAL; }
        case (0xFC0E): { return eAL; }
        case (0xFC0F): { return eAL; }
        case (0xFC10): { return eAL; }
        case (0xFC11): { return eAL; }
        case (0xFC12): { return eAL; }
        case (0xFC13): { return eAL; }
        case (0xFC14): { return eAL; }
        case (0xFC15): { return eAL; }
        case (0xFC16): { return eAL; }
        case (0xFC17): { return eAL; }
        case (0xFC18): { return eAL; }
        case (0xFC19): { return eAL; }
        case (0xFC1A): { return eAL; }
        case (0xFC1B): { return eAL; }
        case (0xFC1C): { return eAL; }
        case (0xFC1D): { return eAL; }
        case (0xFC1E): { return eAL; }
        case (0xFC1F): { return eAL; }
        case (0xFC20): { return eAL; }
        case (0xFC21): { return eAL; }
        case (0xFC22): { return eAL; }
        case (0xFC23): { return eAL; }
        case (0xFC24): { return eAL; }
        case (0xFC25): { return eAL; }
        case (0xFC26): { return eAL; }
        case (0xFC27): { return eAL; }
        case (0xFC28): { return eAL; }
        case (0xFC29): { return eAL; }
        case (0xFC2A): { return eAL; }
        case (0xFC2B): { return eAL; }
        case (0xFC2C): { return eAL; }
        case (0xFC2D): { return eAL; }
        case (0xFC2E): { return eAL; }
        case (0xFC2F): { return eAL; }
        case (0xFC30): { return eAL; }
        case (0xFC31): { return eAL; }
        case (0xFC32): { return eAL; }
        case (0xFC33): { return eAL; }
        case (0xFC34): { return eAL; }
        case (0xFC35): { return eAL; }
        case (0xFC36): { return eAL; }
        case (0xFC37): { return eAL; }
        case (0xFC38): { return eAL; }
        case (0xFC39): { return eAL; }
        case (0xFC3A): { return eAL; }
        case (0xFC3B): { return eAL; }
        case (0xFC3C): { return eAL; }
        case (0xFC3D): { return eAL; }
        case (0xFC3E): { return eAL; }
        case (0xFC3F): { return eAL; }
        case (0xFC40): { return eAL; }
        case (0xFC41): { return eAL; }
        case (0xFC42): { return eAL; }
        case (0xFC43): { return eAL; }
        case (0xFC44): { return eAL; }
        case (0xFC45): { return eAL; }
        case (0xFC46): { return eAL; }
        case (0xFC47): { return eAL; }
        case (0xFC48): { return eAL; }
        case (0xFC49): { return eAL; }
        case (0xFC4A): { return eAL; }
        case (0xFC4B): { return eAL; }
        case (0xFC4C): { return eAL; }
        case (0xFC4D): { return eAL; }
        case (0xFC4E): { return eAL; }
        case (0xFC4F): { return eAL; }
        case (0xFC50): { return eAL; }
        case (0xFC51): { return eAL; }
        case (0xFC52): { return eAL; }
        case (0xFC53): { return eAL; }
        case (0xFC54): { return eAL; }
        case (0xFC55): { return eAL; }
        case (0xFC56): { return eAL; }
        case (0xFC57): { return eAL; }
        case (0xFC58): { return eAL; }
        case (0xFC59): { return eAL; }
        case (0xFC5A): { return eAL; }
        case (0xFC5B): { return eAL; }
        case (0xFC5C): { return eAL; }
        case (0xFC5D): { return eAL; }
        case (0xFC5E): { return eAL; }
        case (0xFC5F): { return eAL; }
        case (0xFC60): { return eAL; }
        case (0xFC61): { return eAL; }
        case (0xFC62): { return eAL; }
        case (0xFC63): { return eAL; }
        case (0xFC64): { return eAL; }
        case (0xFC65): { return eAL; }
        case (0xFC66): { return eAL; }
        case (0xFC67): { return eAL; }
        case (0xFC68): { return eAL; }
        case (0xFC69): { return eAL; }
        case (0xFC6A): { return eAL; }
        case (0xFC6B): { return eAL; }
        case (0xFC6C): { return eAL; }
        case (0xFC6D): { return eAL; }
        case (0xFC6E): { return eAL; }
        case (0xFC6F): { return eAL; }
        case (0xFC70): { return eAL; }
        case (0xFC71): { return eAL; }
        case (0xFC72): { return eAL; }
        case (0xFC73): { return eAL; }
        case (0xFC74): { return eAL; }
        case (0xFC75): { return eAL; }
        case (0xFC76): { return eAL; }
        case (0xFC77): { return eAL; }
        case (0xFC78): { return eAL; }
        case (0xFC79): { return eAL; }
        case (0xFC7A): { return eAL; }
        case (0xFC7B): { return eAL; }
        case (0xFC7C): { return eAL; }
        case (0xFC7D): { return eAL; }
        case (0xFC7E): { return eAL; }
        case (0xFC7F): { return eAL; }
        case (0xFC80): { return eAL; }
        case (0xFC81): { return eAL; }
        case (0xFC82): { return eAL; }
        case (0xFC83): { return eAL; }
        case (0xFC84): { return eAL; }
        case (0xFC85): { return eAL; }
        case (0xFC86): { return eAL; }
        case (0xFC87): { return eAL; }
        case (0xFC88): { return eAL; }
        case (0xFC89): { return eAL; }
        case (0xFC8A): { return eAL; }
        case (0xFC8B): { return eAL; }
        case (0xFC8C): { return eAL; }
        case (0xFC8D): { return eAL; }
        case (0xFC8E): { return eAL; }
        case (0xFC8F): { return eAL; }
        case (0xFC90): { return eAL; }
        case (0xFC91): { return eAL; }
        case (0xFC92): { return eAL; }
        case (0xFC93): { return eAL; }
        case (0xFC94): { return eAL; }
        case (0xFC95): { return eAL; }
        case (0xFC96): { return eAL; }
        case (0xFC97): { return eAL; }
        case (0xFC98): { return eAL; }
        case (0xFC99): { return eAL; }
        case (0xFC9A): { return eAL; }
        case (0xFC9B): { return eAL; }
        case (0xFC9C): { return eAL; }
        case (0xFC9D): { return eAL; }
        case (0xFC9E): { return eAL; }
        case (0xFC9F): { return eAL; }
        case (0xFCA0): { return eAL; }
        case (0xFCA1): { return eAL; }
        case (0xFCA2): { return eAL; }
        case (0xFCA3): { return eAL; }
        case (0xFCA4): { return eAL; }
        case (0xFCA5): { return eAL; }
        case (0xFCA6): { return eAL; }
        case (0xFCA7): { return eAL; }
        case (0xFCA8): { return eAL; }
        case (0xFCA9): { return eAL; }
        case (0xFCAA): { return eAL; }
        case (0xFCAB): { return eAL; }
        case (0xFCAC): { return eAL; }
        case (0xFCAD): { return eAL; }
        case (0xFCAE): { return eAL; }
        case (0xFCAF): { return eAL; }
        case (0xFCB0): { return eAL; }
        case (0xFCB1): { return eAL; }
        case (0xFCB2): { return eAL; }
        case (0xFCB3): { return eAL; }
        case (0xFCB4): { return eAL; }
        case (0xFCB5): { return eAL; }
        case (0xFCB6): { return eAL; }
        case (0xFCB7): { return eAL; }
        case (0xFCB8): { return eAL; }
        case (0xFCB9): { return eAL; }
        case (0xFCBA): { return eAL; }
        case (0xFCBB): { return eAL; }
        case (0xFCBC): { return eAL; }
        case (0xFCBD): { return eAL; }
        case (0xFCBE): { return eAL; }
        case (0xFCBF): { return eAL; }
        case (0xFCC0): { return eAL; }
        case (0xFCC1): { return eAL; }
        case (0xFCC2): { return eAL; }
        case (0xFCC3): { return eAL; }
        case (0xFCC4): { return eAL; }
        case (0xFCC5): { return eAL; }
        case (0xFCC6): { return eAL; }
        case (0xFCC7): { return eAL; }
        case (0xFCC8): { return eAL; }
        case (0xFCC9): { return eAL; }
        case (0xFCCA): { return eAL; }
        case (0xFCCB): { return eAL; }
        case (0xFCCC): { return eAL; }
        case (0xFCCD): { return eAL; }
        case (0xFCCE): { return eAL; }
        case (0xFCCF): { return eAL; }
        case (0xFCD0): { return eAL; }
        case (0xFCD1): { return eAL; }
        case (0xFCD2): { return eAL; }
        case (0xFCD3): { return eAL; }
        case (0xFCD4): { return eAL; }
        case (0xFCD5): { return eAL; }
        case (0xFCD6): { return eAL; }
        case (0xFCD7): { return eAL; }
        case (0xFCD8): { return eAL; }
        case (0xFCD9): { return eAL; }
        case (0xFCDA): { return eAL; }
        case (0xFCDB): { return eAL; }
        case (0xFCDC): { return eAL; }
        case (0xFCDD): { return eAL; }
        case (0xFCDE): { return eAL; }
        case (0xFCDF): { return eAL; }
        case (0xFCE0): { return eAL; }
        case (0xFCE1): { return eAL; }
        case (0xFCE2): { return eAL; }
        case (0xFCE3): { return eAL; }
        case (0xFCE4): { return eAL; }
        case (0xFCE5): { return eAL; }
        case (0xFCE6): { return eAL; }
        case (0xFCE7): { return eAL; }
        case (0xFCE8): { return eAL; }
        case (0xFCE9): { return eAL; }
        case (0xFCEA): { return eAL; }
        case (0xFCEB): { return eAL; }
        case (0xFCEC): { return eAL; }
        case (0xFCED): { return eAL; }
        case (0xFCEE): { return eAL; }
        case (0xFCEF): { return eAL; }
        case (0xFCF0): { return eAL; }
        case (0xFCF1): { return eAL; }
        case (0xFCF2): { return eAL; }
        case (0xFCF3): { return eAL; }
        case (0xFCF4): { return eAL; }
        case (0xFCF5): { return eAL; }
        case (0xFCF6): { return eAL; }
        case (0xFCF7): { return eAL; }
        case (0xFCF8): { return eAL; }
        case (0xFCF9): { return eAL; }
        case (0xFCFA): { return eAL; }
        case (0xFCFB): { return eAL; }
        case (0xFCFC): { return eAL; }
        case (0xFCFD): { return eAL; }
        case (0xFCFE): { return eAL; }
        case (0xFCFF): { return eAL; }
        case (0xFD00): { return eAL; }
        case (0xFD01): { return eAL; }
        case (0xFD02): { return eAL; }
        case (0xFD03): { return eAL; }
        case (0xFD04): { return eAL; }
        case (0xFD05): { return eAL; }
        case (0xFD06): { return eAL; }
        case (0xFD07): { return eAL; }
        case (0xFD08): { return eAL; }
        case (0xFD09): { return eAL; }
        case (0xFD0A): { return eAL; }
        case (0xFD0B): { return eAL; }
        case (0xFD0C): { return eAL; }
        case (0xFD0D): { return eAL; }
        case (0xFD0E): { return eAL; }
        case (0xFD0F): { return eAL; }
        case (0xFD10): { return eAL; }
        case (0xFD11): { return eAL; }
        case (0xFD12): { return eAL; }
        case (0xFD13): { return eAL; }
        case (0xFD14): { return eAL; }
        case (0xFD15): { return eAL; }
        case (0xFD16): { return eAL; }
        case (0xFD17): { return eAL; }
        case (0xFD18): { return eAL; }
        case (0xFD19): { return eAL; }
        case (0xFD1A): { return eAL; }
        case (0xFD1B): { return eAL; }
        case (0xFD1C): { return eAL; }
        case (0xFD1D): { return eAL; }
        case (0xFD1E): { return eAL; }
        case (0xFD1F): { return eAL; }
        case (0xFD20): { return eAL; }
        case (0xFD21): { return eAL; }
        case (0xFD22): { return eAL; }
        case (0xFD23): { return eAL; }
        case (0xFD24): { return eAL; }
        case (0xFD25): { return eAL; }
        case (0xFD26): { return eAL; }
        case (0xFD27): { return eAL; }
        case (0xFD28): { return eAL; }
        case (0xFD29): { return eAL; }
        case (0xFD2A): { return eAL; }
        case (0xFD2B): { return eAL; }
        case (0xFD2C): { return eAL; }
        case (0xFD2D): { return eAL; }
        case (0xFD2E): { return eAL; }
        case (0xFD2F): { return eAL; }
        case (0xFD30): { return eAL; }
        case (0xFD31): { return eAL; }
        case (0xFD32): { return eAL; }
        case (0xFD33): { return eAL; }
        case (0xFD34): { return eAL; }
        case (0xFD35): { return eAL; }
        case (0xFD36): { return eAL; }
        case (0xFD37): { return eAL; }
        case (0xFD38): { return eAL; }
        case (0xFD39): { return eAL; }
        case (0xFD3A): { return eAL; }
        case (0xFD3B): { return eAL; }
        case (0xFD3C): { return eAL; }
        case (0xFD3D): { return eAL; }
        case (0xFD3E): { return eON; }
        case (0xFD3F): { return eON; }
        case (0xFD50): { return eAL; }
        case (0xFD51): { return eAL; }
        case (0xFD52): { return eAL; }
        case (0xFD53): { return eAL; }
        case (0xFD54): { return eAL; }
        case (0xFD55): { return eAL; }
        case (0xFD56): { return eAL; }
        case (0xFD57): { return eAL; }
        case (0xFD58): { return eAL; }
        case (0xFD59): { return eAL; }
        case (0xFD5A): { return eAL; }
        case (0xFD5B): { return eAL; }
        case (0xFD5C): { return eAL; }
        case (0xFD5D): { return eAL; }
        case (0xFD5E): { return eAL; }
        case (0xFD5F): { return eAL; }
        case (0xFD60): { return eAL; }
        case (0xFD61): { return eAL; }
        case (0xFD62): { return eAL; }
        case (0xFD63): { return eAL; }
        case (0xFD64): { return eAL; }
        case (0xFD65): { return eAL; }
        case (0xFD66): { return eAL; }
        case (0xFD67): { return eAL; }
        case (0xFD68): { return eAL; }
        case (0xFD69): { return eAL; }
        case (0xFD6A): { return eAL; }
        case (0xFD6B): { return eAL; }
        case (0xFD6C): { return eAL; }
        case (0xFD6D): { return eAL; }
        case (0xFD6E): { return eAL; }
        case (0xFD6F): { return eAL; }
        case (0xFD70): { return eAL; }
        case (0xFD71): { return eAL; }
        case (0xFD72): { return eAL; }
        case (0xFD73): { return eAL; }
        case (0xFD74): { return eAL; }
        case (0xFD75): { return eAL; }
        case (0xFD76): { return eAL; }
        case (0xFD77): { return eAL; }
        case (0xFD78): { return eAL; }
        case (0xFD79): { return eAL; }
        case (0xFD7A): { return eAL; }
        case (0xFD7B): { return eAL; }
        case (0xFD7C): { return eAL; }
        case (0xFD7D): { return eAL; }
        case (0xFD7E): { return eAL; }
        case (0xFD7F): { return eAL; }
        case (0xFD80): { return eAL; }
        case (0xFD81): { return eAL; }
        case (0xFD82): { return eAL; }
        case (0xFD83): { return eAL; }
        case (0xFD84): { return eAL; }
        case (0xFD85): { return eAL; }
        case (0xFD86): { return eAL; }
        case (0xFD87): { return eAL; }
        case (0xFD88): { return eAL; }
        case (0xFD89): { return eAL; }
        case (0xFD8A): { return eAL; }
        case (0xFD8B): { return eAL; }
        case (0xFD8C): { return eAL; }
        case (0xFD8D): { return eAL; }
        case (0xFD8E): { return eAL; }
        case (0xFD8F): { return eAL; }
        case (0xFD92): { return eAL; }
        case (0xFD93): { return eAL; }
        case (0xFD94): { return eAL; }
        case (0xFD95): { return eAL; }
        case (0xFD96): { return eAL; }
        case (0xFD97): { return eAL; }
        case (0xFD98): { return eAL; }
        case (0xFD99): { return eAL; }
        case (0xFD9A): { return eAL; }
        case (0xFD9B): { return eAL; }
        case (0xFD9C): { return eAL; }
        case (0xFD9D): { return eAL; }
        case (0xFD9E): { return eAL; }
        case (0xFD9F): { return eAL; }
        case (0xFDA0): { return eAL; }
        case (0xFDA1): { return eAL; }
        case (0xFDA2): { return eAL; }
        case (0xFDA3): { return eAL; }
        case (0xFDA4): { return eAL; }
        case (0xFDA5): { return eAL; }
        case (0xFDA6): { return eAL; }
        case (0xFDA7): { return eAL; }
        case (0xFDA8): { return eAL; }
        case (0xFDA9): { return eAL; }
        case (0xFDAA): { return eAL; }
        case (0xFDAB): { return eAL; }
        case (0xFDAC): { return eAL; }
        case (0xFDAD): { return eAL; }
        case (0xFDAE): { return eAL; }
        case (0xFDAF): { return eAL; }
        case (0xFDB0): { return eAL; }
        case (0xFDB1): { return eAL; }
        case (0xFDB2): { return eAL; }
        case (0xFDB3): { return eAL; }
        case (0xFDB4): { return eAL; }
        case (0xFDB5): { return eAL; }
        case (0xFDB6): { return eAL; }
        case (0xFDB7): { return eAL; }
        case (0xFDB8): { return eAL; }
        case (0xFDB9): { return eAL; }
        case (0xFDBA): { return eAL; }
        case (0xFDBB): { return eAL; }
        case (0xFDBC): { return eAL; }
        case (0xFDBD): { return eAL; }
        case (0xFDBE): { return eAL; }
        case (0xFDBF): { return eAL; }
        case (0xFDC0): { return eAL; }
        case (0xFDC1): { return eAL; }
        case (0xFDC2): { return eAL; }
        case (0xFDC3): { return eAL; }
        case (0xFDC4): { return eAL; }
        case (0xFDC5): { return eAL; }
        case (0xFDC6): { return eAL; }
        case (0xFDC7): { return eAL; }
        case (0xFDF0): { return eAL; }
        case (0xFDF1): { return eAL; }
        case (0xFDF2): { return eAL; }
        case (0xFDF3): { return eAL; }
        case (0xFDF4): { return eAL; }
        case (0xFDF5): { return eAL; }
        case (0xFDF6): { return eAL; }
        case (0xFDF7): { return eAL; }
        case (0xFDF8): { return eAL; }
        case (0xFDF9): { return eAL; }
        case (0xFDFA): { return eAL; }
        case (0xFDFB): { return eAL; }
        case (0xFDFC): { return eAL; }
        case (0xFDFD): { return eON; }
        case (0xFE00): { return eNSM; }
        case (0xFE01): { return eNSM; }
        case (0xFE02): { return eNSM; }
        case (0xFE03): { return eNSM; }
        case (0xFE04): { return eNSM; }
        case (0xFE05): { return eNSM; }
        case (0xFE06): { return eNSM; }
        case (0xFE07): { return eNSM; }
        case (0xFE08): { return eNSM; }
        case (0xFE09): { return eNSM; }
        case (0xFE0A): { return eNSM; }
        case (0xFE0B): { return eNSM; }
        case (0xFE0C): { return eNSM; }
        case (0xFE0D): { return eNSM; }
        case (0xFE0E): { return eNSM; }
        case (0xFE0F): { return eNSM; }
        case (0xFE10): { return eON; }
        case (0xFE11): { return eON; }
        case (0xFE12): { return eON; }
        case (0xFE13): { return eON; }
        case (0xFE14): { return eON; }
        case (0xFE15): { return eON; }
        case (0xFE16): { return eON; }
        case (0xFE17): { return eON; }
        case (0xFE18): { return eON; }
        case (0xFE19): { return eON; }
        case (0xFE20): { return eNSM; }
        case (0xFE21): { return eNSM; }
        case (0xFE22): { return eNSM; }
        case (0xFE23): { return eNSM; }
        case (0xFE24): { return eNSM; }
        case (0xFE25): { return eNSM; }
        case (0xFE26): { return eNSM; }
        case (0xFE30): { return eON; }
        case (0xFE31): { return eON; }
        case (0xFE32): { return eON; }
        case (0xFE33): { return eON; }
        case (0xFE34): { return eON; }
        case (0xFE35): { return eON; }
        case (0xFE36): { return eON; }
        case (0xFE37): { return eON; }
        case (0xFE38): { return eON; }
        case (0xFE39): { return eON; }
        case (0xFE3A): { return eON; }
        case (0xFE3B): { return eON; }
        case (0xFE3C): { return eON; }
        case (0xFE3D): { return eON; }
        case (0xFE3E): { return eON; }
        case (0xFE3F): { return eON; }
        case (0xFE40): { return eON; }
        case (0xFE41): { return eON; }
        case (0xFE42): { return eON; }
        case (0xFE43): { return eON; }
        case (0xFE44): { return eON; }
        case (0xFE45): { return eON; }
        case (0xFE46): { return eON; }
        case (0xFE47): { return eON; }
        case (0xFE48): { return eON; }
        case (0xFE49): { return eON; }
        case (0xFE4A): { return eON; }
        case (0xFE4B): { return eON; }
        case (0xFE4C): { return eON; }
        case (0xFE4D): { return eON; }
        case (0xFE4E): { return eON; }
        case (0xFE4F): { return eON; }
        case (0xFE50): { return eCS; }
        case (0xFE51): { return eON; }
        case (0xFE52): { return eCS; }
        case (0xFE54): { return eON; }
        case (0xFE55): { return eCS; }
        case (0xFE56): { return eON; }
        case (0xFE57): { return eON; }
        case (0xFE58): { return eON; }
        case (0xFE59): { return eON; }
        case (0xFE5A): { return eON; }
        case (0xFE5B): { return eON; }
        case (0xFE5C): { return eON; }
        case (0xFE5D): { return eON; }
        case (0xFE5E): { return eON; }
        case (0xFE5F): { return eET; }
        case (0xFE60): { return eON; }
        case (0xFE61): { return eON; }
        case (0xFE62): { return eES; }
        case (0xFE63): { return eES; }
        case (0xFE64): { return eON; }
        case (0xFE65): { return eON; }
        case (0xFE66): { return eON; }
        case (0xFE68): { return eON; }
        case (0xFE69): { return eET; }
        case (0xFE6A): { return eET; }
        case (0xFE6B): { return eON; }
        case (0xFE70): { return eAL; }
        case (0xFE71): { return eAL; }
        case (0xFE72): { return eAL; }
        case (0xFE73): { return eAL; }
        case (0xFE74): { return eAL; }
        case (0xFE76): { return eAL; }
        case (0xFE77): { return eAL; }
        case (0xFE78): { return eAL; }
        case (0xFE79): { return eAL; }
        case (0xFE7A): { return eAL; }
        case (0xFE7B): { return eAL; }
        case (0xFE7C): { return eAL; }
        case (0xFE7D): { return eAL; }
        case (0xFE7E): { return eAL; }
        case (0xFE7F): { return eAL; }
        case (0xFE80): { return eAL; }
        case (0xFE81): { return eAL; }
        case (0xFE82): { return eAL; }
        case (0xFE83): { return eAL; }
        case (0xFE84): { return eAL; }
        case (0xFE85): { return eAL; }
        case (0xFE86): { return eAL; }
        case (0xFE87): { return eAL; }
        case (0xFE88): { return eAL; }
        case (0xFE89): { return eAL; }
        case (0xFE8A): { return eAL; }
        case (0xFE8B): { return eAL; }
        case (0xFE8C): { return eAL; }
        case (0xFE8D): { return eAL; }
        case (0xFE8E): { return eAL; }
        case (0xFE8F): { return eAL; }
        case (0xFE90): { return eAL; }
        case (0xFE91): { return eAL; }
        case (0xFE92): { return eAL; }
        case (0xFE93): { return eAL; }
        case (0xFE94): { return eAL; }
        case (0xFE95): { return eAL; }
        case (0xFE96): { return eAL; }
        case (0xFE97): { return eAL; }
        case (0xFE98): { return eAL; }
        case (0xFE99): { return eAL; }
        case (0xFE9A): { return eAL; }
        case (0xFE9B): { return eAL; }
        case (0xFE9C): { return eAL; }
        case (0xFE9D): { return eAL; }
        case (0xFE9E): { return eAL; }
        case (0xFE9F): { return eAL; }
        case (0xFEA0): { return eAL; }
        case (0xFEA1): { return eAL; }
        case (0xFEA2): { return eAL; }
        case (0xFEA3): { return eAL; }
        case (0xFEA4): { return eAL; }
        case (0xFEA5): { return eAL; }
        case (0xFEA6): { return eAL; }
        case (0xFEA7): { return eAL; }
        case (0xFEA8): { return eAL; }
        case (0xFEA9): { return eAL; }
        case (0xFEAA): { return eAL; }
        case (0xFEAB): { return eAL; }
        case (0xFEAC): { return eAL; }
        case (0xFEAD): { return eAL; }
        case (0xFEAE): { return eAL; }
        case (0xFEAF): { return eAL; }
        case (0xFEB0): { return eAL; }
        case (0xFEB1): { return eAL; }
        case (0xFEB2): { return eAL; }
        case (0xFEB3): { return eAL; }
        case (0xFEB4): { return eAL; }
        case (0xFEB5): { return eAL; }
        case (0xFEB6): { return eAL; }
        case (0xFEB7): { return eAL; }
        case (0xFEB8): { return eAL; }
        case (0xFEB9): { return eAL; }
        case (0xFEBA): { return eAL; }
        case (0xFEBB): { return eAL; }
        case (0xFEBC): { return eAL; }
        case (0xFEBD): { return eAL; }
        case (0xFEBE): { return eAL; }
        case (0xFEBF): { return eAL; }
        case (0xFEC0): { return eAL; }
        case (0xFEC1): { return eAL; }
        case (0xFEC2): { return eAL; }
        case (0xFEC3): { return eAL; }
        case (0xFEC4): { return eAL; }
        case (0xFEC5): { return eAL; }
        case (0xFEC6): { return eAL; }
        case (0xFEC7): { return eAL; }
        case (0xFEC8): { return eAL; }
        case (0xFEC9): { return eAL; }
        case (0xFECA): { return eAL; }
        case (0xFECB): { return eAL; }
        case (0xFECC): { return eAL; }
        case (0xFECD): { return eAL; }
        case (0xFECE): { return eAL; }
        case (0xFECF): { return eAL; }
        case (0xFED0): { return eAL; }
        case (0xFED1): { return eAL; }
        case (0xFED2): { return eAL; }
        case (0xFED3): { return eAL; }
        case (0xFED4): { return eAL; }
        case (0xFED5): { return eAL; }
        case (0xFED6): { return eAL; }
        case (0xFED7): { return eAL; }
        case (0xFED8): { return eAL; }
        case (0xFED9): { return eAL; }
        case (0xFEDA): { return eAL; }
        case (0xFEDB): { return eAL; }
        case (0xFEDC): { return eAL; }
        case (0xFEDD): { return eAL; }
        case (0xFEDE): { return eAL; }
        case (0xFEDF): { return eAL; }
        case (0xFEE0): { return eAL; }
        case (0xFEE1): { return eAL; }
        case (0xFEE2): { return eAL; }
        case (0xFEE3): { return eAL; }
        case (0xFEE4): { return eAL; }
        case (0xFEE5): { return eAL; }
        case (0xFEE6): { return eAL; }
        case (0xFEE7): { return eAL; }
        case (0xFEE8): { return eAL; }
        case (0xFEE9): { return eAL; }
        case (0xFEEA): { return eAL; }
        case (0xFEEB): { return eAL; }
        case (0xFEEC): { return eAL; }
        case (0xFEED): { return eAL; }
        case (0xFEEE): { return eAL; }
        case (0xFEEF): { return eAL; }
        case (0xFEF0): { return eAL; }
        case (0xFEF1): { return eAL; }
        case (0xFEF2): { return eAL; }
        case (0xFEF3): { return eAL; }
        case (0xFEF4): { return eAL; }
        case (0xFEF5): { return eAL; }
        case (0xFEF6): { return eAL; }
        case (0xFEF7): { return eAL; }
        case (0xFEF8): { return eAL; }
        case (0xFEF9): { return eAL; }
        case (0xFEFA): { return eAL; }
        case (0xFEFB): { return eAL; }
        case (0xFEFC): { return eAL; }
        case (0xFEFF): { return eBN; }
        case (0xFF01): { return eON; }
        case (0xFF02): { return eON; }
        case (0xFF03): { return eET; }
        case (0xFF04): { return eET; }
        case (0xFF05): { return eET; }
        case (0xFF06): { return eON; }
        case (0xFF07): { return eON; }
        case (0xFF08): { return eON; }
        case (0xFF09): { return eON; }
        case (0xFF0A): { return eON; }
        case (0xFF0B): { return eES; }
        case (0xFF0C): { return eCS; }
        case (0xFF0D): { return eES; }
        case (0xFF0E): { return eCS; }
        case (0xFF0F): { return eCS; }
        case (0xFF10): { return eEN; }
        case (0xFF11): { return eEN; }
        case (0xFF12): { return eEN; }
        case (0xFF13): { return eEN; }
        case (0xFF14): { return eEN; }
        case (0xFF15): { return eEN; }
        case (0xFF16): { return eEN; }
        case (0xFF17): { return eEN; }
        case (0xFF18): { return eEN; }
        case (0xFF19): { return eEN; }
        case (0xFF1A): { return eCS; }
        case (0xFF1B): { return eON; }
        case (0xFF1C): { return eON; }
        case (0xFF1D): { return eON; }
        case (0xFF1E): { return eON; }
        case (0xFF1F): { return eON; }
        case (0xFF20): { return eON; }
        case (0xFF3B): { return eON; }
        case (0xFF3C): { return eON; }
        case (0xFF3D): { return eON; }
        case (0xFF3E): { return eON; }
        case (0xFF3F): { return eON; }
        case (0xFF40): { return eON; }
        case (0xFF5B): { return eON; }
        case (0xFF5C): { return eON; }
        case (0xFF5D): { return eON; }
        case (0xFF5E): { return eON; }
        case (0xFF5F): { return eON; }
        case (0xFF60): { return eON; }
        case (0xFF61): { return eON; }
        case (0xFF62): { return eON; }
        case (0xFF63): { return eON; }
        case (0xFF64): { return eON; }
        case (0xFF65): { return eON; }
        case (0xFFE0): { return eET; }
        case (0xFFE1): { return eET; }
        case (0xFFE2): { return eON; }
        case (0xFFE3): { return eON; }
        case (0xFFE4): { return eON; }
        case (0xFFE5): { return eET; }
        case (0xFFE6): { return eET; }
        case (0xFFE8): { return eON; }
        case (0xFFE9): { return eON; }
        case (0xFFEA): { return eON; }
        case (0xFFEB): { return eON; }
        case (0xFFEC): { return eON; }
        case (0xFFED): { return eON; }
        case (0xFFEE): { return eON; }
        case (0xFFF9): { return eON; }
        case (0xFFFA): { return eON; }
        case (0xFFFB): { return eON; }
        case (0xFFFC): { return eON; }
        case (0xFFFD): { return eON; }
        default: { return eL; } // 10,328 L's, so used this as default to cut down the size
    }
}
