#ifndef HEADER_OHTOPOLOGYC_META_DATA
#define HEADER_OHTOPOLOGYC_META_DATA

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/TagManager.h>

#include <stddef.h>


namespace OpenHome
{
namespace Topology
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
    virtual ~IInfoMetatext(){}
};

/////////////////////////////////////////

class InfoMetatext : public IInfoMetatext
{
public:
    InfoMetatext();
    ~InfoMetatext();

    InfoMetatext(IMediaMetadata* aMetatext);

    IMediaMetadata& Metatext();

private:
    IMediaMetadata* iMetatext;
};

//////////////////////////////////////////////////////////////////////

class InfoMetadata : public IInfoMetadata, public INonCopyable
{
friend class Network;

public:
    InfoMetadata(IMediaMetadata* aMetadata, const Brx& aUri);
    virtual IMediaMetadata& Metadata();
    virtual const Brx& Uri();

    virtual ~InfoMetadata();

private:
    InfoMetadata();

private:
    IMediaMetadata* iMetadata;
    Bws<500> iUri;
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
friend class Network;

public:
    SenderMetadata(const Brx& aMetadata);

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
};



} // namespace OpenHome

} // namespace Topology


#endif // HEADER_OHTOPOLOGYC_META_DATA
