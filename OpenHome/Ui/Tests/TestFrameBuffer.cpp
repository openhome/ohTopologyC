#include <Linn/TestFramework/TestFramework.h>
#include <Linn/Ui/Drawing.h>
//#include <Linn/Nvms/Store.h>
#include <Linn/Tags.h>
//#include <Linn/Control/Tar.h>
#include <Linn/Ui/AppUi.h>
#include <Linn/Os/Os.h>
#include <Linn/Common/AppCommon.h>

#define STANDARD_DISPLAY

using namespace Linn;
using namespace Linn::Ui;
using namespace Linn::Nvms;
using namespace Linn::TestFramework;
using namespace Linn::Control;
using namespace Linn::Common;

IProduct* gProduct;
AppUi* gUi;


class FrameBufferTestBasic : public Suite
{
public:
    FrameBufferTestBasic(const TChar* aDescription):Suite(aDescription){}
    void Test();
};

void FrameBufferTestBasic::Test()
{
    // Frame Buffer Construction
    FrameBuffer frameBuffer(128, 32, 1);
    TEST(frameBuffer.Width() == 128);
    TEST(frameBuffer.Height() == 32);
    TEST(frameBuffer.BitsPerPixel() == 1);
    TEST(frameBuffer.Pixels().Bytes() == ((128>>3)*32));

    FrameBuffer frameBufferLarge(256, 64, 1);
    TEST(frameBufferLarge.Width() == 256);
    TEST(frameBufferLarge.Height() == 64);
    TEST(frameBufferLarge.BitsPerPixel() == 1);
    TEST(frameBufferLarge.Pixels().Bytes() == ((256>>3)*64));

    FrameBuffer frameBufferSmall(64, 16, 1);
    TEST(frameBufferSmall.Width() == 64);
    TEST(frameBufferSmall.Height() == 16);
    TEST(frameBufferSmall.BitsPerPixel() == 1);
    TEST(frameBufferSmall.Pixels().Bytes() == ((64>>3)*16));

    TEST_THROWS(FrameBuffer(0, 32, 1),AssertionFailed);
    TEST_THROWS(FrameBuffer(127, 32, 1),AssertionFailed);
    TEST_THROWS(FrameBuffer(128, 0, 1),AssertionFailed);
    TEST_THROWS(FrameBuffer(128, 32, 0),AssertionFailed);
    TEST_THROWS(FrameBuffer(128, 32, 2),AssertionFailed);

    // Rectangle Construction
    Rectangle rectangleSmall(0,1,20,10);
    TEST(rectangleSmall.X() == 0);
    TEST(rectangleSmall.Y() == 1);
    TEST(rectangleSmall.Width() == 20);
    TEST(rectangleSmall.Height() == 10);

    Rectangle rectangleLarge(500,250,300,150);
    TEST(rectangleLarge.X() == 500);
    TEST(rectangleLarge.Y() == 250);
    TEST(rectangleLarge.Width() == 300);
    TEST(rectangleLarge.Height() == 150);

    TEST_THROWS(Rectangle(0,0,20,0),AssertionFailed);
    TEST_THROWS(Rectangle(0,0,0,10),AssertionFailed);
}


class FrameBufferTestPixel : public Suite
{
public:
    FrameBufferTestPixel(const TChar* aDescription):Suite(aDescription){}
    void Test();
};

void FrameBufferTestPixel::Test()
{
    gUi->FrameBufferDisplay().Clear();
    FrameBuffer frameBuffer(128, 32, 1);
    Window window(frameBuffer,gUi->FrameBufferDisplay(),0,0, Priority::kHighest);

    window.Close();
    frameBuffer.Clear();

    // set single pixels
    LOG(kUi,"FrameBuffer Set Single Pixles (corners and middle)\n");
    frameBuffer.SetPixel(0,0);
    frameBuffer.SetPixel(127,0);
    frameBuffer.SetPixel(63,15);
    frameBuffer.SetPixel(0,31);
    frameBuffer.SetPixel(127,31);
    window.Open();
    Thread::Current().Sleep(1000);


    window.Close();
    frameBuffer.Fill();

    // clear single pixels
    LOG(kUi,"FrameBuffer Clear Single Pixles (corners and middle)\n");
    frameBuffer.ClearPixel(0,0);
    frameBuffer.ClearPixel(127,0);
    frameBuffer.ClearPixel(63,15);
    frameBuffer.ClearPixel(0,31);
    frameBuffer.ClearPixel(127,31);
    window.Open();
    Thread::Current().Sleep(1000);
    window.Close();

    // horizontal stripes (top row off)
    LOG(kUi,"FrameBuffer Set Single Pixles (horizontal stripes - top row off)\n");
    for (TUint i = 0; i < frameBuffer.Height(); i++)
    {
        for (TUint j = 0; j < frameBuffer.Width(); j++)
        {
            if (i%2 == 0) { frameBuffer.ClearPixel(j,i); }
            else { frameBuffer.SetPixel(j,i); }
        }
    }
    window.Open();
    Thread::Current().Sleep(1000);

    window.Close();
    // vertical stripes (far left row on)
    LOG(kUi,"FrameBuffer Set Single Pixles (vertical stripes - left row on)\n");
    for (TUint i = 0; i < frameBuffer.Height(); i++)
    {
        for (TUint j = 0; j < frameBuffer.Width(); j++)
        {
            if (j%2 == 0) { frameBuffer.SetPixel(j,i); }
            else { frameBuffer.ClearPixel(j,i); }
        }
    }
    window.Open();
    Thread::Current().Sleep(1000);

    TEST_THROWS(frameBuffer.SetPixel(128,0),LimitsExceeded);
    TEST_THROWS(frameBuffer.SetPixel(0,32),LimitsExceeded);
    TEST_THROWS(frameBuffer.ClearPixel(128,0),LimitsExceeded);
    TEST_THROWS(frameBuffer.ClearPixel(0,32),LimitsExceeded);
}


class FrameBufferTestBmp : public Suite
{
public:
    FrameBufferTestBmp(const TChar* aDescription):Suite(aDescription){}
    void Test();
};


void FrameBufferTestBmp::Test()
{
    gUi->FrameBufferDisplay().Clear();
    FrameBuffer frameBuffer(128, 32, 1);
    Window window(frameBuffer,gUi->FrameBufferDisplay(),0,0, Priority::kHighest);

    // cleared
    LOG(kUi,"FrameBuffer Cleared\n");
    window.Close();
    frameBuffer.Clear();
    window.Open();
    Thread::Current().Sleep(1000);

    window.Close();
    // bmp - multiple from tar file
    Tars bmpTar(uTestFrameBufferTar);  // get bitmap from store (single file stored in tar file)
    frameBuffer.Fill((TUint32)0x87654321);

    LOG(kUi,"BMP TestBmp64x16.bmp (tar - 1 per quadrant)\n");
    Bmp testBmp64x16Tar(bmpTar.Find(Brn("TestBmp64x16.bmp")));
    frameBuffer.Write(testBmp64x16Tar, 0, 0);
    frameBuffer.Write(testBmp64x16Tar, 64, 0);
    frameBuffer.Write(testBmp64x16Tar, 0, 16);
    frameBuffer.Write(testBmp64x16Tar, 64, 16);
    window.Open();
    Thread::Current().Sleep(1000);
    window.Close();

    // clear bmp
    LOG(kUi,"BMP cleared from top left and bottom right quadrants\n");
    frameBuffer.Clear(testBmp64x16Tar.Bounds(0, 0));
    frameBuffer.Clear(testBmp64x16Tar.Bounds(64, 16));
    window.Open();
    Thread::Current().Sleep(1000);
    window.Close();
    frameBuffer.Fill((TUint32)0x87654321);

    LOG(kUi,"BMP TestBmp64x16.bmp (tar - 1 per quadrant)\n");
    frameBuffer.Write(testBmp64x16Tar, 0, 0);
    frameBuffer.Write(testBmp64x16Tar, 64, 0);
    frameBuffer.Write(testBmp64x16Tar, 0, 16);
    frameBuffer.Write(testBmp64x16Tar, 64, 16);
    window.Open();
    Thread::Current().Sleep(1000);

    // clear rectangle
    LOG(kUi,"Rectangle cleared from middle of frame buffer (32,8,64,16)\n");
    window.Close();
    frameBuffer.Clear(Rectangle(32,8,64,16));
    window.Open();
    Thread::Current().Sleep(1000);
    window.Close();
    frameBuffer.Clear(Rectangle(0,0,128,32));
    window.Open();
    Thread::Current().Sleep(1000);
    window.Close();
    frameBuffer.Fill((TUint32)0x87654321);

    LOG(kUi,"BMP TestBmpVerticalStripes128x32.bmp (tar)\n");
    Bmp testBmpVsTar(bmpTar.Find(Brn("TestBmpVerticalStripes128x32.bmp")));
    frameBuffer.Write(testBmpVsTar, 0, 0);
    window.Open();
    Thread::Current().Sleep(1000);
    window.Close();
    frameBuffer.Fill((TUint32)0x87654321);

    LOG(kUi,"BMP TestBmpHorizontalStripes128x32.bmp (tar)\n");
    Bmp testBmpHsTar(bmpTar.Find(Brn("TestBmpHorizontalStripes128x32.bmp")));
    frameBuffer.Write(testBmpHsTar, 0, 0);
    window.Open();
    Thread::Current().Sleep(1000);
    window.Close();
    frameBuffer.Clear();

    LOG(kUi,"BMP TestBmp10x10.bmp (tar)\n");
    Bmp testBmp10x10Tar(bmpTar.Find(Brn("TestBmp10x10.bmp")));
    frameBuffer.Write(testBmp10x10Tar, 0, 0);
    frameBuffer.Write(testBmp10x10Tar, 118, 0);
    frameBuffer.Write(testBmp10x10Tar, 0, 22);
    frameBuffer.Write(testBmp10x10Tar, 118, 22);
    window.Open();
    Thread::Current().Sleep(1000);
    window.Close();
    TEST(testBmp10x10Tar.Width() == 10);
    TEST(testBmp10x10Tar.Height() == 10);
    TEST(testBmp10x10Tar.BitsPerPixel() == 1);
    TEST(frameBuffer.Pixels().At(0)   == 0x80);//0
    TEST(frameBuffer.Pixels().At(1)   == 0x40);
    TEST(frameBuffer.Pixels().At(16)  == 0x00);//1
    TEST(frameBuffer.Pixels().At(17)  == 0x00);
    TEST(frameBuffer.Pixels().At(32)  == 0x0C);//2
    TEST(frameBuffer.Pixels().At(33)  == 0x00);
    TEST(frameBuffer.Pixels().At(48)  == 0x0C);//3
    TEST(frameBuffer.Pixels().At(49)  == 0x00);
    TEST(frameBuffer.Pixels().At(64)  == 0x0C);//4
    TEST(frameBuffer.Pixels().At(65)  == 0x00);
    TEST(frameBuffer.Pixels().At(80)  == 0x0C);//5
    TEST(frameBuffer.Pixels().At(81)  == 0x00);
    TEST(frameBuffer.Pixels().At(96)  == 0x0C);//6
    TEST(frameBuffer.Pixels().At(97)  == 0x00);
    TEST(frameBuffer.Pixels().At(112) == 0x0C);//7
    TEST(frameBuffer.Pixels().At(113) == 0x00);
    TEST(frameBuffer.Pixels().At(128) == 0x00);//8
    TEST(frameBuffer.Pixels().At(129) == 0x00);
    TEST(frameBuffer.Pixels().At(144) == 0x80);//9
    TEST(frameBuffer.Pixels().At(145) == 0x40);
    frameBuffer.Fill((TUint32)0x87654321);

    TEST_THROWS(frameBuffer.Write(testBmp10x10Tar, 128, 0),LimitsExceeded);
    TEST_THROWS(frameBuffer.Write(testBmp10x10Tar, 0, 32),LimitsExceeded);
    TEST_THROWS(frameBuffer.Write(testBmp10x10Tar, 119, 0),LimitsExceeded);
    TEST_THROWS(frameBuffer.Write(testBmp10x10Tar, 0, 23),LimitsExceeded);

    TEST_THROWS(frameBuffer.Clear(testBmp10x10Tar.Bounds(128, 0)),LimitsExceeded);
    TEST_THROWS(frameBuffer.Clear(testBmp10x10Tar.Bounds(0, 32)),LimitsExceeded);
    TEST_THROWS(frameBuffer.Clear(testBmp10x10Tar.Bounds(119, 0)),LimitsExceeded);
    TEST_THROWS(frameBuffer.Clear(testBmp10x10Tar.Bounds(0, 23)),LimitsExceeded);

    LOG(kUi,"TestBmpLetterE7x7.bmp - Letter 'e'\n");
    Bmp testBmp7x7Tar(bmpTar.Find(Brn("TestBmpLetterE7x7.bmp")));
    frameBuffer.Write(testBmp7x7Tar, 0, 16);
    window.Open();
    Thread::Current().Sleep(1000);
    window.Close();
    frameBuffer.Fill((TUint32)0x87654321);

    LOG(kUi,"BMP TestBmpCourierText128x32.bmp (tar)\n");
    Bmp testBmpTextTar(bmpTar.Find(Brn("TestBmpCourierText128x32.bmp")));
    frameBuffer.Write(testBmpTextTar, 0, 0);
    window.Open();
    Thread::Current().Sleep(1000);
}


