#include <Linn/Ui/Otb.h>

using namespace Linn;
using namespace Linn::Ui;
using namespace Linn::Arch;

const Brn kOtbTableCmap("cmap");
const Brn kOtbTableEblc("EBLC");
const Brn kOtbTableEbdt("EBDT");

// The Otb file begins with the offset table:
//
//      Version         4 bytes     0x00010000 for version 1.0
//      NumberOfTables  2 bytes     number of tables
//      SearchRange     2 bytes     (Maximum power of 2 <= numberOfTables) x 16
//      EntrySelector   2 bytes     Log2(Maximum power of 2 <= numberOfTables)
//      RangeShift      2 bytes     numberOfTables x 16 - searchRange
//
// This is followed by the table directory, which contains 16 bytes per table entry as follows:
//
//      Tag         4 bytes     4 character name of the table
//      Checksum    4 bytes     checksum of the table
//      Offset      4 bytes     offset to start of table
//      Length      4 bytes     length of the table

//Find a table using the table directory

const Brn Linn::Ui::OtbTable(const Brx& aFile, const Brx& aName)
{
    #define OTB_OFFSET_TABLE_SIZE 12
    #define TABLE_DIRECTORY_OFFSET_INDEX 8
    #define TABLE_DIRECTORY_LENGTH_INDEX 12
    TUint count = aFile.BeUint16At(4);                                  //Get the number of tables
    for(TUint i = 0; i < count; i++)                                    //Search through the table directory
    {
        if (aFile.Split((i << 4) + OTB_OFFSET_TABLE_SIZE, 4) == aName)  //Compare the current table name to our name
        {
            //Get the cmap header address
            const TByte* ptr = aFile.Ptr() + aFile.BeUint32At((i << 4) + OTB_OFFSET_TABLE_SIZE + TABLE_DIRECTORY_OFFSET_INDEX);
            //Get the cmap header length
            TUint length = aFile.BeUint32At((i << 4) + OTB_OFFSET_TABLE_SIZE + TABLE_DIRECTORY_LENGTH_INDEX);
            return (Brn(ptr, length));  //return a buffer containing the cmap header
        }
    }
    // If you reach here --> Otb table not found
    // Most likely due to an incorrect name supplied (programmer error)
    ASSERT(false);
}

// The cmap table header:
//
//      Version         2 bytes     0x0000
//      NumberOfTables  2 bytes     number of tables
//
// This is followed by a number of 8 byte encoding records as follows:
//
//      Platform    2 bytes     platform id
//      Encoding    2 bytes     encoding id
//      Offset      4 bytes     offset to sub table
//
// Each sub table begins as follows:
//
//      Format      2 bytes     type of sub table
//      Length      2 bytes     length of sub table

const Brn Linn::Ui::OtbCmapSubTable(const Brx& aCmapTable, EEncodingId aId)
{
    #define CMAP_ENCODING_RECORD_OFFSET 4

    TUint count = aCmapTable.BeUint16At(2); // Number of sub tables

    for(TUint i = 0; i < count; i++)
    {
        if (aCmapTable.BeUint16At((i << 3) + 2 + CMAP_ENCODING_RECORD_OFFSET) == aId)
        {
            TUint offset = aCmapTable.BeUint32At((i << 3) + 4 + CMAP_ENCODING_RECORD_OFFSET);
            TUint length = aCmapTable.BeUint16At(offset + 2); // look at length of sub table (1st 2 bytes are format)
            return (Brn(aCmapTable.Ptr() + offset, length)); //return cmap subtable
        }
    }
    // If you reach here --> Otb cmap sub table not found
    // Most likely due to an incorrect encoding ID supplied (programmer error)
    ASSERT(false);
}

// The cmap sub table comes in a number of formats. We only support format 4:
//
//      Format          2 bytes     type of sub table (4)
//      Length          2 bytes     length of sub table
//      Language        2 bytes
//      SegCountx2      2 bytes     2 x segment count
//      SearchRange     2 bytes     2 x (2 ** floor(log2(segment count)))
//      EntrySelector   2 bytes     log2(search range / 2)
//      RangeShift      2 bytes     (2 x segment count) - search range
//      EndCount*       2 bytes     end character code for each segment, last = 0xffff
//      reserved        2 bytes     0
//      StartCount*     2 bytes     start character code for each segment
//      IdDelta*        2 bytes     delta for each segment
//      IdRangeOffset   2 bytes     offset into glyphidarray for each segment
//      glyphidarray*   2 bytes     address of glyph id array

