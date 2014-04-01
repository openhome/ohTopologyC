#ifndef HEADER_OHTOPC_TAG
#define HEADER_OHTOPC_TAG


#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Buffer.h>
#include <stddef.h>
#include <map>



namespace OpenHome
{
namespace Av
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
    virtual ETagRealm Realm() = 0;
    virtual Brn Name() = 0;
    virtual Brn FullName() = 0;
    virtual TBool IsKey() = 0;
    virtual TBool IsNumeric() = 0;
    virtual TBool IsSearchable() = 0;
    virtual TBool IsArticled() = 0;
    virtual TBool IsHyper() = 0;
};

/////////////////////////////////////

class ITagRealm
{
public:
    virtual ITag* Tag(const Brx& aName) = 0;
};

////////////////////////////////////////////

/*
    public interface ITagRealmSystem : ITagRealm
    {
        ITag Realm { get; }
        ITag Root { get; }
        ITag Folder { get; }
        ITag Path { get; }
    }
*/

class ITagRealmGlobal : public ITagRealm
{
public:
    virtual ITag* Hash() = 0;
};

////////////////////////////////////////////

class ITagRealmAudio : public ITagRealm
{
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

////////////////////////////////////////////////

/*
    public interface ITagRealmVideo : ITagRealm
    {
    }

    public interface ITagRealmImage : ITagRealm
    {
        ITag Type { get; }
        ITag Title { get; }
        ITag Embedded { get; }
        ITag Codec { get; }
        ITag Width { get; }
        ITag Height { get; }
    }

    public interface ITagRealmPlaylist : ITagRealm
    {
        ITag Playlist { get; }
        ITag PlaylistType { get; }
        ITag PlaylistTitle { get; }
        ITag Artist { get; }
        ITag Title { get; }
        ITag Duration { get; }
        ITag Uri { get; }
    }

    public interface ITagRealmContainer : ITagRealm
    {
        ITag Title { get; }
        ITag Description { get; }
        ITag Artwork { get; }
    }
*/


///////////////////////////////////////////////

class Tag : ITag
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
    virtual ETagRealm Realm();
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
    virtual ITag* Tag(const Brx& aName);

protected:
    TagRealmBase();
    TagRealmBase(ITagRealmGlobal& aGlobal);
    void Add(ITag* aTag, ITagManagerInitialiser& aInitialiser);

private:
    ITagRealmGlobal* iGlobal;
    std::map<Brn, ITag*, BufferCmp> iTags;
};

///////////////////////////////////////////////////////////


class TagRealmGlobal : public TagRealmBase, public ITagRealmGlobal
{
public:
    TagRealmGlobal(ITagManagerInitialiser& aInitialiser);

    // ITagRealmGlobal
    ITag* Hash();

private:
    ITag* iHash;
};

///////////////////////////////////////////////////////////



/*
    internal class TagRealmSystem : TagRealmBase, ITagRealmSystem
    {
        private ITag iRealm;
        private ITag iRoot;
        private ITag iFolder;
        private ITag iPath;

        public TagRealmSystem(ITagManagerInitialiser aInitialiser)
            : base()
        {
            iRealm = Tag.CreateTagText(0, TagRealm.System, "realm");
            iRoot = Tag.CreateTagText(1, TagRealm.System, "root");
            iFolder = Tag.CreateTagText(2, TagRealm.System, "folder");
            iPath = Tag.CreateTagText(3, TagRealm.System, "path");

            Add(iRealm, aInitialiser);
            Add(iRoot, aInitialiser);
            Add(iFolder, aInitialiser);
            Add(iPath, aInitialiser);
        }

        // ITagRealmSystem

        public ITag Realm
        {
            get
            {
                return (iRealm);
            }
        }

        public ITag Root
        {
            get
            {
                return (iRoot);
            }
        }

        public ITag Folder
        {
            get
            {
                return (iFolder);
            }
        }

        public ITag Path
        {
            get
            {
                return (iPath);
            }
        }
    }

*/

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

