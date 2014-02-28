#include <OpenHome/DeviceFactory.h>
#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/Network.h>
#include <OpenHome/Private/Ascii.h>
#include <OpenHome/ServiceProduct.h>
#include <vector>


using namespace OpenHome ;
using namespace OpenHome::Av;
using namespace OpenHome::Net;
using namespace std;



IInjectorDevice* DeviceFactory::CreateDs(INetwork& aNetwork, const Brx& aUdn, ILog& aLog)
{
    return CreateDs(aNetwork, aUdn, Brn("Main Room"), Brn("Mock DS"), Brn("Info Time Volume Sender"), aLog);
}

IInjectorDevice* DeviceFactory::CreateDs(INetwork& aNetwork, const Brx& aUdn, const Brx& aRoom, const Brx& aName, const Brx& aAttributes, ILog& aLog)
{
    InjectorDevice* device = new InjectorDevice(aUdn);
    // add a factory for each type of watchable service

    // product service
    vector<Source*> sources;

    sources.push_back(new Source(Brn("Playlist"), Brn("Playlist"), true));
    sources.push_back(new Source(Brn("Radio"), Brn("Radio"), true));
    sources.push_back(new Source(Brn("UPnP AV"), Brn("UpnpAv"), false));
    sources.push_back(new Source(Brn("Songcast"), Brn("Receiver"), true));
    sources.push_back(new Source(Brn("Net Aux"), Brn("NetAux"), false));

    SrcXml* xml = new SrcXml(sources);


    device->Add(EProxyProduct, new ServiceProductMock(aNetwork, *device, aRoom, aName, 0, xml, true, aAttributes, Brn(""),
                                    Brn("Linn Products Ltd"), Brn("Linn"), Brn("http://www.linn.co.uk"), Brn(""),
                                    Brn("Linn High Fidelity System Component"), Brn("Mock DS"), Brn(""),
                                    Brn(""), Brn("Linn High Fidelity System Component"),
                                    Brn(""),
                                    aUdn,
                                    aLog) );


/*
    // volume service
    device->Add<IProxyVolume>(new ServiceVolumeMock(aNetwork, device, aUdn, 0, 15, 0, 0, false, 50, 100, 100, 1024, 100, 80, aLog));

    // info service
    device->Add<IProxyInfo>(new ServiceInfoMock(aNetwork, device, new InfoDetails(0, 0, Brx:Empty(), 0, false, 0), new InfoMetadata(aNetwork.TagManager.FromDidlLite(Brx:Empty()), Brx:Empty()), new InfoMetatext(aNetwork.TagManager.FromDidlLite(Brx:Empty())), aLog));

    // time service
    device->Add<IProxyTime>(new ServiceTimeMock(aNetwork, device, 0, 0, aLog));

    // sender service
    device->Add<IProxySender>(new ServiceSenderMock(aNetwork, device, aAttributes, Brx:Empty(), false, new SenderMetadata("<DIDL-Lite xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\" xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\"><item id=\"\" parentID=\"\" restricted=\"True\"><dc:title>Main Room:Mock DS</dc:title><res protocolInfo=\"ohz:*:*:u\">ohz://239.255.255.250:51972/" + aUdn + "</res><upnp:albumArtURI>http://10.2.10.27/images/Icon.png</upnp:albumArtURI><upnp:class>object.item.audioItem</upnp:class></item></DIDL-Lite>"), "Enabled", aLog));

    // receiver service
    device->Add<IProxyReceiver>(new ServiceReceiverMock(aNetwork, device, Brx:Empty(), "ohz:*:*:*,ohm:*:*:*,ohu:*.*.*", "Stopped", Brx:Empty(), aLog));

    // radio service
    List<IMediaMetadata> presets = new List<IMediaMetadata>();
    presets.Add(aNetwork.TagManager.FromDidlLite("<DIDL-Lite xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\" xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\"><item id=\"\" parentID=\"\" restricted=\"True\"><dc:title>Linn Radio (Variety)</dc:title><res protocolInfo=\"*:*:*:*\" bitrate=\"40000\">http://opml.radiotime.com/Tune.ashx?id=s122119&amp;formats=mp3,wma,aac,wmvideo,ogg&amp;partnerId=ah2rjr68&amp;username=linnproducts&amp;c=ebrowse</res><upnp:albumArtURI>http://d1i6vahw24eb07.cloudfront.net/s122119q.png</upnp:albumArtURI><upnp:class>object.item.audioItem</upnp:class></item></DIDL-Lite>"));
    presets.Add(aNetwork.TagManager.FromDidlLite("<DIDL-Lite xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\" xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\"><item id=\"\" parentID=\"\" restricted=\"True\"><dc:title>Linn Jazz (Jazz)</dc:title><res protocolInfo=\"*:*:*:*\" bitrate=\"40000\">http://opml.radiotime.com/Tune.ashx?id=s122120&amp;formats=mp3,wma,aac,wmvideo,ogg&amp;partnerId=ah2rjr68&amp;username=linnproducts&amp;c=ebrowse</res><upnp:albumArtURI>http://d1i6vahw24eb07.cloudfront.net/s122120q.png</upnp:albumArtURI><upnp:class>object.item.audioItem</upnp:class></item></DIDL-Lite>"));
    presets.Add(null);
    presets.Add(null);
    presets.Add(aNetwork.TagManager.FromDidlLite("<DIDL-Lite xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\" xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\"><item id=\"\" parentID=\"\" restricted=\"True\"><dc:title>Linn Classical (Classical)</dc:title><res protocolInfo=\"*:*:*:*\" bitrate=\"40000\">http://opml.radiotime.com/Tune.ashx?id=s122116&amp;formats=mp3,wma,aac,wmvideo,ogg&amp;partnerId=ah2rjr68&amp;username=linnproducts&amp;c=ebrowse</res><upnp:albumArtURI>http://d1i6vahw24eb07.cloudfront.net/s122116q.png</upnp:albumArtURI><upnp:class>object.item.audioItem</upnp:class></item></DIDL-Lite>"));
    presets.Add(aNetwork.TagManager.FromDidlLite("<DIDL-Lite xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\" xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\"><item id=\"\" parentID=\"\" restricted=\"True\"><dc:title>BBC World Service (World News)</dc:title><res protocolInfo=\"*:*:*:*\" bitrate=\"4000\">http://opml.radiotime.com/Tune.ashx?id=s50646&amp;formats=mp3,wma,aac,wmvideo,ogg&amp;partnerId=ah2rjr68&amp;username=linnproducts&amp;c=ebrowse</res><upnp:albumArtURI>http://d1i6vahw24eb07.cloudfront.net/s50646q.png</upnp:albumArtURI><upnp:class>object.item.audioItem</upnp:class></item></DIDL-Lite>"));
    presets.Add(aNetwork.TagManager.FromDidlLite("<DIDL-Lite xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\" xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\"><item id=\"\" parentID=\"\" restricted=\"True\"><dc:title>Sky Radio News (World News)</dc:title><res protocolInfo=\"*:*:*:*\" bitrate=\"4000\">http://opml.radiotime.com/Tune.ashx?id=s81093&amp;formats=mp3,wma,aac,wmvideo,ogg&amp;partnerId=ah2rjr68&amp;username=linnproducts&amp;c=ebrowse</res><upnp:albumArtURI>http://d1i6vahw24eb07.cloudfront.net/s81093q.png</upnp:albumArtURI><upnp:class>object.item.audioItem</upnp:class></item></DIDL-Lite>"));

    device->Add<IProxyRadio>(new ServiceRadioMock(aNetwork, device, 0, presets, InfoMetadata.Empty, Brx:Empty(), "Stopped", 100, aLog));

    // playlist service
    List<IMediaMetadata> tracks = new List<IMediaMetadata>();
    device->Add<IProxyPlaylist>(new ServicePlaylistMock(aNetwork, device, 0, tracks, false, false, "Stopped", Brx:Empty(), 1000, aLog));
*/
    return device;
}


