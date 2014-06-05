#include <Linn/Ui/Drawing.h>
#include <Linn/Debug.h>

using namespace Linn;
using namespace Linn::Ui;
using namespace Linn::Control;

//  BMP File Format
//
//  Name                Size            Offset      Description
//
//  FileHeader          14 bytes        0           Windows Structure: BITMAPFILEHEADER
//
//      Signature       2 bytes         0           'BM'
//      FileSize        4 bytes         2           File size in bytes
//      reserved        4 bytes         6           unused (=0)
//      DataOffset      4 bytes         10          File offset to Raster Data
//
//  InfoHeader          40 bytes        14          Windows Structure: BITMAPINFOHEADER
//
//      Size            4 bytes         14          Size of InfoHeader =40
//      Width           4 bytes         18          Bitmap Width (pixels)
//      Height          4 bytes         22          Bitmap Height (pixles)
//      Planes          2 bytes         24          Number of Planes (=1)
//      BitCount        2 bytes         26          Bits per Pixel
//                                                  1 = monochrome palette. NumColors = 1
//                                                  4 = 4bit palletized. NumColors = 16
//                                                  8 = 8bit palletized. NumColors = 256
//                                                  16 = 16bit RGB. NumColors = 65536 (?)
//                                                  24 = 24bit RGB. NumColors = 16M
//      Compression     4 bytes         30          Type of Compression
//                                                  0 = BI_RGB   no compression
//                                                  1 = BI_RLE8 8bit RLE encoding
//                                                  2 = BI_RLE4 4bit RLE encoding
//      ImageSize       4 bytes         34          (compressed) Size of Image
//                                                  It is valid to set this =0 if Compression = 0
//      XpixelsPerM     4 bytes         38          horizontal resolution: Pixels/meter
//      YpixelsPerM     4 bytes         42          vertical resolution: Pixels/meter
//      ColorsUsed      4 bytes         46          Number of actually used colors
//      ColorsImportant 4 bytes         50          Number of important colors
//                                                  0 = all
//
//  ColorTable          4 * NumColors               present only if Info.BitsPerPixel <= 8
//
//      Red             1 byte                      Red intensity
//      Green           1 byte                      Green intensity
//      Blue            1 byte                      Blue intensity
//      reserved        1 byte                      unused (=0)
//
//  Raster              n bytes                     The pixel data

Bmp::Bmp(const Brx& aFile)
{
    // get bmp data and metrics from file
    iWidth = aFile.LeUint32At(18);
    iHeight = aFile.LeUint32At(22);
    iBitsPerPixel = aFile.LeUint16At(28);
    iPixels.Set(aFile.Ptr() + aFile.LeUint32At(10), (aFile.Bytes() - aFile.LeUint32At(10)));

    ASSERT(iWidth > 0); // insure bitmap dimensions are valid
    ASSERT(iHeight > 0); // insure bitmap dimensions are valid
    ASSERT(iBitsPerPixel == 1); // no greyscale or colors currently supported
    ASSERT(iPixels.Bytes() > 0); // insure bitmap data is valid

    LOG(kUi,"Bmp Dimensions: %d(w), %d(h), %d(bpp)\n",iWidth, iHeight, iBitsPerPixel);
}


Rectangle Bmp::Bounds(TUint aX, TUint aY) const
{
    return Bounds(Point(aX, aY));
}

Rectangle Bmp::Bounds(const Point& aPoint) const
{
    return Rectangle(aPoint, iWidth, iHeight);
}

// Rectangle

Rectangle::Rectangle(TUint aX, TUint aY, TUint aWidth, TUint aHeight)
:iOrigin(aX,aY), iWidth(aWidth), iHeight(aHeight)
{
    ASSERT(iWidth > 0); // insure rectangle dimensions are valid
    ASSERT(iHeight > 0); // insure rectangle dimensions are valid
}


Rectangle::Rectangle(const Point& aOrigin, TUint aWidth, TUint aHeight)
:iOrigin(aOrigin), iWidth(aWidth), iHeight(aHeight)
{
    ASSERT(iWidth > 0); // insure rectangle dimensions are valid
    ASSERT(iHeight > 0); // insure rectangle dimensions are valid
}


void Rectangle::Set(TUint aX, TUint aY, TUint aWidth, TUint aHeight)
{
    iOrigin.Set(aX, aY);
    iWidth = aWidth;
    iHeight = aHeight;
    ASSERT(iWidth > 0); // insure rectangle dimensions are valid
    ASSERT(iHeight > 0); // insure rectangle dimensions are valid
}


void Rectangle::Set(const Point& aOrigin, TUint aWidth, TUint aHeight)
{
    iOrigin.Set(aOrigin);
    iWidth = aWidth;
    iHeight = aHeight;
    ASSERT(iWidth > 0); // insure rectangle dimensions are valid
    ASSERT(iHeight > 0); // insure rectangle dimensions are valid
}


void Rectangle::SetWidth(TUint aWidth)
{
    iWidth = aWidth;
    ASSERT(iWidth > 0); // insure rectangle dimensions are valid
}