/*
    internal class TagRealmVideo : TagRealmBase, ITagRealmVideo
    {
        public TagRealmVideo(ITagManagerInitialiser aInitialiser, ITagRealmGlobal aGlobal)
            : base(aGlobal)
        {
        }
    }

    internal class TagRealmImage : TagRealmBase, ITagRealmImage
    {
        private ITag iType;
        private ITag iTitle;
        private ITag iEmbedded;
        private ITag iCodec;
        private ITag iWidth;
        private ITag iHeight;

        public TagRealmImage(ITagManagerInitialiser aInitialiser, ITagRealmGlobal aGlobal)
            : base(aGlobal)
        {
            iType = Tag.CreateTagText(800, TagRealm.Image, "type");
            iTitle = Tag.CreateTagSearchable(801, TagRealm.Image, "title");
            iEmbedded = Tag.CreateTagText(802, TagRealm.Image, "embedded");
            iCodec = Tag.CreateTagText(803, TagRealm.Image, "codec");
            iWidth = Tag.CreateTagNumeric(804, TagRealm.Image, "width");
            iHeight = Tag.CreateTagNumeric(805, TagRealm.Image, "height");

            Add(iType, aInitialiser);
            Add(iTitle, aInitialiser);
            Add(iEmbedded, aInitialiser);
            Add(iCodec, aInitialiser);
            Add(iWidth, aInitialiser);
            Add(iHeight, aInitialiser);
        }

        // ITagRealmImage

        public ITag Type
        {
            get
            {
                return (iType);
            }
        }

        public ITag Title
        {
            get
            {
                return (iTitle);
            }
        }

        public ITag Embedded
        {
            get
            {
                return (iEmbedded);
            }
        }

        public ITag Codec
        {
            get
            {
                return (iCodec);
            }
        }

        public ITag Width
        {
            get
            {
                return (iWidth);
            }
        }

        public ITag Height
        {
            get
            {
                return (iHeight);
            }
        }
    }

    internal class TagRealmPlaylist : TagRealmBase, ITagRealmPlaylist
    {
        private ITag iPlaylist;
        private ITag iPlaylistType;
        private ITag iPlaylistTitle;
        private ITag iArtist;
        private ITag iTitle;
        private ITag iDuration;
        private ITag iUri;

        public TagRealmPlaylist(ITagManagerInitialiser aInitialiser, ITagRealmGlobal aGlobal)
            : base(aGlobal)
        {
            iPlaylist = Tag.CreateTagKey(300, TagRealm.Playlist, "playlist");
            iPlaylistType = Tag.CreateTagText(301, TagRealm.Playlist, "playlist.type");
            iPlaylistTitle = Tag.CreateTagText(302, TagRealm.Playlist, "playlist.title");
            iArtist = Tag.CreateTagText(303, TagRealm.Playlist, "artist");
            iTitle = Tag.CreateTagText(304, TagRealm.Playlist, "title");
            iDuration = Tag.CreateTagNumeric(305, TagRealm.Playlist, "duration");
            iUri = Tag.CreateTagText(306, TagRealm.Playlist, "uri");

            Add(iPlaylist, aInitialiser);
            Add(iPlaylistType, aInitialiser);
            Add(iPlaylistTitle, aInitialiser);
            Add(iArtist, aInitialiser);
            Add(iTitle, aInitialiser);
            Add(iDuration, aInitialiser);
            Add(iUri, aInitialiser);
        }

        // ITagRealmPlaylist

        public ITag Playlist
        {
            get
            {
                return (iPlaylist);
            }
        }

        public ITag PlaylistType
        {
            get
            {
                return (iPlaylistType);
            }
        }

        public ITag PlaylistTitle
        {
            get
            {
                return (iPlaylistTitle);
            }
        }

        public ITag Artist
        {
            get
            {
                return (iArtist);
            }
        }

        public ITag Title
        {
            get
            {
                return (iTitle);
            }
        }

        public ITag Duration
        {
            get
            {
                return (iDuration);
            }
        }

        public ITag Uri
        {
            get
            {
                return (iUri);
            }
        }
    }

    internal class TagRealmContainer : TagRealmBase, ITagRealmContainer
    {
        private ITag iTitle;
        private ITag iDescription;
        private ITag iArtwork;

        public TagRealmContainer(ITagManagerInitialiser aInitialiser)
        {
            iTitle = Tag.CreateTagText(400, TagRealm.Container, "title");
            iDescription = Tag.CreateTagText(401, TagRealm.Container, "description");
            iArtwork = Tag.CreateTagText(402, TagRealm.Container, "artwork");

            Add(iTitle, aInitialiser);
            Add(iDescription, aInitialiser);
            Add(iArtwork, aInitialiser);
        }

        // ITagRealmPlaylist

        public ITag Title
        {
            get
            {
                return (iTitle);
            }
        }

        public ITag Description
        {
            get
            {
                return (iDescription);
            }
        }

        public ITag Artwork
        {
            get
            {
                return (iArtwork);
            }
        }
    }
*/



