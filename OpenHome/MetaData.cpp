#include <OpenHome/MetaData.h>
#include <OpenHome/Net/Private/XmlParser.h>
#include <OpenHome/Network.h>


using namespace OpenHome;
using namespace OpenHome::Av;
using namespace OpenHome::Net;



//////////////////////////////////////////////////////////

InfoMetatext::InfoMetatext()
    :iMetatext(NULL)
{
}

InfoMetatext::InfoMetatext(IMediaMetadata& aMetatext)
    :iMetatext(&aMetatext)
{
}

IMediaMetadata& InfoMetatext::Metatext()
{
    return *iMetatext;
}


//////////////////////////////////////////////////////////

IInfoMetadata* InfoMetadata::iEmpty = NULL;

IInfoMetadata* InfoMetadata::Empty()
{

    if(iEmpty == NULL)
    {
        iEmpty = new InfoMetadata();
    }

    return(iEmpty);
}


InfoMetadata::InfoMetadata()
    :iMetadata(NULL)
{
}


InfoMetadata::InfoMetadata(IMediaMetadata* aMetadata, const Brx& aUri)
    :iMetadata(aMetadata)
    ,iUri(aUri)
{
}


InfoMetadata::~InfoMetadata()
{
    delete iMetadata;
}


void InfoMetadata::DestroyStatics()
{ // static
    delete iEmpty;
    iEmpty = NULL;
}

IMediaMetadata& InfoMetadata::Metadata()
{
    return (*iMetadata);
}


const Brx& InfoMetadata::Uri()
{
    return (iUri);
}

///////////////////////////////////////////////////////////////

SenderMetadata* SenderMetadata::iEmpty = NULL;

SenderMetadata* SenderMetadata::Empty()
{
    if(iEmpty == NULL)
    {
        iEmpty = new SenderMetadata();
    }
    return(iEmpty);
}

void SenderMetadata::DestroyStatics()
{ // static
    delete iEmpty;
    iEmpty = NULL;
}


SenderMetadata::SenderMetadata()
{
}


SenderMetadata::SenderMetadata(const Brx& aMetadata)
{
    iMetadata.Replace(aMetadata);

    try
    {
        iName.Replace(XmlParserBasic::Find(Brn("title"), aMetadata));
        iUri.Replace(XmlParserBasic::Find(Brn("res"), aMetadata));
        iArtworkUri.Replace(XmlParserBasic::Find(Brn("albumArtURI"), aMetadata));

        if (iName.Equals(Brx::Empty()))
        {
                iName.Replace("No name element provided");
        }
    }
    catch(XmlError)
    {
        iName.Replace("Invalid metadata XML");
    }




/*
    try
    {
        XmlDocument doc = new XmlDocument();
        XmlNamespaceManager nsManager = new XmlNamespaceManager(doc.NameTable);
        nsManager.AddNamespace("didl", "urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/");
        nsManager.AddNamespace("upnp", "urn:schemas-upnp-org:metadata-1-0/upnp/");
        nsManager.AddNamespace("dc", "http://purl.org/dc/elements/1.1/");
        doc.LoadXml(aMetadata);

        XmlNode name = doc.FirstChild.SelectSingleNode("didl:item/dc:title", nsManager);
        if (name != null && name.FirstChild != null)
        {
            iName = name.FirstChild.Value;
        }
        else
        {
            iName = "No name element provided";
        }
        XmlNode uri = doc.FirstChild.SelectSingleNode("didl:item/didl:res", nsManager);
        if (uri != null && uri.FirstChild != null)
        {
            iUri = uri.FirstChild.Value;
        }
        XmlNode artworkUri = doc.FirstChild.SelectSingleNode("didl:item/upnp:albumArtURI", nsManager);
        if (artworkUri != null && artworkUri.FirstChild != null)
        {
            iArtworkUri = artworkUri.FirstChild.Value;
        }
    }
    catch (XmlException)
    {
        iName = "Invalid metadata XML";
    }
*/
}

const Brx& SenderMetadata::Name()
{
    return iName;
}

const Brx& SenderMetadata::Uri()
{
    return iUri;
}

const Brx& SenderMetadata::ArtworkUri()
{
    return iArtworkUri;
}

const Brx& SenderMetadata::ToString()
{
    return iMetadata;
}
