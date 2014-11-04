#ifndef HEADER_OHTOPOLOGYC_META_DATA
#define HEADER_OHTOPOLOGYC_META_DATA

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/TagManager.h>

#include <stddef.h>


namespace OpenHome
{
namespace Av
{


//////////////////////////////////////////////////////////////////////

class IInfoMetadata
{
public:
    virtual IMediaMetadata& Metadata() = 0;
    virtual const Brx& Uri() = 0;
    virtual ~IInfoMetadata() {}
};

//////////////////////////////////////////////////////////////////////


/////////////////////////////////////////

class IInfoMetatext
{
public:
    virtual IMediaMetadata& Metatext() = 0;
};

/////////////////////////////////////////

class InfoMetatext : public IInfoMetatext
{
public:
    InfoMetatext();
    InfoMetatext(IMediaMetadata& aMetatext);

    IMediaMetadata& Metatext();

private:
    IMediaMetadata* iMetatext;
};

//////////////////////////////////////////////////////////////////////

class InfoMetadata : public IInfoMetadata, public INonCopyable
{
public:
    static IInfoMetadata* Empty();
    static void DestroyStatics();

    InfoMetadata(IMediaMetadata* aMetadata, const Brx& aUri);
    virtual IMediaMetadata& Metadata();
    virtual const Brx& Uri();

    virtual ~InfoMetadata();

private:
    InfoMetadata();

private:
    IMediaMetadata* iMetadata;
    Bws<500> iUri;
    static IInfoMetadata* iEmpty;
};

//////////////////////////////////////////////////////////////////////

class ISenderMetadata
{
public:
    virtual const Brx& Uri() = 0;
    virtual const Brx& ArtworkUri() = 0;
    virtual const Brx& ToString() = 0;  // added in ohTopologyC
    virtual ~ISenderMetadata() {};
};

//////////////////////////////////////////////////////////////////////

class SenderMetadata : public ISenderMetadata
{
public:
    SenderMetadata(const Brx& aMetadata);

    static SenderMetadata* Empty();
    static void DestroyStatics();

    virtual const Brx& Name();
    virtual const Brx& Uri();
    virtual const Brx& ArtworkUri();
    virtual const Brx& ToString();

private:
    SenderMetadata();

private:
    Bws<100> iName;
    Bws<100> iUri;
    Bws<100> iArtworkUri;
    Bws<1000> iMetadata;
    static SenderMetadata* iEmpty;
};



} // namespace OpenHome

} // namespace Av


#endif // HEADER_OHTOPOLOGYC_META_DATA
