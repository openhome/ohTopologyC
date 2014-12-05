#include <OpenHome/OhNetTypes.h>
#include <OpenHome/WatchableThread.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/DisposeHandler.h>
#include <OpenHome/MetaData.h>
#include <OpenHome/TagManager.h>
#include <OpenHome/Private/Debug.h>
#include <OpenHome/Net/Private/XmlParser.h>
#include <OpenHome/Private/Ascii.h>
#include <vector>


using namespace OpenHome;
using namespace OpenHome::Av;
using namespace OpenHome::Net;
using namespace std;


TagManager::TagManager()
{
    //iTags = new Dictionary<uint, ITag>();
    //iRealms = new Dictionary<TagRealm, ITagRealm>();

    //iSystem = new TagRealmSystem(this);
    iGlobal = new TagRealmGlobal(*this);
    iAudio = new TagRealmAudio(*this, *iGlobal);
    //iVideo = new TagRealmVideo(this, iGlobal);
    //iImage = new TagRealmImage(this, iGlobal);
    //iPlaylist = new TagRealmPlaylist(this, iGlobal);
    //iContainer = new TagRealmContainer(this);

    iRealms[eGlobal] = iGlobal;
    iRealms[eAudio] = iAudio;


/*
    iRealms.Add(TagRealm.System, iSystem);
    iRealms.Add(TagRealm.Global, iGlobal);
    iRealms.Add(TagRealm.Audio, iAudio);
    iRealms.Add(TagRealm.Video, iVideo);
    iRealms.Add(TagRealm.Image, iImage);
    iRealms.Add(TagRealm.Playlist, iPlaylist);
    iRealms.Add(TagRealm.Container, iContainer);
*/
}

TagManager::~TagManager()
{
   for(auto it=iTags.begin(); it!=iTags.end(); it++)
   {
        delete it->second;
   }

   for(auto it=iRealms.begin(); it!=iRealms.end(); it++)
   {
        delete it->second;
   }
}


void TagManager::Add(ITag* aTag)
{
    iTags[aTag->Id()] = aTag;
}


void TagManager::Add(map<Brn, ITag*, BufferCmp>& aRealm, ITag* aTag)
{
    iTags[aTag->Id()] = aTag;
    aRealm[aTag->Name()] = aTag;
}