TUint16 Linn::Ui::OtbCmapGlyphIndex(const Brn& aCmapSubTable, TUnicode aUnicode)
{
    TUint16 glyphIndex = 0;
    TBool   glyphIndexFound = false;
    if (aCmapSubTable.BeUint16At(0) == 4) //get the format of the table
    {
        #define RESERVED_PAD_SIZE 2
        TUint segmentSize = aCmapSubTable.BeUint16At(6); //number of segments in bytes
        Brn endCount(aCmapSubTable.Ptr() + 14, segmentSize);
        Brn startCount(endCount.Ptr() + segmentSize + RESERVED_PAD_SIZE, segmentSize);
        Brn idDelta(startCount.Ptr() + segmentSize, segmentSize);
        Brn idRangeOffset(idDelta.Ptr() + segmentSize, segmentSize);

        for(TUint i=0; i<segmentSize; i+=2)
        {
            if(endCount.BeUint16At(i) >= aUnicode)
            {
                if(startCount.BeUint16At(i) <= aUnicode)
                {
                    if(idRangeOffset.BeUint16At(i) == 0)
                    {
                        glyphIndexFound = true;
                        glyphIndex = aUnicode + idDelta.BeUint16At(i);
                    }
                    else
                    {
                        Brn glyphIdArray(idRangeOffset.Ptr() + segmentSize, (aCmapSubTable.Bytes() - (16 + (4*segmentSize))));
                        TUint16 offset = ((idRangeOffset.BeUint16At(i)>>1) + (aUnicode - startCount.BeUint16At(i))) * sizeof(TUint16);
                        glyphIndex = glyphIdArray.BeUint16At((i+offset) - segmentSize);
                        if (glyphIndex != 0) // 0 implies missing glyph
                        {
                            glyphIndexFound = true;
                            glyphIndex += idDelta.BeUint16At(i);
                        }
                    }
                }
            }
        }
    }
    // Check Otb cmap glyph index was found (glyph data for the requested unicode value exists)
    // Most likely missing if searching for glyph index for a control charater
    ASSERT(glyphIndexFound);

    return (glyphIndex);
}

// The EBLC header
//
//      version         4 bytes     initially define as 0x00020000
//      numSizes        4 bytes     number of bitmapSizeTables
//

// bitmap size table
//      indexSubTableArrayOffset    4 bytes     offset to index subtable
//      indexTableSize              4 bytes     no of bytes in subtables and array
//      noOfIndexSubTables          4 bytes     table for each range/format change
//      colorRef                    4 bytes     0
//      hori                       12 bytes     line metrics structure
//      vert                       12 bytes     line metrics structure
//      startGlyphIndex             2 bytes     lowest glyph index for size
//      endGlyphIndex               2 bytes     highest glyph index for size
//      ppemX                       1 byte      horizontal pizels per Em
//      ppemY                       1 byte      vertical pixels per Em
//      bitDepth                    1 byte      microsoft rasterizer supports 1,2,4,8
//      flags                       1 byte      vertical or horizontal

const Brn Linn::Ui::EblcBitmapTable(const Brx& aEblc, TUint aPointSize)
{
    #define EBLC_HEADER_SIZE  8
    #define BITMAP_TABLE_SIZE 48
    #define PPEMY_INDEX 45

    TUint noOfBitmapSizeTables = aEblc.BeUint32At(4);

    for(TUint i=0; i<noOfBitmapSizeTables; i++)//find the table with the strike of the right size
    {
        if(aEblc.Uint8At(EBLC_HEADER_SIZE + (i * BITMAP_TABLE_SIZE) + PPEMY_INDEX) == (TUint8)aPointSize)
        {
            return(Brn(aEblc.Ptr() + EBLC_HEADER_SIZE + (i * BITMAP_TABLE_SIZE), BITMAP_TABLE_SIZE));
        }
    }
    // If you reach here --> requested font point size not availabl ein given otb file
    // Most likely due to an incorrect point size supplied (programmer error)
    ASSERT(false);
}

