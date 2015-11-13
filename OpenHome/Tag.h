#pragma once

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Buffer.h>

#include <stddef.h>
#include <map>

namespace OpenHome
{
namespace Topology
{

class TagRealm;

//////////////////////////////////////

enum ETagRealm
{
    eSystem,
    eGlobal,
    eAudio,
    eVideo,
    eImage,
    ePlaylist,
    eContainer
};

//////////////

class ITag
{
public:
    virtual TUint Id() = 0;
    virtual ETagRealm& Realm() = 0;
    virtual Brn Name() = 0;
    virtual Brn FullName() = 0;
    virtual TBool IsKey() = 0;
    virtual TBool IsNumeric() = 0;
    virtual TBool IsSearchable() = 0;
    virtual TBool IsArticled() = 0;
    virtual TBool IsHyper() = 0;
    virtual ~ITag() {}
};

/////////////////////////////////////

class ITagRealm
{
public:
    virtual ITag* Tag(const Brx& aName) = 0;
    virtual ~ITagRealm() {}
};

////////////////////////////////////////////

class ITagRealmGlobal : public ITagRealm
{
public:
    virtual ITag* Hash() = 0;
};

////////////////////////////////////////////

class ITagRealmAudio : public ITagRealm
{
public:
    virtual ITag* Type() = 0;
    virtual ITag* Description() = 0;
    virtual ITag* Channels() = 0;
    virtual ITag* Bitdepth() = 0;
    virtual ITag* Samplerate() = 0;
    virtual ITag* Bitrate() = 0;
    virtual ITag* Duration() = 0;
    virtual ITag* Codec() = 0;
    virtual ITag* Artist() = 0;
    virtual ITag* Bpm() = 0;
    virtual ITag* Composer() = 0;
    virtual ITag* Conductor() = 0;
    virtual ITag* Disc() = 0;
    virtual ITag* Genre() = 0;
    virtual ITag* Grouping() = 0;
    virtual ITag* Lyrics() = 0;
    virtual ITag* Title() = 0;
    virtual ITag* Track() = 0;
    virtual ITag* Tracks() = 0;
    virtual ITag* Year() = 0;
    virtual ITag* Artwork() = 0;
    virtual ITag* Uri() = 0;
    virtual ITag* Weight() = 0;
    virtual ITag* Album() = 0;
    virtual ITag* AlbumTitle() = 0;
    virtual ITag* AlbumArtist() = 0;
    virtual ITag* AlbumArtworkFilename() = 0;
    virtual ITag* AlbumArtworkCodec() = 0;
    virtual ITag* AlbumDiscs() = 0;
    virtual ITag* AlbumYear() = 0;
    virtual ITag* AlbumGenre() = 0;
};

///////////////////////////////////////////////

class Tag : public ITag
{
private:
    static const TUint kMaxFullNameBytes = 100; //

public:
    static ITag* CreateTagText(TUint aId, ETagRealm aRealm, const Brx& aName);
    static ITag* CreateTagNumeric(TUint aId, ETagRealm aRealm, const Brx& aName);
    static ITag* CreateTagKey(TUint aId, ETagRealm aRealm, const Brx& aName);
    static ITag* CreateTagSearchable(TUint aId, ETagRealm aRealm, const Brx& aName);
    static ITag* CreateTagArticled(TUint aId, ETagRealm aRealm, const Brx& aName);
    static Brn RealmToString(ETagRealm);

    virtual Brn ToString();

    // ITag
    virtual TUint Id();
    virtual ETagRealm& Realm();
    virtual Brn Name();
    virtual Brn FullName();
    virtual TBool IsKey();
    virtual TBool IsNumeric();
    virtual TBool IsSearchable();
    virtual TBool IsArticled();
    virtual TBool IsHyper();

private:
    Tag(TUint aId, ETagRealm aRealm, const Brx& aName, TBool aKey, TBool aNumeric, TBool aSearchable, TBool aArticled);

private:
    TUint iId;
    ETagRealm iRealm;
    Brn iName;
    TBool iKey;
    TBool iNumeric;
    TBool iSearchable;
    TBool iArticled;
    Bws<kMaxFullNameBytes> iFullName;
};

////////////////////////////////////////////////////

class ITagManagerInitialiser
{
public:
    virtual void Add(ITag* aTag) = 0;
};

////////////////////////////////////////////////////


class ITagRealmGlobal;


class TagRealmBase
{
public:
    // ITagRealm
    ITag* Tag(const Brx& aName);

protected:
    TagRealmBase();
    TagRealmBase(ITagRealmGlobal& aGlobal);
    void Add(ITag* aTag, ITagManagerInitialiser& aInitialiser);

protected:
    std::map<Brn, ITag*, BufferCmp> iTags;

private:
    ITagRealmGlobal* iGlobal;
};

///////////////////////////////////////////////////////////


class TagRealmGlobal : public TagRealmBase, public ITagRealmGlobal
{
public:
    TagRealmGlobal(ITagManagerInitialiser& aInitialiser);

    // ITagRealmGlobal
    virtual ITag* Hash();
    virtual ITag* Tag(const Brx& aName);

private:
    ITag* iHash;
};


class TagRealmAudio : public TagRealmBase, public ITagRealmAudio
{
public:
    TagRealmAudio(ITagManagerInitialiser& aInitialiser, ITagRealmGlobal& aGlobal);

    // ITagRealmAudio
    virtual ITag* Type();
    virtual ITag* Description();
    virtual ITag* Channels();
    virtual ITag* Bitdepth();
    virtual ITag* Samplerate();
    virtual ITag* Bitrate();
    virtual ITag* Duration();
    virtual ITag* Codec();
    virtual ITag* Artist();
    virtual ITag* Bpm();
    virtual ITag* Composer();
    virtual ITag* Conductor();
    virtual ITag* Disc();
    virtual ITag* Genre();
    virtual ITag* Grouping();
    virtual ITag* Lyrics();
    virtual ITag* Title();
    virtual ITag* Track();
    virtual ITag* Tracks();
    virtual ITag* Year();
    virtual ITag* Artwork();
    virtual ITag* Uri();
    virtual ITag* Weight();
    virtual ITag* Album();
    virtual ITag* AlbumTitle();
    virtual ITag* AlbumArtist();
    virtual ITag* AlbumArtworkFilename();
    virtual ITag* AlbumArtworkCodec();
    virtual ITag* AlbumDiscs();
    virtual ITag* AlbumYear();
    virtual ITag* AlbumGenre();

    // ITagRealmAudio
    virtual ITag* Tag(const Brx& aName);

private:
    ITag* iType;
    ITag* iDescription;
    ITag* iChannels;
    ITag* iBitdepth;
    ITag* iSamplerate;
    ITag* iBitrate;
    ITag* iDuration;
    ITag* iCodec;
    ITag* iArtist;
    ITag* iBpm;
    ITag* iComposer;
    ITag* iConductor;
    ITag* iDisc;
    ITag* iGenre;
    ITag* iGrouping;
    ITag* iLyrics;
    ITag* iTitle;
    ITag* iTrack;
    ITag* iTracks;
    ITag* iYear;
    ITag* iArtwork;
    ITag* iUri;
    ITag* iWeight;
    ITag* iAlbum;
    ITag* iAlbumTitle;
    ITag* iAlbumArtist;
    ITag* iAlbumArtworkFilename;
    ITag* iAlbumArtworkCodec;
    ITag* iAlbumDiscs;
    ITag* iAlbumYear;
    ITag* iAlbumGenre;

};

} // Topology
} // OpenHome
