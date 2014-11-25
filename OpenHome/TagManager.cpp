#include <OpenHome/OhNetTypes.h>
#include <OpenHome/WatchableThread.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/DisposeHandler.h>
#include <OpenHome/MetaData.h>
#include <OpenHome/TagManager.h>
#include <OpenHome/Private/Debug.h>
#include <vector>


using namespace OpenHome;
using namespace OpenHome::Av;
using namespace std;



IMediaMetadata* TagManager::FromDidlLite(const Brx& aMetadata)
{
    MediaMetadata* metadata = new MediaMetadata();

    //if (!string.IsNullOrEmpty(aMetadata))
    if (!aMetadata.Equals(Brx::Empty()))
    {

    LOG(kTrace, aMetadata);


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
    }

    //metadata->Add(System().Folder(), aMetadata);

    return metadata;

}



void TagManager::ToDidlLite(IMediaMetadata& aMetadata, Brh& aBuf)
{
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
    //iValues = new List<string>(new string[] { aValue });
    iValues.push_back(Brn(aValue));
}


MediaValue::MediaValue(vector<Brn> aValues)
    :iValues(aValues)
    ,iValue(aValues[0])
{
    //iValues = new List<string>(aValues);

/*
    for(TUint i=0; i<aValues.size(); i++)
    {
        iValues.push_back(aValues.Next());
    }
*/
}

// IMediaServerValue
Brn MediaValue::Value()
{
    return (iValue);
}


const vector<Brn> MediaValue::Values()
{
    return (iValues);
}


/*
IEnumerable<Brn> MediaValue::Values()
{
    return (iValues);
}
*/

/////////////////////////////////////////////////

MediaDictionary::MediaDictionary()
{
    //iMetadata = new Dictionary<ITag, IMediaValue>();
}


MediaDictionary::MediaDictionary(IMediaMetadata& aMetadata)
    :iMetadata(aMetadata.Values())
{
    //iMetadata = new Dictionary<ITag, IMediaValue>(aMetadata.ToDictionary(x => x.Key, x => x.Value));

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


const std::map<ITag*, IMediaValue*> MediaMetadata::Values()
{
    return(iMetadata);
}


/*
// IEnumerable<KeyValuePair<ITag, IMediaServer>>
IEnumerator<KeyValuePair<ITag, IMediaValue>> MediaMetadata::GetEnumerator()
{
    return (iMetadata.GetEnumerator());
}

// IEnumerable
IEnumerator MediaMetadata::GetEnumerator()
{
    return (iMetadata.GetEnumerator());
}
*/