// EBLC index subtable array
//      firstGlyphIndex                 2 bytes     first glyph code of this range
//      lastGlyphIndex                  2 bytes     last glyph code of this range
//      additionalOffsetToIndexSubtable 4 bytes     add to indexSubTableArrayOffset to get offset

// EBLC index subtable header
//      indexFormat                     2 bytes     format of this indexSubTable
//      imageFormat                     2 bytes     format of EBDT image
//      imageDataOffset                 4 bytes     offset to image data of EBDT table

// EBLC index subtable format 1:
//      header                          8 bytes     as above
//      offsetArray                     4 bytes     offsetArray[glyphIndex] + imageDataOffset = glyphDataOffset

const Brn Linn::Ui::EblcSubTableArray(const Brx& aEblc, TUint aPointSize, TUint aGlyphIndex)
{
    Brn eblcBitmapTable = EblcBitmapTable(aEblc, aPointSize);

    TUint indexSubTableArrayOffset =  eblcBitmapTable.BeUint32At(0);
    TUint sizeOfSubTableData = eblcBitmapTable.BeUint32At(4);
    TUint noOfIndexSubTables =  eblcBitmapTable.BeUint32At(8);  ///BigEndian4(eblcBitmapSizeTable->noOfIndexSubTables);

    //Create buffer of array data
    Brn eblcSubTableArray(aEblc.Ptr() + indexSubTableArrayOffset, sizeOfSubTableData);

    #define EBLC_SUBTABLE_ARRAY_SIZE 8
    //Find the array element of the right size
    for(TUint i=0; i<noOfIndexSubTables; i++)
    {
        if(  (eblcSubTableArray.BeUint16At(i*EBLC_SUBTABLE_ARRAY_SIZE) <= aGlyphIndex)
           &&(eblcSubTableArray.BeUint16At((i*EBLC_SUBTABLE_ARRAY_SIZE) + 2) >= aGlyphIndex))
        {
            return(Brn(eblcSubTableArray.Ptr() + (i*EBLC_SUBTABLE_ARRAY_SIZE), EBLC_SUBTABLE_ARRAY_SIZE));
        }
    }
    // If you reach here --> Otb eblc sub table array element not found
    // Most likely due to an incorrect glyph index supplied (programmer error)
    ASSERT(false);
}


// class Otb

Otb::Otb(const Brx& aFile) : iFile(aFile)
{
}