void Rectangle::SetHeight(TUint aHeight)
{
    iHeight = aHeight;
    ASSERT(iHeight > 0); // insure rectangle dimensions are valid
}


Point Rectangle::TopLeft() const
{
    return iOrigin;
}


Point Rectangle::TopRight() const
{
    return Point(iOrigin.X() + iWidth - 1, iOrigin.Y());
}


Point Rectangle::BottomLeft() const
{
    return Point(iOrigin.X(), iOrigin.Y() + iHeight - 1);
}


Point Rectangle::BottomRight() const
{
    return Point(iOrigin.X() + iWidth - 1, iOrigin.Y() + iHeight - 1);
}


Point Rectangle::OutsideTopLeft() const
{
    TUint x = iOrigin.X();
    TUint y = iOrigin.Y();
    ASSERT(x > 0);
    ASSERT(y > 0);
    return Point(x - 1, y - 1);
}


Point Rectangle::OutsideTopRight() const
{
    TUint x = iOrigin.X();
    TUint y = iOrigin.Y();
    ASSERT(y > 0);
    return Point(x + iWidth, y - 1);
}


Point Rectangle::OutsideBottomLeft() const
{
    TUint x = iOrigin.X();
    TUint y = iOrigin.Y();
    ASSERT(x > 0);
    return Point(x - 1, y + iHeight);
}


Point Rectangle::OutsideBottomRight() const
{
    TUint x = iOrigin.X();
    TUint y = iOrigin.Y();
    return Point(x + iWidth, y + iHeight);
}


Point Rectangle::NextRight() const
{
    return Point(iOrigin.X() + iWidth, iOrigin.Y());
}

Point Rectangle::NextDown() const
{
    return Point(iOrigin.X(), iOrigin.Y() + iHeight);
}

TBool Rectangle::IsInside(const Point& aPoint) const
{
    TUint x = iOrigin.X();
    TUint y = iOrigin.Y();
    if ( (aPoint.X() < x) || (aPoint.X() > x + iWidth - 1) ||
         (aPoint.Y() < y) || (aPoint.Y() > y + iHeight - 1) )
    {
        return false;
    }
    return true;
}

TBool Rectangle::IsInside(const Rectangle& aRectangle) const
{
    if (IsInside(aRectangle.TopLeft())) {
        return (IsInside(aRectangle.BottomRight()));
    }
    return false;
}

// Vector

Vector::Vector(TInt aX, TInt aY) : iX(aY),iY(aY)
{
}

void Vector::Set(TInt aX, TInt aY)
{
    iX = aX;
    iY = aY;
}

void Vector::Set(const Vector& aVector)
{
    iX = aVector.iX;
    iY = aVector.iY;
}

// Point

Point::Point(TUint aX, TUint aY) :iX(aX), iY(aY)
{
}

void Point::Set(const Point& aPoint)
{
    iX = aPoint.iX;
    iY = aPoint.iY;
}


Point& Point::Add(const Vector& aVector)
{
    return(Add(aVector.X(), aVector.Y()));
}


Point& Point::Add(TInt aX, TInt aY)
{
    iX += aX;
    iY += aY;
    return(*this);
}


FrameBuffer::FrameBuffer(TUint aWidth, TUint aHeight, TUint aBitsPerPixel)
    : iWidth(aWidth), iHeight(aHeight), iBitsPerPixel(aBitsPerPixel), iPixels(PixelBytes())
{
    ASSERT((iWidth&0x1f) == 0); // frame buffer width has to be word aligned
    ASSERT(iWidth > 0); // ensure frame buffer dimensions are valid
    ASSERT(iHeight > 0); // ensure frame buffer dimensions are valid
    ASSERT(iBitsPerPixel == 1); // no greyscale or colors currently supported

    LOG(Debug::kUi,"FrameBuffer(%d, %d, %d)\n", aWidth, aHeight, aBitsPerPixel);
    iPixels.SetBytes(iPixels.MaxBytes());
    iPixels.Fill(0x00);
}


Rectangle FrameBuffer::Bounds(TUint aX, TUint aY) const
{
    Rectangle bounds(aX,aY,iWidth,iHeight);
    return bounds;
}


TUint FrameBuffer::PixelBytes() const
{
    TUint bytes = (iWidth + 7) >> 3;
    bytes *= iHeight;
    bytes *= iBitsPerPixel;
    return (bytes);
}


void FrameBuffer::Clear()
{
    iPixels.Fill(0x00);
    Refresh();
}


void FrameBuffer::Fill(TByte aFillByte)
{
    iPixels.Fill(aFillByte);
    Refresh();
}


void FrameBuffer::Fill(TUint32 aFillWord)
{
    TUint32* inPixels = (TUint32*)Pixels().Ptr();
    for (TUint i = 0; i < PixelBytes()>>2; i++) { *inPixels++ = Arch::BigEndian4(aFillWord); }
    Refresh();
}


