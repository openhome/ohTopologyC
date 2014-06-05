#ifndef HEADER_UI_UNICODE
#define HEADER_UI_UNICODE

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Buffer.h>


EXCEPTION(InvalidString);

namespace OpenHome {

typedef TUint32 TUnicode; // Big endian

class Utf8
{
public:
    Utf8();
    Utf8(const Brx& aBuffer);
    void Set(const Brx& aBuffer);
    TUnicode Next(); // get next raw unicode value
    TBool End();
    static TUint MaxBufferSize(const Brx& aBuffer, TUint aMaxSizeAllowed); // determine max correct size if having to limit a string size (i.e. make sure the string is not split part way through a multiple byte character, leading to malformed utf8)

private:
    TByte FirstByte();
    TByte NextByte();
    static TBool IsValidFirst(TByte aByte);
    static TBool IsValidFollowing(TByte aByte);
    static TUint FollowingBytesExpected(TByte aByte);

    static const TUint8 kFirstByteMaskWidth1 = 0x80;
    static const TUint8 kFirstByteMaskWidth2 = 0xe0;
    static const TUint8 kFirstByteMaskWidth3 = 0xf0;
    static const TUint8 kFirstByteMaskWidth4 = 0xf8;
    static const TUint8 kFollowingBytesMask = 0xC0;

    enum FirstByteType
    {
        eOneByte = 0x00,
        eTwoBytes = 0xc0,
        eThreeBytes = 0xe0,
        eFourBytes= 0xf0,
    };

    Brn iBuffer;
    TUint iIndex;
};

///////////////////////////////////////////////////////////////

class String // create a unicode string
{
public:
    enum Encoding {
        eUtf8, // 1-4 bytes per character
        eLatin1, // 1 byte per character only
        eRawUnicode, // 4 bytes per character
    };

public:
    String(Encoding aOriginalEncoding = eUtf8);
    String(const Brx& aBuffer, Encoding aOriginalEncoding = eUtf8);
    TUnicode At(TUint aCharIndex) const;
    void Append(const Brx& aBuffer);
    void Prepend(const Brx& aBuffer);
    void Replace(const Brx& aBuffer);
    void Shrink(TUint aCharIndex, TUint aChars);
    TUint Chars() const;
    void ToUtf8(Bwx& aBuffer);
    TBool IsRightToLeft();
    void ReorderRightToLeft();
    const Brx& Buffer() const; // for test code

private:
    enum EBidiCharType {
        //Table 4. Bidirectional Character Types, from Unicode Bidirectional Algorithm (USA #9): http://www.unicode.org/reports/tr9/
        //Strong
        eL = 0, //00 Left-to-Right              LRM, most alphabetic, syllabic, Han ideographs, non-European or non-Arabic digits, ...
        eLRE,   //01 Left-to-Right Embedding    LRE
        eLRO,   //02 Left-to-Right Override     LRO
        eR,     //03 Right-to-Left              RLM, Hebrew alphabet, and related punctuation
        eAL,    //04 Right-to-Left              Arabic  Arabic, Thaana, and Syriac alphabets, most punctuation specific to those scripts, ...
        eRLE,   //05 Right-to-Left Embedding    RLE
        eRLO,   //06 Right-to-Left Override     RLO
        //Weak
        ePDF,   //07 Pop Directional Format     PDF
        eEN,    //08 European Number            European digits, Eastern Arabic-Indic digits, ...
        eES,    //09 European Number Separator  Plus sign, minus sign
        eET,    //0A European Number Terminator Degree sign, currency symbols, ...
        eAN,    //0B Arabic Number              Arabic-Indic digits, Arabic decimal and thousands separators, ...
        eCS,    //0C Common Number Separator    Colon, comma, full stop (period), No-break space, ...
        eNSM,   //0D Nonspacing Mark            Characters marked Mn (Nonspacing_Mark) and Me (Enclosing_Mark) in the Unicode Character Database
        eBN,    //0E Boundary Neutral           Default ignorables, non-characters, and control characters, other than those explicitly given other types.
        //Neutral
        eB,     //0F Paragraph Separator        Paragraph separator, appropriate Newline Functions, higher-level protocol paragraph determination
        eS,     //10 Segment Separator          Tab
        eWS,    //11 Whitespace                 Space, figure space, line separator, form feed, General Punctuation spaces, ...
        eON,    //12 Other Neutrals             All other characters, including OBJECT REPLACEMENT CHARACTER
        eNL     //13 New Line Char 0x0A         Special Case: normally treated as eB for multiline text, but for single line only used as break between fields so needs to assume eR instead if string is RTL (ensures \n never takes on eL if surrounded by eL chars in an RTL string), can't be hard coded as eR as this would make all string RTL
    };

    EBidiCharType BidiCharType(TUnicode aUnicode);

    Bwh iBuffer;
    Encoding iOriginalEncoding;
};

namespace Unicode {

TBool OrderedString(const Brx& aOriginal, Bwx& aOrdered, String::Encoding aOriginalEncoding = String::eUtf8); // return true if RTL, return false if LTR

} //namespace Unicode

} //namespace OpenHome

#endif//HEADER_UNICODE
