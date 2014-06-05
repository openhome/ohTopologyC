#include <OpenHome/Ui/Tests/TestScriptHttpReader.h>
#include <OpenHome/Private/SuiteUnitTest.h>
#include <OpenHome/OsWrapper.h>
#include <OpenHome/Private/Arch.h>
#include <OpenHome/Private/Converter.h>
#include <OpenHome/Private/Thread.h>
#include <OpenHome/Ui/Display.h>
#include <OpenHome/Private/Debug.h>

using namespace OpenHome;
using namespace OpenHome::Ui;
using namespace OpenHome::TestFramework;


namespace OpenHome {

namespace Ui {


///////////////////////////////////////////////////////////////////////////////////

class SuiteDisplay : public SuiteUnitTest, public INonCopyable
{
public:
    SuiteDisplay(Environment& aEnv, std::vector<Brn>& aArgs);

    TUint CallbackCount() {return(iCallbackCount);}

private:
    // from SuiteDisplay
    void Setup();
    void TearDown();

    void Test1();
    void Test2();
    void Test3();


    void Callback();

    void LoadFile(const Brx& aFilename, Bwx& aFile);

private:
    Environment& iEnv;
    std::vector<Brn>& iArgs;
    TUint iCallbackCount;
    Bws<4096> iReadPixels;
    Bws<30*1024> file1;
};


} // Ui

} // OpenHome

/////////////////////////////////////////////////////////////


SuiteDisplay::SuiteDisplay(Environment& aEnv, std::vector<Brn>& aArgs)
    :SuiteUnitTest("SuiteDisplay")
    ,iEnv(aEnv)
    ,iArgs(aArgs)
    ,iCallbackCount(0)
    ,iReadPixels(Brx::Empty())
{
    if(aArgs.size()<2)
    {
        aArgs.push_back(Brn("--path"));
        aArgs.push_back(Brn("~eamonnb/TestDataFiles"));
    }

    AddTest(MakeFunctor(*this, &SuiteDisplay::Test1));
    AddTest(MakeFunctor(*this, &SuiteDisplay::Test2));
    AddTest(MakeFunctor(*this, &SuiteDisplay::Test3));
}


void SuiteDisplay::LoadFile(const Brx& aFilename, Bwx& aFile)
{
    std::vector<Brn>& args(iArgs);
    args.push_back(Brn("--file"));
    args.push_back(Brn(aFilename));

    TestScriptHttpReader reader(iEnv, args);

    aFile.SetBytes(0);
    TBool eof = false;

    do
    {
        try
        {
            aFile.Append(reader.Read(1));
        }
        catch(ReaderError)
        {
            eof = true;
        }
    } while(!eof);


    //Log::Print("aFile.Bytes()=%d \n", aFile.Bytes());
}



void SuiteDisplay::Setup()
{
}


void SuiteDisplay::TearDown()
{

}

void SuiteDisplay::Callback()
{
    iCallbackCount++;
}