// standard bit mask (index 32 = index 0: saves calculation)
const TUint32 FrameBuffer::kMask[33] = {0x00,0x01,0x03,0x07,0x0f,0x1f,0x3f,0x7f,0xff,0x1ff,0x3ff,0x7ff,0xfff,0x1fff,0x3fff,0x7fff,0xffff,0x1ffff,0x3ffff,0x7ffff,0xfffff,0x1fffff,0x3fffff,0x7fffff,0xffffff,0x1ffffff,0x3ffffff,0x7ffffff,0xfffffff,0x1fffffff,0x3fffffff,0x7fffffff,0x00};
// standard bit mask inverted (ie ~ operator already invoked - ones complement) (index 32 = index 0: saves calculation)
const TUint32 FrameBuffer::kMaskInv[33] = {0xffffffff,0xfffffffe,0xfffffffc,0xfffffff8,0xfffffff0,0xffffffe0,0xffffffc0,0xffffff80,0xffffff00,0xfffffe00,0xfffffc00,0xfffff800,0xfffff000,0xffffe000,0xffffc000,0xffff8000,0xffff0000,0xfffe0000,0xfffc0000,0xfff80000,0xfff00000,0xffe00000,0xffc00000,0xff800000,0xff000000,0xfe000000,0xfc000000,0xf8000000,0xf0000000,0xe0000000,0xc0000000,0x80000000,0xffffffff};

void FrameBuffer::Write(const Bmp& aBitmap, TUint aX, TUint aY)
{
    if (!(Bounds().IsInside(aBitmap.Bounds(aX,aY)))) { THROW(LimitsExceeded); } // test bmp rectangle stays inside buffer

    // bmp data is word aligned
    // bmp lines are stored in reverse (bottom line first, top line last - need to flip - decrement inPtr)
    // similar to framebuffer to framebuffer with rectangle(0,0,bmp width, bmp height) - inBitOffset always 0
    TUint widthWordMasked = aBitmap.Width()&0x1f;
    TUint inLineWidthInWords = (aBitmap.Width()>>5) + (widthWordMasked ? 1:0);
    TUint inLineWidthInBytes = inLineWidthInWords<<2;
    TUint32* inPtr = (TUint32*)(aBitmap.Pixels().Ptr() + (aBitmap.Height()*inLineWidthInBytes) - inLineWidthInBytes);

    TUint outLineWidthInWords = Width()>>5;
    TUint outYOffsetInBytes = aY * (Width()>>3); //Offset from start of frame buffer (Y Direction Only)
    TUint outXOffsetInWords = aX>>5; //Offset from start of frame buffer (X Direction Only)
    TUint32* outPtr = (TUint32*)(Pixels().Ptr() + outYOffsetInBytes + (outXOffsetInWords<<2));
    TUint outBitOffset = aX - (outXOffsetInWords<<5);
    TUint outBitOffset32 = 32 - outBitOffset;

    TUint height = aBitmap.Height();
    TUint rightEdgeBit = aX + aBitmap.Width();
    TUint rightHandMaskIndex = ((rightEdgeBit&0xffe0)+32) - rightEdgeBit;

    TUint inWidthWords = inLineWidthInWords; // inBitOffset always 0
    TUint outWidthWords = inLineWidthInWords + (((widthWordMasked ? widthWordMasked:32)+outBitOffset)>32 ? 1:0);
    TBool extraOutputWord = (outWidthWords > inWidthWords);
    TUint widthWords = extraOutputWord ? inWidthWords:outWidthWords; // width to use in inner loop
    inLineWidthInWords = inLineWidthInWords<<1; //use for inPtr decrement - go back 2 lines from end location

    TUint width;
    TUint32 inWord, inWordPrev, rightHandMask;

    // all three bit shift algorithms combined (blitting)
    // shift left algorithm: outWidthWords will always be <= inWidthWords (inBitOffset > outBitOffset)
    // shift right algorithm: inputWidthWords will always be <= outWidthWords (inBitOffset < outBitOffset)
    // no shift algorithm: inputWidthWords will always be = outWidthWords (inBitOffset = outBitOffset)

    while (height--) //rows
    {
        rightHandMask = Arch::BigEndian4(*(outPtr + (outWidthWords-1))) & kMask[rightHandMaskIndex];
        // keep left hand original output data in tact
        inWord = (TUint32)(((TUint64)(Arch::BigEndian4(*outPtr)))>>outBitOffset32);

        width = widthWords; //use smallest of in or out- left: out, right: in, none: in = out
        while (width--) //columns
        {
            inWordPrev = inWord;
            inWord = (TUint32)((Arch::BigEndian8(*(TUint64*)inPtr++))>>32); // inBitOffset32 always 32
            *outPtr++ = Arch::BigEndian4(((TUint32)(((TUint64)(inWordPrev))<<outBitOffset32)) | (inWord>>outBitOffset));
        }
        //spillover into extra output word (right only)
        if (extraOutputWord) { *outPtr++ = Arch::BigEndian4(((TUint32)(((TUint64)(inWord))<<outBitOffset32))); }

        // keep right hand original output data in tact
        *(outPtr-1) = Arch::BigEndian4((Arch::BigEndian4(*(outPtr-1)) & kMaskInv[rightHandMaskIndex]) | rightHandMask);

        inPtr -= inLineWidthInWords;
        outPtr += outLineWidthInWords - outWidthWords;
    }
    Refresh();
}