class OtbInternalTest : public Suite
{
public:
    OtbInternalTest(const TChar* aDescription):Suite(aDescription){}
    void Test();
};

void OtbInternalTest::Test()
{
    Tars fontTar(uTestFrameBufferTar);  // get font file (otb) from store (single file stored in tar file)
    Brn fontFile(fontTar.Find(Brn("CourierP16TestOnly.otb")));

    //Test OffsetTable
    TEST(fontFile.BeUint32At(0)     == 0x00010000); //      Version         4 bytes     0x00010000 for version 1.0
    TEST(fontFile.BeUint16At(4)     == 0x0010);     //      NumberOfTables  2 bytes     number of tables
    TEST(fontFile.BeUint16At(6)     == 0x0100);     //      SearchRange     2 bytes     (Maximum power of 2 <= numberOfTables) x 16
    TEST(fontFile.BeUint16At(8)     == 0x0004);     //      EntrySelector   2 bytes     Log2(Maximum power of 2 <= numberOfTables)
    TEST(fontFile.BeUint16At(10)    == 0x0000);     //      RangeShift      2 bytes     numberOfTables x (16 - searchRange)

    const Brn kOtbTableCmap("cmap");
    const Brn kOtbTableEblc("EBLC");
    const Brn kOtbTableEbdt("EBDT");
    const TUint kPointSize16 = 16;

    //First we need to find the index for the unicode value into the glyph data.
    //We get this from the character to glyph mapping table (cmap)

    //cmap offset table at offset 0x7c from start of file
    //Find the cmap offset table, return the offset (0x3b0) and length (0x1ca)
    const Brn cmap = OtbTable(fontFile, kOtbTableCmap);
    TEST(TUint32(cmap.Ptr()) == (TUint32(fontFile.Ptr()) + 0x3b0));

    //cmap subtable at 0x3c4 - Plaform id = 0x0003 Encoding id = 0x0003 0ffset = 0x0000001c
    const Brn cmapSubTable = OtbCmapSubTable(cmap, eUnicode);
    TEST(TUint32(cmapSubTable.Ptr()) == (TUint32(fontFile.Ptr()) + 0x3cc));

    //Encoding subtable tests (Encoding table at 0x3cc)
    TEST(cmapSubTable.BeUint16At(0) == 0x0004);    //encoding table format
    TEST(cmapSubTable.BeUint16At(2) == 0x00a8);    //length
    TEST(cmapSubTable.BeUint16At(6) == 0x0026);    //segCount << 1 (segmentCount = 19)

    {
    //endCount begins 0x007f, 0x00ff..
    //startCount beings 0x0020, 0x00a0..
    //looking for unicode 'e' = 0x65 lies between 0x20 and 0x7f which is index 0 into the array
    //the range offset is 0 which means the glyph index formula is simple, add glyph code to delta and take modulo 65536
    //of the result -> 0x65 + 0xFFE3 = 0x10048. 0x10048 % 0x10000 = 0x48.
    TUint glyphIndex = OtbCmapGlyphIndex(cmapSubTable, TUint('e'));
    TEST(glyphIndex == 0x48);

    //We now need to find the position in the file of the font data of the desired font size.
    //This information is stored in the Embedded Bitmap Location Table (EBLC)

    const Brn eblc = OtbTable(fontFile, kOtbTableEblc);
    TEST(TUint32(eblc.Ptr()) == (TUint32(fontFile.Ptr()) + 0xba0));
    const Brn eblcBitmapTable = EblcBitmapTable(eblc, kPointSize16);
    TEST(TUint32(eblcBitmapTable.Ptr()) == (TUint32(fontFile.Ptr()) + 0xba8));
    const Brn eblcSubTableArray = EblcSubTableArray(eblc, kPointSize16, glyphIndex);
    TEST(TUint32(eblcSubTableArray.Ptr()) == (TUint32(fontFile.Ptr()) + 0xc00));

    //We now need to find the data in the Embedded Bitmap Data Table (EBDT)

    const Brn ebdt = OtbTable(fontFile, kOtbTableEbdt);
    TEST(TUint32(ebdt.Ptr()) == (TUint32(fontFile.Ptr()) + 0xe4c));
    #define EBLC_SUBTABLE_SIZE_WITHOUT_DATA 16
    Brn subTableNoData(eblc.Ptr() + eblcBitmapTable.BeUint32At(0) + eblcSubTableArray.BeUint32At(4), EBLC_SUBTABLE_SIZE_WITHOUT_DATA);
    TEST(TUint32(subTableNoData.Ptr()) == (TUint32(eblc.Ptr()) + 0x38 + 0xd0));//0xca8 from the start of the file

    //The purpose of all the preceeding code and test cases is to extract from the font the following three values
    const GlyphMetrics* glyphMetrics = 0;
    EGlyphAlignment glyphAlignment = eInvalid;
    const TByte* pixels = 0;

    TEST(subTableNoData.BeUint16At(0) == 1); //index format (1: variable glyph metrics for each glyph stored in EBDT)
    #define INDEX_SUBHEADER_SIZE 8
    //We deduct the index value of the first supported glyphcode in the range from our glyph index to get a relative index
    //then use the index to with the offset array  (glyphDataOffset = offsetArray[index])
    TUint offsetArraySize = (eblcSubTableArray.BeUint16At(2) - eblcSubTableArray.BeUint16At(0) + 1);
    Brn offsetArray(subTableNoData.Ptr() + INDEX_SUBHEADER_SIZE, offsetArraySize);
    TUint32 glyphDataOffset = offsetArray.BeUint32At((glyphIndex - eblcSubTableArray.BeUint16At(0)) * sizeof (TUint32));
    TEST(glyphDataOffset == 0x6b);
    #define SMALL_METRICS_SIZE 5
    //The EBDT table has a 4 byte header.  0x339 + 0x12 + 0x5
    pixels = (TByte*) &ebdt.At(subTableNoData.BeUint32At(4) + glyphDataOffset + SMALL_METRICS_SIZE);
    TEST(TUint32(pixels) == (TUint32(fontFile.Ptr()) + 0x11F5));

    glyphMetrics = (GlyphMetrics*) (ebdt.Ptr() + subTableNoData.BeUint32At(4) + glyphDataOffset);
    TEST(glyphMetrics->iHeight      == 7);
    TEST(glyphMetrics->iWidth       == 7);
    TEST(glyphMetrics->iBearingX    == 1);
    TEST(glyphMetrics->iBearingY    == 6);
    TEST(glyphMetrics->iAdvance     == 9);

    //Ensure the alignment of data is as expected
    TEST(subTableNoData.BeUint16At(2) == 2); // image format 2: small metrics, bit alligned image data (EBDT)
    glyphAlignment = eBitAligned;
    }

    {
    //Repeat for C -> 0x43 + 0xFFE3 = 0x10026. 0x10026 % 0x10000 = 0x26.
    TUint glyphIndex = OtbCmapGlyphIndex(cmapSubTable, TUint('C'));
    TEST(glyphIndex == 0x26);

    //We now need to find the position in the file of the font data of the desired font size.  This information is stored
    //in the Embedded Bitmap Location Table (EBLC)
    const Brn eblc = OtbTable(fontFile, kOtbTableEblc);
    TEST(TUint32(eblc.Ptr()) == (TUint32(fontFile.Ptr()) + 0xba0));
    const Brn eblcBitmapTable = EblcBitmapTable(eblc, kPointSize16);
    TEST(TUint32(eblcBitmapTable.Ptr()) == (TUint32(fontFile.Ptr()) + 0xba8));
    const Brn eblcSubTableArray = EblcSubTableArray(eblc, kPointSize16, glyphIndex);
    TEST(TUint32(eblcSubTableArray.Ptr()) == (TUint32(fontFile.Ptr()) + 0xbf8));

    //We now need to find the data in the Embedded Bitmap Data Table (EBDT)
    const Brn ebdt = OtbTable(fontFile, kOtbTableEbdt);
    TEST(TUint32(ebdt.Ptr()) == (TUint32(fontFile.Ptr()) + 0xe4c));
    #define EBLC_SUBTABLE_SIZE_WITHOUT_DATA 16
    Brn subTableNoData(eblc.Ptr() + eblcBitmapTable.BeUint32At(0) + eblcSubTableArray.BeUint32At(4), EBLC_SUBTABLE_SIZE_WITHOUT_DATA);
    TEST(TUint32(subTableNoData.Ptr()) == (TUint32(eblc.Ptr()) + 0x38 + 0xbc));//0xc94 from the start of the file

    //The purpose of all the preceeding code and test cases is to extract from the font the following three values
    const GlyphMetrics* glyphMetrics = 0;
    EGlyphAlignment glyphAlignment = eInvalid;
    const TByte* pixels = 0;

    TEST(subTableNoData.BeUint16At(0) == 2); //index format (2: identical glyph metrics for each glyph in range)
    #define INDEX_SUBHEADER_SIZE 8
    #define BIG_METRICS_SIZE 8
    #define IMAGE_SIZE 4
    Brn indexFormat2Data(subTableNoData.Ptr() + INDEX_SUBHEADER_SIZE, IMAGE_SIZE + BIG_METRICS_SIZE);
    TUint32 imageSize = indexFormat2Data.BeUint32At(0);
    TEST(imageSize == 0xe);
    glyphMetrics = (GlyphMetrics*) (indexFormat2Data.Ptr() + IMAGE_SIZE);

    //We deduct the index value of the first supported glyphcode in the range from our glyph index to get a relative index
    //then use the index to with the offset array  (glyphDataOffset = offsetArray[index])
    //(0x26 - 0x0d) * e = 0x15e
    TUint32 glyphDataOffset = ((glyphIndex - eblcSubTableArray.BeUint16At(0)) * imageSize);
    TEST(glyphDataOffset == 0x15e);

    //The EBDT table has a 4 byte header. 0x8b +  0x15e = 0x1e9
    pixels = (TByte*) &ebdt.At(subTableNoData.BeUint32At(4) + glyphDataOffset);
    TEST(TUint32(pixels) == (TUint32(fontFile.Ptr()) + 0x1035));

    glyphMetrics = (GlyphMetrics*) &indexFormat2Data.At(4);
    TEST(glyphMetrics->iHeight      == 12);
    TEST(glyphMetrics->iWidth       == 9);
    TEST(glyphMetrics->iBearingX    == 0);
    TEST(glyphMetrics->iBearingY    == 8);
    TEST(glyphMetrics->iAdvance     == 9);

    //Ensure the alignment of data is as expected
    TEST(subTableNoData.BeUint16At(2) == 5);  //image format 5: metrics in EBLC, bit alligned image data only (EBDT)
    glyphAlignment = eBitAligned;
    }

    //Now prove tha we can initialise a glyph and extract the correct data

    Otb otbCourier16(fontFile);
    {
    //Test the initalisation of a glyph
    Glyph glyph_e;
    otbCourier16.InitialiseGlyph(glyph_e, 'e', kPointSize16);
    TEST(glyph_e.Height()       == 7);
    TEST(glyph_e.Width()        == 7);
    TEST(glyph_e.BearingX()     == 1);
    TEST(glyph_e.BearingY()     == 6);
    TEST(glyph_e.Advance()      == 9);
    TEST(glyph_e.Alignment()    == eBitAligned);

    Glyph glyph_C;
    otbCourier16.InitialiseGlyph(glyph_C, 'C', kPointSize16);
    TEST(glyph_C.Height()       == 12);
    TEST(glyph_C.Width()        == 9);
    TEST(glyph_C.BearingX()     == 0);
    TEST(glyph_C.BearingY()     == 8);
    TEST(glyph_C.Advance()      == 9);
    TEST(glyph_C.Alignment()    == eBitAligned);
    }

    //Now test the initalisation of a glyph range (standard ascii - no control characters)
    TUint unicodeFirst = 0x20;
    TUint unicodeLast = 0x7e;
    GlyphRange glyphRange(unicodeFirst, unicodeLast, kPointSize16, otbCourier16);
    for (TUint i = unicodeFirst; i <= unicodeLast; i++)
    {
        TEST(glyphRange.IsInRange(i));
    }
    const Glyph& glyph_e = glyphRange.GlyphInRange((TUint)'e');
    TEST(glyph_e.Height()      == 7);
    TEST(glyph_e.Width()       == 7);
    TEST(glyph_e.BearingX()    == 1);
    TEST(glyph_e.BearingY()    == 6);
    TEST(glyph_e.Advance()     == 9);
    TEST(glyph_e.Alignment()   == eBitAligned);

    TUint unicodeRangeOneStart = (TUint)'a';
    TUint unicodeRangeOneStop  = (TUint)'e';
    TUint unicodeRangeTwoStart = (TUint)'A';
    TUint unicodeRangeTwoStop  = (TUint)'E';
    GlyphRange* glyphRangeOne = new GlyphRange(unicodeRangeOneStart, unicodeRangeOneStop, kPointSize16, otbCourier16);
    GlyphRange* glyphRangeTwo = new GlyphRange(unicodeRangeTwoStart, unicodeRangeTwoStop, kPointSize16, otbCourier16);

    GlyphTable glyphTable;
    TEST(glyphTable.IsInTable(unicodeRangeOneStart) == false);
    TEST(glyphTable.IsInTable(unicodeRangeOneStop) == false);
    TEST(glyphTable.IsInTable(unicodeRangeTwoStart) == false);
    TEST(glyphTable.IsInTable(unicodeRangeTwoStop) == false);
    glyphTable.AddRange(glyphRangeOne);
    TEST(glyphTable.IsInTable(unicodeRangeOneStart) == true);
    TEST(glyphTable.IsInTable(unicodeRangeOneStop) == true);
    TEST(glyphTable.IsInTable(unicodeRangeTwoStart) == false);
    TEST(glyphTable.IsInTable(unicodeRangeTwoStop) == false);
    glyphTable.AddRange(glyphRangeTwo);
    TEST(glyphTable.IsInTable(unicodeRangeOneStart) == true);
    TEST(glyphTable.IsInTable(unicodeRangeOneStop) == true);
    TEST(glyphTable.IsInTable(unicodeRangeTwoStart) == true);
    TEST(glyphTable.IsInTable(unicodeRangeTwoStop) == true);

    const Glyph& glyph_e2 = glyphTable.GlyphInTable((TUint)'e');
    TEST(glyph_e2.Alignment() == eBitAligned);
    TEST(glyph_e2.Height()      == 7);
    TEST(glyph_e2.Width()       == 7);
    TEST(glyph_e2.BearingX()    == 1);
    TEST(glyph_e2.BearingY()    == 6);
    TEST(glyph_e2.Advance()     == 9);
}