void Otb::InitialiseGlyph(Glyph& aGlyph, TUnicode aUnicode, TUint aPointSize) const
{
    const GlyphMetrics* glyphMetrics = 0;
    EGlyphAlignment glyphAlignment = eInvalid;
    const TByte* pixelsLocation = 0;
    TUint pixelsBits = 0;
    TUint pixelsBytes = 0;

    const Brn cmap = OtbTable(iFile, kOtbTableCmap); // Find cmap table
    const Brn cmapSubTable = OtbCmapSubTable(cmap, eUnicode); // Find unicode encoding sub table
    TUint glyphIndex = OtbCmapGlyphIndex(cmapSubTable, aUnicode); // Find glyph index
    const Brn eblc = OtbTable(iFile, kOtbTableEblc); // Find eblc table
    const Brn eblcBitmapTable = EblcBitmapTable(eblc, aPointSize); // Get the bitmap table for the given point size
    const Brn eblcSubTableArray = EblcSubTableArray(eblc, aPointSize, glyphIndex);
    const Brn ebdt = OtbTable(iFile, kOtbTableEbdt); // Find ebdt table

    #define EBLC_SUBTABLE_SIZE_WITHOUT_DATA 16
    Brn subTableNoData(eblc.Ptr() + eblcBitmapTable.BeUint32At(0) + eblcSubTableArray.BeUint32At(4), EBLC_SUBTABLE_SIZE_WITHOUT_DATA);

    #define INDEX_SUBHEADER_SIZE 8
    #define SMALL_METRICS_SIZE 5

    //We deduct the index value of the first supported glyphcode in the range from our glyph index to get a relative index
    //then use the index to with the offset array  (glyphDataOffset = offsetArray[index])

    TUint offsetArraySize = (eblcSubTableArray.BeUint16At(2) - eblcSubTableArray.BeUint16At(0) + 2) * sizeof (TUint32);
    Brn offsetArray(subTableNoData.Ptr() + INDEX_SUBHEADER_SIZE, offsetArraySize);

    TUint16 indexFormat = subTableNoData.BeUint16At(0);
    TUint16 imageFormat = subTableNoData.BeUint16At(2);

    if ( (indexFormat == 1) && (imageFormat == 2) ) // index = 1: variable glyph metrics for each glyph stored in EBDT
    {
        TUint32 glyphDataOffset = offsetArray.BeUint32At((glyphIndex - eblcSubTableArray.BeUint16At(0)) * sizeof (TUint32));
        glyphMetrics = (GlyphMetrics*) (ebdt.Ptr() + subTableNoData.BeUint32At(4) + glyphDataOffset);
        pixelsLocation = (TByte*) &ebdt.At(subTableNoData.BeUint32At(4) + glyphDataOffset + SMALL_METRICS_SIZE);
        pixelsBits = glyphMetrics->iHeight * glyphMetrics->iWidth;
        pixelsBytes = ((pixelsBits)>>3) + (((pixelsBits&0x07)==0) ? 0:1);
        //LOG(kUi,"Unicode 0x%x, EBDT location: 0x%x\n",aUnicode,subTableNoData.BeUint32At(4) + glyphDataOffset + SMALL_METRICS_SIZE + (ebdt.Ptr() - iFile.Ptr()));
        glyphAlignment = eBitAligned; // image format 2: small metrics, bit alligned image data (EBDT)
        aGlyph.Set(Brn(pixelsLocation,pixelsBytes), glyphAlignment, glyphMetrics);
    }
    else if ( (indexFormat == 2) && (imageFormat == 5) ) // index = 2: identical glyph metrics for each glyph in range
    {
        // get the metrics from the EBLC table directly
        glyphMetrics = (GlyphMetrics*) (subTableNoData.Ptr() + 12);
        // get the image data from EBDT - don't need offset as every glyph is the same size
        TUint32 imageSize = subTableNoData.BeUint32At(8);
        pixelsLocation = (TByte*) &ebdt.At(subTableNoData.BeUint32At(4) + imageSize * (glyphIndex - eblcSubTableArray.BeUint16At(0)));
        pixelsBits = glyphMetrics->iHeight * glyphMetrics->iWidth;
        pixelsBytes = ((pixelsBits)>>3) + (((pixelsBits&0x07)==0) ? 0:1);
        //LOG(kUi,"Unicode 0x%x, EBDT location: 0x%x\n",aUnicode,subTableNoData.BeUint32At(4) + (imageSize * (glyphIndex - eblcSubTableArray.BeUint16At(0))) + (ebdt.Ptr() - iFile.Ptr()));
        glyphAlignment = eBitAligned; // image format 5: metrics in EBLC, bit alligned image data only (EBDT)
        aGlyph.Set(Brn(pixelsLocation,pixelsBytes), glyphAlignment, glyphMetrics);
    }
    else
    {
        // If you reach here --> Otb table format not supported (only support bit alligned)
        // Most likely due to an incorrect font file supplied (programmer error)
        ASSERT(false);
    }
}


// class Glyph

void Glyph::Set(const Brx& aPixels, EGlyphAlignment aGlyphAlignment, const GlyphMetrics* aMetrics)
{
    iPixels.Set(aPixels);
    iAlignment = aGlyphAlignment;
    iMetrics = aMetrics;

    ASSERT(iMetrics->iWidth > 0); // insure glyph has valid dimensions
    ASSERT(iMetrics->iHeight > 0); // insure glyph has valid dimensions
    ASSERT(iPixels.Bytes() > 0); // insure glyph has bitmap data
}


// class GlyphRange

