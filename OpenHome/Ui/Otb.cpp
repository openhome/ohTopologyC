#include <OpenHome/Ui/Otb.h>
#include <OpenHome/Ui/Glyph.h>
#include <OpenHome/Private/Converter.h>

using namespace OpenHome;
using namespace OpenHome::Ui;
//using namespace Linn::Arch;

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

const Brn Otb::OtbTable(const Brx& aFile, const Brx& aName)
{
    #define OTB_OFFSET_TABLE_SIZE 12
    #define TABLE_DIRECTORY_OFFSET_INDEX 8
    #define TABLE_DIRECTORY_LENGTH_INDEX 12
    TUint count = Converter::BeUint16At(aFile, 4);                                  //Get the number of tables
    for(TUint i = 0; i < count; i++)                                    //Search through the table directory
    {
        if (aFile.Split((i << 4) + OTB_OFFSET_TABLE_SIZE, 4) == aName)  //Compare the current table name to our name
        {
            //Get the cmap header address
            const TByte* ptr = aFile.Ptr() + Converter::BeUint32At(aFile, (i << 4) + OTB_OFFSET_TABLE_SIZE + TABLE_DIRECTORY_OFFSET_INDEX);
            //Get the cmap header length
            TUint length = Converter::BeUint32At(aFile, (i << 4) + OTB_OFFSET_TABLE_SIZE + TABLE_DIRECTORY_LENGTH_INDEX);
            return (Brn(ptr, length));  //return a buffer containing the cmap header
        }
    }
    // If you reach here --> Otb table not found
    // Most likely due to an incorrect name supplied (programmer error)
    ASSERTS();
    return(Brx::Empty());
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

const Brn Otb::OtbCmapSubTable(const Brx& aCmapTable, EEncodingId aId)
{
    #define CMAP_ENCODING_RECORD_OFFSET 4

    TUint count = Converter::BeUint16At(aCmapTable, 2); // Number of sub tables

    for(TUint i = 0; i < count; i++)
    {
        if (Converter::BeUint16At(aCmapTable, (i << 3) + 2 + CMAP_ENCODING_RECORD_OFFSET) == aId)
        {
            TUint offset = Converter::BeUint32At(aCmapTable, (i << 3) + 4 + CMAP_ENCODING_RECORD_OFFSET);
            TUint length = Converter::BeUint16At(aCmapTable, (offset + 2)); // look at length of sub table (1st 2 bytes are format)
            return (Brn(aCmapTable.Ptr() + offset, length)); //return cmap subtable
        }
    }
    // If you reach here --> Otb cmap sub table not found
    // Most likely due to an incorrect encoding ID supplied (programmer error)
    ASSERTS();
    return(Brx::Empty());
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

TUint16 Otb::OtbCmapGlyphIndex(const Brn& aCmapSubTable, TUnicode aUnicode)
{
    TUint16 glyphIndex = 0;
    TBool   glyphIndexFound = false;
    if (Converter::BeUint16At(aCmapSubTable ,0) == 4) //get the format of the table
    {
        #define RESERVED_PAD_SIZE 2
        TUint segmentSize =  Converter::BeUint16At(aCmapSubTable ,6); //number of segments in bytes
        Brn endCount(aCmapSubTable.Ptr() + 14, segmentSize);
        Brn startCount(endCount.Ptr() + segmentSize + RESERVED_PAD_SIZE, segmentSize);
        Brn idDelta(startCount.Ptr() + segmentSize, segmentSize);
        Brn idRangeOffset(idDelta.Ptr() + segmentSize, segmentSize);

        for(TUint i=0; i<segmentSize; i+=2)
        {
            if( Converter::BeUint16At(endCount ,i) >= aUnicode)
            {
                if( Converter::BeUint16At(startCount ,i) <= aUnicode)
                {
                    if( Converter::BeUint16At(idRangeOffset ,i) == 0)
                    {
                        glyphIndexFound = true;
                        glyphIndex = aUnicode +  Converter::BeUint16At(idDelta ,i);
                    }
                    else
                    {
                        Brn glyphIdArray(idRangeOffset.Ptr() + segmentSize, (aCmapSubTable.Bytes() - (16 + (4*segmentSize))));
                        TUint16 offset = (( Converter::BeUint16At(idRangeOffset ,i)>>1) + (aUnicode -  Converter::BeUint16At(startCount ,i))) * sizeof(TUint16);
                        glyphIndex = Converter::BeUint16At(glyphIdArray, (i+offset) - segmentSize);
                        if (glyphIndex != 0) // 0 implies missing glyph
                        {
                            glyphIndexFound = true;
                            glyphIndex +=  Converter::BeUint16At(idDelta ,i);
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

const Brn Otb::EblcBitmapTable(const Brx& aEblc, TUint aPointSize)
{
    #define EBLC_HEADER_SIZE  8
    #define BITMAP_TABLE_SIZE 48
    #define PPEMY_INDEX 45

    TUint noOfBitmapSizeTables = Converter::BeUint32At(aEblc, 4);

    for(TUint i=0; i<noOfBitmapSizeTables; i++)//find the table with the strike of the right size
    {
        if(aEblc.At(EBLC_HEADER_SIZE + (i * BITMAP_TABLE_SIZE) + PPEMY_INDEX) == (TUint8)aPointSize)
        {
            return(Brn(aEblc.Ptr() + EBLC_HEADER_SIZE + (i * BITMAP_TABLE_SIZE), BITMAP_TABLE_SIZE));
        }
    }
    // If you reach here --> requested font point size not availabl ein given otb file
    // Most likely due to an incorrect point size supplied (programmer error)
    ASSERTS();
    return(Brx::Empty());
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

const Brn Otb::EblcSubTableArray(const Brx& aEblc, TUint aPointSize, TUint aGlyphIndex)
{
    Brn eblcBitmapTable = EblcBitmapTable(aEblc, aPointSize);

    TUint indexSubTableArrayOffset =  Converter::BeUint32At(eblcBitmapTable ,0);
    TUint sizeOfSubTableData = Converter::BeUint32At(eblcBitmapTable ,4);
    TUint noOfIndexSubTables =  Converter::BeUint32At(eblcBitmapTable ,8);  ///BigEndian4(eblcBitmapSizeTable->noOfIndexSubTables);

    //Create buffer of array data
    Brn eblcSubTableArray(aEblc.Ptr() + indexSubTableArrayOffset, sizeOfSubTableData);

    #define EBLC_SUBTABLE_ARRAY_SIZE 8
    //Find the array element of the right size
    for(TUint i=0; i<noOfIndexSubTables; i++)
    {
        if(  ( Converter::BeUint16At(eblcSubTableArray ,i*EBLC_SUBTABLE_ARRAY_SIZE) <= aGlyphIndex)
           &&( Converter::BeUint16At(eblcSubTableArray ,(i*EBLC_SUBTABLE_ARRAY_SIZE) + 2) >= aGlyphIndex))
        {
            return(Brn(eblcSubTableArray.Ptr() + (i*EBLC_SUBTABLE_ARRAY_SIZE), EBLC_SUBTABLE_ARRAY_SIZE));
        }
    }
    // If you reach here --> Otb eblc sub table array element not found
    // Most likely due to an incorrect glyph index supplied (programmer error)
    ASSERTS();
    return(Brx::Empty());
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
    Brn subTableNoData(eblc.Ptr() + Converter::BeUint32At(eblcBitmapTable ,0) + Converter::BeUint32At(eblcSubTableArray ,4), EBLC_SUBTABLE_SIZE_WITHOUT_DATA);

    #define INDEX_SUBHEADER_SIZE 8
    #define SMALL_METRICS_SIZE 5

    //We deduct the index value of the first supported glyphcode in the range from our glyph index to get a relative index
    //then use the index to with the offset array  (glyphDataOffset = offsetArray[index])

    TUint offsetArraySize = (Converter::BeUint16At(eblcSubTableArray ,2) - Converter::BeUint16At(eblcSubTableArray ,0) + 2) * sizeof (TUint32);
    Brn offsetArray(subTableNoData.Ptr() + INDEX_SUBHEADER_SIZE, offsetArraySize);

    TUint16 indexFormat =  Converter::BeUint16At(subTableNoData ,0);
    TUint16 imageFormat =  Converter::BeUint16At(subTableNoData ,2);

    if ( (indexFormat == 1) && (imageFormat == 2) ) // index = 1: variable glyph metrics for each glyph stored in EBDT
    {
        TUint32 glyphDataOffset = Converter::BeUint32At(offsetArray ,(glyphIndex -  Converter::BeUint16At(eblcSubTableArray ,0)) * sizeof (TUint32));
        glyphMetrics = (GlyphMetrics*) (ebdt.Ptr() + Converter::BeUint32At(subTableNoData ,4) + glyphDataOffset);
        pixelsLocation = (TByte*) &ebdt.At(Converter::BeUint32At(subTableNoData ,4) + glyphDataOffset + SMALL_METRICS_SIZE);
        pixelsBits = glyphMetrics->iHeight * glyphMetrics->iWidth;
        pixelsBytes = ((pixelsBits)>>3) + (((pixelsBits&0x07)==0) ? 0:1);
        //LOG(kUi,"Unicode 0x%x, EBDT location: 0x%x\n",aUnicode, Converter::BeUint32At(subTableNoData ,4) + glyphDataOffset + SMALL_METRICS_SIZE + (ebdt.Ptr() - iFile.Ptr()));
        glyphAlignment = eBitAligned; // image format 2: small metrics, bit alligned image data (EBDT)
        aGlyph.Set(Brn(pixelsLocation,pixelsBytes), glyphAlignment, glyphMetrics);
    }
    else if ( (indexFormat == 2) && (imageFormat == 5) ) // index = 2: identical glyph metrics for each glyph in range
    {
        // get the metrics from the EBLC table directly
        glyphMetrics = (GlyphMetrics*) (subTableNoData.Ptr() + 12);
        // get the image data from EBDT - don't need offset as every glyph is the same size
        TUint32 imageSize = Converter::BeUint32At(subTableNoData ,8);
        pixelsLocation = (TByte*) &ebdt.At(Converter::BeUint32At(subTableNoData ,4) + imageSize * (glyphIndex -  Converter::BeUint16At(eblcSubTableArray ,0)));
        pixelsBits = glyphMetrics->iHeight * glyphMetrics->iWidth;
        pixelsBytes = ((pixelsBits)>>3) + (((pixelsBits&0x07)==0) ? 0:1);
        //LOG(kUi,"Unicode 0x%x, EBDT location: 0x%x\n",aUnicode, Converter::BeUint32At(subTableNoData ,4) + (imageSize * (glyphIndex - Converter::BeUint16At(eblcSubTableArray ,0))) + (ebdt.Ptr() - iFile.Ptr()));
        glyphAlignment = eBitAligned; // image format 5: metrics in EBLC, bit alligned image data only (EBDT)
        aGlyph.Set(Brn(pixelsLocation,pixelsBytes), glyphAlignment, glyphMetrics);
    }
    else
    {
        // If you reach here --> Otb table format not supported (only support bit alligned)
        // Most likely due to an incorrect font file supplied (programmer error)
        ASSERTS();
    }
}