class FrameBufferTestOtb : public Suite
{
public:
    FrameBufferTestOtb(const TChar* aDescription):Suite(aDescription){}
    void Test();
};

void FrameBufferTestOtb::Test()
{
    gUi->FrameBufferDisplay().Clear();
    FrameBuffer frameBuffer(128, 32, 1);
    Window window(frameBuffer,gUi->FrameBufferDisplay(),0,0, Priority::kHighest);

    Tars fontTar(uTestFrameBufferTar); // get font file (otb) from store (single file stored in tar file)
    Brn otbFile(fontTar.Find(Brn("NcsbP17.otb")));
    TUint kPointSize = 17;
    Otb otbFontFile(otbFile);

    // construct strike then with standard unicode range (0x00 to 0xff - no control characters)
    Strike strike1(kPointSize);
    strike1.AddRange(otbFontFile, Strike::eLatin1);

    // letter e at 0,16
    window.Close();
    frameBuffer.Clear();
    frameBuffer.Write(strike1, Brn("e"), 0, 16);
    window.Open();
    Thread::Current().Sleep(1000);
    window.Close();
    frameBuffer.Clear(strike1.Bounds(Brn("e"), 0, 16));

    frameBuffer.Fill();
    frameBuffer.Write(strike1, Brn("e"), 0, 16);
    window.Open();
    Thread::Current().Sleep(1000);
    window.Close();
    frameBuffer.Clear(strike1.Bounds(Brn("e"), 0, 16));
    frameBuffer.Clear();

    // characters in each corner on buffer limits - no test needed - will throw an incorrect excpetion
    // @ has bearing x of -1
    frameBuffer.Write(strike1, Brn("@"), 1, 11);
    frameBuffer.Write(strike1, Brn("@"), 116, 11);
    frameBuffer.Write(strike1, Brn("@"), 1, 30);
    frameBuffer.Write(strike1, Brn("@"), 116, 30);
    frameBuffer.Write(strike1, Brn("@@@@@@@@"), 14, 19);
    window.Open();
    Thread::Current().Sleep(1000);
    window.Close();
    frameBuffer.Clear(strike1.Bounds(Brn("@"), 1, 11));
    frameBuffer.Clear(strike1.Bounds(Brn("@"), 116, 11));
    frameBuffer.Clear(strike1.Bounds(Brn("@"), 1, 30));
    frameBuffer.Clear(strike1.Bounds(Brn("@"), 116, 30));
    frameBuffer.Clear(strike1.Bounds(Brn("@@@@@@@@"), 14, 19));
    window.Open();
    Thread::Current().Sleep(1000);

    // exceeds left buffer limit (by 1 pixel): NCSB P17 '@' symbol has bearingX of -1
    TEST_THROWS(frameBuffer.Write(strike1, Brn("@"), 0, 16),LimitsExceeded);
    TEST_THROWS(frameBuffer.Clear(strike1.Bounds(Brn("@"), 0, 16)),LimitsExceeded);
    // exceeds right buffer limit (by 1 pixel): NCSB P17 '@' symbol has advance of 12
    TEST_THROWS(frameBuffer.Write(strike1, Brn("@"), 117, 16),LimitsExceeded);
    TEST_THROWS(frameBuffer.Clear(strike1.Bounds(Brn("@"), 117, 16)),LimitsExceeded);
    // exceeds top buffer limit (by 1 pixel): NCSB P17 '@' symbol has bearingY of 12
    TEST_THROWS(frameBuffer.Write(strike1, Brn("@"), 64, 10),LimitsExceeded);
    TEST_THROWS(frameBuffer.Clear(strike1.Bounds(Brn("@"), 64, 10)),LimitsExceeded);
    // exceeds bottom buffer limit (by 1 pixel): NCSB P17 '@' symbol has drop of 1
    TEST_THROWS(frameBuffer.Write(strike1, Brn("@"), 64, 31),LimitsExceeded);
    TEST_THROWS(frameBuffer.Clear(strike1.Bounds(Brn("@"), 64, 31)),LimitsExceeded);

    // construct blank strike then add ranges (A-N, then O-Z)
    Strike strike2(kPointSize);
    strike2.AddRange(otbFontFile, 0x41,0x4E);

    TEST_THROWS(frameBuffer.Write(strike2, Brn("ANOZ"), 0, 16),UnknownCharacter);
    TEST_THROWS(frameBuffer.Clear(strike2.Bounds(Brn("ANOZ"), 0, 16)),UnknownCharacter);

    strike2.AddRange(otbFontFile, 0x4F,0x5A);

    LOG(kUi,"Strike Construction: range A-N and O-Z 'ANOZ'");
    window.Close();
    frameBuffer.Write(strike2, Brn("ANOZ"), 0, 16);
    window.Open();
    Thread::Current().Sleep(1000);
    window.Close();
    frameBuffer.Clear(strike2.Bounds(Brn("ANOZ"), 0, 16));
    window.Open();

    // construct strike with range (lowercase: a-z)
    Strike* strike3 = new Strike(kPointSize);
    strike3->AddRange(otbFontFile, Strike::eLowerCaseLetters);

    TEST_THROWS(frameBuffer.Write(*strike3, Brn("anoZ"), 0, 16),UnknownCharacter);
    TEST_THROWS(frameBuffer.Clear(strike3->Bounds(Brn("Anoz"), 0, 16)),UnknownCharacter);

    LOG(kUi,"Strike Construction: lowercase range (a-z) 'anoz'");
    window.Close();
    frameBuffer.Write(*strike3, Brn("anoz"), 0, 16);
    window.Open();
    Thread::Current().Sleep(1000);
    window.Close();;
    frameBuffer.Clear(strike3->Bounds(Brn("anoz"), 0, 16));

    delete strike3;

    // Test with various font type and point sizes
    Brn otbFileNcsbP17(fontTar.Find(Brn("NcsbP17.otb")));
    kPointSize = 17;
    Otb otbFontFileNcsbP17(otbFileNcsbP17);
    Strike strikeNcsbP17(kPointSize);
    strikeNcsbP17.AddRange(otbFontFileNcsbP17, Strike::eLatin1);
    frameBuffer.Write(strikeNcsbP17, Brn("AaBb17"), 0, 14);

    Brn otbFileLucidaP12(fontTar.Find(Brn("LucidaP12.otb")));
    kPointSize = 12;
    Otb otbFontFileLucidaP12(otbFileLucidaP12);
    Strike strikeLucidaP12(kPointSize);
    strikeLucidaP12.AddRange(otbFontFileLucidaP12, Strike::eLatin1);
    frameBuffer.Write(strikeLucidaP12, Brn("AaBb12"), 0, 27);

    Brn otbFileTimesP08(fontTar.Find(Brn("TimesP08.otb")));
    kPointSize = 8;
    Otb otbFontFileTimesP08(otbFileTimesP08);
    Strike strikeTimesP08(kPointSize);
    strikeTimesP08.AddRange(otbFontFileTimesP08, Strike::eLatin1);
    frameBuffer.Write(strikeTimesP08, Brn("AaBb08"), 64, 27);

    Brn otbFileHelveticaP14(fontTar.Find(Brn("HelveticaP14.otb")));
    kPointSize = 14;
    Otb otbFontFileHelveticaP14(otbFileHelveticaP14);
    Strike strikeHelveticaP14(kPointSize);
    strikeHelveticaP14.AddRange(otbFontFileHelveticaP14, Strike::eLatin1);
    frameBuffer.Write(strikeHelveticaP14, Brn("AaBb14"), 64, 14);

    // frame buffer should contain 4 strikes of different font type and point size
    window.Open();
    Thread::Current().Sleep(1000);
    window.Close();

    frameBuffer.Clear(strikeNcsbP17.Bounds(Brn("AaBb17"), 0, 14));
    frameBuffer.Clear(strikeLucidaP12.Bounds(Brn("AaBb12"), 0, 27));
    frameBuffer.Clear(strikeTimesP08.Bounds(Brn("AaBb08"), 64, 27));
    frameBuffer.Clear(strikeHelveticaP14.Bounds(Brn("AaBb14"), 64, 14));

    // f - has width > advance
    frameBuffer.Write(strikeHelveticaP14, Brn("f"), 0, 16);
    window.Open();
    Thread::Current().Sleep(1000);
    window.Close();
    frameBuffer.Clear(strike1.Bounds(Brn("f"), 0, 16));
    window.Open();
    Thread::Current().Sleep(1000);

    // add range that includes 2 byte characters in unicode (0x100 to 0x1ff)
    LOG(kUi,"Otb test (Helvetica P14) - 2 Byte Unicode Character 'G' with double line (0x%x, %d)\n",0x1e4,0x1e4);
    window.Close();
    strikeHelveticaP14.AddRange(otbFontFileHelveticaP14,0x1e4,0x1e4);
    String unicodeStr(Brn((const TByte*)"\x00\x00\x01\xe4",4), String::eRawUnicode); // 'G' with double line
    Bws<4> utf8;
    unicodeStr.ToUtf8(utf8);
    frameBuffer.Write(strikeHelveticaP14, utf8, 64, 16);
    window.Open();
    Thread::Current().Sleep(1000);
    window.Close();
    frameBuffer.Clear(strikeHelveticaP14.Bounds(utf8, 64, 16));

    strikeHelveticaP14.AddRange(otbFontFileHelveticaP14,0x129,0x129);
    unicodeStr.Replace(Brn((const TByte*)"\x00\x00\x01\x29",4)); // 'i' with tilde
    unicodeStr.ToUtf8(utf8);
    frameBuffer.Write(strikeHelveticaP14, utf8, 64, 16);
    window.Open();
    Thread::Current().Sleep(1000);
    window.Close();
    frameBuffer.Clear(strikeHelveticaP14.Bounds(utf8, 64, 16));
    window.Open();
    Thread::Current().Sleep(1000);
}


class FrameBufferTestOtbExtended : public Suite
{
public:
    FrameBufferTestOtbExtended(const TChar* aDescription):Suite(aDescription){}
    void Test();
};