/*
    public class TagManager : ITagManager, ITagManagerInitialiser
    {
        Dictionary<TUint, ITag> iTags;
        Dictionary<TagRealm, ITagRealm> iRealms;

        ITagRealmSystem iSystem;

        ITagRealmGlobal iGlobal;
        ITagRealmAudio iAudio;
        ITagRealmVideo iVideo;
        ITagRealmImage iImage;
        ITagRealmPlaylist iPlaylist;
        ITagRealmContainer iContainer;

        public TagManager()
        {
            iTags = new Dictionary<TUint, ITag>();

            iSystem = new TagRealmSystem(this);
            iGlobal = new TagRealmGlobal(this);
            iAudio = new TagRealmAudio(this, iGlobal);
            iVideo = new TagRealmVideo(this, iGlobal);
            iImage = new TagRealmImage(this, iGlobal);
            iPlaylist = new TagRealmPlaylist(this, iGlobal);
            iContainer = new TagRealmContainer(this);

            iRealms = new Dictionary<TagRealm, ITagRealm>();

            iRealms.Add(TagRealm.System, iSystem);
            iRealms.Add(TagRealm.Global, iGlobal);
            iRealms.Add(TagRealm.Audio, iAudio);
            iRealms.Add(TagRealm.Video, iVideo);
            iRealms.Add(TagRealm.Image, iImage);
            iRealms.Add(TagRealm.Playlist, iPlaylist);
            iRealms.Add(TagRealm.Container, iContainer);
        }

        private void Add(Dictionary<string, ITag> aRealm, ITag aTag)
        {
            iTags.Add(aTag.Id, aTag);
            aRealm.Add(aTag.Name, aTag);
        }

        // ITagManager

        public TUint MaxSystemTagId
        {
            get
            {
                return (49);
            }
        }

        public ITag this[TUint aId]
        {
            get
            {
                return (iTags[aId]);
            }
        }

        public ITagRealm this[TagRealm aRealm]
        {
            get
            {
                return (iRealms[aRealm]);
            }
        }

        public ITagRealmSystem System
        {
            get
            {
                return (iSystem);
            }
        }

        public ITagRealmGlobal Global
        {
            get
            {
                return (iGlobal);
            }
        }

        public ITagRealmAudio Audio
        {
            get
            {
                return (iAudio);
            }
        }

        public ITagRealmVideo Video
        {
            get
            {
                return (iVideo);
            }
        }

        public ITagRealmImage Image
        {
            get
            {
                return (iImage);
            }
        }

        public ITagRealmPlaylist Playlist
        {
            get
            {
                return (iPlaylist);
            }
        }

        public ITagRealmContainer Container
        {
            get
            {
                return (iContainer);
            }
        }

        // ITagManagerInitialiser

        public void Add(ITag aTag)
        {
            iTags.Add(aTag.Id, aTag);
        }
    }
*/


} // Av

} // OpenHome


#endif // HEADER_OHTOPC_TAG
