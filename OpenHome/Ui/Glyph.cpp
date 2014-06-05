#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/Ui/Glyph.h>
#include <vector>
#include <OpenHome/Private/Debug.h>


using namespace OpenHome;
using namespace OpenHome::Ui;
using namespace std;



///////////////////////////////////////////////////////////////////////////////////////////////////

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

GlyphRange::GlyphRange(TUnicode aUnicodeFirst, TUnicode aUnicodeLast, TUint aPointSize, const Otb& aOtb)
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

/////////////////////////////////////////////////////////////////////////////////////////////////////

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
    ASSERTS();
    THROW(AssertionFailed);
}


////////////////////////////////////////////////////////////////////////////////////////////////