void FrameBuffer::Write(const Bmp& aBitmap, const Point& aPoint)
{
    Write(aBitmap, aPoint.X(), aPoint.Y());
}


void FrameBuffer::Write(const Glyph& aGlyph, TUint aX, TUint aY)
{
    if (!((aX + aGlyph.BearingX()) >= 0)) { THROW(LimitsExceeded); } //left (glyph only: BearingX < 0)
    if (!((TInt)aY >= (aGlyph.BearingY()-1))) { THROW(LimitsExceeded); } //top (glyph only: character would extend past the buffer top)
    if (!(((TInt)iHeight - (TInt)aY) > ((TInt)aGlyph.Height() - (TInt)aGlyph.BearingY()))) { THROW(LimitsExceeded); } //bottom (given character would drop below buffer limit: height)
    if (!((aX + aGlyph.Advance()) <= iWidth)) { THROW(LimitsExceeded); } //right (character would extend past the buffer edge: width)

    aX += aGlyph.BearingX(); // move origin to start of pixels in x direction (top-> bottom/left->right)
    aY -= (aGlyph.BearingY()-1); // move origin to start of pixels in y direction (top-> bottom/left->right)

    if (!(aX < iWidth)) { THROW(LimitsExceeded); } // test x,y point starts in buffer
    if (!(aY < iHeight)) { THROW(LimitsExceeded); } // test x,y point starts in buffer

    // glyph pixel Data is bit aligned (not word aligned) - stored in correct orientation
    // similar to framebuffer to framebuffer with rectangle(0,0,glyph width, glyph height) - inBitOffset always 0

    TUint32* inPtr = (TUint32*)(aGlyph.Pixels().Ptr());
    TUint inBitOffset32 = 32; // glyph data always starts at 0,0 - inBitOffset always 0

    TUint outLineWidthInWords = Width()>>5;
    TUint outYOffsetInBytes = aY * (Width()>>3); //Offset from start of frame buffer (Y Direction Only)
    TUint outXOffsetInWords = aX>>5; //Offset from start of frame buffer (X Direction Only)
    TUint32* outPtr = (TUint32*)(Pixels().Ptr() + outYOffsetInBytes + (outXOffsetInWords<<2));
    TUint outBitOffset = aX - (outXOffsetInWords<<5);
    TUint outBitOffset32 = 32 - outBitOffset;

    TUint height = aGlyph.Height();
    TUint rightEdgeBit = aX + aGlyph.Width();
    TUint rightHandMaskIndex = ((rightEdgeBit&0xffe0)+32) - rightEdgeBit;

    TUint widthWordMasked = aGlyph.Width()&0x1f;
    TUint widthWords = (aGlyph.Width()>>5) + (widthWordMasked ? 1:0);
    TUint inWidthWords = widthWords; // inBitOffset always 0
    TUint outWidthWords = widthWords + (((widthWordMasked ? widthWordMasked:32)+outBitOffset)>32 ? 1:0);
    TBool extraOutputWord = (outWidthWords > inWidthWords);
    widthWords = extraOutputWord ? inWidthWords:outWidthWords; // width to use in inner loop

    TUint width;
    TUint32 inWord, inWordPrev, rightHandMask;

    // all three bit shift algorithms combined (blitting)
    // shift left algorithm: outWidthWords will always be <= inWidthWords (inBitOffset > outBitOffset)
    // shift right algorithm: inputWidthWords will always be <= outWidthWords (inBitOffset < outBitOffset)
    // no shift algorithm: inputWidthWords will always be = outWidthWords (inBitOffset = outBitOffset)

    while (height--) //rows
    {
        rightHandMask = Arch::BigEndian4(*(outPtr + (outWidthWords-1))) & kMask[rightHandMaskIndex];
        // keep left hand original output data in tact
        inWord = (TUint32)(((TUint64)(Arch::BigEndian4(*outPtr)))>>outBitOffset32);

        width = widthWords; //use smallest of in or out- left: out, right: in, none: in = out
        while (width--) //columns
        {
            inWordPrev = inWord;
            inWord = (TUint32)((Arch::BigEndian8(*(TUint64*)inPtr))>>inBitOffset32);
            if (aGlyph.Width() > inBitOffset32) { inPtr++; inBitOffset32 += 32; }
            inBitOffset32 -= aGlyph.Width();
            *outPtr++ = Arch::BigEndian4(((TUint32)(((TUint64)(inWordPrev))<<outBitOffset32)) | (inWord>>outBitOffset));
        }
        //spillover into extra output word (right only)
        if (extraOutputWord) { *outPtr++ = Arch::BigEndian4(((TUint32)(((TUint64)(inWord))<<outBitOffset32))); }

        // keep right hand original output data in tact
        *(outPtr-1) = Arch::BigEndian4((Arch::BigEndian4(*(outPtr-1)) & kMaskInv[rightHandMaskIndex]) | rightHandMask);

        outPtr += outLineWidthInWords - outWidthWords;
    }
    LOG(Debug::kUi|Debug::kVerbose,"Glyph Write: (%dx,%dy) , (%dw,%dh)\n", aX, aY, aGlyph.Width(), aGlyph.Height());
}


