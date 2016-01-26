#include <OpenHome/OhNetTypes.h>
#include <OpenHome/WatchableThread.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/DisposeHandler.h>
#include <OpenHome/MetaData.h>
#include <OpenHome/TagManager.h>
#include <OpenHome/Private/Debug.h>
#include <OpenHome/Net/Private/XmlParser.h>
#include <OpenHome/Private/Ascii.h>
#include <OpenHome/Private/Converter.h>
#include <vector>


using namespace OpenHome;
using namespace OpenHome::Topology;
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

Brn TagManager::GoFind(const Brx& aTag, const Brx& aMetadata)
{
    Brn metadata(aMetadata);
    try
    {
        Brn value = XmlParserBasic::Find(aTag, metadata);
        Bwn escaped(value.Ptr(), value.Bytes(), value.Bytes());
        Converter::FromXmlEscaped(escaped);
        value.Set(escaped.Ptr(), escaped.Bytes());
        return value;
    }
    catch(XmlError&)
    {
        return Brx::Empty();
    }
}

Brn TagManager::GoFindAttribute(const Brx& aTag, const Brx& aAttribute, const Brx& aMetadata)
{
    Brn metadata(aMetadata);
    try
    {
        Brn value = XmlParserBasic::Find(aTag, aAttribute, metadata);
        Bwn escaped(value.Ptr(), value.Bytes(), value.Bytes());
        Converter::FromXmlEscaped(escaped);
        value.Set(escaped.Ptr(), escaped.Bytes());
        return value;
    }
    catch(XmlError&)
    {
        return Brx::Empty();
    }
}

Brn TagManager::GoFindElement(const Brx& aTag, const Brx& aElements, const Brx& aMetadata)
{
    Brn metadata(aMetadata);
    try
    {
        Brn value = XmlParserBasic::Find(aTag, aElements, metadata);
        Bwn escaped(value.Ptr(), value.Bytes(), value.Bytes());
        Converter::FromXmlEscaped(escaped);
        value.Set(escaped.Ptr(), escaped.Bytes());
        return value;
    }
    catch(XmlError&)
    {
        return Brx::Empty();
    }
}

