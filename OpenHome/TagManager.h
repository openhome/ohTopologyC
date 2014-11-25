#ifndef HEADER_OHTOPOLOGYC_TAGMANAGER
#define HEADER_OHTOPOLOGYC_TAGMANAGER

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/WatchableThread.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/DisposeHandler.h>
//#include <OpenHome/MetaData.h>
#include <OpenHome/Tag.h>
#include <vector>
#include <map>



namespace OpenHome
{
namespace Av
{

//class IMediaMetatdata;

class IMediaValue
{
public:
    virtual Brn Value() = 0;
    virtual const std::vector<Brn> Values() = 0;
};

/////////////////////////////////////////

class IMediaMetadata //: public IEnumerable<KeyValuePair<ITag, IMediaValue>>
{
public:
    virtual IMediaValue* Value(ITag* aTag) = 0;
    virtual const std::map<ITag*, IMediaValue*> Values() = 0;
    virtual ~IMediaMetadata() {}
};

/////////////////////////////////////////


class ITagManager
{
public:
    virtual TUint MaxSystemTagId() = 0;
    virtual ITag* Tag(TUint aId) = 0;
    //ITagRealm this[TagRealm aRealm]) = 0;
    virtual ITagRealm& Realm(ETagRealm aRealm) = 0;

    //virtual ITagRealmSystem System() = 0;
    //virtual ITagRealmGlobal Global() = 0;
    virtual ITagRealmAudio& Audio() = 0;
    //virtual ITagRealmVideo Video() = 0;
    //virtual ITagRealmImage Image() = 0;
    //virtual ITagRealmPlaylist Playlist() = 0;
    //virtual ITagRealmContainer Container() = 0;

    virtual IMediaMetadata* FromDidlLite(const Brx& aMetadata) = 0;
    virtual void ToDidlLite(IMediaMetadata& aMetadata, Brh& aBuf) = 0;
    virtual ~ITagManager() {}
};

//////////////////////////////////////////////////////////////////////

class TagManager : public ITagManager//, public ITagManagerInitialiser
{
private:
    static const TUint kMaxSystemTagId = 49;

public:
    virtual IMediaMetadata* FromDidlLite(const Brx& aMetadata);
    virtual void ToDidlLite(IMediaMetadata& aMetadata, Brh& aBuf);
    virtual TUint MaxSystemTagId();
    virtual ITag* Tag(TUint aId);
    virtual ITagRealm& Realm(ETagRealm aRealm);
    virtual ITagRealmAudio& Audio();

private:
    std::map<TUint, ITag*> iTags;
    std::map<ETagRealm, ITagRealm*> iRealms;

    //ITagRealmSystem* iSystem;
    //ITagRealmGlobal* iGlobal;
    ITagRealmAudio* iAudio;
    //ITagRealmVideo* iVideo;
    //ITagRealmImage* iImage;
    //ITagRealmPlaylist* iPlaylist;
    //ITagRealmContainer* iContainer;
};

/////////////////////////////////////////////////


class IMediaDatum : public IMediaMetadata
{
public:
    virtual Brn Id() = 0;
    //virtual IEnumerable<ITag> Type() = 0;
    virtual std::vector<ITag*> Type() = 0;
};

/////////////////////////////////////////

class MediaValue : public IMediaValue
{
public:
    MediaValue(const Brx& aValue);
    MediaValue(const std::vector<Brn> aValues);

    // IMediaServerValue
    virtual Brn Value();
    virtual const std::vector<Brn> Values();

private:
    std::vector<Brn> iValues;
    Brn iValue;
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

protected:
    std::map<ITag*, IMediaValue*> iMetadata;
};

///////////////////////////////////////////////////

class MediaMetadata : public MediaDictionary//, public IMediaMetadata
{
public:
    MediaMetadata();

    const std::map<ITag*, IMediaValue*> Values();

    // IEnumerable<KeyValuePair<ITag, IMediaServer>>
    //IEnumerator<KeyValuePair<ITag, IMediaValue>> GetEnumerator();

private:
    // IEnumerable
    //IEnumerator IEnumerable.GetEnumerator();
};

///////////////////////////////////////////////////



} // Av
} // OpenHome


#endif // HEADER_OHTOPOLOGYC_TAGMANAGER
