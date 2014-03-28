#include <OpenHome/Tag.h>


using namespace OpenHome;
using namespace OpenHome::Av;

Tag::Tag(TUint aId, ETagRealm aRealm, const Brx& aName, TBool aKey, TBool aNumeric, TBool aSearchable, TBool aArticled)
    :iId(aId)
    ,iRealm(aRealm)
    ,iName(aName)
    ,iKey(aKey)
    ,iNumeric(aNumeric)
    ,iSearchable(aSearchable)
    ,iArticled(aArticled)
{
    iFullName.Replace(RealmToString(iRealm));
    iFullName.Append(".");
    iFullName.Append(iName);
}

ITag* Tag::CreateTagText(TUint aId, ETagRealm aRealm, const Brx& aName)
{
    return (new Tag(aId, aRealm, aName, false, false, false, false));
}

ITag* Tag::CreateTagNumeric(TUint aId, ETagRealm aRealm, const Brx& aName)
{
    return (new Tag(aId, aRealm, aName, false, true, false, false));
}

ITag* Tag::CreateTagKey(TUint aId, ETagRealm aRealm, const Brx& aName)
{
    return (new Tag(aId, aRealm, aName, true, true, false, false));
}

ITag* Tag::CreateTagSearchable(TUint aId, ETagRealm aRealm, const Brx& aName)
{
    return (new Tag(aId, aRealm, aName, false, false, true, false));
}

ITag* Tag::CreateTagArticled(TUint aId, ETagRealm aRealm, const Brx& aName)
{
    return (new Tag(aId, aRealm, aName, false, false, true, true));
}

Brn Tag::ToString()
{
    return FullName();
}

// ITag

TUint Tag::Id()
{
    return (iId);
}

ETagRealm Tag::Realm()
{
    return (iRealm);
}

Brn Tag::Name()
{
    return (iName);
}

Brn Tag::FullName()
{
    return(Brn(iFullName));
    //return (iRealm.ToString().ToLowerInvariant() + "." + iName);
}

Brn Tag::RealmToString(ETagRealm aRealm)
{
    switch(aRealm)
    {
        case eSystem:
            return(Brn("system"));
        case eGlobal:
            return(Brn("global"));
        case eAudio:
            return(Brn("audio"));
        case eVideo:
            return(Brn("video"));
        case eImage:
            return(Brn("image"));
        case ePlaylist:
            return(Brn("playlist"));
        case eContainer:
            return(Brn("container"));
        default:
            ASSERTS();
            break;
    };

    return(Brx::Empty());
}


TBool Tag::IsKey()
{
    return (iKey);
}

TBool Tag::IsNumeric()
{
    return (iNumeric);
}

TBool Tag::IsSearchable()
{
    return (iSearchable);
}

TBool Tag::IsArticled()
{
    return (iArticled);
}

TBool Tag::IsHyper()
{
    return (false);
}

///////////////////////////////////////////////////////////////


TagRealmBase::TagRealmBase()
    :iGlobal(NULL)
{
}

TagRealmBase::TagRealmBase(ITagRealmGlobal& aGlobal)
    :iGlobal(&aGlobal)
{
}

void TagRealmBase::Add(ITag* aTag, ITagManagerInitialiser& aInitialiser)
{
    iTags[aTag->Name()] = aTag;
    aInitialiser.Add(aTag);
}

// ITagRealm

ITag* TagRealmBase::Tag(const Brx& aName)
{
    if (iTags.count(Brn(aName))>0)
    {
        return(iTags[Brn(aName)]);
    }
    else
    {
        if (iGlobal != NULL)
        {
            return(iGlobal->Tag(Brn(aName)));
        }
    }


    return(NULL);

/*
    ITag tag;

    iTags.TryGetValue(aName, out tag);

    if (tag == null)
    {
        if (iGlobal != null)
        {
            tag = iGlobal[aName];
        }
    }

    return (tag);
*/
}

///////////////////////////////////////////////////////////////



