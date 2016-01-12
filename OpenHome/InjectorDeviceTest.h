#pragma once
#include <OpenHome/Device.h>
#include <OpenHome/Injector.h>
#include <OpenHome/Functor.h>

///////////////////////////////////////////////////////////////
namespace OpenHome {
namespace Topology {


class InjectorDeviceTest : public IInjectorDevice
{
public:
  InjectorDeviceTest(INetwork& aNetwork);
  ~InjectorDeviceTest();
public: //IInjectorDevice
  Brn Udn() override;
  INetwork& Network() const override;
  void Create(FunctorGeneric<IProxy*>, EServiceType aServiceType, IDevice& aDevice) override;
  TBool HasService(EServiceType aServiceType) override;
  TBool Wait() override;
public: //IJoinable
  void Join(Functor aAction) override;
  void Unjoin(Functor aAction) override;
public:  // IMockable
  void Execute(ICommandTokens& aTokens) override;
public: // IDisposable
  void Dispose() override;
private:
  INetwork& iNetwork;
  Bws<37> iUdn;
};


} // Topology
} // OpenHome