void FrameBuffer::Write(const Glyph& aGlyph, const Point& aPoint)
{
    Write(aGlyph, aPoint.X(), aPoint.Y());
}


void FrameBuffer::Write(const FrameBuffer& aFrameBuffer, TUint aX, TUint aY, const Rectangle& aRectangle)
{
    if(!(aFrameBuffer.Bounds().IsInside(aRectangle))) { THROW(LimitsExceeded); } // test rectangle stays inside aFrameBuffer (input)
    if(!(Bounds().IsInside(Rectangle(aX,aY,aRectangle.Width(),aRectangle.Height())))) { THROW(LimitsExceeded); } // test rectangle stays inside this frame buffer at new X,Y (output)

    TUint inLineWidthInWords = (aFrameBuffer.Width()>>5); // frame buffer width always word aligned
    TUint inYOffsetInBytes = aRectangle.Y() * (aFrameBuffer.Width()>>3); //Offset from start of frame buffer (Y Direction Only)
    TUint inXOffsetInWords = aRectangle.X()>>5; //Offset from start of frame buffer (X Direction Only)
    TUint32* inPtr = (TUint32*)(aFrameBuffer.Pixels().Ptr() + inYOffsetInBytes + (inXOffsetInWords<<2));
    TUint inBitOffset = aRectangle.X() - (inXOffsetInWords<<5);
    TUint inBitOffset32 = 32 - inBitOffset;

    TUint outLineWidthInWords = Width()>>5;
    TUint outYOffsetInBytes = aY * (Width()>>3); //Offset from start of frame buffer (Y Direction Only)
    TUint outXOffsetInWords = aX>>5; //Offset from start of frame buffer (X Direction Only)
    TUint32* outPtr = (TUint32*)(Pixels().Ptr() + outYOffsetInBytes + (outXOffsetInWords<<2));
    TUint outBitOffset = aX - (outXOffsetInWords<<5);
    TUint outBitOffset32 = 32 - outBitOffset;

    TUint height = aRectangle.Height();
    TUint rightEdgeBit = aX + aRectangle.Width();
    TUint rightHandMaskIndex = ((rightEdgeBit&0xffe0)+32) - rightEdgeBit;

    TUint widthWordMasked = aRectangle.Width()&0x1f;
    TUint widthWords = (aRectangle.Width()>>5) + (widthWordMasked ? 1:0);
    TUint inWidthWords = widthWords + (((widthWordMasked ? widthWordMasked:32)+inBitOffset)>32 ? 1:0);
    TUint outWidthWords = widthWords + (((widthWordMasked ? widthWordMasked:32)+outBitOffset)>32 ? 1:0);
    TBool extraOutputWord = (outWidthWords > inWidthWords);
    widthWords = extraOutputWord ? inWidthWords:outWidthWords; // width to use in inner loop

    TUint width;
    TUint32 inWord, inWordPrev, rightHandMask;

    // all three bit shift algorithms combined
    // shift left algorithm: outWidthWords will always be <= inWidthWords (inBitOffset > outBitOffset)
    // shift right algorithm: inputWidthWords will always be <= outWidthWords (inBitOffset < outBitOffset)
    // no shift algorithm: inputWidthWords will always be = outWidthWords (inBitOffset = outBitOffset)

    while (height--) //rows
    {
        rightHandMask = Arch::BigEndian4(*(outPtr + (outWidthWords-1))) & kMask[rightHandMaskIndex];
        // keep left hand original output data in tact
        inWord = (TUint32)(((TUint64)(Arch::BigEndian4(*outPtr)))>>outBitOffset32);

        width = widthWords; //use smallest of in or out- left: out, right: in, none: in = out
        while (width--) //columns
        {
            inWordPrev = inWord;
            inWord = (TUint32)((Arch::BigEndian8(*(TUint64*)inPtr++))>>inBitOffset32);
            *outPtr++ = Arch::BigEndian4(((TUint32)(((TUint64)(inWordPrev))<<outBitOffset32)) | (inWord>>outBitOffset));
        }
        //spillover into extra output word (right only)
        if (extraOutputWord) { *outPtr++ = Arch::BigEndian4(((TUint32)(((TUint64)(inWord))<<outBitOffset32))); }

        // keep right hand original output data in tact
        *(outPtr-1) = Arch::BigEndian4((Arch::BigEndian4(*(outPtr-1)) & kMaskInv[rightHandMaskIndex]) | rightHandMask);

        inPtr += inLineWidthInWords - widthWords;
        outPtr += outLineWidthInWords - outWidthWords;
    }
    Refresh();
}


void FrameBuffer::Write(const FrameBuffer& aFrameBuffer, const Point& aPoint, const Rectangle& aRectangle)
{
    Write(aFrameBuffer, aPoint.X(), aPoint.Y(), aRectangle);
}


