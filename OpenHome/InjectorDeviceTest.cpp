#include <OpenHome/InjectorDeviceTest.h>

using namespace OpenHome;
using namespace OpenHome::Topology;


///////////////////////////////////////////////////////////////


InjectorDeviceTest::InjectorDeviceTest(INetwork& aNetwork)
  : iNetwork(aNetwork)
  , iUdn("12345678-9123-4567-8912-345678912345")
{
}

InjectorDeviceTest::~InjectorDeviceTest()
{

}

Brn InjectorDeviceTest::Udn()
{
  return Brn(iUdn);
}

INetwork& InjectorDeviceTest::Network() const
{
  return iNetwork;
}

void InjectorDeviceTest::Create(FunctorGeneric<IProxy*>, EServiceType /*aServiceType*/, IDevice& /*aDevice*/)
{

}

TBool InjectorDeviceTest::HasService(EServiceType /*aServiceType*/)
{
  return true;
}

TBool InjectorDeviceTest::Wait()
{
  return true;
}

void InjectorDeviceTest::Join(Functor /*aAction*/)
{

}

void InjectorDeviceTest::Unjoin(Functor /*aAction*/)
{

}

void InjectorDeviceTest::Execute(ICommandTokens& /*aTokens*/)
{

}

void InjectorDeviceTest::Dispose()
{

}
