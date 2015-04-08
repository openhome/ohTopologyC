#ifndef HEADER_SERVICE_VOLUME
#define HEADER_SERVICE_VOLUME

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
    virtual IWatchable<TUint>& VolumeMilliDbPerStep() = 0;
    virtual IWatchable<TUint>& VolumeSteps() = 0;
    virtual IWatchable<TUint>& VolumeUnity() = 0;

    virtual void SetBalance(TInt aValue) = 0;
    virtual void SetFade(TInt aValue) = 0;
    virtual void SetMute(TBool aValue) = 0;
    virtual void SetVolume(TUint aValue) = 0;
    virtual void VolumeDec() = 0;
    virtual void VolumeInc() = 0;

    virtual TUint BalanceMax() = 0;
    virtual TUint FadeMax() = 0;
    virtual TUint VolumeMax() = 0;
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
    virtual IWatchable<TUint>& VolumeMilliDbPerStep();
    virtual IWatchable<TUint>& VolumeSteps();
    virtual IWatchable<TUint>& VolumeUnity();

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
    Watchable<TUint>* iVolumeMilliDbPerStep;
    Watchable<TUint>* iVolumeSteps;
    Watchable<TUint>* iVolumeUnity;
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
    void HandleVolumeUnityChanged();
    void HandleVolumeStepsChanged();
    void HandleVolumeMilliDbPerStepChanged();
    void HandleVolumeLimitChanged();
    void HandleVolumeChanged();
    void HandleMuteChanged();
    void HandleFadeChanged();
    void HandleBalanceChanged();

    void VolumeUnityChangedCallback1(void*);
    void VolumeUnityChangedCallback2(void*);
    void VolumeStepsChangedCallback1(void*);
    void VolumeStepsChangedCallback2(void*);
    void VolumeMilliDbPerStepChangedCallback1(void*);
    void VolumeMilliDbPerStepChangedCallback2(void*);
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
    virtual IWatchable<TUint>& VolumeMilliDbPerStep();
    virtual IWatchable<TUint>& VolumeSteps();
    virtual IWatchable<TUint>& VolumeUnity();
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

/*
class ServiceVolumeMock : ServiceVolume, IMockable
{
    public ServiceVolumeMock(INetwork aNetwork, IInjectorDevice aDevice, string aId, TInt aBalance, TUint aBalanceMax, TInt aFade, TUint aFadeMax, TBool aMute, TUint aValue, TUint aVolumeLimit, TUint aVolumeMax,
        TUint aVolumeMilliDbPerStep, TUint aVolumeSteps, TUint aVolumeUnity, ILog aLog)
        : base(aNetwork, aDevice, aLog)
    {
        TUint volumeLimit = aVolumeLimit;
        if (volumeLimit > aVolumeMax)
        {
            volumeLimit = aVolumeMax;
        }
        iCurrentVolumeLimit = volumeLimit;

        TUint value = aValue;
        if (value > aVolumeLimit)
        {
            value = aVolumeLimit;
        }
        iCurrentVolume = value;

        iBalance.Update(aBalance);
        iFade.Update(aFade);
        iMute.Update(aMute);
        iValue.Update(value);
        iVolumeLimit.Update(volumeLimit);
        iVolumeMilliDbPerStep.Update(aVolumeMilliDbPerStep);
        iVolumeSteps.Update(aVolumeSteps);
        iVolumeUnity.Update(aVolumeUnity);
    }

    public virtual Task SetBalance(TInt aValue)
    {
        Task task = Task.Factory.StartNew(() =>
        {
            iNetwork.Schedule(() =>
            {
                iBalance.Update(aValue);
            });
        });
        return task;
    }

    public virtual Task SetFade(TInt aValue)
    {
        Task task = Task.Factory.StartNew(() =>
        {
            iNetwork.Schedule(() =>
            {
                iFade.Update(aValue);
            });
        });
        return task;
    }

    public virtual Task SetMute(TBool aValue)
    {
        Task task = Task.Factory.StartNew(() =>
        {
            iNetwork.Schedule(() =>
            {
                iMute.Update(aValue);
            });
        });
        return task;
    }

    public virtual Task SetVolume(TUint aValue)
    {
        Task task = Task.Factory.StartNew(() =>
        {
            TUint value = aValue;
            if (value > iCurrentVolumeLimit)
            {
                value = iCurrentVolumeLimit;
            }

            if (value != iCurrentVolume)
            {
                iCurrentVolume = value;
                iNetwork.Schedule(() =>
                {
                    iValue.Update(aValue);
                });
            }
        });
        return task;
    }

    public virtual Task VolumeDec()
    {
        Task task = Task.Factory.StartNew(() =>
        {
            if (iCurrentVolume > 0)
            {
                --iCurrentVolume;
                iNetwork.Schedule(() =>
                {
                    iValue.Update(iCurrentVolume);
                });
            }
        });
        return task;
    }

    public virtual Task VolumeInc()
    {
        Task task = Task.Factory.StartNew(() =>
        {
            if (iCurrentVolume < iCurrentVolumeLimit)
            {
                ++iCurrentVolume;
                iNetwork.Schedule(() =>
                {
                    iValue.Update(iCurrentVolume);
                });
            }
        });
        return task;
    }

    public virtual void Execute(IEnumerable<string> aValue)
    {
        string command = aValue.First().ToLowerInvariant();
        if (command == "balance")
        {
            IEnumerable<string> value = aValue.Skip(1);
            iBalance.Update(TInt.Parse(value.First()));
        }
        else if (command == "fade")
        {
            IEnumerable<string> value = aValue.Skip(1);
            iFade.Update(TInt.Parse(value.First()));
        }
        else if (command == "mute")
        {
            IEnumerable<string> value = aValue.Skip(1);
            iMute.Update(TBool.Parse(value.First()));
        }
        else if (command == "value")
        {
            IEnumerable<string> value = aValue.Skip(1);
            iValue.Update(TUint.Parse(value.First()));
        }
        else if (command == "volumeinc")
        {
            VolumeInc();
        }
        else if (command == "volumedec")
        {
            VolumeDec();
        }
        else
        {
            throw new NotSupportedException();
        }
    }

    private TUint iCurrentVolume;
    private TUint iCurrentVolumeLimit;
}
*/

} // Topology
} // OpenHome

#endif //HEADER_SERVICE_VOLUME