void FrameBufferTestOtbExtended::Test()
{
    gUi->FrameBufferDisplay().Clear();
    FrameBuffer frameBuffer(128, 32, 1);
    Window window(frameBuffer,gUi->FrameBufferDisplay(),0,0, Priority::kHighest);

    Tars fontTar(uTestFrameBufferTar); // get font file (otb) from store (single file stored in tar file)
    TUint kPointSize;
    window.Close();
    frameBuffer.Clear();

    // NCSB P17 Strike
    Brn otbFileNcsbP17(fontTar.Find(Brn("NcsbP17.otb")));
    kPointSize = 17;
    Otb otbFontFileNcsbP17(otbFileNcsbP17);
    Strike strikeNcsbP17(kPointSize);
    strikeNcsbP17.AddRange(otbFontFileNcsbP17, Strike::eLatin1);

    // NCSB P17 - test alignment, spacing, bit location, clearing
    for (TUint i = 0; i < 10; i++)
    {
        LOG(kUi,"Otb test (NCSB P17) - alignment and spacing string '([{A aBb.$/?\\|ÿ' - x offset of %d\n",i);
        frameBuffer.Write(strikeNcsbP17, Brn("([{A aBb.$/?\\|\xc3\xbf"), i, 16); // last char utf8 for ÿ (U+00FF)
        window.Open();
        Thread::Current().Sleep(100);
        window.Close();
        frameBuffer.Clear(strikeNcsbP17.Bounds(Brn("([{A aBb.$/?\\|\xc3\xbf"), i, 16));
        TEST(frameBuffer.Pixels().Bytes() == 512);
        for (TUint j = 0; j < frameBuffer.Pixels().Bytes(); j++) { TEST(frameBuffer.Pixels().At(j) == 0); }
    }

    // NCSB P17 - test full range of characters
    for (TUint i=0x00; i <= 0x7f; i++)
    {
        TByte index = (TByte)i;
        if ( (i <= 0x1f) || (i >= 0x7f) ) // ignore control characters (all print blank)
        {
            if (i == '\n')
            {
                frameBuffer.Write(strikeNcsbP17, Brn((TByte*)&index,1), 16, 16);
                TEST_THROWS(frameBuffer.Clear(strikeNcsbP17.Bounds(Brn((TByte*)&index,1), 16, 16)),LimitsExceeded);
            }
            else
            {
                frameBuffer.Write(strikeNcsbP17, Brn((TByte*)&index,1), 16, 16);
                window.Open();
                Thread::Current().Sleep(100);
                window.Close();
                frameBuffer.Clear(strikeNcsbP17.Bounds(Brn((TByte*)&index,1), 16, 16));
            }
        }
        else
        {
            LOG(kUi,"Otb test (NCSB P17) - Unicode Character [ %c ] (0x%x, %d)\n",i,i,i);
            frameBuffer.Write(strikeNcsbP17, Brn((TByte*)&index,1), 16, 16);
            window.Open();
            Thread::Current().Sleep(100);
            window.Close();
            frameBuffer.Clear(strikeNcsbP17.Bounds(Brn((TByte*)&index,1), 16, 16));
            TEST(frameBuffer.Pixels().Bytes() == 512);
            for (TUint j = 0; j < frameBuffer.Pixels().Bytes(); j++) { TEST(frameBuffer.Pixels().At(j) == 0); }
        }
    }

    // Helvetica P14 Strike
    Brn otbFileHelveticaP14(fontTar.Find(Brn("HelveticaP14.otb")));
    kPointSize = 14;
    Otb otbFontFileHelveticaP14(otbFileHelveticaP14);
    Strike strikeHelveticaP14(kPointSize);
    strikeHelveticaP14.AddRange(otbFontFileHelveticaP14, Strike::eLatin1);

    // Helvetica P14 - test alignment, spacing, bit location, clearing
    for (TUint i = 0; i < 10; i++)
    {
        LOG(kUi,"Otb test (Helvetica P14) - alignment and spacing string '([{A aBb.$/?\\|ÿ' - x offset of %d\n",i);
        frameBuffer.Write(strikeHelveticaP14, Brn("([{A aBb.$/?\\|\xc3\xbf"), i, 16); // last char utf8 for ÿ (U+00FF)
        window.Open();
        Thread::Current().Sleep(100);
        window.Close();
        frameBuffer.Clear(strikeHelveticaP14.Bounds(Brn("([{A aBb.$/?\\|\xc3\xbf"), i, 16));
        TEST(frameBuffer.Pixels().Bytes() == 512);
        for (TUint j = 0; j < frameBuffer.Pixels().Bytes(); j++) { TEST(frameBuffer.Pixels().At(j) == 0); }
    }

    // Helvetica P14 - test full range of characters
    for (TUint i=0x00; i <= 0x7f; i++)
    {
        TByte index = (TByte)i;
        if ( (i <= 0x1f) || (i >= 0x7f) ) // ignore control characters (all print blank)
        {
            if (i == '\n')
            {
                frameBuffer.Write(strikeHelveticaP14, Brn((TByte*)&index,1), 16, 16);
                TEST_THROWS(frameBuffer.Clear(strikeHelveticaP14.Bounds(Brn((TByte*)&index,1), 16, 16)),LimitsExceeded);
            }
            else
            {
                frameBuffer.Write(strikeHelveticaP14, Brn((TByte*)&index,1), 16, 16);
                frameBuffer.Clear(strikeHelveticaP14.Bounds(Brn((TByte*)&index,1), 16, 16));
            }
        }
        else
        {
            LOG(kUi,"Otb test (Helvetica P14) - Unicode Character [ %c ] (0x%x, %d)\n",i,i,i);
            frameBuffer.Write(strikeHelveticaP14, Brn((TByte*)&index,1), 16, 16);
            window.Open();
            Thread::Current().Sleep(100);
            window.Close();
            frameBuffer.Clear(strikeHelveticaP14.Bounds(Brn((TByte*)&index,1), 16, 16));
            TEST(frameBuffer.Pixels().Bytes() == 512);
            for (TUint j = 0; j < frameBuffer.Pixels().Bytes(); j++) { TEST(frameBuffer.Pixels().At(j) == 0); }
        }
    }

    // Helvetica P14 - add range that includes 2 byte characters in unicode (0x100 to 0x17e)
    strikeHelveticaP14.AddRange(otbFontFileHelveticaP14,0x100,0x17e);
    String unicodeStr(String::eRawUnicode);
    Bws<4> utf8;

    // Helvetica P14 - test full range of characters (2 bytes)
    for (TUint i=0xa0; i <= 0x17e; i++)
    {
        TByte buffer[4];
        buffer[0] = 0x00;
        buffer[1] = 0x00;
        buffer[2] = i>>8;
        buffer[3] = i&0xff;
        LOG(kUi,"Otb test (Helvetica P14) - 2 Byte Unicode Character (0x%x, %d)\n",i,i);
        unicodeStr.Replace(Brn(buffer,4));
        unicodeStr.ToUtf8(utf8);
        frameBuffer.Write(strikeHelveticaP14, utf8, 16, 16);
        window.Open();
        Thread::Current().Sleep(100);
        window.Close();
        frameBuffer.Clear(strikeHelveticaP14.Bounds(utf8, 16, 16));
        TEST(frameBuffer.Pixels().Bytes() == 512);
        for (TUint j = 0; j < frameBuffer.Pixels().Bytes(); j++) { TEST(frameBuffer.Pixels().At(j) == 0); }
    }

    // Lucida P12 Strike
    Brn otbFileLucidaP12(fontTar.Find(Brn("LucidaP12.otb")));
    kPointSize = 12;
    Otb otbFontFileLucidaP12(otbFileLucidaP12);
    Strike strikeLucidaP12(kPointSize);
    strikeLucidaP12.AddRange(otbFontFileLucidaP12, Strike::eLatin1);

    // Lucida P12 - test alignment, spacing, bit location, clearing
    for (TUint i = 0; i < 10; i++)
    {
        LOG(kUi,"Otb test (Lucida P12) - alignment and spacing string '([{A aBb.$/?\\|ÿ' - x offset of %d\n",i);
        frameBuffer.Write(strikeLucidaP12, Brn("([{A aBb.$/?\\|\xc3\xbf"), i, 16); // last char utf8 for ÿ (U+00FF)
        window.Open();
        Thread::Current().Sleep(100);
        window.Close();
        frameBuffer.Clear(strikeLucidaP12.Bounds(Brn("([{A aBb.$/?\\|\xc3\xbf"), i, 16));
        TEST(frameBuffer.Pixels().Bytes() == 512);
        for (TUint j = 0; j < frameBuffer.Pixels().Bytes(); j++) { TEST(frameBuffer.Pixels().At(j) == 0); }
    }

    // Lucida P12 - test full range of characters
    for (TUint i=0x00; i <= 0x7f; i++)
    {
        TByte index = (TByte)i;
        if ( (i <= 0x1f) || (i >= 0x7f) ) // ignore control characters (all print blank)
        {
            if (i == '\n')
            {
                frameBuffer.Write(strikeLucidaP12, Brn((TByte*)&index,1), 16, 16);
                TEST_THROWS(frameBuffer.Clear(strikeLucidaP12.Bounds(Brn((TByte*)&index,1), 16, 16)),LimitsExceeded);
            }
            else
            {
                frameBuffer.Write(strikeLucidaP12, Brn((TByte*)&index,1), 16, 16);
                frameBuffer.Clear(strikeLucidaP12.Bounds(Brn((TByte*)&index,1), 16, 16));
            }
        }
        else
        {
            LOG(kUi,"Otb test (Lucida P12) - Unicode Character [ %c ] (0x%x, %d)\n",i,i,i);
            frameBuffer.Write(strikeLucidaP12, Brn((TByte*)&index,1), 16, 16);
            window.Open();
            Thread::Current().Sleep(100);
            window.Close();
            frameBuffer.Clear(strikeLucidaP12.Bounds(Brn((TByte*)&index,1), 16, 16));
            TEST(frameBuffer.Pixels().Bytes() == 512);
            for (TUint j = 0; j < frameBuffer.Pixels().Bytes(); j++) { TEST(frameBuffer.Pixels().At(j) == 0); }
        }
    }

    // Time P08 Strike
    Brn otbFileTimesP08(fontTar.Find(Brn("TimesP08.otb")));
    kPointSize = 8;
    Otb otbFontFileTimesP08(otbFileTimesP08);
    Strike strikeTimesP08(kPointSize);
    strikeTimesP08.AddRange(otbFontFileTimesP08, Strike::eLatin1);

    // Times P08 - test alignment, spacing, bit location, clearing
    for (TUint i = 0; i < 10; i++)
    {
        LOG(kUi,"Otb test (Times P08) - alignment and spacing string '([{A aBb.$/?\\|ÿ' - x offset of %d\n",i);
        frameBuffer.Write(strikeTimesP08, Brn("([{A aBb.$/?\\|\xc3\xbf"), i, 16); // last char utf8 for ÿ (U+00FF)
        window.Open();
        Thread::Current().Sleep(100);
        window.Close();
        frameBuffer.Clear(strikeTimesP08.Bounds(Brn("([{A aBb.$/?\\|\xc3\xbf"), i, 16));
        TEST(frameBuffer.Pixels().Bytes() == 512);
        for (TUint j = 0; j < frameBuffer.Pixels().Bytes(); j++) { TEST(frameBuffer.Pixels().At(j) == 0); }
    }

    // Times P08 - test full range of characters
    for (TUint i=0x00; i <= 0x7f; i++)
    {
        TByte index = (TByte)i;
        if ( (i <= 0x1f) || (i >= 0x7f) ) // ignore control characters (all print blank)
        {
            if (i == '\n')
            {
                frameBuffer.Write(strikeTimesP08, Brn((TByte*)&index,1), 16, 16);
                TEST_THROWS(frameBuffer.Clear(strikeTimesP08.Bounds(Brn((TByte*)&index,1), 16, 16)),LimitsExceeded);
            }
            else
            {
                frameBuffer.Write(strikeTimesP08, Brn((TByte*)&index,1), 16, 16);
                frameBuffer.Clear(strikeTimesP08.Bounds(Brn((TByte*)&index,1), 16, 16));
            }
        }
        else
        {
            LOG(kUi,"Otb test (Times P08) - Unicode Character [ %c ] (0x%x, %d)\n",i,i,i);
            frameBuffer.Write(strikeTimesP08, Brn((TByte*)&index,1), 16, 16);
            window.Open();
            Thread::Current().Sleep(100);
            window.Close();
            frameBuffer.Clear(strikeTimesP08.Bounds(Brn((TByte*)&index,1), 16, 16));
            TEST(frameBuffer.Pixels().Bytes() == 512);
            for (TUint j = 0; j < frameBuffer.Pixels().Bytes(); j++) { TEST(frameBuffer.Pixels().At(j) == 0); }
        }
    }
    window.Open();
}


class FrameBufferTestComposition : public Suite
{
public:
    FrameBufferTestComposition(const TChar* aDescription):Suite(aDescription){}
    void Test();
};

