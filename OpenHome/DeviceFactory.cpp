#include <OpenHome/DeviceFactory.h>
#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/Network.h>
#include <OpenHome/Private/Ascii.h>
#include <OpenHome/ServiceProduct.h>
#include <OpenHome/ServiceSender.h>
#include <OpenHome/ServiceReceiver.h>
#include <OpenHome/ServiceVolume.h>
#include <OpenHome/ServiceRadio.h>
#include <OpenHome/ServicePlaylist.h>
#include <OpenHome/ServiceInfo.h>
#include <OpenHome/ServiceTime.h>
#include <vector>
#include <memory>


using namespace OpenHome ;
using namespace OpenHome::Topology;
using namespace OpenHome::Net;
using namespace std;



IInjectorDevice* DeviceFactory::CreateDs(INetwork& aNetwork, const Brx& aUdn)
{
    LOG(kApplication7, ">DeviceFactory::CreateDs \n");
    return CreateDs(aNetwork, aUdn, Brn("Main Room"), Brn("Mock DS"), Brn("Info Time Volume Sender"));
}

IInjectorDevice* DeviceFactory::CreateDs(INetwork& aNetwork, const Brx& aUdn, const Brx& aRoom, const Brx& aName, const Brx& aAttributes)
{
    //Log::Print(">DeviceFactory::CreateDs \n");
    InjectorDevice* device = new InjectorDevice(aNetwork, aUdn);

    //Log::Print("-DeviceFactory::CreateDs 1\n");

    // add a factory for each type of watchable service

    // product service
    std::unique_ptr<SrcXml> xml(new SrcXml());

    xml->Add(unique_ptr<TopologySource>(new TopologySource(Brn("Playlist"), Brn("Playlist"), true)));
    xml->Add(unique_ptr<TopologySource>(new TopologySource(Brn("Radio"), Brn("Radio"), true)));
    xml->Add(unique_ptr<TopologySource>(new TopologySource(Brn("UPnP AV"), Brn("UpnpAv"), false)));
    xml->Add(unique_ptr<TopologySource>(new TopologySource(Brn("Songcast"), Brn("Receiver"), true)));
    xml->Add(unique_ptr<TopologySource>(new TopologySource(Brn("Net Aux"), Brn("NetAux"), false)));



    device->Add(eProxyProduct, new ServiceProductMock(*device, aRoom, aName, 0, move(xml), true, aAttributes, Brx::Empty(),
                                    Brn("Linn Products Ltd"), Brn("Linn"), Brn("http://www.linn.co.uk"), Brx::Empty(),
                                    Brn("Linn High Fidelity System Component"), Brn("Mock DS"), Brx::Empty(), Brx::Empty(),
                                    Brn("Linn High Fidelity System Component"), Brx::Empty(), aUdn) );


    // sender service

    ServiceVolumeMock* svm = new ServiceVolumeMock(aNetwork, *device, aUdn, 0, 15, 0, 0, false, 50, 100, 100, 1024, 100, 80);
    device->Add(eProxyVolume, svm);

    auto details = new InfoDetails(0, 0, Brx::Empty(), 0, false, 0);
    auto infoMetaData = new InfoMetadata(aNetwork.GetTagManager().FromDidlLite(Brx::Empty()), Brx::Empty());
    auto infoMetatext = new InfoMetatext(aNetwork.GetTagManager().FromDidlLite(Brx::Empty()));
    ServiceInfoMock* sim = new ServiceInfoMock(*device, details, infoMetaData, infoMetatext);
    device->Add(eProxyInfo, sim);
    delete details;

    ServiceTimeMock* stm = new ServiceTimeMock(*device, 0, 0);
    device->Add(eProxyTime, stm);


    Bwh senderMeta(Brn("<DIDL-Lite xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\" xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\"><item id=\"\" parentID=\"\" restricted=\"True\"><dc:title>Main Room:Mock DS</dc:title><res protocolInfo=\"ohz:*:*:u\">ohz://239.255.255.250:51972/"));
    senderMeta.Grow(2000);
    senderMeta.Append(aUdn);
    senderMeta.Append(Brn("</res><upnp:albumArtURI>http://10.2.10.27/images/Icon.png</upnp:albumArtURI><upnp:class>object.item.audioItem</upnp:class></item></DIDL-Lite>"));




    SenderMetadata* smd = new SenderMetadata(senderMeta);
    ServiceSenderMock* ssm = new ServiceSenderMock(*device, aAttributes, Brx::Empty(), false, smd, Brn("Enabled"));
    device->Add(eProxySender, ssm);



    // receiver service

    ServiceReceiverMock* srm =  new ServiceReceiverMock(*device, Brx::Empty(), Brn("ohz:*:*:*,ohm:*:*:*,ohu:*.*.*"), Brn("Stopped"), Brx::Empty());
    device->Add(eProxyReceiver, srm);

    std::vector<IMediaMetadata*> tracks;
    ServicePlaylistMock* spm = new ServicePlaylistMock(*device, 0, tracks, false, false, Brn("Stopped"), Brx::Empty(), 1000);
    device->Add(eProxyPlaylist, spm);


    return device;
}



