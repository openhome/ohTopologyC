#pragma once

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/TagManager.h>
#include <memory>
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

    InfoMetatext(std::shared_ptr<IMediaMetadata> aMetatext);

    IMediaMetadata& Metatext();
private:
    std::shared_ptr<IMediaMetadata> iMetatext;
};

//////////////////////////////////////////////////////////////////////

class InfoMetadata : public IInfoMetadata//, public INonCopyable
{
friend class Network;

public:
    InfoMetadata(std::shared_ptr<IMediaMetadata> aMetadata, const Brx& aUri);
    ~InfoMetadata();

    IMediaMetadata& Metadata() override;
    const Brx& Uri() override;
private:
    InfoMetadata();

private:
    std::shared_ptr<IMediaMetadata> iMetadata;
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