void FrameBufferTestComposition::Test()
{
    gUi->FrameBufferDisplay().Clear();
    FrameBuffer frameBuffer(128, 32, 1);
    Window window(frameBuffer,gUi->FrameBufferDisplay(),0,0, Priority::kHighest);

    FrameBuffer frameBufferExtra(128, 32, 1);
    FrameBuffer frameBufferLarge(256, 64, 1);
    FrameBuffer frameBufferSmall(64, 16, 1);
    Rectangle rectangle;

    rectangle.Set(0,0,10,10);
    TEST_THROWS(frameBuffer.Write(frameBufferSmall,128,0,rectangle),LimitsExceeded);
    TEST_THROWS(frameBuffer.Write(frameBufferSmall,0,32,rectangle),LimitsExceeded);
    TEST_THROWS(frameBuffer.Write(frameBufferSmall,119,0,rectangle),LimitsExceeded);
    TEST_THROWS(frameBuffer.Write(frameBufferSmall,0,23,rectangle),LimitsExceeded);

    rectangle.Set(64,0,10,10);
    TEST_THROWS(frameBuffer.Write(frameBufferSmall,0,0,rectangle),LimitsExceeded);
    rectangle.Set(0,16,10,10);
    TEST_THROWS(frameBuffer.Write(frameBufferSmall,0,0,rectangle),LimitsExceeded);
    rectangle.Set(55,0,10,10);
    TEST_THROWS(frameBuffer.Write(frameBufferSmall,0,0,rectangle),LimitsExceeded);
    rectangle.Set(0,7,10,10);
    TEST_THROWS(frameBuffer.Write(frameBufferSmall,0,0,rectangle),LimitsExceeded);

    TEST_THROWS(frameBuffer.Write(frameBufferLarge,0,0),LimitsExceeded);
    TEST_THROWS(frameBuffer.Write(frameBufferSmall,65,0),LimitsExceeded);
    TEST_THROWS(frameBuffer.Write(frameBufferSmall,0,17),LimitsExceeded);

    window.Close();
    frameBuffer.Fill((TUint32)0x87654321);
    window.Open();
    Thread::Current().Sleep(1000);
    window.Close();

    frameBuffer.Fill();
    frameBufferExtra.Clear();
    for (TUint j = 0; j < 128; j+=8)
    {
        rectangle.Set(j,j&0x1f,1,1);
        for (TUint i = 0; i < 128; i++)
        {
            frameBuffer.Write(frameBufferExtra,i,i&0x1f,rectangle);
            frameBuffer.Write(frameBufferExtra,127-i,i&0x1f,rectangle);
        }
        window.Open();
        Thread::Current().Sleep(50);
        window.Close();
        frameBuffer.Fill();
    }
    frameBuffer.Clear();
    frameBufferExtra.Fill();
    rectangle.Set(31,16,2,2);
    for (TUint i = 0; i < 127; i++)
    {
        frameBuffer.Write(frameBufferExtra,i,i&0xf,rectangle);
        window.Open();
        Thread::Current().Sleep(50);
        window.Close();
        frameBuffer.Clear();
    }
    frameBuffer.Fill();
    frameBufferExtra.Clear();
    rectangle.Set(31,16,2,2);
    for (TUint i = 0; i < 127; i++)
    {
        frameBuffer.Write(frameBufferExtra,i,i&0xf,rectangle);
        window.Open();
        Thread::Current().Sleep(50);
        window.Close();
    }
    frameBuffer.Fill();
    frameBufferExtra.Fill((TUint32)0x87654321);
    rectangle.Set(0,0,96,1);
    for (TUint i = 0; i < 32; i++)
    {
        frameBuffer.Write(frameBufferExtra,i,i,rectangle);
        window.Open();
        Thread::Current().Sleep(100);
        window.Close();
    }
    for (TUint i = 0; i < 32; i++)
    {
        frameBuffer.Write(frameBufferExtra,i,31-i,rectangle);
        window.Open();
        Thread::Current().Sleep(100);
        window.Close();
    }

    frameBufferExtra.Fill((TByte)0xa0);
    frameBuffer.Fill();

    rectangle.Set(35,3,76,5);
    frameBuffer.Write(frameBufferExtra,25,9,rectangle);
    window.Open();
    Thread::Current().Sleep(1000);
    window.Close();
    for (TUint i = 0; i < 147; i++) { TEST(frameBuffer.Pixels().At(i) == 0xff); }
    for (TUint i = 0; i < 9; i++) { TEST(frameBuffer.Pixels().At(147+i) == 0x82); }
    TEST(frameBuffer.Pixels().At(156) == 0x87);
    for (TUint i = 157; i < 160; i++) { TEST(frameBuffer.Pixels().At(i) == 0xff); }

    frameBuffer.Fill();

    rectangle.Set(35,3,63,5);
    frameBuffer.Write(frameBufferExtra,25,9,rectangle);
    window.Open();
    Thread::Current().Sleep(1000);
    window.Close();
    for (TUint i = 0; i < 147; i++) { TEST(frameBuffer.Pixels().At(i) == 0xff); }
    for (TUint i = 0; i < 8; i++) { TEST(frameBuffer.Pixels().At(147+i) == 0x82); }
    for (TUint i = 155; i < 160; i++) { TEST(frameBuffer.Pixels().At(i) == 0xff); }

    frameBuffer.Clear();

    rectangle.Set(35,3,64,5);
    frameBuffer.Write(frameBufferExtra,25,9,rectangle);
    window.Open();
    Thread::Current().Sleep(1000);
    window.Close();
    for (TUint i = 0; i < 147; i++) { TEST(frameBuffer.Pixels().At(i) == 0x00); }
    TEST(frameBuffer.Pixels().At(147) == 0x02);
    for (TUint i = 0; i < 7; i++) { TEST(frameBuffer.Pixels().At(148+i) == 0x82); }
    TEST(frameBuffer.Pixels().At(155) == 0x80);
    for (TUint i = 156; i < 160; i++) { TEST(frameBuffer.Pixels().At(i) == 0x00); }

    frameBuffer.Fill();

    rectangle.Set(35,3,65,5);
    frameBuffer.Write(frameBufferExtra,25,9,rectangle);
    window.Open();
    Thread::Current().Sleep(1000);
    window.Close();
    for (TUint i = 0; i < 147; i++) { TEST(frameBuffer.Pixels().At(i) == 0xff); }
    for (TUint i = 0; i < 8; i++) { TEST(frameBuffer.Pixels().At(147+i) == 0x82); }
    TEST(frameBuffer.Pixels().At(155) == 0xbf);
    for (TUint i = 156; i < 160; i++) { TEST(frameBuffer.Pixels().At(i) == 0xff); }

    frameBuffer.Fill();

    rectangle.Set(35,3,66,5);
    frameBuffer.Write(frameBufferExtra,25,9,rectangle);
    window.Open();
    Thread::Current().Sleep(1000);
    window.Close();
    for (TUint i = 0; i < 147; i++) { TEST(frameBuffer.Pixels().At(i) == 0xff); }
    for (TUint i = 0; i < 8; i++) { TEST(frameBuffer.Pixels().At(147+i) == 0x82); }
    TEST(frameBuffer.Pixels().At(155) == 0x9f);
    for (TUint i = 156; i < 160; i++) { TEST(frameBuffer.Pixels().At(i) == 0xff); }

    frameBuffer.Fill();

    rectangle.Set(57,3,45,5);
    frameBuffer.Write(frameBufferExtra,3,9,rectangle);
    window.Open();
    Thread::Current().Sleep(1000);
    window.Close();
    for (TUint i = 0; i < 144; i++) { TEST(frameBuffer.Pixels().At(i) == 0xff); }
    TEST(frameBuffer.Pixels().At(144) == 0xe8);
    for (TUint i = 0; i < 5; i++) { TEST(frameBuffer.Pixels().At(145+i) == 0x28); }
    for (TUint i = 150; i < 160; i++) { TEST(frameBuffer.Pixels().At(i) == 0xff); }

    frameBuffer.Fill();

    rectangle.Set(57,3,53,5);
    frameBuffer.Write(frameBufferExtra,3,9,rectangle);
    window.Open();
    Thread::Current().Sleep(1000);
    window.Close();
    for (TUint i = 0; i < 144; i++) { TEST(frameBuffer.Pixels().At(i) == 0xff); }
    TEST(frameBuffer.Pixels().At(144) == 0xe8);
    for (TUint i = 0; i < 6; i++) { TEST(frameBuffer.Pixels().At(145+i) == 0x28); }
    for (TUint i = 151; i < 160; i++) { TEST(frameBuffer.Pixels().At(i) == 0xff); }

    frameBuffer.Clear();

    rectangle.Set(57,3,26,5);
    frameBuffer.Write(frameBufferExtra,3,9,rectangle);
    window.Open();
    Thread::Current().Sleep(1000);
    window.Close();
    for (TUint i = 0; i < 144; i++) { TEST(frameBuffer.Pixels().At(i) == 0x00); }
    TEST(frameBuffer.Pixels().At(144) == 0x08);
    for (TUint i = 0; i < 3; i++) { TEST(frameBuffer.Pixels().At(145+i) == 0x28); }
    for (TUint i = 148; i < 160; i++) { TEST(frameBuffer.Pixels().At(i) == 0x00); }

    Tars bmpTar(uTestFrameBufferTar); // get bitmap from store (single file stored in tar file)
    Bmp testBmp64x16Tar(bmpTar.Find(Brn("TestBmp64x16.bmp")));
    frameBuffer.Clear();
    frameBuffer.Fill((TUint32)0x87654321);
    frameBufferExtra.Write(testBmp64x16Tar, 32, 3);

    rectangle.Set(32,3,64,16);
    frameBuffer.Write(frameBufferExtra, 56, 10, rectangle); // right
    window.Open();
    Thread::Current().Sleep(1000);
    window.Close();
    frameBuffer.Fill((TUint32)0x87654321);

    frameBuffer.Write(frameBufferExtra, 32, 10, rectangle); //none
    window.Open();
    Thread::Current().Sleep(1000);
    window.Close();
    frameBuffer.Fill((TUint32)0x87654321);

    frameBuffer.Clear();
    frameBufferExtra.Write(testBmp64x16Tar, 33, 3);
    rectangle.Set(33,3,64,16);

    frameBuffer.Write(frameBufferExtra, 32, 10, rectangle); // left
    window.Open();
    Thread::Current().Sleep(1000);
    window.Close();
    frameBuffer.Fill((TUint32)0x87654321);

    frameBuffer.Write(frameBufferExtra, 33, 10, rectangle); //none
    window.Open();
    Thread::Current().Sleep(1000);
    window.Close();
    frameBuffer.Clear();

    frameBuffer.Fill((TUint32)0x87654321);
    frameBufferExtra.Fill((TUint32)0x12345678);
    // performance tests - test on lava board
    TUint startTime;
    rectangle.Set(57,3,64,5);
    startTime = Os::TimeInMs();
    for (TUint i = 0; i < 50000; i++) { frameBufferExtra.Write(frameBuffer,3,9,rectangle); }
    startTime = (Os::TimeInMs()-startTime)/50;
    LOG(kUi, "Bit Bliiting Shift Left Algorithm Time: %d uS\n",startTime);

    startTime = Os::TimeInMs();
    for (TUint i = 0; i < 50000; i++) { frameBufferExtra.Write(frameBuffer,18,9,rectangle); }
    startTime = (Os::TimeInMs()-startTime)/50;
    LOG(kUi, "Bit Bliiting Shift Right Algorithm Time: %d uS\n",startTime);

    startTime = Os::TimeInMs();
    for (TUint i = 0; i < 50000; i++) { frameBufferExtra.Write(frameBuffer,25,9,rectangle); }
    startTime = (Os::TimeInMs()-startTime)/50;
    LOG(kUi, "Bit Bliiting No Shift Algorithm Time: %d uS\n",startTime);
    window.Open();
}


class FrameBufferTestSizes : public Suite
{
public:
    FrameBufferTestSizes(const TChar* aDescription):Suite(aDescription){}
    void Test();
};


void FrameBufferTestSizes::Test()
{
    gUi->FrameBufferDisplay().Clear();
    FrameBuffer frameBuffer(128, 32, 1);
    Window window(frameBuffer,gUi->FrameBufferDisplay(),0,0, Priority::kHighest);

    FrameBuffer* frameBufferSmall = new FrameBuffer(64, 16, 1); //smaller non-nominated frame buffer
    FrameBuffer* frameBufferLarge = new FrameBuffer(256, 64, 1); //larger non-nominated frame buffer

    VfdViewer testDisplayDriverSmall(frameBufferSmall, Priority::kHighest); //viewer will display small frame buffer
    VfdViewer testDisplayDriverLarge(frameBufferLarge, Priority::kHighest); //viewer will display large frame buffer
    testDisplayDriverSmall.SetEnabled(true);
    testDisplayDriverLarge.SetEnabled(true);

    Tars bmpTar(uTestFrameBufferTar); // get bitmap from store (single file stored in tar file)
    Bmp testBmp64x16Tar(bmpTar.Find(Brn("TestBmp64x16.bmp")));

    // fill small 64 x 16 x 1 frame buffer with BMP image
    frameBufferSmall->Write(testBmp64x16Tar, 0, 0);
    Thread::Current().Sleep(1000); // small frame buffer displayed to viewer only (no hardware for this size)

    // test limits
    window.Close();
    frameBuffer.Write(*frameBufferSmall, 0, 0); // right on limit - passes by not throwing an exception
    window.Open();
    Thread::Current().Sleep(1000);
    window.Close();
    frameBuffer.Clear(frameBufferSmall->Bounds(0, 0)); // right on limit - passes by not throwing an exception
    window.Open();
    Thread::Current().Sleep(1000);
    window.Close();
    frameBuffer.Write(*frameBufferSmall, 64, 16); // right on limit - passes by not throwing an exception
    window.Open();
    Thread::Current().Sleep(1000);
    window.Close();
    frameBuffer.Clear(frameBufferSmall->Bounds(64, 16)); // right on limit - passes by not throwing an exception
    window.Open();
    Thread::Current().Sleep(1000);
    window.Close();
    TEST_THROWS(frameBuffer.Write(*frameBufferSmall, 65, 10),LimitsExceeded);
    TEST_THROWS(frameBuffer.Clear(frameBufferSmall->Bounds(65, 10)),LimitsExceeded);
    TEST_THROWS(frameBuffer.Write(*frameBufferSmall, 60, 17),LimitsExceeded);
    TEST_THROWS(frameBuffer.Clear(frameBufferSmall->Bounds(60, 17)),LimitsExceeded);

    // fill large 256 x 64 x 1 frame buffer
    frameBufferLarge->Fill((TUint32)0x87654321);
    window.Open();
    Thread::Current().Sleep(1000); // large frame buffer displayed to viewer only (no hardware for this size)
    window.Close();
    frameBuffer.Write(*frameBufferLarge,0,0,Rectangle(16,0,128,32));
    window.Open();
    Thread::Current().Sleep(1000);

    testDisplayDriverSmall.SetEnabled(false);
    testDisplayDriverLarge.SetEnabled(false);
}