void FrameBuffer::Write(const FrameBuffer& aFrameBuffer, TUint aX, TUint aY)
{
    // extra function that allows a full frame buffer to be written to another frame buffer
    Rectangle rectangle(0,0,aFrameBuffer.Width(),aFrameBuffer.Height());
    Write(aFrameBuffer, aX, aY, rectangle);
}


void FrameBuffer::Write(const FrameBuffer& aFrameBuffer, const Point& aPoint)
{
    Write(aFrameBuffer, aPoint.X(), aPoint.Y());
}


void FrameBuffer::Write(const Strike& aStrike, const Brx& aString, TUint aX, TUint aY)
{
    TUint xOffset = aX;
    const Glyph* glyph;
    String unicodeString(aString);

    for(TUint i=0; i<unicodeString.Chars(); i++) // 4 bytes per character
    {
        //write a character
        try { glyph = &(aStrike.FindGlyph(unicodeString.At(i))); }
        catch (UnknownCharacter) { glyph = &(aStrike.FindGlyph(Strike::kUnknownCharacter)); }
        Write(*glyph, xOffset, aY); // pass the current glyph and pen position
        xOffset += glyph->Advance();
        LOG(Debug::kUi|Debug::kVerbose,"Glyph Metrics (0x%x): %d(h), %d(w), %d(BearX), %d(BearY), %d(adv), %d(align)\n", unicodeString.At(i), glyph->Height(), glyph->Width(), glyph->BearingX(), glyph->BearingY(), glyph->Advance(), glyph->Alignment());
    }
    Refresh();
}


void FrameBuffer::Write(const Strike& aStrike, const Brx& aString, const Point& aPoint)
{
    Write(aStrike, aString, aPoint.X(), aPoint.Y());
}


const Brn FrameBuffer::kTestPatterns[7] = {
    Brn("TestBmp10x10.bmp"),
    Brn("TestBmp64x16.bmp"),
    Brn("TestBmpBorder128x32.bmp"),
    Brn("TestBmpCourierText128x32.bmp"),
    Brn("TestBmpHorizontalStripes128x32.bmp"),
    Brn("TestBmpLetterE7x7.bmp"),
    Brn("TestBmpVerticalStripes128x32.bmp") };

void FrameBuffer::TestPattern(TUint aTestPattern)
{
    ASSERT(aTestPattern <= 6);
    Tars bmpTar(uUiTar);
    Bmp bmp(bmpTar.Find(kTestPatterns[aTestPattern]));
    Write(bmp, 0, 0);
}


void FrameBuffer::Clear(const Rectangle& aRectangle)
{
    TUint aX, aY, aWidth, aHeight;
    aX = aRectangle.X(); aY = aRectangle.Y(); aWidth = aRectangle.Width(); aHeight = aRectangle.Height();

    if(!(Bounds().IsInside(aRectangle))) { THROW(LimitsExceeded); } // test rectangle stays inside buffer

    TUint lineWidthInWords = Width()>>5;
    TUint yOffsetInBytes = aY * (Width()>>3); //Offset from start of frame buffer (Y Direction Only)
    TUint xOffsetInWords = aX>>5; //Offset from start of frame buffer (X Direction Only)
    TUint32* wordPtr = (TUint32*)(Pixels().Ptr() + yOffsetInBytes + (xOffsetInWords<<2));
    TUint bitOffset = aX - (xOffsetInWords<<5);
    TUint bitOffset32 = 32 - bitOffset;

    TUint height = aHeight;
    TUint rightEdgeBit = aX + aWidth;
    TUint rightHandMaskIndex = ((rightEdgeBit&0xffe0)+32) - rightEdgeBit;

    TUint widthWordMasked = aWidth&0x1f;
    TUint widthWords = (aWidth>>5) + (widthWordMasked ? 1:0);
    widthWords += (((widthWordMasked ? widthWordMasked:32)+bitOffset)>32 ? 1:0);

    TUint width;
    TUint32 word, wordPrev, rightHandMask;

    while (height--) //rows
    {
        rightHandMask = Arch::BigEndian4(*(wordPtr + (widthWords-1))) & kMask[rightHandMaskIndex];
        word = (TUint32)(((TUint64)(Arch::BigEndian4(*wordPtr)))>>bitOffset32);

        width = widthWords;
        while (width--) //columns
        {
            wordPrev = word;
            word = 0x00;
            *wordPtr++ = Arch::BigEndian4((TUint32)(((TUint64)(wordPrev))<<bitOffset32));
        }

        // keep right hand original output data in tact
        *(wordPtr-1) = Arch::BigEndian4((Arch::BigEndian4(*(wordPtr-1)) & kMaskInv[rightHandMaskIndex]) | rightHandMask);

        wordPtr += lineWidthInWords - widthWords;
    }
    Refresh();
}