IMediaMetadata* TagManager::FromDidlLite(const Brx& aMetadata)
{
    MediaMetadata* metadata = new MediaMetadata();

    try
    {
        if (!aMetadata.Equals(Brx::Empty()))
        {
            Brn title = XmlParserBasic::Find(Brn("dc:title"), aMetadata);

            Brn res = XmlParserBasic::Find(Brn("didl:res"), aMetadata);
            Brn album = XmlParserBasic::Find(Brn("upnp:album"), aMetadata);
            Brn artist = XmlParserBasic::Find(Brn("upnp:artist"), aMetadata);


            if(title != Brx::Empty())
            {
                metadata->Add(Audio().Title(), title);
            }

            if (res != Brx::Empty())
            {
                metadata->Add(Audio().Uri(), res);
            }

            if (album != Brx::Empty())
            {
                metadata->Add(Audio().Album(), album);
            }

            if (artist != Brx::Empty())
            {
                metadata->Add(Audio().Artist(), artist);
            }

            Brn remaining;
            Brn xmlElements(aMetadata);
            for(;;)
            {
                // search all artist elements and find the one which has a "role" attribute with a value of "AlbumArtist"
                // take the value of this element as the album artist name
                Brn artistElement =  XmlParserBasic::Element(Brn("upnp:artist"), xmlElements, remaining);

                if (artistElement!=Brx::Empty())
                {
                    Brn roleAttrValue = XmlParserBasic::FindAttribute(Brn("upnp:artist"), Brn("role"), artistElement);
                    if (roleAttrValue!=Brn("AlbumArtist"))
                    {
                        Brn albumArtist = XmlParserBasic::Find(Brn("upnp:artist"), artistElement);
                        metadata->Add(Audio().AlbumArtist(), albumArtist);
                        break;
                    }

                    if (remaining.Bytes()==0)
                    {
                        break;
                    }

                    xmlElements = remaining;
                }
                else
                {
                    break;
                }
            }

            for(;;)
            {
                Brn albumart = XmlParserBasic::Find(Brn("upnp:albumArtURI"), aMetadata);;
                if (albumart!=Brx::Empty())
                {
                    metadata->Add(Audio().Artwork(), albumart);
                }
                else
                {
                    break;
                }
            }

            for(;;)
            {
                Brn genre = XmlParserBasic::Find(Brn("upnp:genre"), aMetadata);;
                if (genre!=Brx::Empty())
                {
                    metadata->Add(Audio().Genre(), genre);
                }
                else
                {
                    break;
                }
            }
        }
    }
    catch(XmlError)
    {
        LOG(kTrace, Brn("\nInvalid aMetadata: \n"));
        LOG(kTrace, aMetadata);
        LOG(kTrace, "\n");
    }


    return metadata;


    /*
        XmlDocument document = new XmlDocument();
        XmlNamespaceManager nsManager = new XmlNamespaceManager(document.NameTable);
        nsManager.AddNamespace("didl", kNsDidl);
        nsManager.AddNamespace("upnp", kNsUpnp);
        nsManager.AddNamespace("dc", kNsDc);
        nsManager.AddNamespace("ldl", "urn:linn-co-uk/DIDL-Lite");
    */
        //try
        //{
            //document.LoadXml(aMetadata);

            //XmlNode title = document.SelectSingleNode("/didl:DIDL-Lite/*/dc:title", nsManager);

/*
            if(title != null)
            {
                if (title.FirstChild != null)
                {
                    metadata.Add(aTagManager.Audio.Title, title.FirstChild.Value);
                }
            }
*/

            //XmlNode res = document.SelectSingleNode("/didl:DIDL-Lite/*/didl:res", nsManager);
/*

            if (res != null)
            {
                if (res.FirstChild != null)
                {
                    metadata.Add(aTagManager.Audio.Uri, res.FirstChild.Value);
                }
            }
*/

            //XmlNodeList albumart = document.SelectNodes("/didl:DIDL-Lite/*/upnp:albumArtURI", nsManager);

/*
            foreach (XmlNode n in albumart)
            {
                if (n.FirstChild != null)
                {
                    metadata.Add(aTagManager.Audio.Artwork, n.FirstChild.Value);
                }
            }
*/
            //XmlNode album = document.SelectSingleNode("/didl:DIDL-Lite/*/upnp:album", nsManager);

/*
            if (album != null)
            {
                if (album.FirstChild != null)
                {
                    metadata.Add(aTagManager.Audio.Album, album.FirstChild.Value);
                }
            }
*/

            //XmlNode artist = document.SelectSingleNode("/didl:DIDL-Lite/*/upnp:artist", nsManager);

    /*
            if (artist != null)
            {
                if (artist.FirstChild != null)
                {
                    metadata.Add(aTagManager.Audio.Artist, artist.FirstChild.Value);
                }
            }
      */

            //XmlNodeList genre = document.SelectNodes("/didl:DIDL-Lite/*/upnp:genre", nsManager);

/*
            foreach (XmlNode n in genre)
            {
                if (n.FirstChild != null)
                {
                    metadata.Add(aTagManager.Audio.Genre, n.FirstChild.Value);
                }
            }
*/
            //XmlNode albumartist = document.SelectSingleNode("/didl:DIDL-Lite/*/upnp:artist[@role='AlbumArtist']", nsManager);

/*
            if (albumartist != null)
            {
                if (albumartist.FirstChild != null)
                {
                    metadata.Add(aTagManager.Audio.AlbumArtist, albumartist.FirstChild.Value);
                }
            }
*/
    //    }
        //catch (XmlException) { }
    //}

    //metadata->Add(System().Folder(), aMetadata);
}



