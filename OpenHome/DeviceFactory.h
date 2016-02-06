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
    static IInjectorDevice* CreateDs(INetwork& aNetwork, const Brx& aUdnaLog);
    static IInjectorDevice* CreateDs(INetwork& aNetwork, const Brx& aUdn, const Brx& aRoom, const Brx& aName, const Brx& aAttributes);
    static IInjectorDevice* CreateDsm(INetwork& aNetwork, const Brx& aUdn);
    static IInjectorDevice* CreateDsm(INetwork& aNetwork, const Brx& aUdn, const Brx& aRoom, const Brx& aName, const Brx& aAttributes);
};

} // Topology
} // OpenHome
