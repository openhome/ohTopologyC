#pragma once

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/Watchable.h>
#include <OpenHome/Service.h>
#include <OpenHome/Net/Core/CpDevice.h>
#include <OpenHome/Net/Core/FunctorAsync.h>

#include <vector>
#include <memory>

namespace OpenHome
{
namespace Net
{
    class CpProxyAvOpenhomeOrgVolume1;
}

namespace Topology
{

class IProxyVolume : public IProxy
{
public:
    virtual IWatchable<TInt>& Balance() = 0;
    virtual IWatchable<TInt>& Fade() = 0;
    virtual IWatchable<TBool>& Mute() = 0;
    virtual IWatchable<TUint>& Value() = 0;
    virtual IWatchable<TUint>& VolumeLimit() = 0;

    virtual void SetBalance(TInt aValue) = 0;
    virtual void SetFade(TInt aValue) = 0;
    virtual void SetMute(TBool aValue) = 0;
    virtual void SetVolume(TUint aValue) = 0;
    virtual void VolumeDec() = 0;
    virtual void VolumeInc() = 0;

    virtual TUint BalanceMax() = 0;
    virtual TUint FadeMax() = 0;
    virtual TUint VolumeMax() = 0;
    virtual TUint VolumeMilliDbPerStep() = 0;
    virtual TUint VolumeSteps() = 0;
    virtual TUint VolumeUnity() = 0;
};

////////////////////////////////////////////////////////

class ServiceVolume : public Service
{
protected :
    ServiceVolume(IInjectorDevice& aDevice, ILog& aLog);
    ~ServiceVolume();

public:
    virtual void Dispose();
    virtual IProxy* OnCreate(IDevice& aDevice);
    virtual IWatchable<TInt>& Balance();
    virtual IWatchable<TInt>& Fade();
    virtual IWatchable<TBool>& Mute();
    virtual IWatchable<TUint>& Value();
    virtual IWatchable<TUint>& VolumeLimit();
    virtual TUint BalanceMax();
    virtual TUint FadeMax();
    virtual TUint VolumeMax();
    virtual TUint VolumeMilliDbPerStep();
    virtual TUint VolumeSteps();
    virtual TUint VolumeUnity();

    virtual void SetBalance(TInt aValue) = 0;
    virtual void SetFade(TInt aValue) = 0;
    virtual void SetMute(TBool aValue) = 0;
    virtual void SetVolume(TUint aValue) = 0;
    virtual void VolumeDec() = 0;
    virtual void VolumeInc() = 0;

protected:
    TUint iBalanceMax;
    TUint iFadeMax;
    TUint iVolumeMax;
    Watchable<TInt>* iBalance;
    Watchable<TInt>* iFade;
    Watchable<TBool>* iMute;
    Watchable<TUint>* iValue;
    Watchable<TUint>* iVolumeLimit;
    TUint iVolumeMilliDbPerStep;
    TUint iVolumeSteps;
    TUint iVolumeUnity;
};

//////////////////////////////////////////////////////

class ServiceVolumeNetwork : public ServiceVolume
{
public:
    ServiceVolumeNetwork(IInjectorDevice& aDevice, Net::CpProxyAvOpenhomeOrgVolume1* aService, ILog& aLog);
    ~ServiceVolumeNetwork();

    virtual void Dispose();

    virtual void SetBalance(TInt aValue);
    virtual void SetFade(TInt aValue);
    virtual void SetMute(TBool aValue);
    virtual void SetVolume(TUint aValue);
    virtual void VolumeDec();
    virtual void VolumeInc();

protected:
    virtual TBool OnSubscribe();
    virtual void OnCancelSubscribe();
    virtual void OnUnsubscribe();

private:
    void HandleInitialEvent();
    void HandleVolumeLimitChanged();
    void HandleVolumeChanged();
    void HandleMuteChanged();
    void HandleFadeChanged();
    void HandleBalanceChanged();

    void VolumeLimitChangedCallback1(void*);
    void VolumeLimitChangedCallback2(void*);
    void VolumeChangedCallback1(void*);
    void VolumeChangedCallback2(void*);
    void MuteChangedCallback1(void*);
    void MuteChangedCallback2(void*);
    void FadeChangedCallback1(void*);
    void FadeChangedCallback2(void*);
    void BalanceChangedCallback1(void*);
    void BalanceChangedCallback2(void*);


private:
    Net::CpProxyAvOpenhomeOrgVolume1* iService;
    TBool iSubscribed;
};

////////////////////////////////////////////////////////////////////

class ProxyVolume : public IProxyVolume, public INonCopyable//public Proxy<ServiceVolume>
{
public:
    ProxyVolume(ServiceVolume& aService, IDevice& aDevice);

    virtual IWatchable<TInt>& Balance();
    virtual IWatchable<TInt>& Fade();
    virtual IWatchable<TBool>& Mute();
    virtual IWatchable<TUint>& Value();
    virtual IWatchable<TUint>& VolumeLimit();
    virtual TUint VolumeMilliDbPerStep();
    virtual TUint VolumeSteps();
    virtual TUint VolumeUnity();
    virtual TUint BalanceMax();
    virtual TUint FadeMax();
    virtual TUint VolumeMax();
    virtual void SetBalance(TInt aValue);
    virtual void SetFade(TInt aValue);
    virtual void SetMute(TBool aValue);
    virtual void SetVolume(TUint aValue);
    virtual void VolumeDec();
    virtual void VolumeInc();

    // IProxy
    virtual IDevice& Device();

    // IDisposable
    virtual void Dispose();

protected:
    ServiceVolume& iService;

private:
    IDevice& iDevice;
};


class ServiceVolumeMock : public ServiceVolume
{
public:
    ServiceVolumeMock(INetwork& aNetwork, IInjectorDevice& aDevice, const Brx& aId, TInt aBalance, TUint aBalanceMax, TInt aFade, TUint aFadeMax, TBool aMute, TUint aValue, TUint aVolumeLimit, TUint aVolumeMax,
        TUint aVolumeMilliDbPerStep, TUint aVolumeSteps, TUint aVolumeUnity, ILog& aLog);
    ~ServiceVolumeMock();
public:
    void SetBalance(TInt aValue) override;
    void CallbackSetBalance(void*);

    void SetFade(TInt aValue) override;
    void CallbackSetFade(void*);

    void SetMute(TBool aValue) override;
    void CallbackSetMute(void*);

    void SetVolume(TUint aValue) override;
    void CallbackSetVolume(void*);

    void VolumeDec() override;
    void CallbackVolumeDec(void*);

    void VolumeInc() override;
    void CallbackVolumeInc(void*);

    void Execute(ICommandTokens& aValue) override;
private:
    TUint iCurrentVolume;
    TUint iCurrentVolumeLimit;
private: //variables for setter callbacks
    INetwork& iNetwork;
};


} // Topology
} // OpenHome
