#pragma once

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/WatchableThread.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/DisposeHandler.h>
#include <OpenHome/Tag.h>
#include <OpenHome/Net/Private/XmlParser.h>
#include <memory>
#include <vector>
#include <map>

namespace OpenHome
{
namespace Topology
{

class IMediaValue
{
public:
    virtual Brn Value() = 0;
    virtual const std::vector<Brn>& Values() = 0;
    virtual ~IMediaValue(){}
};

///////////////////////////////////////////////////

class MediaValue : public IMediaValue
{
public:
    MediaValue(const Brx& aValue);
    MediaValue(const std::vector<Brn>& aValues);

    // IMediaServerValue
    virtual Brn Value();
    virtual const std::vector<Brn>& Values();

private:
    std::vector<Brn> iValues;
    Bws<2083> iValue; // FIXME: random capacity. 2083 is longest URI accepted by IE
};


/////////////////////////////////////////

class IMediaMetadata
{
public:
    virtual IMediaValue* Value(ITag* aTag) = 0;
    virtual const std::map<ITag*, IMediaValue*> Values() = 0;
    virtual ~IMediaMetadata() {}
};

////////////////////////////////////////////////////

class MediaDictionary : public IMediaMetadata
{
public:
    virtual void Add(ITag* aTag, const Brx& aValue);
    virtual void Add(ITag* aTag, IMediaValue& aValue);
    virtual void Add(ITag* aTag, IMediaMetadata& aMetadata);
    // IMediaServerMetadata
    virtual IMediaValue* Value(ITag* aTag);

protected:
    MediaDictionary();
    MediaDictionary(IMediaMetadata& aMetadata);
    virtual ~MediaDictionary();

protected:
    std::map<ITag*, IMediaValue*> iMetadata;
};




/////////////////////////////////////////

class MediaMetadata : public MediaDictionary
{
public:
    MediaMetadata();
    const std::map<ITag*, IMediaValue*> Values();
};

///////////////////////////////////////////////////



class ITagManager
{
public:
    virtual TUint MaxSystemTagId() = 0;
    virtual ITag* Tag(TUint aId) = 0;
    virtual ITagRealm& Realm(ETagRealm aRealm) = 0;
    virtual ITagRealmAudio& Audio() = 0;

    virtual MediaMetadata* FromDidlLite(const Brx& aMetadata) = 0;
    virtual void ToDidlLite(IMediaMetadata& aMetadata, Bwx& aBuf) = 0;
    virtual ~ITagManager() {}
};

//////////////////////////////////////////////////////////////////////

class TagManager : public ITagManager, public ITagManagerInitialiser
{
private:
    static const TUint kMaxSystemTagId = 49;

public:
    TagManager();
    ~TagManager();

    virtual MediaMetadata* FromDidlLite(const Brx& aMetadata);
    virtual void ToDidlLite(IMediaMetadata& aMetadata, Bwx& aBuf);
    virtual TUint MaxSystemTagId();
    virtual ITag* Tag(TUint aId);
    virtual ITagRealm& Realm(ETagRealm aRealm);
    virtual ITagRealmAudio& Audio();
    virtual void Add(std::map<Brn, ITag*, BufferCmp>& aRealm, ITag* aTag);

    // ITagManagerInitialiser
    virtual void Add(ITag* aTag);

private: //Private utility functions for Xml Parsing in FromDidlLite
    Brn GoFind(const Brx& aTag, const Brx& aMetadata);
    Brn GoFindAttribute(const Brx& aTag, const Brx& aAttribute, const Brx& aMetadata);
    Brn GoFindElement(const Brx& aTag, const Brx& aAttribute, const Brx& aMetadata);
private:
    std::map<TUint, ITag*> iTags;
    std::map<ETagRealm, ITagRealm*> iRealms;

    ITagRealmGlobal* iGlobal;
    ITagRealmAudio* iAudio;
};






} // Topology
} // OpenHome