void FrameBuffer::Inverse(const Rectangle& aRectangle)
{
    TUint aX, aY, aWidth, aHeight;
    aX = aRectangle.X(); aY = aRectangle.Y(); aWidth = aRectangle.Width(); aHeight = aRectangle.Height();

    if(!(Bounds().IsInside(aRectangle))) { THROW(LimitsExceeded); } // test rectangle stays inside buffer

    TUint lineWidthInWords = Width()>>5;
    TUint yOffsetInBytes = aY * (Width()>>3); //Offset from start of frame buffer (Y Direction Only)
    TUint xOffsetInWords = aX>>5; //Offset from start of frame buffer (X Direction Only)
    TUint32* wordPtr = (TUint32*)(Pixels().Ptr() + yOffsetInBytes + (xOffsetInWords<<2));
    TUint bitOffset = aX - (xOffsetInWords<<5);
    TUint bitOffset32 = 32 - bitOffset;

    TUint height = aHeight;
    TUint rightEdgeBit = aX + aWidth;
    TUint rightHandMaskIndex = ((rightEdgeBit&0xffe0)+32) - rightEdgeBit;

    TUint widthWordMasked = aWidth&0x1f;
    TUint widthWords = (aWidth>>5) + (widthWordMasked ? 1:0);
    widthWords += (((widthWordMasked ? widthWordMasked:32)+bitOffset)>32 ? 1:0);

    TUint width;
    TUint32 word, wordPrev, rightHandMask;

    while (height--) //rows
    {
        rightHandMask = Arch::BigEndian4(*(wordPtr + (widthWords-1))) & kMask[rightHandMaskIndex];
        word = (TUint32)(((TUint64)(Arch::BigEndian4(*wordPtr)))>>bitOffset32);

        width = widthWords;
        while (width--) //columns
        {
            wordPrev = word;
            word = ~(Arch::BigEndian4(*wordPtr));
            *wordPtr++ = Arch::BigEndian4(((TUint32)(((TUint64)(wordPrev))<<bitOffset32)) | (word>>bitOffset));
        }

        // keep right hand original output data in tact
        *(wordPtr-1) = Arch::BigEndian4((Arch::BigEndian4(*(wordPtr-1)) & kMaskInv[rightHandMaskIndex]) | rightHandMask);

        wordPtr += lineWidthInWords - widthWords;
    }
    Refresh();
}


void FrameBuffer::SetPixel(TUint aX, TUint aY)
{
    if(!(aX < iWidth)) { THROW(LimitsExceeded); } // test x,y point starts in buffer
    if(!(aY < iHeight)) { THROW(LimitsExceeded); } // test x,y point starts in buffer

    static const TByte kBitMask[8] = {0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};

    //find correct byte and bit location in this frame buffer
    TUint yOffsetInBytes = aY * (Width()>>3); //Offset from start of frame buffer (Y Direction Only)
    TUint xOffsetInBytes = aX>>3; //Offset from start of frame buffer (X Direction Only)
    TUint bitOffset = aX - (xOffsetInBytes<<3);
    TByte* bytePtr = (TByte*)(Pixels().Ptr() + yOffsetInBytes + xOffsetInBytes);

    //set that bit only
    *bytePtr |= kBitMask[bitOffset];

    Refresh();
}


void FrameBuffer::SetPixel(const Point& aPoint)
{
    SetPixel(aPoint.X(), aPoint.Y());
}


void FrameBuffer::ClearPixel(TUint aX, TUint aY)
{
    if(!(aX < iWidth)) { THROW(LimitsExceeded); } // test x,y point starts in buffer
    if(!(aY < iHeight)) { THROW(LimitsExceeded); } // test x,y point starts in buffer

    static const TByte kBitMaskInv[8] = {0x7f,0xbf,0xdf,0xef,0xf7,0xfb,0xfd,0xfe}; //~kBitMask

    //find correct byte and bit location in this frame buffer
    TUint yOffsetInBytes = aY * (Width()>>3); //Offset from start of frame buffer (Y Direction Only)
    TUint xOffsetInBytes = aX>>3; //Offset from start of frame buffer (X Direction Only)
    TUint bitOffset = aX - (xOffsetInBytes<<3);
    TByte* bytePtr = (TByte*)(Pixels().Ptr() + yOffsetInBytes + xOffsetInBytes);

    //set that bit only
    *bytePtr &= kBitMaskInv[bitOffset];

    Refresh();
}


void FrameBuffer::ClearPixel(const Point& aPoint)
{
    ClearPixel(aPoint.X(), aPoint.Y());
}


void FrameBuffer::Refresh()
{
    iPixelsChanged.Signal();
}


// window class

Window::Window(FrameBuffer& aInput, FrameBuffer& aOutput, TUint aX, TUint aY, TUint aPriority) :
    iPixelsChanged(MakeFunctor(*this, &Window::PixelsChanged)),
    iFrameBufferIn(aInput), iFrameBufferOut(aOutput), iX(aX), iY(aY),
    iChanged(false), iOpen(true), iMutexOpen("WOPN"), iMutexLock("WLCK")
{
    iPoint.Set(aX,aY);
    iRectangle.Set(0,0,iFrameBufferIn.Width(),iFrameBufferIn.Height());
    iPixelsChanged.Subscribe(iFrameBufferIn.PixelsChanged(), aPriority);
}