std::unique_ptr<MediaMetadata> TagManager::FromDidlLite(const Brx& aMetadata)
{
    std::unique_ptr<MediaMetadata> metadata(new MediaMetadata()); //!FIXME Change to make_unique when C++14 supported

    if (!aMetadata.Equals(Brx::Empty()))
    {
        Brn title(GoFind(Brn("title"), aMetadata));
        Brn res(GoFind(Brn("res"), aMetadata));
        Brn album(GoFind(Brn("album"), aMetadata));
        Brn artist(GoFind(Brn("artist"), aMetadata));


        if (title != Brx::Empty())
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


            //<res channels="2" bitrate="0" duration="00:04:15" protocolInfo="http-get:*:audio/x-flac:*">http://10.2.10.154:4000/linn.co.uk.kazooserver/ms/audio/e6b8305a1f3ccddd3f0e619533b7d271</res>
            //XmlNode protocolInfo = document.SelectSingleNode("//*/didl:res/@protocolInfo", nsManager);

        Brn protocolInfo = GoFindAttribute(Brn("res"), Brn("protocolInfo"), aMetadata);
        if (protocolInfo != Brx::Empty())
        {
            if (Ascii::Contains(protocolInfo, Brn("flac")))
            {
                    metadata->Add(Audio().Codec(), Brn("flac"));
            }
            else if ( Ascii::Contains(protocolInfo, Brn("alac")) || Ascii::Contains(protocolInfo, Brn("m4a")) )
            {
                    metadata->Add(Audio().Codec(), Brn("alac"));
            }
            else if ( Ascii::Contains(protocolInfo, Brn("mpeg")) || Ascii::Contains(protocolInfo, Brn("mp1")) )
            {
                    metadata->Add(Audio().Codec(), Brn("mp3"));
            }
            else if (Ascii::Contains(protocolInfo, Brn("wma")))
            {
                    metadata->Add(Audio().Codec(), Brn("wma"));
            }
            else if (Ascii::Contains(protocolInfo, Brn("wav")))
            {
                    metadata->Add(Audio().Codec(), Brn("wav"));
            }
            else if (Ascii::Contains(protocolInfo, Brn("ogg")))
            {
                    metadata->Add(Audio().Codec(), Brn("ogg"));
            }
            else if (Ascii::Contains(protocolInfo, Brn("aac")))
            {
                    metadata->Add(Audio().Codec(), Brn("aac"));
            }
            else if (Ascii::Contains(protocolInfo, Brn("aiff")))
            {
                    metadata->Add(Audio().Codec(), Brn("aiff"));
            }
        }


        Brn bitrate = GoFindAttribute(Brn("res"), Brn("bitrate"), aMetadata);
        if (bitrate != Brx::Empty())
        {
            // convert from bytes/s to bits/s
            // convert string to int, multiply by 8, convert back to string
            TUint bytesPerSec = Ascii::Uint(bitrate)*8;
            Bws<20> bytesPerSecBuf;
            Ascii::AppendDec(bytesPerSecBuf, bytesPerSec);

            metadata->Add(Audio().Bitrate(), bytesPerSecBuf);
        }
        Brn sampleFrequency = GoFindAttribute(Brn("res"), Brn("sampleFrequency"), aMetadata);
        if (sampleFrequency != Brx::Empty())
        {
            metadata->Add(Audio().Samplerate(), sampleFrequency);
        }
        Brn bitsPerSample = GoFindAttribute(Brn("res"), Brn("bitsPerSample"), aMetadata);
        if (bitsPerSample != Brx::Empty())
        {
            metadata->Add(Audio().Bitdepth(), bitsPerSample);
        }



        Brn remaining;
        Brn xmlElements(aMetadata);
        for(;;)
        {
            // search all artist elements and find the one which has a "role" attribute with a value of "AlbumArtist"
            // take the value of this element as the album artist name
            Brn artistElement =  GoFindElement(Brn("upnp:artist"), xmlElements, remaining);

            if (artistElement!=Brx::Empty())
            {
                Brn roleAttrValue = GoFindAttribute(Brn("upnp:artist"), Brn("role"), artistElement);
                if (roleAttrValue!=Brn("AlbumArtist"))
                {
                    Brn albumArtist = GoFind(Brn("upnp:artist"), artistElement);
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
            Brn albumart = GoFind(Brn("upnp:albumArtURI"), aMetadata);;
            if (albumart!=Brx::Empty())
            {
                metadata->Add(Audio().Artwork(), albumart);
                break;
            }
            else
            {
                break;
            }
        }

        for(;;)
        {
            Brn genre = GoFind(Brn("upnp:genre"), aMetadata);;
            if (genre!=Brx::Empty())
            {
                metadata->Add(Audio().Genre(), genre);
                break;
            }
            else
            {
                break;
            }
        }
    }

    return metadata;
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

    auto metadata = aMetadata.Values();  // map

    aBuf.Replace(Brn("<DIDL-Lite xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\">"));
    aBuf.Append(Brn("<item"));

    if (metadata[iAudio->Title()] != NULL)
    {
        aBuf.Append(Brn("<dc:title>"));
        aBuf.Append(metadata[iAudio->Title()]->Value());
        aBuf.Append(Brn("</dctitle>"));
    }

    aBuf.Append("<upnp:class xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\">object.item.audioItem.musicTrack</upnp:class>");


    if (metadata[iAudio->Artwork()] != NULL)
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


    if (metadata[iAudio->AlbumTitle()] != NULL)
    {
        // <upnp:album xmlns:upnp="urn:schemas-upnp-org:metadata-1-0/upnp/">Back in Black</upnp:album>
        aBuf.Append(Brn("<upnp:album xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\">"));
        aBuf.Append(metadata[iAudio->AlbumTitle()]->Value());
        aBuf.Append(Brn("</upnp:album>"));
    }



    if (metadata[iAudio->Artist()] != NULL)
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

    if (metadata[iAudio->AlbumArtist()] != NULL)
    {
        // <upnp:artist role="albumartist" xmlns:upnp="urn:schemas-upnp-org:metadata-1-0/upnp/">AC/DC</upnp:artist>
        auto albumArtistValues = metadata[iAudio->AlbumArtist()]->Values();
        for (TUint i=0; i<albumArtistValues.size(); i++)
        {
            aBuf.Append(Brn("<upnp:artist role=\"Albumartist\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\">"));
            aBuf.Append(albumArtistValues[i]);
            aBuf.Append(Brn("</upnp:artist>"));
        }
    }

    if (metadata[iAudio->Composer()] != NULL)
    {
        auto composerValues = metadata[iAudio->Composer()]->Values();
        for (TUint i=0; i<composerValues.size(); i++)
        {
            /*
            XmlElement composer = document.CreateElement("upnp", "artist", kNsUpnp);
            composer.AppendChild(document.CreateTextNode(a));
            XmlAttribute role = document.CreateAttribute("role");
            role.AppendChild(document.CreateTextNode("Composer"));
            composer.Attributes.Append(role);
            container.AppendChild(composer);

            */
            aBuf.Append(Brn("<upnp:artist role=\"Composer\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\">"));
            aBuf.Append(composerValues[i]);
            aBuf.Append(Brn("</upnp:artist>"));
        }
    }

    if (metadata[iAudio->Year()] != NULL)
    {
        /*
        XmlElement date = document.CreateElement("dc", "date", kNsDc);
        date.AppendChild(document.CreateTextNode(aMetadata[aTagManager.Audio.Year].Value));
        container.AppendChild(date);
        */
        aBuf.Append(Brn("<dc:date xmlns:dc=\"urn:schemas-upnp-org:metadata-1-0/upnp/\">"));
        aBuf.Append(metadata[iAudio->Year()]->Value());
        aBuf.Append(Brn("</dc:date>"));
    }


    if (metadata[iAudio->Genre()] != NULL)
    {
        auto genreValues = metadata[iAudio->Genre()]->Values();
        for (TUint i=0; i<genreValues.size(); i++)
        {
            /*
            XmlElement genre = document.CreateElement("upnp", "genre", kNsUpnp);
            genre.AppendChild(document.CreateTextNode(a));
            container.AppendChild(genre);
            */
            aBuf.Append(Brn("<upnp:genre xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\">"));
            aBuf.Append(genreValues[i]);
            aBuf.Append(Brn("</upnp:genre>"));

        }
    }


    if (metadata[iAudio->Uri()] != NULL)
    {
        // <res channels="2" bitrate="0" duration="00:04:15" protocolInfo="http-get:*:audio/x-flac:*">
        // http://10.2.10.154:4000/linn.co.uk.kazooserver/ms/audio/e6b8305a1f3ccddd3f0e619533b7d271
        // </res>
        aBuf.Append("<res ");

        if (metadata[iAudio->Samplerate()] != NULL)
        {
            aBuf.Append("sampleFrequency=\" ");
            aBuf.Append(metadata[iAudio->Samplerate()]->Value());
            aBuf.Append("\"");
        }

        if (metadata[iAudio->Bitdepth()] != NULL)
        {
            aBuf.Append("bitsPerSample=\" ");
            aBuf.Append(metadata[iAudio->Bitdepth()]->Value());
            aBuf.Append("\"");
        }

        if (metadata[iAudio->Bitrate()] != NULL)
        {
            aBuf.Append("bitrate=\"");
            // convert from bits/s to bytes/s
            // convert string to int, divide by 8, convert back to string
            TUint bitsPerSec = Ascii::Uint(metadata[iAudio->Bitrate()]->Value());
            TUint bytesPerSec = bitsPerSec/8;
            Ascii::AppendDec(aBuf, bytesPerSec);
            aBuf.Append("\"");
        }

        aBuf.Append(">");
        aBuf.Append(metadata[iAudio->Uri()]->Value());
        aBuf.Append("</res>");

    }
/*
            if (aMetadata[aTagManager.Audio.Uri] != null)
            {
                XmlElement res = document.CreateElement("res", kNsDidl);
                res.AppendChild(document.CreateTextNode(aMetadata[aTagManager.Audio.Uri].Value));

                if (aMetadata[aTagManager.Audio.Samplerate] != null)
                {
                    XmlAttribute sampleFrequency = document.CreateAttribute("sampleFrequency");
                    sampleFrequency.AppendChild(document.CreateTextNode(aMetadata[aTagManager.Audio.Samplerate].Value));
                    res.Attributes.Append(sampleFrequency);
                }
                if (aMetadata[aTagManager.Audio.Bitdepth] != null)
                {
                    XmlAttribute bitdepth = document.CreateAttribute("bitsPerSample");
                    bitdepth.AppendChild(document.CreateTextNode(aMetadata[aTagManager.Audio.Bitdepth].Value));
                    res.Attributes.Append(bitdepth);
                }
                if (aMetadata[aTagManager.Audio.Bitrate] != null)
                {
                    uint parsedBitrate;
                    if (uint.TryParse(aMetadata[aTagManager.Audio.Bitrate].Value, out parsedBitrate))
                    {
                        XmlAttribute bitrate = document.CreateAttribute("bitrate");
                        // convert from bits/s to bytes/s
                        bitrate.AppendChild(document.CreateTextNode((parsedBitrate / 8).ToString()));
                        res.Attributes.Append(bitrate);
                    }
                }
                container.AppendChild(res);
            }


*/

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

MediaDictionary::~MediaDictionary()
{
    for(auto it=iMetadata.begin(); it!=iMetadata.end(); it++)
    {
        delete it->second;
    }
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
