#ifndef HEADER_OHTOPOLOGYC_TAGMANAGER
#define HEADER_OHTOPOLOGYC_TAGMANAGER

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/WatchableThread.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/DisposeHandler.h>
#include <OpenHome/MetaData.h>
#include <vector>
#include <map>



namespace OpenHome
{
namespace Av
{

/////////////////////////////////////////////////

class ITag
{

};

/////////////////////////////////////////

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
};

/////////////////////////////////////////

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


class ITagManager
{
public:
//    TUint MaxSystemTagId() = 0;
//    ITag* Tag(TUint aId) = 0;
    //ITag this[uint aId]() = 0;
    //ITagRealm this[TagRealm aRealm]() = 0;
//    ITagRealm& TagRealm(ETagRealm aRealm) = 0;
    //ITagRealmSystem System() = 0;
//    ITagRealmGlobal& Global() = 0;
//    ITagRealmAudio& Audio() = 0;
    //ITagRealmVideo Video() = 0;
    //ITagRealmImage Image() = 0;
    //ITagRealmPlaylist Playlist() = 0;
    //ITagRealmContainer Container() = 0;

    virtual IMediaMetadata* FromDidlLite(const Brx& aMetadata);
};



class TagManager : public ITagManager//, public ITagManagerInitialiser
{
public:
    virtual IMediaMetadata* FromDidlLite(const Brx& aMetadata);
};


} // Av
} // OpenHome


#endif // HEADER_OHTOPOLOGYC_TAGMANAGER