Linn::Ui::GlyphRange::GlyphRange(TUnicode aUnicodeFirst, TUnicode aUnicodeLast, TUint aPointSize, const Otb& aOtb)
    : iUnicodeFirst(aUnicodeFirst), iUnicodeLast(aUnicodeLast), iPointSize(aPointSize), iOtb(aOtb)
{
    ASSERT(aUnicodeFirst <= aUnicodeLast); // check unicode range is valid

    iGlyph.clear();

    for (TUint i = iUnicodeFirst; i <= iUnicodeLast; i++)
    {
        Glyph* glyph = new Glyph;
        iGlyph.push_back(glyph); // append new glyph to collection (vector)
    }
}


GlyphRange::~GlyphRange()
{
    // delete glyph collection (vector)
    // iterate over the vector iGlyph. Each i is a Glyph* - stepping in forward order
    for(Vector::const_iterator i = iGlyph.begin(); i != iGlyph.end(); i++) { delete *i; }
}


TBool GlyphRange::IsInRange(TUnicode aUnicode) const
{
    return ((iUnicodeFirst <= aUnicode) && (aUnicode <= iUnicodeLast));
}


const Glyph& GlyphRange::GlyphInRange(TUnicode aUnicode) const
{
    ASSERT(IsInRange(aUnicode));
    Glyph* glyph = iGlyph[aUnicode - iUnicodeFirst];
    if (!glyph->IsInitialised())
    {
        iOtb.InitialiseGlyph(*glyph, aUnicode, iPointSize);
    }
    return *glyph;
}


// class GlyphTable

GlyphTable::GlyphTable()
{
    iGlyphRange.clear();
}


GlyphTable::~GlyphTable()
{
    // delete glyph range collection (vector)
    // iterate over the vector iGlyphRange. Each i is a GlyphRange* - stepping in forward order
    for(Vector::const_iterator i = iGlyphRange.begin(); i != iGlyphRange.end(); i++) { delete *i; }
}


void GlyphTable::AddRange(const GlyphRange* aGlyphRange)
{
    iGlyphRange.push_back(aGlyphRange); // append new glyph range to collection (vector)
}


TBool GlyphTable::IsInTable(TUnicode aUnicode) const
{
    // iterate over the vector iGlyphRange. Each i is a GlyphRange* - stepping in forward order
    for(Vector::const_iterator i = iGlyphRange.begin(); i != iGlyphRange.end(); i++)
    {
        if((*i)->IsInRange(aUnicode)) { return(true); }
    }
    return(false);
}


const Glyph& GlyphTable::GlyphInTable(TUnicode aUnicode) const
{
    // iterate over the vector iGlyphRange. Each i is a GlyphRange* - stepping in forward order
    for(Vector::const_iterator i = iGlyphRange.begin(); i != iGlyphRange.end(); i++)
    {
        if((*i)->IsInRange(aUnicode))
        {
            return((*i)->GlyphInRange(aUnicode));
        }
    }
    // If you reach here --> unicode value not in glyph table
    // Every glyph should be checked IsInTable before this is called (programmer error)
    ASSERT(false);
}


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
        LOG(kUi,"Missing Unicode Value in Glyph Table: %lx\n",aUnicode);
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

    LOG(Debug::kUi|Debug::kVerbose,"Unicode Bounds: %d(x), %d(y), %d(w), %d(h)\n",x,y,width,height);

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

    LOG(Debug::kUi|Debug::kVerbose,"Unicode Orgnal String: ");
    LOG(Debug::kUi|Debug::kVerbose,aString);
    LOG(Debug::kUi|Debug::kVerbose,"\n");
    LOG(Debug::kUi|Debug::kVerbose,"Unicode Fitted String: ");
    LOG(Debug::kUi|Debug::kVerbose,iFittedString);
    LOG(Debug::kUi|Debug::kVerbose,"\n");

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

    LOG(Debug::kUi|Debug::kVerbose,"Unicode Orgnal String: ");
    LOG(Debug::kUi|Debug::kVerbose,aString);
    LOG(Debug::kUi|Debug::kVerbose,"\n");
    LOG(Debug::kUi|Debug::kVerbose,"Unicode Fitted String: ");
    LOG(Debug::kUi|Debug::kVerbose,iFittedString);
    LOG(Debug::kUi|Debug::kVerbose,"\n");

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