IInjectorDevice* DeviceFactory::CreateDsm(INetwork& aNetwork, const Brx& aUdn)
{
    return CreateDsm(aNetwork, aUdn, Brn("Main Room"), Brn("Mock Dsm"), Brn("Info Time Volume Sender"));
}

IInjectorDevice* DeviceFactory::CreateDsm(INetwork& aNetwork, const Brx& aUdn, const Brx& aRoom, const Brx& aName, const Brx& aAttributes)
{
    InjectorDevice* device = new InjectorDevice(aNetwork, aUdn);
    // add a factory for each type of watchable service

    // product service
    std::unique_ptr<SrcXml> xml(new SrcXml());
    xml->Add(unique_ptr<TopologySource>(new TopologySource(Brn("Playlist"), Brn("Playlist"), true)));
    xml->Add(unique_ptr<TopologySource>(new TopologySource(Brn("Radio"), Brn("Radio"), true)));
    xml->Add(unique_ptr<TopologySource>(new TopologySource(Brn("UPnP AV"), Brn("UpnpAv"), false)));
    xml->Add(unique_ptr<TopologySource>(new TopologySource(Brn("Songcast"), Brn("Receiver"), true)));
    xml->Add(unique_ptr<TopologySource>(new TopologySource(Brn("Net Aux"), Brn("NetAux"), false)));
    xml->Add(unique_ptr<TopologySource>(new TopologySource(Brn("Analog1"), Brn("Analog"), true)));
    xml->Add(unique_ptr<TopologySource>(new TopologySource(Brn("Analog2"), Brn("Analog"), true)));
    xml->Add(unique_ptr<TopologySource>(new TopologySource(Brn("Phono"), Brn("Analog"), true)));
    xml->Add(unique_ptr<TopologySource>(new TopologySource(Brn("SPDIF1"), Brn("Digital"), true)));
    xml->Add(unique_ptr<TopologySource>(new TopologySource(Brn("SPDIF2"), Brn("Digital"), true)));
    xml->Add(unique_ptr<TopologySource>(new TopologySource(Brn("TOSLINK1"), Brn("Digital"), true)));
    xml->Add(unique_ptr<TopologySource>(new TopologySource(Brn("TOSLINK2"), Brn("Digital"), true)));


    ServiceProductMock* spm = new ServiceProductMock(*device, aRoom, aName, 0, std::move(xml), true, aAttributes, Brx::Empty(),
        Brn("Linn Products Ltd"), Brn("Linn"), Brn("http://www.linn.co.uk"), Brx::Empty(),
        Brn("Linn High Fidelity System Component"), Brn("Mock DSM"), Brx::Empty(), Brx::Empty(),
        Brn("Linn High Fidelity System Component"), Brx::Empty(), aUdn);
    device->Add(eProxyProduct, spm);

    ServiceVolumeMock* svm = new ServiceVolumeMock(aNetwork, *device, aUdn, 0, 15, 0, 0, false, 50, 100, 100, 1024, 100, 80);
    device->Add(eProxyVolume, svm);

    auto details = new InfoDetails(0, 0, Brx::Empty(), 0, false, 0);
    ServiceInfoMock* sim = new ServiceInfoMock(*device, details, new InfoMetadata(aNetwork.GetTagManager().FromDidlLite(Brx::Empty()), Brx::Empty()), new InfoMetatext(aNetwork.GetTagManager().FromDidlLite(Brx::Empty())));
    device->Add(eProxyInfo, sim);
    delete details;

    ServiceTimeMock* stm = new ServiceTimeMock(*device, 0, 0);
    device->Add(eProxyTime, stm);
    // sender service
    Bwh senderMeta(Brn("<DIDL-Lite xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\" xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\"><item id=\"\" parentID=\"\" restricted=\"True\"><dc:title>Main Room:Mock DSM</dc:title><res protocolInfo=\"ohz:*:*:u\">ohz://239.255.255.250:51972/"));
    senderMeta.Grow(2000);
    senderMeta.Append(aUdn);
    senderMeta.Append(Brn("</res><upnp:albumArtURI>http://10.2.10.27/images/Icon.png</upnp:albumArtURI><upnp:class>object.item.audioItem</upnp:class></item></DIDL-Lite>"));

    device->Add(eProxySender, new ServiceSenderMock(*device, aAttributes, Brx::Empty(), false, new SenderMetadata(senderMeta), Brn("Enabled")));

    // receiver service
    device->Add(eProxyReceiver, new ServiceReceiverMock(*device, Brx::Empty(), Brn("ohz:*:*:*,ohm:*:*:*,ohu:*.*.*"), Brn("Stopped"), Brx::Empty()));

    return device;
}