Window::Window(FrameBuffer& aInput, FrameBuffer& aOutput, const Point& aPoint, TUint aPriority) :
    iPixelsChanged(MakeFunctor(*this, &Window::PixelsChanged)),
    iFrameBufferIn(aInput), iFrameBufferOut(aOutput), iX(aPoint.X()), iY(aPoint.Y()),
    iChanged(false), iOpen(true), iMutexOpen("WOPN"), iMutexLock("WLCK")
{
    iPoint = aPoint;
    iRectangle.Set(0,0,iFrameBufferIn.Width(),iFrameBufferIn.Height());
    iPixelsChanged.Subscribe(iFrameBufferIn.PixelsChanged(), aPriority);
}


Window::Window(FrameBuffer& aInput, FrameBuffer& aOutput, TUint aX, TUint aY, const Rectangle& aRectangle, TUint aPriority) :
    iPixelsChanged(MakeFunctor(*this, &Window::PixelsChanged)),
    iFrameBufferIn(aInput), iFrameBufferOut(aOutput), iX(aX), iY(aY),
    iChanged(false), iOpen(true), iMutexOpen("WOPN"), iMutexLock("WLCK")
{
    iPoint.Set(aX,aY);
    iRectangle.Set(aRectangle.X(),aRectangle.Y(),aRectangle.Width(),aRectangle.Height());
    iPixelsChanged.Subscribe(iFrameBufferIn.PixelsChanged(), aPriority);
}


Window::Window(FrameBuffer& aInput, FrameBuffer& aOutput, const Point& aPoint, const Rectangle& aRectangle, TUint aPriority) :
    iPixelsChanged(MakeFunctor(*this, &Window::PixelsChanged)),
    iFrameBufferIn(aInput), iFrameBufferOut(aOutput), iX(aPoint.X()), iY(aPoint.Y()),
    iChanged(false), iOpen(true), iMutexOpen("WOPN"), iMutexLock("WLCK")
{
    iPoint = aPoint;
    iRectangle.Set(aRectangle.X(),aRectangle.Y(),aRectangle.Width(),aRectangle.Height());
    iPixelsChanged.Subscribe(iFrameBufferIn.PixelsChanged(), aPriority);
}


void Window::Close()
{
    iMutexLock.Wait(); // insure no other thread writes to this window until it is opened again

    iMutexOpen.Wait();
    ASSERT(iOpen); // window must always be closed then opened
    iOpen = false;
    iMutexOpen.Signal();
}


void Window::Open()
{
    iMutexOpen.Wait();
    ASSERT(!iOpen); // window must always be closed then opened
    iOpen = true;
    iMutexOpen.Signal();

    if (iChanged) // update display if changed while closed
    {
        iFrameBufferOut.Write(iFrameBufferIn, iX, iY, iRectangle);
        iChanged = false;
    }

    iMutexLock.Signal(); // allow other threads to write to this window
}


TBool Window::Opened()
{
    iMutexOpen.Wait();
    bool open = iOpen;
    iMutexOpen.Signal();
    return open;
}


void Window::PixelsChanged()
{
    if (iMutexLock.TryWait()) // gaurantees thread safety
    {
        // mutex obtained (window open)
        iFrameBufferOut.Write(iFrameBufferIn, iX, iY, iRectangle);
        iMutexLock.Signal();
    }
    else
    {
        // mutex not available (window closed)
        iChanged = true;
    }
}


void Window::Set(TUint aX, TUint aY)
{
    ASSERT(iMutexLock.TryWait()); // mutex obtained (window open)
    iX = aX;
    iY = aY;
    iPoint.Set(aX,aY);
    iRectangle.Set(0,0,iFrameBufferIn.Width(),iFrameBufferIn.Height());
    iMutexLock.Signal();
}


void Window::Set(const Point& aPoint)
{
    ASSERT(iMutexLock.TryWait()); // mutex obtained (window open)
    iX = aPoint.X();
    iY = aPoint.Y();
    iPoint = aPoint;
    iRectangle.Set(0,0,iFrameBufferIn.Width(),iFrameBufferIn.Height());
    iMutexLock.Signal();
}


void Window::Set(TUint aX, TUint aY, const Rectangle& aRectangle)
{
    ASSERT(iMutexLock.TryWait()); // mutex obtained (window open)
    iX = aX;
    iY = aY;
    iPoint.Set(aX,aY);
    iRectangle.Set(aRectangle.X(),aRectangle.Y(),aRectangle.Width(),aRectangle.Height());
    iMutexLock.Signal();
}


void Window::Set(const Point& aPoint, const Rectangle& aRectangle)
{
    ASSERT(iMutexLock.TryWait()); // mutex obtained (window open)
    iX = aPoint.X();
    iY = aPoint.Y();
    iPoint = aPoint;
    iRectangle.Set(aRectangle.X(),aRectangle.Y(),aRectangle.Width(),aRectangle.Height());
    iMutexLock.Signal();
}