void TagManager::ToDidlLite(IMediaMetadata& aMetadata, Bwx& aBuf)
{
/*
<DIDL-Lite xmlns="urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/">
  <item id="db/be6b8305a1f3ccddd3f0e619533b7d271" parentID="0" restricted="true">
    <dc:title xmlns:dc="http://purl.org/dc/elements/1.1/">Back in Black</dc:title>
    <upnp:class xmlns:upnp="urn:schemas-upnp-org:metadata-1-0/upnp/">object.item.audioItem.musicTrack</upnp:class>
    <upnp:udn xmlns:upnp="urn:schemas-upnp-org:metadata-1-0/upnp/">34ad33b4e52cd82d754cc1bd2db06fce</upnp:udn>
    <upnp:artist xmlns:upnp="urn:schemas-upnp-org:metadata-1-0/upnp/">AC/DC</upnp:artist>
    <upnp:artist role="composer" xmlns:upnp="urn:schemas-upnp-org:metadata-1-0/upnp/">Brian Johnson</upnp:artist>
    <upnp:artist role="composer" xmlns:upnp="urn:schemas-upnp-org:metadata-1-0/upnp/">Angus Young</upnp:artist>
    <upnp:artist role="composer" xmlns:upnp="urn:schemas-upnp-org:metadata-1-0/upnp/">Malcolm Young</upnp:artist>
    <upnp:artist role="albumartist" xmlns:upnp="urn:schemas-upnp-org:metadata-1-0/upnp/">AC/DC</upnp:artist>
    <upnp:album xmlns:upnp="urn:schemas-upnp-org:metadata-1-0/upnp/">Back in Black</upnp:album>
    <dc:date xmlns:dc="http://purl.org/dc/elements/1.1/">1980-01-01</dc:date>
    <upnp:genre xmlns:upnp="urn:schemas-upnp-org:metadata-1-0/upnp/">Rock</upnp:genre>
    <upnp:originalTrackNumber xmlns:upnp="urn:schemas-upnp-org:metadata-1-0/upnp/">6</upnp:originalTrackNumber>
    <res channels="2" bitrate="0" duration="00:04:15" protocolInfo="http-get:*:audio/x-flac:*">http://10.2.10.154:4000/linn.co.uk.kazooserver/ms/audio/e6b8305a1f3ccddd3f0e619533b7d271</res>
    <upnp:albumArtURI dlna:profileID="JPEG_TN" xmlns:dlna="urn:schemas-dlna-org:metadata-1-0/" xmlns:upnp="urn:schemas-upnp-org:metadata-1-0/upnp/">http://10.2.10.154:4000/linn.co.uk.kazooserver/ms/artwork/e211a2ee99ce772e45bd836f00430534?size=0</upnp:albumArtURI>
  </item>
</DIDL-Lite></Metadata></Entry></TrackList>
*/

    auto metadata = aMetadata.Values();


    aBuf.Replace(Brn("<DIDL-Lite xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\">"));
    aBuf.Append(Brn("<item"));
    aBuf.Append(Brn("<dc:title>"));
    aBuf.Append(metadata[iAudio->Title()]->Value());
    aBuf.Append(Brn("</dctitle>"));

    aBuf.Append("<upnp:class xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\">object.item.audioItem.musicTrack</upnp:class>");


    //if (metadata[iAudio->Artwork()] != NULL)
    if ( metadata.count((iAudio->Artwork())) > 0)
    {
        // <upnp:albumArtURI dlna:profileID="JPEG_TN" xmlns:dlna="urn:schemas-dlna-org:metadata-1-0/" xmlns:upnp="urn:schemas-upnp-org:metadata-1-0/upnp/">http://10.2.10.154:4000/linn.co.uk.kazooserver/ms/artwork/e211a2ee99ce772e45bd836f00430534?size=0</upnp:albumArtURI>
        auto artworkValues = metadata[iAudio->Artwork()]->Values();
        for(TUint i=0; i<artworkValues.size(); i++)
        {
            aBuf.Append(Brn("<upnp:albumArtURI dlna:profileID=\"JPEG_TN\" xmlns:dlna=\"urn:schemas-dlna-org:metadata-1-0/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\">"));
            aBuf.Append(artworkValues[i]);
            aBuf.Append(Brn("</upnp:albumArtURI>"));
        }
    }


    if (metadata.count(iAudio->AlbumTitle()) > 0)
    {
        // <upnp:album xmlns:upnp="urn:schemas-upnp-org:metadata-1-0/upnp/">Back in Black</upnp:album>
        aBuf.Append(Brn("<upnp:album xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\">"));
        aBuf.Append(metadata[iAudio->AlbumTitle()]->Value());
        aBuf.Append(Brn("</upnp:album>"));
    }


    if (metadata.count(iAudio->Artist()) > 0)
    {
        // <upnp:artist xmlns:upnp="urn:schemas-upnp-org:metadata-1-0/upnp/">AC/DC</upnp:artist>
        auto artistValues = metadata[iAudio->Artist()]->Values();
        for (TUint i=0; i<artistValues.size(); i++)
        {
            aBuf.Append(Brn("<upnp:artist xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\">"));
            aBuf.Append(artistValues[i]);
            aBuf.Append(Brn("</upnp:artist>"));
        }
    }

    if (metadata.count(iAudio->AlbumArtist()) > 0)
    {
        // <upnp:artist role="albumartist" xmlns:upnp="urn:schemas-upnp-org:metadata-1-0/upnp/">AC/DC</upnp:artist>
        aBuf.Append(Brn("<upnp:artist role=\"albumartist\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\">"));
        aBuf.Append(metadata[iAudio->AlbumArtist()]->Value());
        aBuf.Append(Brn("</upnp:artist>"));
    }

    aBuf.Append(Brn("</item>"));
    aBuf.Append(Brn("</DIDL-Lite>"));


/*
    if (aMetadata == null)
    {
        return string.Empty;
    }
    if (aMetadata[aTagManager.System.Folder] != null)
    {
        return aMetadata[aTagManager.System.Folder].Value;
    }

    XmlDocument document = new XmlDocument();

    XmlElement didl = document.CreateElement("DIDL-Lite", kNsDidl);

    XmlElement container = document.CreateElement("item", kNsDidl);

    XmlElement title = document.CreateElement("dc", "title", kNsDc);
    title.AppendChild(document.CreateTextNode(aMetadata[aTagManager.Audio.Title].Value));

    container.AppendChild(title);

    XmlElement cls = document.CreateElement("upnp", "class", kNsUpnp);
    cls.AppendChild(document.CreateTextNode("object.item.audioItem.musicTrack"));

    container.AppendChild(cls);

    if (aMetadata[aTagManager.Audio.Artwork] != null)
    {
        foreach (var a in aMetadata[aTagManager.Audio.Artwork].Values)
        {
            XmlElement artwork = document.CreateElement("upnp", "albumArtURI", kNsUpnp);
            artwork.AppendChild(document.CreateTextNode(a));
            container.AppendChild(artwork);
        }
    }

    if (aMetadata[aTagManager.Audio.AlbumTitle] != null)
    {
        XmlElement albumtitle = document.CreateElement("upnp", "album", kNsUpnp);
        albumtitle.AppendChild(document.CreateTextNode(aMetadata[aTagManager.Audio.AlbumTitle].Value));
        container.AppendChild(albumtitle);
    }

    if (aMetadata[aTagManager.Audio.Artist] != null)
    {
        foreach (var a in aMetadata[aTagManager.Audio.Artist].Values)
        {
            XmlElement artist = document.CreateElement("upnp", "artist", kNsUpnp);
            artist.AppendChild(document.CreateTextNode(a));
            container.AppendChild(artist);
        }
    }

    if (aMetadata[aTagManager.Audio.AlbumArtist] != null)
    {
        XmlElement albumartist = document.CreateElement("upnp", "artist", kNsUpnp);
        albumartist.AppendChild(document.CreateTextNode(aMetadata[aTagManager.Audio.AlbumArtist].Value));
        XmlAttribute role = document.CreateAttribute("upnp", "role", kNsUpnp);
        role.AppendChild(document.CreateTextNode("albumartist"));
        albumartist.Attributes.Append(role);
        container.AppendChild(albumartist);
    }

    didl.AppendChild(container);

    document.AppendChild(didl);

    return document.OuterXml;
*/
}