class FrameBufferTestDisplayDriver : public Suite
{
public:
    FrameBufferTestDisplayDriver(const TChar* aDescription) : Suite(aDescription) {}
    void Test();
};

void FrameBufferTestDisplayDriver::Test()
{
    gUi->FrameBufferDisplay().Clear();
    FrameBuffer frameBuffer(128, 32, 1);
    Window window(frameBuffer,gUi->FrameBufferDisplay(),0,0,Priority::kHighest);
    RenderControl render(window,gUi->FontStandard(),Priority::kHighest);

    Tars bmpTar(uTestFrameBufferTar); // get bitmap from store (single file stored in tar file)

    Brn otbFile(bmpTar.Find(Brn("NcsbP17.otb")));
    TUint kPointSize = 17;
    Otb otbFontFile(otbFile);
    Strike strike(kPointSize);
    strike.AddRange(otbFontFile, Strike::eLatin1);

    window.Close();
    frameBuffer.Fill((TUint32)0x87654321);
    window.Open();
    Thread::Current().Sleep(1000);

    LOG(kUi,"Frame Buffer Cleared\n");
    window.Close();
    frameBuffer.Clear();
    window.Open();
    Thread::Current().Sleep(1000);

    LOG(kUi,"Frame Buffer Filled\n");
    window.Close();
    frameBuffer.Fill();
    window.Open();
    Thread::Current().Sleep(1000);

    LOG(kUi,"BMP TestBmpVerticalStripes128x32.bmp (tar)\n");
    Bmp testBmpVsTar(bmpTar.Find(Brn("TestBmpVerticalStripes128x32.bmp")));
    window.Close();
    frameBuffer.Clear();
    frameBuffer.Write(testBmpVsTar, 0, 0);
    window.Open();
    Thread::Current().Sleep(1000);

    LOG(kUi,"BMP TestBmpHorizontalStripes128x32.bmp (tar)\n");
    Bmp testBmpHsTar(bmpTar.Find(Brn("TestBmpHorizontalStripes128x32.bmp")));
    window.Close();
    frameBuffer.Clear();
    frameBuffer.Write(testBmpHsTar, 0, 0);
    window.Open();
    Thread::Current().Sleep(1000);

    LOG(kUi,"BMP TestBmp64x16.bmp (tar - 1 per quadrant)\n");
    Bmp testBmp64x16Tar(bmpTar.Find(Brn("TestBmp64x16.bmp")));
    window.Close();
    frameBuffer.Clear();
    frameBuffer.Write(testBmp64x16Tar, 0, 0);
    frameBuffer.Write(testBmp64x16Tar, 64, 0);
    frameBuffer.Write(testBmp64x16Tar, 0, 16);
    frameBuffer.Write(testBmp64x16Tar, 64, 16);
    window.Open();
    Thread::Current().Sleep(1000);

    // scrolling text
    gProduct->Parameters().SetParameter(Common::kParameterTargetDisplay, Common::kParameterNameDisplayScrollText, Common::kParameterValueTrue);
    Brn text("Some scrolling text to look at WgWgWgWg");
    FrameBuffer frameBufferScroll(128,32,1);
    // text inside border of 3,3,3,3
    gUi->FrameBufferDisplay().Fill();
    Rectangle rectScroll(0,0,122,26);
    Window windowScroll(frameBufferScroll,gUi->FrameBufferDisplay(),3,3,rectScroll,Priority::kHighest);
    RenderControl renderScroll(windowScroll,gUi->FontStandard(),Priority::kHighest);

    renderScroll.ScrollMetadata(text);
    windowScroll.Close();
    windowScroll.Open();

    // text inside border of 3,0,3,6
    gUi->FrameBufferDisplay().Fill();
    rectScroll.Set(0,0,122,26);
    windowScroll.Set(3,0,rectScroll);

    renderScroll.ScrollMetadata(text);
    windowScroll.Close();
    windowScroll.Open();

    // text inside border of 0,0,6,6
    gUi->FrameBufferDisplay().Fill();
    rectScroll.Set(0,0,122,26);
    windowScroll.Set(0,0,rectScroll);

    renderScroll.ScrollMetadata(text);
    windowScroll.Close();
    windowScroll.Open();

    // text inside border of 0,0,0,6
    gUi->FrameBufferDisplay().Fill();
    FrameBuffer frameBufferScroll1(128,26,1);
    Window windowScroll1(frameBufferScroll1,gUi->FrameBufferDisplay(),0,0,Priority::kHighest);
    RenderControl renderScroll1(windowScroll1,gUi->FontStandard(),Priority::kHighest);

    renderScroll1.ScrollMetadata(text);
    windowScroll1.Close();
    windowScroll1.Open();

    // text portion of text (small width)
    gUi->FrameBufferDisplay().Fill();
    rectScroll.Set(0,0,20,26);
    windowScroll.Set(3,3,rectScroll);

    renderScroll.ScrollMetadata(text);
    windowScroll.Close();
    windowScroll.Open();

    // text portion of text (small box)
    gUi->FrameBufferDisplay().Fill();
    rectScroll.Set(0,0,20,10);
    windowScroll.Set(3,3,rectScroll);

    renderScroll.ScrollMetadata(text);
    windowScroll.Close();
    windowScroll.Open();

    // abort tests
    render.ScrollMetadata(text);
    Thread::Current().Sleep(2000);
    render.Pause();
    Thread::Current().Sleep(2000);
    render.Resume();
    Thread::Current().Sleep(2000);
    render.Stop();
    Thread::Current().Sleep(2000);
    render.ScrollMetadata(text);
    Thread::Current().Sleep(2000);
    render.Pause();
    Thread::Current().Sleep(2000);
    render.Stop();
    Thread::Current().Sleep(2000);
    window.Close();
    window.Open();
    Brn text1("Some scrolling text to look at WgWgWgWg£");
    TEST_THROWS(render.ScrollMetadata(text1), RenderFailed);
    window.Close();
    window.Open();

    Brn text2("Title: Josh's Song\nArtist: Josh\nAlbum: Josh's Album");
    render.ScrollMetadata(text2);
    Thread::Current().Sleep(2000);
    render.Pause();
    Thread::Current().Sleep(2000);
    render.Resume();
    Thread::Current().Sleep(2000);
    render.Stop();
    Thread::Current().Sleep(2000);
    render.ScrollMetadata(text2);
    Thread::Current().Sleep(5000);
    render.Pause();
    Thread::Current().Sleep(2000);
    render.Stop();
    Thread::Current().Sleep(2000);

    window.Close();
    window.Open();
    Brn text3("Title: Josh£s Song\nArtist: Josh\nAlbum: Josh's Album");
    TEST_THROWS(render.ScrollMetadata(text3), RenderFailed);
    window.Close();
    window.Open();
    Brn text4("Title: Josh's Song\nArtist£ Josh\nAlbum: Josh's Album");
    TEST_THROWS(render.ScrollMetadata(text4), RenderFailed);
    window.Close();
    window.Open();
    Brn text5("Title: Josh's Song\nArtist: Josh\nAlbum: Josh£s Album");
    TEST_THROWS(render.ScrollMetadata(text5), RenderFailed);
    window.Close();
    window.Open();
    Brn text6("Title: Josh£s Song\nArtist£ Josh\nAlbum: Josh£s Album");
    TEST_THROWS(render.ScrollMetadata(text6), RenderFailed);
    window.Close();
    window.Open();
    render.ScrollMetadata(text2);
    window.Close();
    window.Open();

    // test various brightness levels - show numbers
    gUi->FrameBufferDisplay().Clear();
    window.Close();
    Bmp testBmpBorder128x32Tar(bmpTar.Find(Brn("TestBmpBorder128x32.bmp")));
    frameBuffer.Write(testBmpBorder128x32Tar, 0, 0);
    Bws<3> numberStr;
    numberStr.SetBytes(3);
    for (TUint i = 0; i <= 100; i++)
    {
        numberStr[0] = 0x30;
        if (i == 100) { numberStr.Replace(Brn("100")); }
        else
        {
            numberStr[1] = (i/10) + 0x30;
            numberStr[2] = (i%10) + 0x30;
        }
        gUi->Display().SetBrightness(i);
        frameBuffer.Write(strike, numberStr, 45, 20);
        window.Open();
        Thread::Current().Sleep(200);
        window.Close();
        frameBuffer.Clear(strike.Bounds(numberStr, 45, 20));
    }

    frameBuffer.Fill();
    window.Open();

    // test various brightness levels - show all pixels set
    for (TUint j = 0; j < 2; j++)
    {
        for (TUint i = 0; i <= 100; i++)
        {
            gUi->Display().SetBrightness(i);
            Thread::Current().Sleep(50);
        }
    }
}


class FrameBufferTestReverseVideo : public Suite
{
public:
    FrameBufferTestReverseVideo(const TChar* aDescription):Suite(aDescription){}
    void Test();
};


void FrameBufferTestReverseVideo::Test()
{
    FrameBuffer frameBuffer(128, 32, 1);
    Window window(frameBuffer,gUi->FrameBufferDisplay(),0,0, Priority::kHighest);

    Tars bmpTar(uTestFrameBufferTar); // get bitmap from store (single file stored in tar file)

    // inverse - rectangle
    window.Close();
    frameBuffer.Clear();
    frameBuffer.Inverse(Rectangle(0,0,128,32)); // filled
    window.Open();
    Thread::Current().Sleep(1000);
    window.Close();
    frameBuffer.Inverse(Rectangle(0,0,128,32)); // cleared
    window.Open();
    Thread::Current().Sleep(1000);


    LOG(kUi,"BMP TestBmp64x16.bmp (tar - 1 per quadrant)\n");
    window.Close();
    Bmp testBmp64x16Tar(bmpTar.Find(Brn("TestBmp64x16.bmp")));
    frameBuffer.Write(testBmp64x16Tar, 0, 0);
    frameBuffer.Write(testBmp64x16Tar, 64, 0);
    frameBuffer.Write(testBmp64x16Tar, 0, 16);
    frameBuffer.Write(testBmp64x16Tar, 64, 16);

    window.Open();
    Thread::Current().Sleep(1000);
    window.Close();
    // inverse - bitmap (top right quadrant)
    frameBuffer.Inverse(testBmp64x16Tar.Bounds(64, 0));
    window.Open();
    Thread::Current().Sleep(1000);
    window.Close();
    // inverse - rectangle
    frameBuffer.Inverse(Rectangle(0,0,128,32));
    window.Open();
    Thread::Current().Sleep(1000);
    window.Close();
    // inverse - frame buffer
    frameBuffer.Inverse(frameBuffer.Bounds());
    window.Open();
    Thread::Current().Sleep(1000);
    window.Close();

    frameBuffer.Clear();

    Brn otbFile(bmpTar.Find(Brn("NcsbP17.otb")));
    TUint kPointSize = 17;
    Otb otbFontFile(otbFile);
    Strike strike(kPointSize);
    strike.AddRange(otbFontFile, Strike::eLatin1);

    // inverse - character (ascii)
    frameBuffer.Write(strike,Brn("A"),0,19);
    frameBuffer.Inverse(strike.Bounds(Brn("A"),0,19));
    window.Open();
    Thread::Current().Sleep(1000);
    window.Close();
    frameBuffer.Clear();

    // inverse - character (32)
    Brn otbFileHelveticaP14(bmpTar.Find(Brn("HelveticaP14.otb")));
    kPointSize = 14;
    Otb otbFontFileHelveticaP14(otbFileHelveticaP14);
    Strike strikeHelveticaP14(kPointSize);
    strikeHelveticaP14.AddRange(otbFontFileHelveticaP14,Strike::eLatin1);
    strikeHelveticaP14.AddRange(otbFontFileHelveticaP14,0x1e4,0x1e4);

    String unicodeStr(Brn((const TByte*)"\x00\x00\x01\xe4",4), String::eRawUnicode);
    Bws<4> utf8;
    unicodeStr.ToUtf8(utf8);
    frameBuffer.Write(strikeHelveticaP14, utf8, 0, 19);
    frameBuffer.Inverse(strikeHelveticaP14.Bounds(utf8, 0, 19));
    window.Open();
    Thread::Current().Sleep(1000);
    window.Close();
    frameBuffer.Clear();

    // inverse - string
    frameBuffer.Write(strike,Brn("AaBbCc17"),0,19);
    frameBuffer.Inverse(strike.Bounds(Brn("AaBbCc17"),0,19));
    window.Open();
    Thread::Current().Sleep(1000);
    window.Close();
    frameBuffer.Clear();
    window.Open();
}