/*
    class TagRealmSystem : TagRealmBase, ITagRealmSystem
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


TagRealmGlobal::TagRealmGlobal(ITagManagerInitialiser& aInitialiser)
    : TagRealmBase()
{
    iHash = Tag::CreateTagText(50, eGlobal, Brn("hash"));

    Add(iHash, aInitialiser);
}

ITag* TagRealmGlobal::Hash()
{
    return (iHash);
}


//////////////////////////////////////////////////////////////////////////////////////////////////

TagRealmAudio::TagRealmAudio(ITagManagerInitialiser& aInitialiser, ITagRealmGlobal& aGlobal)
    : TagRealmBase(aGlobal)
{

    iType = Tag::CreateTagText(100, eAudio, Brn("type"));
    iDescription = Tag::CreateTagText(101, eAudio, Brn("description"));
    iChannels = Tag::CreateTagNumeric(102, eAudio, Brn("channels"));
    iBitdepth = Tag::CreateTagNumeric(103, eAudio, Brn("bitdepth"));
    iSamplerate = Tag::CreateTagNumeric(104, eAudio, Brn("samplerate"));
    iBitrate = Tag::CreateTagNumeric(105, eAudio, Brn("bitrate"));
    iDuration = Tag::CreateTagNumeric(106, eAudio, Brn("duration"));
    iCodec = Tag::CreateTagText(107, eAudio, Brn("codec"));
    iArtist = Tag::CreateTagArticled(108, eAudio, Brn("artist"));
    iBpm = Tag::CreateTagNumeric(109, eAudio, Brn("bpm"));
    iComposer = Tag::CreateTagSearchable(110, eAudio, Brn("composer"));
    iConductor = Tag::CreateTagSearchable(111, eAudio, Brn("conductor"));
    iDisc = Tag::CreateTagNumeric(112, eAudio, Brn("disc"));
    iGenre = Tag::CreateTagText(114, eAudio, Brn("genre"));
    iGrouping = Tag::CreateTagText(115, eAudio, Brn("grouping"));
    iLyrics = Tag::CreateTagText(116, eAudio, Brn("lyrics"));
    iTitle = Tag::CreateTagSearchable(118, eAudio, Brn("title"));
    iTrack = Tag::CreateTagNumeric(119, eAudio, Brn("track"));
    iTracks = Tag::CreateTagNumeric(120, eAudio, Brn("tracks"));
    iYear = Tag::CreateTagNumeric(121, eAudio, Brn("year"));
    iArtwork = Tag::CreateTagNumeric(122, eAudio, Brn("artwork"));
    iUri = Tag::CreateTagNumeric(123, eAudio, Brn("uri"));
    iWeight = Tag::CreateTagText(124, eAudio, Brn("weight"));

    iAlbum = Tag::CreateTagKey(200, eAudio, Brn("album"));
    iAlbumTitle = Tag::CreateTagSearchable(201, eAudio, Brn("album.title"));
    iAlbumArtist = Tag::CreateTagArticled(202, eAudio, Brn("album.artist"));
    iAlbumArtworkFilename = Tag::CreateTagText(203, eAudio, Brn("album.artwork.filename"));
    iAlbumArtworkCodec = Tag::CreateTagText(204, eAudio, Brn("album.artwork.codec"));
    iAlbumDiscs = Tag::CreateTagNumeric(205, eAudio, Brn("album.discs"));
    iAlbumYear = Tag::CreateTagNumeric(206, eAudio, Brn("album.year"));
    iAlbumGenre = Tag::CreateTagText(207, eAudio, Brn("album.genre"));


    Add(iType, aInitialiser);
    Add(iDescription, aInitialiser);
    Add(iChannels, aInitialiser);
    Add(iBitdepth, aInitialiser);
    Add(iSamplerate, aInitialiser);
    Add(iBitrate, aInitialiser);
    Add(iDuration, aInitialiser);
    Add(iCodec, aInitialiser);
    Add(iArtist, aInitialiser);
    Add(iBpm, aInitialiser);
    Add(iComposer, aInitialiser);
    Add(iConductor, aInitialiser);
    Add(iDisc, aInitialiser);
    Add(iGenre, aInitialiser);
    Add(iGrouping, aInitialiser);
    Add(iLyrics, aInitialiser);
    Add(iTitle, aInitialiser);
    Add(iTrack, aInitialiser);
    Add(iTracks, aInitialiser);
    Add(iYear, aInitialiser);
    Add(iArtwork, aInitialiser);
    Add(iUri, aInitialiser);
    Add(iWeight, aInitialiser);

    Add(iAlbum, aInitialiser);
    Add(iAlbumTitle, aInitialiser);
    Add(iAlbumArtist, aInitialiser);
    Add(iAlbumArtworkFilename, aInitialiser);
    Add(iAlbumArtworkCodec, aInitialiser);
    Add(iAlbumDiscs, aInitialiser);
    Add(iAlbumYear, aInitialiser);
    Add(iAlbumGenre, aInitialiser);

}

    // ITagRealmAudio

ITag* TagRealmAudio::Type()
{
    return (iType);
}

ITag* TagRealmAudio::Description()
{
    return (iDescription);
}

ITag* TagRealmAudio::Channels()
{
    return (iChannels);
}

ITag* TagRealmAudio::Bitdepth()
{
    return (iBitdepth);
}

ITag* TagRealmAudio::Samplerate()
{
    return (iSamplerate);
}

ITag* TagRealmAudio::Bitrate()
{
    return (iBitrate);
}

ITag* TagRealmAudio::Duration()
{
    return (iDuration);
}

ITag* TagRealmAudio::Codec()
{
    return (iCodec);
}

ITag* TagRealmAudio::Artist()
{
    return (iArtist);
}

ITag* TagRealmAudio::Bpm()
{
    return (iBpm);
}

ITag* TagRealmAudio::Composer()
{
    return (iComposer);
}

ITag* TagRealmAudio::Conductor()
{
    return (iConductor);
}

ITag* TagRealmAudio::Disc()
{
    return (iDisc);
}

ITag* TagRealmAudio::Genre()
{
    return (iGenre);
}

ITag* TagRealmAudio::Grouping()
{
    return (iGrouping);
}

ITag* TagRealmAudio::Lyrics()
{
    return (iLyrics);
}

ITag* TagRealmAudio::Title()
{
    return (iTitle);
}

ITag* TagRealmAudio::Track()
{
    return (iTrack);
}

ITag* TagRealmAudio::Tracks()
{
    return (iTracks);
}

ITag* TagRealmAudio::Year()
{
    return (iYear);
}

ITag* TagRealmAudio::Artwork()
{
    return (iArtwork);
}

ITag* TagRealmAudio::Uri()
{
    return (iUri);
}

ITag* TagRealmAudio::Weight()
{
    return (iWeight);
}

ITag* TagRealmAudio::Album()
{
    return (iAlbum);
}

ITag* TagRealmAudio::AlbumTitle()
{
    return (iAlbumTitle);
}

ITag* TagRealmAudio::AlbumArtist()
{
    return (iAlbumArtist);
}

ITag* TagRealmAudio::AlbumArtworkFilename()
{
    return (iAlbumArtworkFilename);
}

ITag* TagRealmAudio::AlbumArtworkCodec()
{
    return (iAlbumArtworkCodec);
}

ITag* TagRealmAudio::AlbumDiscs()
{
    return (iAlbumDiscs);
}

ITag* TagRealmAudio::AlbumYear()
{
    return (iAlbumYear);
}

ITag* TagRealmAudio::AlbumGenre()
{
    return (iAlbumGenre);
}

//////////////////////////////////////////////////////////////////

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
            iType = Tag::CreateTagText(800, eImage, Brn("type"));
            iTitle = Tag::CreateTagSearchable(801, eImage, Brn("title"));
            iEmbedded = Tag::CreateTagText(802, eImage, Brn("embedded"));
            iCodec = Tag::CreateTagText(803, eImage, Brn("codec"));
            iWidth = Tag::CreateTagNumeric(804, eImage, Brn("width"));
            iHeight = Tag::CreateTagNumeric(805, eImage, Brn("height"));

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
            iPlaylist = Tag::CreateTagKey(300, ePlaylist, Brn("playlist"));
            iPlaylistType = Tag::CreateTagText(301, ePlaylist, Brn("playlist.type"));
            iPlaylistTitle = Tag::CreateTagText(302, ePlaylist, Brn("playlist.title"));
            iArtist = Tag::CreateTagText(303, ePlaylist, Brn("artist"));
            iTitle = Tag::CreateTagText(304, ePlaylist, Brn("title"));
            iDuration = Tag::CreateTagNumeric(305, ePlaylist, Brn("duration"));
            iUri = Tag::CreateTagText(306, ePlaylist, Brn("uri"));

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
            iTitle = Tag::CreateTagText(400, eContainer, Brn("title"));
            iDescription = Tag::CreateTagText(401, eContainer, Brn("description"));
            iArtwork = Tag::CreateTagText(402, eContainer, Brn("artwork"));

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

            iRealms.Add(eSystem, iSystem);
            iRealms.Add(eGlobal, iGlobal);
            iRealms.Add(eAudio, iAudio);
            iRealms.Add(eVideo, iVideo);
            iRealms.Add(eImage, iImage);
            iRealms.Add(ePlaylist, iPlaylist);
            iRealms.Add(eContainer, iContainer);
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
}