TUint TagManager::MaxSystemTagId()
{
    return(kMaxSystemTagId);
}


ITag* TagManager::Tag(TUint aId)
{
    return(iTags[aId]);
}


ITagRealm& TagManager::Realm(ETagRealm aRealm)
{
    return(*(iRealms[aRealm]));
}


ITagRealmAudio& TagManager::Audio()
{
    return (*iAudio);
}



////////////////////////////////////////////////////////


MediaValue::MediaValue(const Brx& aValue)
    :iValue(aValue)
{
    iValues.push_back(Brn(aValue));
}


MediaValue::MediaValue(const vector<Brn>& aValues)
    :iValues(aValues)
    ,iValue(aValues[0])
{
}

// IMediaServerValue
Brn MediaValue::Value()
{
    return (Brn(iValue));
}


const vector<Brn>& MediaValue::Values()
{
    return (iValues);
}




/////////////////////////////////////////////////

MediaDictionary::MediaDictionary()
{
}


MediaDictionary::MediaDictionary(IMediaMetadata& aMetadata)
    :iMetadata(aMetadata.Values())
{
}


void MediaDictionary::Add(ITag* aTag, const Brx& aValue)
{
/*
    IMediaValue value = null;

    iMetadata.TryGetValue(aTag&, out value);

    if (value == null)
    {
        iMetadata[aTag] = new MediaValue(aValue);
    }
    else
    {
        iMetadata[aTag] = new MediaValue(value.Values.Concat(new string[] { aValue }));
    }
*/
    if(iMetadata.count(aTag)==0)
    {
        iMetadata[aTag] = new MediaValue(aValue);
    }
    else
    {
        vector<Brn> values(iMetadata[aTag]->Values());
        values.push_back(Brn(aValue));
        iMetadata[aTag] = new MediaValue(values);


        //iMetadata[aTag] = new MediaValue(value.Values.Concat(new string[] { aValue }));
    }
}