class FrameBufferWindowTest : public Suite
{
public:
    FrameBufferWindowTest(const TChar* aDescription):Suite(aDescription){}
    void Test();
};


void FrameBufferWindowTest::Test()
{
    gUi->FrameBufferDisplay().Clear();
#ifdef STANDARD_DISPLAY
    FrameBuffer frameBufferTopLeft(64,16,1);
    FrameBuffer frameBufferTopRight(64,16,1);
    FrameBuffer frameBufferBottomLeft(64,16,1);
    FrameBuffer frameBufferBottomRight(64,16,1);

    Window window1(frameBufferTopLeft,gUi->FrameBufferDisplay(),0,0, Priority::kHighest);
    Window window2(frameBufferTopRight,gUi->FrameBufferDisplay(),64,0, Priority::kHighest);
    Window window3(frameBufferBottomLeft,gUi->FrameBufferDisplay(),0,16, Priority::kHighest);
    Window window4(frameBufferBottomRight,gUi->FrameBufferDisplay(),64,16,Rectangle(0,0,15,12), Priority::kHighest);

    Tars fontTar(uTestFrameBufferTar); // get font from store (single file stored in tar file)
    Brn otbFile(fontTar.Find(Brn("NcsbP17.otb")));
    TUint kPointSize = 17;
    Otb otbFontFile(otbFile);
    Strike strike(kPointSize);
    strike.AddRange(otbFontFile, Strike::eLatin1);

    window1.Close();
    window2.Close();
    window3.Close();
    window4.Close();
    frameBufferTopLeft.Write(strike,Brn("Test"),0,11);
    frameBufferTopRight.Write(strike,Brn("Hello"),0,11);
    frameBufferBottomLeft.Write(strike,Brn("World"),0,11);
    frameBufferBottomRight.Write(strike,Brn("Josh"),0,11);
    window1.Open();
    window2.Open();
    window3.Open();
    window4.Open();
    Thread::Current().Sleep(1000);
    window1.Close();
    frameBufferTopLeft.Clear(strike.Bounds(Brn("Test"),0,11));
    frameBufferTopLeft.Write(strike,Brn("101"),0,11);
    Thread::Current().Sleep(1000);
    window1.Open();
    Thread::Current().Sleep(1000);
    window4.Close();
    frameBufferBottomRight.Clear(strike.Bounds(Brn("Josh"),0,11));
    frameBufferBottomRight.Write(strike,Brn("999"),0,11);
    window4.Open();
    Thread::Current().Sleep(1000);
#else
    FrameBuffer fb1(192,32,1);
    Window win2(fb1,gUi->FrameBufferDisplay(),12,0,Rectangle(0,0,180,32), Priority::kHighest);
    RenderControl* ren2 = new RenderControl(win2,gUi->FontStandard(),gUi->Display().Priority());

    ren2->LeftJustify(gUi->BmpStop(), false, RenderControl::eBmpJustifyTop);
    Thread::Current().Sleep(1000);
    ren2->LeftJustify(gUi->BmpStop(), false, RenderControl::eBmpJustifyMiddle);
    Thread::Current().Sleep(1000);
    ren2->LeftJustify(gUi->BmpStop(), false, RenderControl::eBmpJustifyBottom);
    Thread::Current().Sleep(1000);
    ren2->CenterJustify(gUi->BmpStop(), false, RenderControl::eBmpJustifyTop);
    Thread::Current().Sleep(1000);
    ren2->CenterJustify(gUi->BmpStop(), false, RenderControl::eBmpJustifyMiddle);
    Thread::Current().Sleep(1000);
    ren2->CenterJustify(gUi->BmpStop(), false, RenderControl::eBmpJustifyBottom);
    Thread::Current().Sleep(1000);
    ren2->RightJustify(gUi->BmpStop(), false, RenderControl::eBmpJustifyTop);
    Thread::Current().Sleep(1000);
    ren2->RightJustify(gUi->BmpStop(), false, RenderControl::eBmpJustifyMiddle);
    Thread::Current().Sleep(1000);
    ren2->RightJustify(gUi->BmpStop(), false, RenderControl::eBmpJustifyBottom);
    Thread::Current().Sleep(1000);
    ren2->LeftJustify(Brn("Test1"));
    Thread::Current().Sleep(1000);
    ren2->CenterJustify(Brn("Test2"));
    Thread::Current().Sleep(1000);
    ren2->RightJustify(Brn("Test3"));
    Thread::Current().Sleep(1000);
    ren2->RightJustify(Brn("Thisisaveryverylongstringtodisplay"),true);
    Thread::Current().Sleep(1000);
    ren2->RightJustify(Brn("Thisisaveryverylongstringtodisplay"),false);
    Thread::Current().Sleep(1000);
    ren2->ScrollMetadata(Brn("Thisisaveryverylongstringtodisplay"));
    Thread::Current().Sleep(15000);
    ren2->Clear();
    gUi->FrameBufferDisplay().Clear();

    FrameBuffer fb3(96,10,1);
    FrameBuffer fb4(96,10,1);
    FrameBuffer fb5(192,22,1);
    Window win3(fb3,gUi->FrameBufferDisplay(),12,22,Rectangle(0,0,90,10), Priority::kHighest);
    Window win4(fb4,gUi->FrameBufferDisplay(),102,22,Rectangle(0,0,90,10), Priority::kHighest);
    Window win5(fb5,gUi->FrameBufferDisplay(),12,0,Rectangle(0,0,180,22), Priority::kHighest);
    RenderControl* ren3 = new RenderControl(win3,gUi->Display().Priority());
    RenderControl* ren4 = new RenderControl(win4,gUi->Display().Priority());
    RenderControl* ren5 = new RenderControl(win5,gUi->FontStandard(),gUi->Display().Priority());

    ren3->LeftJustify(gUi->BmpPlay(), true);
    ren3->CenterJustify(gUi->BmpPlay(), true);
    ren3->RightJustify(gUi->BmpPlay(), false);
    Thread::Current().Sleep(1000);

    ren4->LeftJustify(gUi->BmpPause(), true);
    ren4->CenterJustify(gUi->BmpPause(), true);
    ren4->RightJustify(gUi->BmpPause(), false);
    Thread::Current().Sleep(1000);

    ren5->ScrollNavDataCenter(Brn("Thisisaveryverylongstringtodisplay"));
    Thread::Current().Sleep(10000);

    ren4->LeftJustify(gUi->BmpStop(), true);
    ren4->CenterJustify(gUi->BmpStop(), true);
    ren4->RightJustify(gUi->BmpStop(), false);
    Thread::Current().Sleep(1000);

    ren3->LeftJustify(gUi->BmpStop(), true);
    ren3->CenterJustify(gUi->BmpStop(), true);
    ren3->RightJustify(gUi->BmpStop(), false);
    Thread::Current().Sleep(1000);
#endif
}


class FrameBufferTestBounds : public Suite
{
public:
    FrameBufferTestBounds(const TChar* aDescription):Suite(aDescription){}
    void Test();
};