/*
IInjectorDevice* DeviceFactory::CreateDsm(INetwork aNetwork, const Brx& aUdn, ILog aLog)
{
    return CreateDsm(aNetwork, aUdn, "Main Room", "Mock Dsm", "Info Time Volume Sender", aLog);
}

IInjectorDevice* DeviceFactory::CreateDsm(INetwork aNetwork, const Brx& aUdn, const Brx& aRoom, const Brx& aName, const Brx& aAttributes, ILog aLog)
{
    InjectorDevice device = new InjectorDevice(aUdn);
    // add a factory for each type of watchable service

    // product service
    List<Source> sources = new List<Source>();
    sources.Add(new Source("Playlist", "Playlist", true));
    sources.Add(new Source("Radio", "Radio", true));
    sources.Add(new Source("UPnP AV", "UpnpAv", false));
    sources.Add(new Source("Songcast", "Receiver", true));
    sources.Add(new Source("Net Aux", "NetAux", false));
    sources.Add(new Source("Analog1", "Analog", true));
    sources.Add(new Source("Analog2", "Analog", true));
    sources.Add(new Source("Phono", "Analog", true));
    sources.Add(new Source("SPDIF1", "Digital", true));
    sources.Add(new Source("SPDIF2", "Digital", true));
    sources.Add(new Source("TOSLINK1", "Digital", true));
    sources.Add(new Source("TOSLINK2", "Digital", true));
    SourceXml xml = new SourceXml(sources.ToArray());

    device->Add<IProxyProduct>(new ServiceProductMock(aNetwork, device, aRoom, aName, 0, xml, true, aAttributes,
        "", "Linn Products Ltd", "Linn", "http://www.linn.co.uk",
        "", "Linn High Fidelity System Component", "Mock DSM", "",
        "", "Linn High Fidelity System Component", "", aUdn, aLog));

    // volume service
    device->Add<IProxyVolume>(new ServiceVolumeMock(aNetwork, device, aUdn, 0, 15, 0, 0, false, 50, 100, 100, 1024, 100, 80, aLog));

    // info service
    device->Add<IProxyInfo>(new ServiceInfoMock(aNetwork, device, new InfoDetails(0, 0, Brx:Empty(), 0, false, 0), new InfoMetadata(aNetwork.TagManager.FromDidlLite(Brx:Empty()), Brx:Empty()), new InfoMetatext(aNetwork.TagManager.FromDidlLite(Brx:Empty())), aLog));

    // time service
    device->Add<IProxyTime>(new ServiceTimeMock(aNetwork, device, 0, 0, aLog));

    // sender service
    device->Add<IProxySender>(new ServiceSenderMock(aNetwork, device, aAttributes, Brx:Empty(), false, new SenderMetadata("<DIDL-Lite xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\" xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\"><item id=\"\" parentID=\"\" restricted=\"True\"><dc:title>Main Room:Mock DSM</dc:title><res protocolInfo=\"ohz:*:*:u\">ohz://239.255.255.250:51972/" + aUdn + "</res><upnp:albumArtURI>http://10.2.10.27/images/Icon.png</upnp:albumArtURI><upnp:class>object.item.audioItem</upnp:class></item></DIDL-Lite>"), "Enabled", aLog));

    // receiver service
    device->Add<IProxyReceiver>(new ServiceReceiverMock(aNetwork, device, Brx:Empty(), "ohz:*:*:*,ohm:*:*:*,ohu:*.*.*", "Stopped", Brx:Empty(), aLog));

    // radio service
    List<IMediaMetadata> presets = new List<IMediaMetadata>();
    presets.Add(aNetwork.TagManager.FromDidlLite("<DIDL-Lite xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\" xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\"><item id=\"\" parentID=\"\" restricted=\"True\"><dc:title>Linn Radio (Variety)</dc:title><res protocolInfo=\"*:*:*:*\" bitrate=\"40000\">http://opml.radiotime.com/Tune.ashx?id=s122119&amp;formats=mp3,wma,aac,wmvideo,ogg&amp;partnerId=ah2rjr68&amp;username=linnproducts&amp;c=ebrowse</res><upnp:albumArtURI>http://d1i6vahw24eb07.cloudfront.net/s122119q.png</upnp:albumArtURI><upnp:class>object.item.audioItem</upnp:class></item></DIDL-Lite>"));
    presets.Add(aNetwork.TagManager.FromDidlLite("<DIDL-Lite xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\" xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\"><item id=\"\" parentID=\"\" restricted=\"True\"><dc:title>Linn Jazz (Jazz)</dc:title><res protocolInfo=\"*:*:*:*\" bitrate=\"40000\">http://opml.radiotime.com/Tune.ashx?id=s122120&amp;formats=mp3,wma,aac,wmvideo,ogg&amp;partnerId=ah2rjr68&amp;username=linnproducts&amp;c=ebrowse</res><upnp:albumArtURI>http://d1i6vahw24eb07.cloudfront.net/s122120q.png</upnp:albumArtURI><upnp:class>object.item.audioItem</upnp:class></item></DIDL-Lite>"));
    presets.Add(null);
    presets.Add(null);
    presets.Add(aNetwork.TagManager.FromDidlLite("<DIDL-Lite xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\" xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\"><item id=\"\" parentID=\"\" restricted=\"True\"><dc:title>Linn Classical (Classical)</dc:title><res protocolInfo=\"*:*:*:*\" bitrate=\"40000\">http://opml.radiotime.com/Tune.ashx?id=s122116&amp;formats=mp3,wma,aac,wmvideo,ogg&amp;partnerId=ah2rjr68&amp;username=linnproducts&amp;c=ebrowse</res><upnp:albumArtURI>http://d1i6vahw24eb07.cloudfront.net/s122116q.png</upnp:albumArtURI><upnp:class>object.item.audioItem</upnp:class></item></DIDL-Lite>"));
    presets.Add(aNetwork.TagManager.FromDidlLite("<DIDL-Lite xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\" xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\"><item id=\"\" parentID=\"\" restricted=\"True\"><dc:title>BBC World Service (World News)</dc:title><res protocolInfo=\"*:*:*:*\" bitrate=\"4000\">http://opml.radiotime.com/Tune.ashx?id=s50646&amp;formats=mp3,wma,aac,wmvideo,ogg&amp;partnerId=ah2rjr68&amp;username=linnproducts&amp;c=ebrowse</res><upnp:albumArtURI>http://d1i6vahw24eb07.cloudfront.net/s50646q.png</upnp:albumArtURI><upnp:class>object.item.audioItem</upnp:class></item></DIDL-Lite>"));
    presets.Add(aNetwork.TagManager.FromDidlLite("<DIDL-Lite xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\" xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\"><item id=\"\" parentID=\"\" restricted=\"True\"><dc:title>Sky Radio News (World News)</dc:title><res protocolInfo=\"*:*:*:*\" bitrate=\"4000\">http://opml.radiotime.com/Tune.ashx?id=s81093&amp;formats=mp3,wma,aac,wmvideo,ogg&amp;partnerId=ah2rjr68&amp;username=linnproducts&amp;c=ebrowse</res><upnp:albumArtURI>http://d1i6vahw24eb07.cloudfront.net/s81093q.png</upnp:albumArtURI><upnp:class>object.item.audioItem</upnp:class></item></DIDL-Lite>"));

    device->Add<IProxyRadio>(new ServiceRadioMock(aNetwork, device, 0, presets, InfoMetadata.Empty, Brx:Empty(), "Stopped", 100, aLog));

    // playlist service
    List<IMediaMetadata> tracks = new List<IMediaMetadata>();
    device->Add<IProxyPlaylist>(new ServicePlaylistMock(aNetwork, device, 0, tracks, false, false, "Stopped", Brx:Empty(), 1000, aLog));

    return device;
}

IInjectorDevice* DeviceFactory::CreateMediaServer(INetwork aNetwork, const Brx& aUdn, const Brx& aResourceRoot, ILog aLog)
{
    return (new DeviceMediaEndpointMock(aNetwork, aUdn, aResourceRoot, aLog));
}
*/