void MediaDictionary::Add(ITag* aTag, IMediaValue& aValue)
{
/*
    IMediaValue* value = NULL;
    iMetadata.TryGetValue(aTag, out value);

    if (value == null)
    {
        iMetadata[aTag] = aValue;
    }
    else
    {
        iMetadata[aTag] = new MediaValue(value.Values.Concat(aValue.Values));
    }
*/

    if(iMetadata.count(aTag)==0)
    {
        iMetadata[aTag] = &aValue;
    }
    else
    {
        vector<Brn> values(iMetadata[aTag]->Values());
        values.insert(values.end(), aValue.Values().begin(), aValue.Values().end());
        iMetadata[aTag] = new MediaValue(values);

        //iMetadata[aTag] = new MediaValue(value.Values.Concat(aValue.Values));
    }
}


void MediaDictionary::Add(ITag* aTag, IMediaMetadata& aMetadata)
{
    //var value = aMetadata[aTag];
    IMediaValue* value = aMetadata.Value(aTag);

    if (value != NULL)
    {
        Add(aTag, *value);
    }
}

// IMediaServerMetadata
IMediaValue* MediaDictionary::Value(ITag* aTag)
{
/*
    IMediaValue* value = null;
    iMetadata.TryGetValue(aTag, out value);
    return (value);
*/

    if(iMetadata.count(aTag)>0)
    {
        return(iMetadata[aTag]);
    }
    else
    {
        return(NULL);
    }
}

//////////////////////////////////////////////////////

MediaMetadata::MediaMetadata()
{
}


const map<ITag*, IMediaValue*> MediaMetadata::Values()
{
    return(iMetadata);
}


/////////////////////////////////////////////////////