void FrameBufferTestBounds::Test()
{
    gUi->FrameBufferDisplay().Clear();
    FrameBuffer frameBuffer(128, 32, 1);
    Window window(frameBuffer,gUi->FrameBufferDisplay(),0,0, Priority::kHighest);

    FrameBuffer frameBufferLarge(256,64,1);
    FrameBuffer frameBufferSmall(64,16,1);
    Rectangle rectangle;
    Tars tar(uTestFrameBufferTar); // get font and bmp files from store (single file stored in tar file)

    // frame buffer bounds
    rectangle = frameBuffer.Bounds();
    TEST(rectangle.X() == 0);
    TEST(rectangle.Y() == 0);
    TEST(rectangle.Width() == 128);
    TEST(rectangle.Height() == 32);

    TEST(frameBuffer.Bounds(11,22).X() == 11);
    TEST(frameBuffer.Bounds(11,22).Y() == 22);
    TEST(frameBuffer.Bounds(11,22).Width() == 128);
    TEST(frameBuffer.Bounds(11,22).Height() == 32);

    rectangle = frameBufferLarge.Bounds(1,1);
    TEST(rectangle.X() == 1);
    TEST(rectangle.Y() == 1);
    TEST(rectangle.Width() == 256);
    TEST(rectangle.Height() == 64);

    TEST(frameBufferSmall.Bounds(12,23).X() == 12);
    TEST(frameBufferSmall.Bounds(12,23).Y() == 23);
    TEST(frameBufferSmall.Bounds(12,23).Width() == 64);
    TEST(frameBufferSmall.Bounds(12,23).Height() == 16);

    // bmp bounds
    Bmp testBmp64x16Tar(tar.Find(Brn("TestBmp64x16.bmp")));
    Bmp testBmpVsTar(tar.Find(Brn("TestBmpVerticalStripes128x32.bmp")));
    Bmp testBmp10x10Tar(tar.Find(Brn("TestBmp10x10.bmp")));

    rectangle = testBmp64x16Tar.Bounds();
    TEST(rectangle.X() == 0);
    TEST(rectangle.Y() == 0);
    TEST(rectangle.Width() == 64);
    TEST(rectangle.Height() == 16);
    rectangle = testBmpVsTar.Bounds(8,9);
    TEST(rectangle.X() == 8);
    TEST(rectangle.Y() == 9);
    TEST(rectangle.Width() == 128);
    TEST(rectangle.Height() == 32);
    rectangle = testBmp10x10Tar.Bounds(100,100);
    TEST(rectangle.X() == 100);
    TEST(rectangle.Y() == 100);
    TEST(rectangle.Width() == 10);
    TEST(rectangle.Height() == 10);

    // strike (+string) bounds
    Brn otbFile(tar.Find(Brn("NcsbP17.otb")));
    TUint kPointSize = 17;
    Otb otbFontFile(otbFile);
    Strike strike(kPointSize);
    strike.AddRange(otbFontFile, Strike::eLatin1);

    rectangle = strike.Bounds(Brn("Hello"),0,13);
    TEST(rectangle.X() == 0);
    TEST(rectangle.Y() == 2);
    TEST(rectangle.Width() == 40);
    TEST(rectangle.Height() == 12);

    rectangle = strike.Bounds(Brn("Hello"),10,15);
    TEST(rectangle.X() == 10);
    TEST(rectangle.Y() == 4);
    TEST(rectangle.Width() == 40);
    TEST(rectangle.Height() == 12);

    rectangle = strike.Bounds(Brn("l"),40,17);
    TEST(rectangle.X() == 40);
    TEST(rectangle.Y() == 6);
    TEST(rectangle.Width() == 5);
    TEST(rectangle.Height() == 12);

    rectangle = strike.Bounds(Brn("Hello World"),2,16);
    TEST(rectangle.X() == 2);
    TEST(rectangle.Y() == 5);
    TEST(rectangle.Width() == 90);
    TEST(rectangle.Height() == 12);

    rectangle = strike.Bounds(Brn("@"), 116, 30);
    TEST(rectangle.X() == 115);
    TEST(rectangle.Y() == 19);
    TEST(rectangle.Width() == 13);
    TEST(rectangle.Height() == 13);

    rectangle = strike.Bounds(Brn("@@@@@@@@"), 14, 19);
    TEST(rectangle.X() == 13);
    TEST(rectangle.Y() == 8);
    TEST(rectangle.Width() == 97);
    TEST(rectangle.Height() == 13);

    // Helvetica P14 Strike
    Brn otbFileHelveticaP14(tar.Find(Brn("HelveticaP14.otb")));
    kPointSize = 14;
    Otb otbFontFileHelveticaP14(otbFileHelveticaP14);
    Strike strikeHelveticaP14(kPointSize);
    strikeHelveticaP14.AddRange(otbFontFileHelveticaP14, Strike::eLatin1);
    strikeHelveticaP14.AddRange(otbFontFileHelveticaP14,0x129,0x129);
    strikeHelveticaP14.AddRange(otbFontFileHelveticaP14,0x1e4,0x1e4);

    String unicodeStr(Brn((const TByte*)"\x00\x00\x01\xe4",4), String::eRawUnicode);
    Bws<4> utf8;
    unicodeStr.ToUtf8(utf8);
    rectangle = strikeHelveticaP14.Bounds(utf8,64,16);
    TEST(rectangle.X() == 64);
    TEST(rectangle.Y() == 6);
    TEST(rectangle.Width() == 11);
    TEST(rectangle.Height() == 11);

    unicodeStr.Replace(Brn((const TByte*)"\x00\x00\x01\x29",4));
    unicodeStr.ToUtf8(utf8);
    rectangle = strikeHelveticaP14.Bounds(utf8,64,16);
    TEST(rectangle.X() == 63);
    TEST(rectangle.Y() == 6);
    TEST(rectangle.Width() == 5);
    TEST(rectangle.Height() == 11);

    // Test corner functions
    rectangle.Set(0,0,128,32);
    TEST(rectangle.TopLeft().X() == 0);
    TEST(rectangle.TopLeft().Y() == 0);
    TEST(rectangle.TopRight().X() == 127);
    TEST(rectangle.TopRight().Y() == 0);
    TEST(rectangle.BottomLeft().X() == 0);
    TEST(rectangle.BottomLeft().Y() == 31);
    TEST(rectangle.BottomRight().X() == 127);
    TEST(rectangle.BottomRight().Y() == 31);

    rectangle.Set(10,10,128,32);
    TEST(rectangle.TopLeft().X() == 10);
    TEST(rectangle.TopLeft().Y() == 10);
    TEST(rectangle.TopRight().X() == 137);
    TEST(rectangle.TopRight().Y() == 10);
    TEST(rectangle.BottomLeft().X() == 10);
    TEST(rectangle.BottomLeft().Y() == 41);
    TEST(rectangle.BottomRight().X() == 137);
    TEST(rectangle.BottomRight().Y() == 41);

    rectangle.Set(10,10,118,22);
    TEST(rectangle.TopLeft().X() == 10);
    TEST(rectangle.TopLeft().Y() == 10);
    TEST(rectangle.TopRight().X() == 127);
    TEST(rectangle.TopRight().Y() == 10);
    TEST(rectangle.BottomLeft().X() == 10);
    TEST(rectangle.BottomLeft().Y() == 31);
    TEST(rectangle.BottomRight().X() == 127);
    TEST(rectangle.BottomRight().Y() == 31);

    rectangle.Set(0,0,128,32);
    TEST_THROWS(rectangle.OutsideTopLeft(), AssertionFailed);
    TEST_THROWS(rectangle.OutsideTopRight(), AssertionFailed);
    TEST_THROWS(rectangle.OutsideBottomLeft(), AssertionFailed);
    TEST(rectangle.OutsideBottomRight().X() == 128);
    TEST(rectangle.OutsideBottomRight().Y() == 32);

    rectangle.Set(0,1,128,32);
    TEST_THROWS(rectangle.OutsideTopLeft(), AssertionFailed);
    TEST(rectangle.OutsideTopRight().X() == 128);
    TEST(rectangle.OutsideTopRight().Y() == 0);
    TEST_THROWS(rectangle.OutsideBottomLeft(), AssertionFailed);
    TEST(rectangle.OutsideBottomRight().X() == 128);
    TEST(rectangle.OutsideBottomRight().Y() == 33);

    rectangle.Set(1,0,128,32);
    TEST_THROWS(rectangle.OutsideTopLeft(), AssertionFailed);
    TEST_THROWS(rectangle.OutsideTopRight(), AssertionFailed);
    TEST(rectangle.OutsideBottomLeft().X() == 0);
    TEST(rectangle.OutsideBottomLeft().Y() == 32);
    TEST(rectangle.OutsideBottomRight().X() == 129);
    TEST(rectangle.OutsideBottomRight().Y() == 32);

    rectangle.Set(1,1,126,30);
    TEST(rectangle.OutsideTopLeft().X() == 0);
    TEST(rectangle.OutsideTopLeft().Y() == 0);
    TEST(rectangle.OutsideTopRight().X() == 127);
    TEST(rectangle.OutsideTopRight().Y() == 0);
    TEST(rectangle.OutsideBottomLeft().X() == 0);
    TEST(rectangle.OutsideBottomLeft().Y() == 31);
    TEST(rectangle.OutsideBottomRight().X() == 127);
    TEST(rectangle.OutsideBottomRight().Y() == 31);

    rectangle.Set(10,10,116,20);
    TEST(rectangle.OutsideTopLeft().X() == 9);
    TEST(rectangle.OutsideTopLeft().Y() == 9);
    TEST(rectangle.OutsideTopRight().X() == 126);
    TEST(rectangle.OutsideTopRight().Y() == 9);
    TEST(rectangle.OutsideBottomLeft().X() == 9);
    TEST(rectangle.OutsideBottomLeft().Y() == 30);
    TEST(rectangle.OutsideBottomRight().X() == 126);
    TEST(rectangle.OutsideBottomRight().Y() == 30);

    // Test small rectangle Is Inside this rectangle
    rectangle.Set(10,10,30,10);
    Rectangle rectangleSmall(10,10,10,5);
    TEST(rectangle.IsInside(rectangleSmall));
    rectangleSmall.Set(30,10,10,5);
    TEST(rectangle.IsInside(rectangleSmall));
    rectangleSmall.Set(10,15,10,5);
    TEST(rectangle.IsInside(rectangleSmall));
    rectangleSmall.Set(30,15,10,5);
    TEST(rectangle.IsInside(rectangleSmall));
    rectangleSmall.Set(11,11,10,5);
    TEST(rectangle.IsInside(rectangleSmall));

    rectangleSmall.Set(9,10,10,5);
    TEST(!rectangle.IsInside(rectangleSmall));
    rectangleSmall.Set(10,9,10,5);
    TEST(!rectangle.IsInside(rectangleSmall));
    rectangleSmall.Set(31,10,10,5);
    TEST(!rectangle.IsInside(rectangleSmall));
    rectangleSmall.Set(10,16,10,5);
    TEST(!rectangle.IsInside(rectangleSmall));

    TEST(frameBuffer.Bounds().IsInside(strikeHelveticaP14.Bounds(Brn("123456789abcdefg"),0,16)));
    TEST(!frameBuffer.Bounds().IsInside(strikeHelveticaP14.Bounds(Brn("123456789abcdefgh"),0,16)));

    window.Close();
    TEST_THROWS(frameBuffer.Write(testBmp64x16Tar,65,0),LimitsExceeded);
    TEST_THROWS(frameBuffer.Write(testBmp64x16Tar,0,17),LimitsExceeded);
    TEST_THROWS(frameBuffer.Inverse(testBmp64x16Tar.Bounds(65,0)),LimitsExceeded);
    TEST_THROWS(frameBuffer.Inverse(testBmp64x16Tar.Bounds(0,17)),LimitsExceeded);
    TEST_THROWS(frameBuffer.Clear(testBmp64x16Tar.Bounds(65,0)),LimitsExceeded);
    TEST_THROWS(frameBuffer.Clear(testBmp64x16Tar.Bounds(0,17)),LimitsExceeded);
    TEST_THROWS(frameBuffer.Write(frameBufferLarge,0,0,Rectangle(0,0,256,64)),LimitsExceeded);
    frameBuffer.Write(frameBufferLarge,0,0,Rectangle(0,0,128,32));
    frameBuffer.Write(frameBufferLarge,1,1,Rectangle(0,0,127,31));
    TEST_THROWS(frameBuffer.Write(frameBufferLarge,0,0,Rectangle(0,0,129,32)),LimitsExceeded);
    TEST_THROWS(frameBuffer.Write(frameBufferLarge,0,0,Rectangle(0,0,128,33)),LimitsExceeded);
    frameBuffer.Write(frameBufferSmall,0,0,Rectangle(0,0,64,16));
    TEST_THROWS(frameBuffer.Write(frameBufferSmall,0,0,Rectangle(0,0,65,16)),LimitsExceeded);
    TEST_THROWS(frameBuffer.Write(frameBufferSmall,0,0,Rectangle(0,0,64,17)),LimitsExceeded);
    frameBuffer.Write(frameBuffer,0,0);
    TEST_THROWS(frameBuffer.Write(frameBuffer,1,0),LimitsExceeded);
    TEST_THROWS(frameBuffer.Write(frameBuffer,0,1),LimitsExceeded);

    frameBuffer.Clear();

    // fill frame buffer with various sized bmps using bounds/corner functions/IsInside Funtion
    Rectangle yBounds = testBmp10x10Tar.Bounds(); //10x10

    // fill down
    while (frameBuffer.Bounds().IsInside(yBounds))
    {
        Rectangle xBounds = yBounds;
        // fill accross
        while (frameBuffer.Bounds().IsInside(xBounds))
        {
            frameBuffer.Write(testBmp10x10Tar, xBounds.Origin());
            xBounds.SetOrigin(xBounds.NextRight());
        }
        yBounds.SetOrigin(yBounds.NextDown());
    }
    window.Open();
    Thread::Current().Sleep(1000);

    window.Close();
    frameBuffer.Clear();

    // fill frame buffer with various sized bmps using bounds/corner functions/IsInside Funtion
    yBounds = testBmp64x16Tar.Bounds(); //64x16

    // fill down
    while (frameBuffer.Bounds().IsInside(yBounds))
    {
        Rectangle xBounds = yBounds;
        // fill accross
        while (frameBuffer.Bounds().IsInside(xBounds))
        {
            frameBuffer.Write(testBmp10x10Tar, xBounds.Origin());
            xBounds.SetOrigin(xBounds.NextRight());
        }
        yBounds.SetOrigin(yBounds.NextDown());
    }

    window.Open();
    Thread::Current().Sleep(1000);

    window.Close();
    frameBuffer.Clear();

    // fill frame buffer with various sized bmps using bounds/corner functions/IsInside Funtion
    yBounds = testBmpVsTar.Bounds(); //128x32

    // fill down
    while (frameBuffer.Bounds().IsInside(yBounds))
    {
        Rectangle xBounds = yBounds;
        // fill accross
        while (frameBuffer.Bounds().IsInside(xBounds))
        {
            frameBuffer.Write(testBmp10x10Tar, xBounds.Origin());
            xBounds.SetOrigin(xBounds.NextRight());
        }
        yBounds.SetOrigin(yBounds.NextDown());
    }

    window.Open();
    Thread::Current().Sleep(1000);
}


void Linn::Main()
{
    AppCommon common;
    Debug::SetLevel(Debug::kUi);

#ifdef STANDARD_DISPLAY
    const TUint32 kDisplayBase = 0x400A0000;
    FrameBuffer* frameBufferDisplay = new FrameBuffer(128,32,1);

#ifdef DEFINE_TRACE
    VfdItron* hardwareDisplay = new VfdItron(frameBufferDisplay, kDisplayBase, Priority::kHighest, eDisplayTypeStandardVfd); //display->VfdItron on board (main display)
    VfdViewer* viewerDisplay = new VfdViewer(frameBufferDisplay, Priority::kHighest); //display->viewer (secondary display)  (requires debug level of kUi|kVerbose)
    IDisplay* display = new VfdDual(hardwareDisplay, viewerDisplay);
#else
    IDisplay* display = new VfdItron(frameBufferDisplay, kDisplayBase, Priority::kHighest, eDisplayTypeStandardVfd); //display->VfdItron on board (main display only)
#endif

#else
    FrameBuffer* frameBufferDisplay = new FrameBuffer(192,32,1);
    IDisplay* display = new VfdViewer(frameBufferDisplay, Priority::kHighest);  (requires debug level of kUi|kVerbose)
#endif
    gProduct = common.CreateProduct();
    UiSystem* system = new UiSystem(Priority::kLow, true);
    gUi = new AppUi(*gProduct, display, 0, system, 0, 0, false);
    common.Start();

    gUi->SetEnabled(true);
    gUi->RenderFullScreen().TransitionTimerStop();
    gUi->RenderFullScreen().Clear();

    Runner runner("Test Frame Buffer");
#ifdef STANDARD_DISPLAY
    runner.Add(new FrameBufferTestBasic("Basic Frame buffer functionality"));
    runner.Add(new FrameBufferTestPixel("Pixel by Pixel (set and clear)"));
    runner.Add(new FrameBufferTestBmp("Bmp Files"));
    runner.Add(new FrameBufferTestComposition("Composed with contents of another frame buffer"));
    runner.Add(new FrameBufferTestSizes("Test Various Frame Buffer Sizes"));
    runner.Add(new OtbInternalTest("FrameBufferTest: Otb File Internal Test"));
    runner.Add(new FrameBufferTestOtb("Otb (font) Files Basic Display"));
    runner.Add(new FrameBufferTestOtbExtended("Otb (font) Files: Extended Display"));
    runner.Add(new FrameBufferTestReverseVideo("Reverse Video"));
    runner.Add(new FrameBufferTestDisplayDriver("Test Frame Buffer Display Driver using VFD Hardware"));
    runner.Add(new FrameBufferWindowTest("Test Frame Buffer Window"));
    runner.Add(new FrameBufferTestBounds("Bounds"));
#else
    runner.Add(new FrameBufferWindowTest("Test Frame Buffer Window"));
#endif
    runner.Run();
}