/*
IInjectorDevice* DeviceFactory::Create(INetwork& aNetwork, CpDevice& aDevice, ILog aLog)
{
    InjectorDevice* device = new InjectorDevice(aDevice.Udn());
    Brh value;

    if (aDevice.GetAttribute("Upnp.Service.av-openhome-org.Product", value))
    {
        //if (uint.Parse(value) == 1)
        if (Ascii::Uint(value)==1)
        {
            device->Add<IProxyProduct>(new ServiceProductNetwork(aNetwork, device, aDevice, aLog));
        }
    }

    if (aDevice.GetAttribute("Upnp.Service.av-openhome-org.Info", value))
    {
        //if (uint.Parse(value) == 1)
        if (Ascii::Uint(value)==1)
        {
            device->Add<IProxyInfo>(new ServiceInfoNetwork(aNetwork, device, aDevice, aLog));
        }
    }
    if (aDevice.GetAttribute("Upnp.Service.av-openhome-org.Time", value))
    {
        //if (uint.Parse(value) == 1)
        if (Ascii::Uint(value)==1)
        {
            device->Add<IProxyTime>(new ServiceTimeNetwork(aNetwork, device, aDevice, aLog));
        }
    }
    if (aDevice.GetAttribute("Upnp.Service.av-openhome-org.Sender", value))
    {
        //if (uint.Parse(value) == 1)
        if (Ascii::Uint(value)==1)
        {
            device->Add<IProxySender>(new ServiceSenderNetwork(aNetwork, device, aDevice, aLog));
        }
    }
    if (aDevice.GetAttribute("Upnp.Service.av-openhome-org.Volume", value))
    {
        //if (uint.Parse(value) == 1)
        if (Ascii::Uint(value)==1)
        {
            device->Add<IProxyVolume>(new ServiceVolumeNetwork(aNetwork, device, aDevice, aLog));
        }
    }
    if (aDevice.GetAttribute("Upnp.Service.av-openhome-org.Playlist", value))
    {
        //if (uint.Parse(value) == 1)
        if (Ascii::Uint(value)==1)
        {
            device->Add<IProxyPlaylist>(new ServicePlaylistNetwork(aNetwork, device, aDevice, aLog));
        }
    }
    if (aDevice.GetAttribute("Upnp.Service.av-openhome-org.Radio", value))
    {
        //if (uint.Parse(value) == 1)
        if (Ascii::Uint(value)==1)
        {
            device->Add<IProxyRadio>(new ServiceRadioNetwork(aNetwork, device, aDevice, aLog));
        }
    }
    if (aDevice.GetAttribute("Upnp.Service.av-openhome-org.Receiver", value))
    {
        //if (uint.Parse(value) == 1)
        if (Ascii::Uint(value)==1)
        {
            device->Add<IProxyReceiver>(new ServiceReceiverNetwork(aNetwork, device, aDevice, aLog));
        }
    }
    if (aDevice.GetAttribute("Upnp.Service.linn-co-uk.Sdp", value))
    {
        //if (uint.Parse(value) == 1)
        if (Ascii::Uint(value)==1)
        {
            device->Add<IProxySdp>(new ServiceSdpNetwork(aNetwork, device, aDevice, aLog));
        }
    }

    return device;
}
*/



