#ifndef HEADER_OHTOPOLOGYC_META_DATA
#define HEADER_OHTOPOLOGYC_META_DATA

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Buffer.h>
#include <stddef.h>


namespace OpenHome
{
namespace Av
{

class IMediaMetadata;

//////////////////////////////////////////////////////////////////////

class IInfoDetails
{
    virtual TUint BitDepth() = 0;
    virtual TUint BitRate() = 0;
    virtual Brn CodecName() = 0;
    virtual TUint Duration() = 0;
    virtual TBool Lossless() = 0;
    virtual TUint SampleRate() = 0;
};

//////////////////////////////////////////////////////////////////////

class IInfoMetadata
{
public:
    virtual IMediaMetadata& Metadata() = 0;
    virtual Brn Uri() = 0;
};

//////////////////////////////////////////////////////////////////////

class IInfoMetatext
{
public:
    virtual IMediaMetadata& Metatext() = 0;
};

//////////////////////////////////////////////////////////////////////

class InfoMetadata : public IInfoMetadata
{
public:
    static IInfoMetadata* Empty();
    InfoMetadata(IMediaMetadata& aMetadata, const Brx& aUri);
    virtual IMediaMetadata& Metadata();
    virtual Brn Uri();

private:
    InfoMetadata();

private:
    IMediaMetadata* iMetadata;
    Brn iUri;
};

//////////////////////////////////////////////////////////////////////

class InfoMetatext : public IInfoMetatext
{
public:
    InfoMetatext(IMediaMetadata& aMetatext);
    IMediaMetadata& Metatext();

private:
    /*internal */InfoMetatext();

private:
    IMediaMetadata* iMetatext;
};



} // namespace OpenHome

} // namespace Av


#endif // HEADER_OHTOPOLOGYC_META_DATA