void SuiteDisplay::Test1() // FrameBuffer Construction
{
    FrameBuffer frameBuffer(128, 32);
    TEST(frameBuffer.Width() == 128);
    TEST(frameBuffer.Height() == 32);
    frameBuffer.Pixels(iReadPixels);
    TEST(iReadPixels.Bytes() == ((128>>3)*32));

    //OpenHome::Log::Print("iReadPixels.Bytes()= %d \n", iReadPixels.Bytes());

    FrameBuffer frameBufferLarge(256, 64);
    TEST(frameBufferLarge.Width() == 256);
    TEST(frameBufferLarge.Height() == 64);
    frameBufferLarge.Pixels(iReadPixels);
    TEST(iReadPixels.Bytes() == ((256>>3)*64));

    FrameBuffer frameBufferSmall(64, 16);
    TEST(frameBufferSmall.Width() == 64);
    TEST(frameBufferSmall.Height() == 16);
    frameBufferSmall.Pixels(iReadPixels);
    TEST(iReadPixels.Bytes() == ((64>>3)*16));

    TEST_THROWS(FrameBuffer(0, 32),AssertionFailed);
    TEST_THROWS(FrameBuffer(127, 32),AssertionFailed);
    TEST_THROWS(FrameBuffer(128, 0),AssertionFailed);

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


void SuiteDisplay::Test2() // FrameWriters/FrameReaders
{
    FrameBuffer* frameBuffer = new FrameBuffer(128, 32);

    FrameWriter* writer1 = frameBuffer->CreateWriter();
    FrameReader* reader1 = frameBuffer->CreateReader(MakeFunctor(*this, &SuiteDisplay::Callback));

    TEST_THROWS(writer1->Write(Brn("test")), AssertionFailed); // can't write until locked
    TEST_THROWS(writer1->Unlock(), AssertionFailed); // can't unlock until locked
    writer1->Lock();
    TEST_THROWS(writer1->Lock(), AssertionFailed); // can't lock twice

    writer1->Write(Brn("test1"));

    TEST(CallbackCount()==0);
    TEST(!iReadPixels.Equals(Brn("test1"))); // not yet...
    writer1->Unlock();
    TEST(CallbackCount()==1);
    reader1->Read(iReadPixels);
    TEST(iReadPixels.Equals(Brn("test1")));

    // two readers
    FrameReader* reader2 = frameBuffer->CreateReader(MakeFunctor(*this, &SuiteDisplay::Callback));
    TEST(CallbackCount()==1);

    writer1->Lock();
    writer1->Unlock();
    TEST(CallbackCount()==3);

    reader2->Read(iReadPixels);
    TEST(iReadPixels.Equals(Brn("test1")));

    writer1->Lock();
    writer1->Write(Brn("test1xxxx"));
    writer1->Unlock();
    TEST(CallbackCount()==5);
    reader1->Read(iReadPixels);
    TEST(iReadPixels.Equals(Brn("test1xxxx")));
    reader2->Read(iReadPixels);
    TEST(iReadPixels.Equals(Brn("test1xxxx")));

    // two writers
    FrameWriter* writer2 = frameBuffer->CreateWriter();

    writer1->Lock();
    writer1->Write(Brn("test1234"));
    reader1->Read(iReadPixels);
    TEST(iReadPixels.Equals(Brn("test1xxxx")));
    reader2->Read(iReadPixels);
    TEST(iReadPixels.Equals(Brn("test1xxxx")));
    writer2->Lock();
    TEST(CallbackCount()==5);
    writer1->Write(Brn("test5678"));
    writer1->Unlock();
    TEST(CallbackCount()==5);
    reader1->Read(iReadPixels);
    TEST(iReadPixels.Equals(Brn("test1xxxx")));
    reader2->Read(iReadPixels);
    TEST(iReadPixels.Equals(Brn("test1xxxx")));
    writer2->Write(Brn("test000"));

    writer2->Unlock();
    TEST(CallbackCount()==7);
    reader1->Read(iReadPixels);
    TEST(iReadPixels.Equals(Brn("test000")));
    reader2->Read(iReadPixels);
    TEST(iReadPixels.Equals(Brn("test000")));

    delete frameBuffer;
    delete writer1;
    delete writer2;
    delete reader1;
    delete reader2;
}


void SuiteDisplay::Test3() // OtbInternalTest
{
    //TestScriptHttpReader reader(iEnv, Brn("~eamonnb/CourierP16TestOnly.otb"));

//    Tars fontTar(uTestFrameBufferTar);  // get font file (otb) from store (single file stored in tar file)
//    Brn fontFile(fontTar.Find(Brn("CourierP16TestOnly.otb")));

    Bwh fontFile(30*1024);
    LoadFile(Brn("CourierP16TestOnly.otb"), fontFile);

/*
    Log::Print("CourierP16TestOnly.otb: \n");

    for(TUint i=0; i<100; i++)
    {
        Log::Print("%x ", fontFile[i]);
    }
*/


    //Test OffsetTable
    TEST(Converter::BeUint32At(fontFile, 0)     == 0x00010000); //      Version         4 bytes     0x00010000 for version 1.0
    TEST(Converter::BeUint16At(fontFile, 4)     == 0x0010);     //      NumberOfTables  2 bytes     number of tables
    TEST(Converter::BeUint16At(fontFile, 6)     == 0x0100);     //      SearchRange     2 bytes     (Maximum power of 2 <= numberOfTables) x 16
    TEST(Converter::BeUint16At(fontFile, 8)     == 0x0004);     //      EntrySelector   2 bytes     Log2(Maximum power of 2 <= numberOfTables)
    TEST(Converter::BeUint16At(fontFile, 10)    == 0x0000);     //      RangeShift      2 bytes     numberOfTables x (16 - searchRange)

    const Brn kOtbTableCmap("cmap");
    const Brn kOtbTableEblc("EBLC");
    const Brn kOtbTableEbdt("EBDT");
    const TUint kPointSize16 = 16;

    //First we need to find the index for the unicode value into the glyph data.
    //We get this from the character to glyph mapping table (cmap)

    //cmap offset table at offset 0x7c from start of file
    //Find the cmap offset table, return the offset (0x3b0) and length (0x1ca)
    const Brn cmap = Otb::OtbTable(fontFile, kOtbTableCmap);
    TEST(TUint32(cmap.Ptr()) == (TUint32(fontFile.Ptr()) + 0x3b0));

    //cmap subtable at 0x3c4 - Plaform id = 0x0003 Encoding id = 0x0003 0ffset = 0x0000001c
    const Brn cmapSubTable = Otb::OtbCmapSubTable(cmap, eUnicode);
    TEST(TUint32(cmapSubTable.Ptr()) == (TUint32(fontFile.Ptr()) + 0x3cc));

    //Encoding subtable tests (Encoding table at 0x3cc)
    TEST(Converter::BeUint16At(cmapSubTable, 0) == 0x0004);    //encoding table format
    TEST(Converter::BeUint16At(cmapSubTable, 2) == 0x00a8);    //length
    TEST(Converter::BeUint16At(cmapSubTable, 6) == 0x0026);    //segCount << 1 (segmentCount = 19)

    {
    //endCount begins 0x007f, 0x00ff..
    //startCount beings 0x0020, 0x00a0..
    //looking for unicode 'e' = 0x65 lies between 0x20 and 0x7f which is index 0 into the array
    //the range offset is 0 which means the glyph index formula is simple, add glyph code to delta and take modulo 65536
    //of the result -> 0x65 + 0xFFE3 = 0x10048. 0x10048 % 0x10000 = 0x48.
    TUint glyphIndex = Otb::OtbCmapGlyphIndex(cmapSubTable, TUint('e'));
    TEST(glyphIndex == 0x48);

    //We now need to find the position in the file of the font data of the desired font size.
    //This information is stored in the Embedded Bitmap Location Table (EBLC)

    const Brn eblc = Otb::OtbTable(fontFile, kOtbTableEblc);
    TEST(TUint32(eblc.Ptr()) == (TUint32(fontFile.Ptr()) + 0xba0));
    const Brn eblcBitmapTable = Otb::EblcBitmapTable(eblc, kPointSize16);
    TEST(TUint32(eblcBitmapTable.Ptr()) == (TUint32(fontFile.Ptr()) + 0xba8));
    const Brn eblcSubTableArray = Otb::EblcSubTableArray(eblc, kPointSize16, glyphIndex);
    TEST(TUint32(eblcSubTableArray.Ptr()) == (TUint32(fontFile.Ptr()) + 0xc00));

    //We now need to find the data in the Embedded Bitmap Data Table (EBDT)

    const Brn ebdt = Otb::OtbTable(fontFile, kOtbTableEbdt);
    TEST(TUint32(ebdt.Ptr()) == (TUint32(fontFile.Ptr()) + 0xe4c));
    #define EBLC_SUBTABLE_SIZE_WITHOUT_DATA 16
    Brn subTableNoData(eblc.Ptr() + Converter::BeUint32At(eblcBitmapTable, 0) + Converter::BeUint32At(eblcSubTableArray, 4), EBLC_SUBTABLE_SIZE_WITHOUT_DATA);
    TEST(TUint32(subTableNoData.Ptr()) == (TUint32(eblc.Ptr()) + 0x38 + 0xd0));//0xca8 from the start of the file

    //The purpose of all the preceeding code and test cases is to extract from the font the following three values
    const GlyphMetrics* glyphMetrics = 0;
    //EGlyphAlignment glyphAlignment = eInvalid;
    const TByte* pixels = 0;

    TEST(Converter::BeUint16At(subTableNoData, 0) == 1); //index format (1: variable glyph metrics for each glyph stored in EBDT)
    #define INDEX_SUBHEADER_SIZE 8
    //We deduct the index value of the first supported glyphcode in the range from our glyph index to get a relative index
    //then use the index to with the offset array  (glyphDataOffset = offsetArray[index])
    TUint offsetArraySize = (Converter::BeUint16At(eblcSubTableArray, 2) - Converter::BeUint16At(eblcSubTableArray, 0) + 1);
    Brn offsetArray(subTableNoData.Ptr() + INDEX_SUBHEADER_SIZE, offsetArraySize);
    TUint32 glyphDataOffset = Converter::BeUint32At(offsetArray, (glyphIndex - Converter::BeUint16At(eblcSubTableArray, 0)) * sizeof (TUint32));
    TEST(glyphDataOffset == 0x6b);
    #define SMALL_METRICS_SIZE 5
    //The EBDT table has a 4 byte header.  0x339 + 0x12 + 0x5
    pixels = (TByte*) &ebdt.At(Converter::BeUint32At(subTableNoData, 4) + glyphDataOffset + SMALL_METRICS_SIZE);
    TEST(TUint32(pixels) == (TUint32(fontFile.Ptr()) + 0x11F5));

    glyphMetrics = (GlyphMetrics*) (ebdt.Ptr() + Converter::BeUint32At(subTableNoData, 4) + glyphDataOffset);
    TEST(glyphMetrics->iHeight      == 7);
    TEST(glyphMetrics->iWidth       == 7);
    TEST(glyphMetrics->iBearingX    == 1);
    TEST(glyphMetrics->iBearingY    == 6);
    TEST(glyphMetrics->iAdvance     == 9);

    //Ensure the alignment of data is as expected
    TEST(Converter::BeUint16At(subTableNoData, 2) == 2); // image format 2: small metrics, bit alligned image data (EBDT)
    //glyphAlignment = eBitAligned;
    }

    {
    //Repeat for C -> 0x43 + 0xFFE3 = 0x10026. 0x10026 % 0x10000 = 0x26.
    TUint glyphIndex = Otb::OtbCmapGlyphIndex(cmapSubTable, TUint('C'));
    TEST(glyphIndex == 0x26);

    //We now need to find the position in the file of the font data of the desired font size.  This information is stored
    //in the Embedded Bitmap Location Table (EBLC)
    const Brn eblc = Otb::OtbTable(fontFile, kOtbTableEblc);
    TEST(TUint32(eblc.Ptr()) == (TUint32(fontFile.Ptr()) + 0xba0));
    const Brn eblcBitmapTable = Otb::EblcBitmapTable(eblc, kPointSize16);
    TEST(TUint32(eblcBitmapTable.Ptr()) == (TUint32(fontFile.Ptr()) + 0xba8));
    const Brn eblcSubTableArray = Otb::EblcSubTableArray(eblc, kPointSize16, glyphIndex);
    TEST(TUint32(eblcSubTableArray.Ptr()) == (TUint32(fontFile.Ptr()) + 0xbf8));

    //We now need to find the data in the Embedded Bitmap Data Table (EBDT)
    const Brn ebdt = Otb::OtbTable(fontFile, kOtbTableEbdt);
    TEST(TUint32(ebdt.Ptr()) == (TUint32(fontFile.Ptr()) + 0xe4c));
    #define EBLC_SUBTABLE_SIZE_WITHOUT_DATA 16
    Brn subTableNoData(eblc.Ptr() + Converter::BeUint32At(eblcBitmapTable, 0) + Converter::BeUint32At(eblcSubTableArray, 4), EBLC_SUBTABLE_SIZE_WITHOUT_DATA);
    TEST(TUint32(subTableNoData.Ptr()) == (TUint32(eblc.Ptr()) + 0x38 + 0xbc));//0xc94 from the start of the file

    //The purpose of all the preceeding code and test cases is to extract from the font the following three values
    const GlyphMetrics* glyphMetrics = 0;
    //EGlyphAlignment glyphAlignment = eInvalid;
    const TByte* pixels = 0;

    TEST(Converter::BeUint16At(subTableNoData, 0) == 2); //index format (2: identical glyph metrics for each glyph in range)
    #define INDEX_SUBHEADER_SIZE 8
    #define BIG_METRICS_SIZE 8
    #define IMAGE_SIZE 4
    Brn indexFormat2Data(subTableNoData.Ptr() + INDEX_SUBHEADER_SIZE, IMAGE_SIZE + BIG_METRICS_SIZE);
    TUint32 imageSize = Converter::BeUint32At(indexFormat2Data, 0);
    TEST(imageSize == 0xe);
    glyphMetrics = (GlyphMetrics*) (indexFormat2Data.Ptr() + IMAGE_SIZE);

    //We deduct the index value of the first supported glyphcode in the range from our glyph index to get a relative index
    //then use the index to with the offset array  (glyphDataOffset = offsetArray[index])
    //(0x26 - 0x0d) * e = 0x15e
    TUint32 glyphDataOffset = ((glyphIndex - Converter::BeUint16At(eblcSubTableArray, 0)) * imageSize);
    TEST(glyphDataOffset == 0x15e);

    //The EBDT table has a 4 byte header. 0x8b +  0x15e = 0x1e9
    pixels = (TByte*) &ebdt.At(Converter::BeUint32At(subTableNoData, 4) + glyphDataOffset);
    TEST(TUint32(pixels) == (TUint32(fontFile.Ptr()) + 0x1035));

    glyphMetrics = (GlyphMetrics*) &indexFormat2Data.At(4);
    TEST(glyphMetrics->iHeight      == 12);
    TEST(glyphMetrics->iWidth       == 9);
    TEST(glyphMetrics->iBearingX    == 0);
    TEST(glyphMetrics->iBearingY    == 8);
    TEST(glyphMetrics->iAdvance     == 9);

    //Ensure the alignment of data is as expected
    TEST(Converter::BeUint16At(subTableNoData, 2) == 5);  //image format 5: metrics in EBLC, bit alligned image data only (EBDT)
    //glyphAlignment = eBitAligned;
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



///////////////////////////////////////////////////////////////////////////

void TestDisplay(Environment& aEnv, std::vector<Brn>& aArgs)
{
    Runner runner("FrameBuffer tests\n");
    runner.Add(new SuiteDisplay(aEnv, aArgs));
    runner.Run();
}



