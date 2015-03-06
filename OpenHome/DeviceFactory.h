#include <OpenHome/Network.h>
#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/Net/Core/CpDevice.h>



namespace OpenHome
{
namespace Topology
{


class DeviceFactory
{
public:
    static IInjectorDevice* Create(INetwork& aNetwork, Net::CpDevice& aDevice, ILog& aLog);
    static IInjectorDevice* CreateDs(INetwork& aNetwork, const Brx& aUdn, ILog& aLog);
    static IInjectorDevice* CreateDs(INetwork& aNetwork, const Brx& aUdn, const Brx& aRoom, const Brx& aName, const Brx& aAttributes, ILog& aLog);
    static IInjectorDevice* CreateDsm(INetwork& aNetwork, const Brx& aUdn, ILog& aLog);
    static IInjectorDevice* CreateDsm(INetwork& aNetwork, const Brx& aUdn, const Brx& aRoom, const Brx& aName, const Brx& aAttributes, ILog& aLog);
    //static IInjectorDevice* CreateMediaServer(INetwork& aNetwork, const Brx& aUdn, const Brx& aResourceRoot, ILog& aLog);
};


} // Av

} // OpenHome
