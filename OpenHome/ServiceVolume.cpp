#include <OpenHome/OhNetTypes.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/ServiceVolume.h>
#include <OpenHome/Network.h>
#include <OpenHome/Private/Ascii.h>
#include <OpenHome/Private/Debug.h>
#include <Generated/CpAvOpenhomeOrgVolume1.h>
#include <vector>

using namespace OpenHome;
using namespace OpenHome::Topology;
using namespace OpenHome::Net;
using namespace std;



ServiceVolume::ServiceVolume(IInjectorDevice& aDevice)
    :Service(aDevice)
    ,iBalance(new Watchable<TInt>(iNetwork, Brn("Balance"), 0))
    ,iFade(new Watchable<TInt>(iNetwork, Brn("Fade"), 0))
    ,iMute(new Watchable<TBool>(iNetwork, Brn("Mute"), false))
    ,iValue(new Watchable<TUint>(iNetwork, Brn("Value"), 0))
    ,iVolumeLimit(new Watchable<TUint>(iNetwork, Brn("VolumeLimit"), 0))
{
}

ServiceVolume::~ServiceVolume()
{
    delete iBalance;
    delete iFade;
    delete iMute;
    delete iValue;
    delete iVolumeLimit;
    //delete iVolumeMilliDbPerStep;
    //delete iVolumeSteps;
    //delete iVolumeUnity;
}

void ServiceVolume::Dispose()
{
    Service::Dispose();
    iBalance->Dispose();
    iFade->Dispose();
    iMute->Dispose();
    iValue->Dispose();
    iVolumeLimit->Dispose();
    //iVolumeMilliDbPerStep->Dispose();
    //iVolumeSteps->Dispose();
    //iVolumeUnity->Dispose();
}

IProxy* ServiceVolume::OnCreate(IDevice& aDevice)
{
    return (new ProxyVolume(*this, aDevice));
}

IWatchable<TInt>& ServiceVolume::Balance()
{
    return *iBalance;
}

IWatchable<TInt>& ServiceVolume::Fade()
{
    return *iFade;
}

IWatchable<TBool>& ServiceVolume::Mute()
{
    return *iMute;
}

IWatchable<TUint>& ServiceVolume::Value()
{
    return *iValue;
}

IWatchable<TUint>& ServiceVolume::VolumeLimit()
{
    return *iVolumeLimit;
}

TUint ServiceVolume::BalanceMax()
{
    return iBalanceMax;
}

TUint ServiceVolume::FadeMax()
{
    return iFadeMax;
}

TUint ServiceVolume::VolumeMax()
{
    return iVolumeMax;
}

TUint ServiceVolume::VolumeMilliDbPerStep()
{
    return iVolumeMilliDbPerStep;
}

TUint ServiceVolume::VolumeSteps()
{
    return iVolumeSteps;
}

TUint ServiceVolume::VolumeUnity()
{
    return iVolumeUnity;
}

////////////////////////////////////////////////////////

ServiceVolumeNetwork::ServiceVolumeNetwork(IInjectorDevice& aDevice, CpProxyAvOpenhomeOrgVolume1* aService)
    :ServiceVolume(aDevice)
    ,iService(aService)
    ,iSubscribed(false)
{
    Functor f1 = MakeFunctor(*this, &ServiceVolumeNetwork::HandleBalanceChanged);
    iService->SetPropertyBalanceChanged(f1);

    Functor f2 = MakeFunctor(*this, &ServiceVolumeNetwork::HandleFadeChanged);
    iService->SetPropertyFadeChanged(f2);

    Functor f3 = MakeFunctor(*this, &ServiceVolumeNetwork::HandleMuteChanged);
    iService->SetPropertyMuteChanged(f3);

    Functor f4 = MakeFunctor(*this, &ServiceVolumeNetwork::HandleVolumeChanged);
    iService->SetPropertyVolumeChanged(f4);

    Functor f5 = MakeFunctor(*this, &ServiceVolumeNetwork::HandleVolumeLimitChanged);
    iService->SetPropertyVolumeLimitChanged(f5);

/*
    Functor f6 = MakeFunctor(*this, &ServiceVolumeNetwork::HandleVolumeMilliDbPerStepChanged);
    iService->SetPropertyVolumeMilliDbPerStepChanged(f6);

    Functor f7 = MakeFunctor(*this, &ServiceVolumeNetwork::HandleVolumeStepsChanged);
    iService->SetPropertyVolumeStepsChanged(f7);

    Functor f8 = MakeFunctor(*this, &ServiceVolumeNetwork::HandleVolumeUnityChanged);
    iService->SetPropertyVolumeUnityChanged(f8);
*/
    Functor f9 = MakeFunctor(*this, &ServiceVolumeNetwork::HandleInitialEvent);
    iService->SetPropertyInitialEvent(f9);
}


ServiceVolumeNetwork::~ServiceVolumeNetwork()
{
    delete iService;
}


void ServiceVolumeNetwork::Dispose()
{
    ServiceVolume::Dispose();
}


TBool ServiceVolumeNetwork::OnSubscribe()
{
    iService->Subscribe();
    iSubscribed = true;
    return(false); // false = not mock
/*
    ASSERT(iSubscribedSource == NULL);
    iSubscribedSource = new TaskCompletionSource<TBool>();
    iService->Subscribe();
    return iSubscribedSource.Task.ContinueWith((t) => { });
*/
}


void ServiceVolumeNetwork::OnCancelSubscribe()
{
/*
    if (iSubscribedSource != NULL)
    {
        iSubscribedSource->iCancelled = true;
        //iSubscribedSource.TrySetCanceled();
    }
*/
}


void ServiceVolumeNetwork::HandleInitialEvent()
{
    iService->PropertyBalanceMax(iBalanceMax);
    iService->PropertyFadeMax(iFadeMax);
    iService->PropertyVolumeMax(iVolumeMax);
    iService->PropertyVolumeMilliDbPerStep(iVolumeMilliDbPerStep);
    iService->PropertyVolumeSteps(iVolumeSteps);
    iService->PropertyVolumeUnity(iVolumeUnity);

    //if (!iSubscribedSource->iCancelled)
    //{
        SubscribeCompleted();
    //}
}


void ServiceVolumeNetwork::OnUnsubscribe()
{
    if (iService != NULL)
    {
        iService->Unsubscribe();
    }

    iSubscribed = false;
}


void ServiceVolumeNetwork::SetBalance(TInt aValue)
{
    FunctorAsync f;
    iService->BeginSetBalance(aValue, f);
}


void ServiceVolumeNetwork::SetFade(TInt aValue)
{
    FunctorAsync f;
    iService->BeginSetFade(aValue, f);
}


void ServiceVolumeNetwork::SetMute(TBool aValue)
{
    FunctorAsync f;
    iService->BeginSetMute(aValue, f);
}


void ServiceVolumeNetwork::SetVolume(TUint aValue)
{
    FunctorAsync f;
    iService->BeginSetVolume(aValue, f);
}


void ServiceVolumeNetwork::VolumeDec()
{
    FunctorAsync f;
    iService->BeginVolumeDec(f);
}


void ServiceVolumeNetwork::VolumeInc()
{
    FunctorAsync f;
    iService->BeginVolumeInc(f);
}

/*
void ServiceVolumeNetwork::HandleVolumeUnityChanged()
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceVolumeNetwork::VolumeUnityChangedCallback1);
    Schedule(f, NULL);
}


void ServiceVolumeNetwork::VolumeUnityChangedCallback1(void*)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceVolumeNetwork::VolumeUnityChangedCallback2);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}


void ServiceVolumeNetwork::VolumeUnityChangedCallback2(void*)
{
    if (iSubscribed)
    {
        TUint unity;
        iService->PropertyVolumeUnity(unity);
        iVolumeUnity->Update(unity);
    }
}


void ServiceVolumeNetwork::HandleVolumeStepsChanged()
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceVolumeNetwork::VolumeStepsChangedCallback1);
    Schedule(f, NULL);
}


void ServiceVolumeNetwork::VolumeStepsChangedCallback1(void*)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceVolumeNetwork::VolumeStepsChangedCallback2);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}


void ServiceVolumeNetwork::VolumeStepsChangedCallback2(void*)
{
    if (iSubscribed)
    {
        TUint steps;
        iService->PropertyVolumeSteps(steps);
        iVolumeSteps->Update(steps);
    }
}

void ServiceVolumeNetwork::HandleVolumeMilliDbPerStepChanged()
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceVolumeNetwork::VolumeMilliDbPerStepChangedCallback1);
    Schedule(f, NULL);
}


void ServiceVolumeNetwork::VolumeMilliDbPerStepChangedCallback1(void*)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceVolumeNetwork::VolumeMilliDbPerStepChangedCallback2);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}


void ServiceVolumeNetwork::VolumeMilliDbPerStepChangedCallback2(void*)
{
    if (iSubscribed)
    {
        TUint mdbs;
        iService->PropertyVolumeMilliDbPerStep(mdbs);
        iVolumeMilliDbPerStep->Update(mdbs);
    }
}
*/

void ServiceVolumeNetwork::HandleVolumeLimitChanged()
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceVolumeNetwork::VolumeLimitChangedCallback1);
    Schedule(f, NULL);
}


void ServiceVolumeNetwork::VolumeLimitChangedCallback1(void*)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceVolumeNetwork::VolumeLimitChangedCallback2);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}


void ServiceVolumeNetwork::VolumeLimitChangedCallback2(void*)
{
    if (iSubscribed)
    {
        TUint limit;
        iService->PropertyVolumeLimit(limit);
        iVolumeLimit->Update(limit);
    }
}

void ServiceVolumeNetwork::HandleVolumeChanged()
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceVolumeNetwork::VolumeChangedCallback1);
    Schedule(f, NULL);
}


void ServiceVolumeNetwork::VolumeChangedCallback1(void*)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceVolumeNetwork::VolumeChangedCallback2);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}


void ServiceVolumeNetwork::VolumeChangedCallback2(void*)
{
    if (iSubscribed)
    {
        TUint volume;
        iService->PropertyVolume(volume);
        iValue->Update(volume);
    }
}

void ServiceVolumeNetwork::HandleMuteChanged()
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceVolumeNetwork::MuteChangedCallback1);
    Schedule(f, NULL);
}


void ServiceVolumeNetwork::MuteChangedCallback1(void*)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceVolumeNetwork::MuteChangedCallback2);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}


void ServiceVolumeNetwork::MuteChangedCallback2(void*)
{
    if (iSubscribed)
    {
        TBool mute;
        iService->PropertyMute(mute);
        iMute->Update(mute);
    }
}

void ServiceVolumeNetwork::HandleFadeChanged()
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceVolumeNetwork::FadeChangedCallback1);
    Schedule(f, NULL);
}


void ServiceVolumeNetwork::FadeChangedCallback1(void*)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceVolumeNetwork::FadeChangedCallback2);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}


void ServiceVolumeNetwork::FadeChangedCallback2(void*)
{
    if (iSubscribed)
    {
        TInt fade;
        iService->PropertyFade(fade);
        iFade->Update(fade);
    }
}

void ServiceVolumeNetwork::HandleBalanceChanged()
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceVolumeNetwork::BalanceChangedCallback1);
    Schedule(f, NULL);
}


void ServiceVolumeNetwork::BalanceChangedCallback1(void*)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceVolumeNetwork::BalanceChangedCallback2);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}


void ServiceVolumeNetwork::BalanceChangedCallback2(void*)
{
    if (iSubscribed)
    {
        TInt balance;
        iService->PropertyBalance(balance);
        iBalance->Update(balance);
    }
}


    /////////////////////////////////////////////////////////////


ServiceVolumeMock::ServiceVolumeMock(INetwork& aNetwork, IInjectorDevice& aDevice, const Brx& /*aId*/, TInt aBalance, TUint aBalanceMax, TInt aFade, TUint aFadeMax, TBool aMute, TUint aValue, TUint aVolumeLimit, TUint aVolumeMax,
    TUint aVolumeMilliDbPerStep, TUint aVolumeSteps, TUint aVolumeUnity)
    : ServiceVolume(aDevice)
    , iNetwork(aNetwork)

{
    iBalanceMax = aBalanceMax;
    iFadeMax = aFadeMax;
    iVolumeMax = aVolumeMax;
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

    iBalance->Update(aBalance);
    iFade->Update(aFade);
    iMute->Update(aMute);
    iValue->Update(value);
    iVolumeLimit->Update(volumeLimit);
    iVolumeMilliDbPerStep = aVolumeMilliDbPerStep;
    iVolumeSteps = aVolumeSteps;
    iVolumeUnity = aVolumeUnity;
}

ServiceVolumeMock::~ServiceVolumeMock()
{}

void ServiceVolumeMock::SetBalance(TInt aValue)
{
    auto balance = new TInt(aValue);
    iNetwork.Schedule(MakeFunctorGeneric(*this, &ServiceVolumeMock::CallbackSetBalance), balance);
}

void ServiceVolumeMock::CallbackSetBalance(void* aValue)
{
    auto balance = (TInt*)aValue;
    iBalance->Update(*balance);
    delete balance;
}

void ServiceVolumeMock::SetFade(TInt aValue)
{
    auto fade = new TInt(aValue);
    iNetwork.Schedule(MakeFunctorGeneric(*this, &ServiceVolumeMock::CallbackSetFade), fade);
}

void ServiceVolumeMock::CallbackSetFade(void* aValue)
{
    auto fade = (TInt*)aValue;
    iFade->Update(*fade);
    delete fade;
}

void ServiceVolumeMock::SetMute(TBool aValue)
{
    auto mute = new TBool(aValue);
    iNetwork.Schedule(MakeFunctorGeneric(*this, &ServiceVolumeMock::CallbackSetMute), mute);
}

void ServiceVolumeMock::CallbackSetMute(void* aValue)
{
    auto mute = (TBool*)aValue;
    iMute->Update(*mute);
    delete mute;
}

void ServiceVolumeMock::SetVolume(TUint aValue)
{
    auto volume = new TUint(aValue);
    iNetwork.Schedule(MakeFunctorGeneric(*this, &ServiceVolumeMock::CallbackSetVolume), volume);
}

void ServiceVolumeMock::CallbackSetVolume(void* aValue)
{
    auto volume = (TUint*)aValue;
    iValue->Update(*volume);
    delete volume;
}

void ServiceVolumeMock::VolumeDec()
{
    if (iCurrentVolume > 0)
    {
        --iCurrentVolume;
        iNetwork.Schedule(MakeFunctorGeneric(*this, &ServiceVolumeMock::CallbackVolumeDec), nullptr);
    }
}

void ServiceVolumeMock::CallbackVolumeDec(void*)
{
    iValue->Update(iCurrentVolume);
}

void ServiceVolumeMock::VolumeInc()
{
    if (iCurrentVolume < iCurrentVolumeLimit)
    {
        ++iCurrentVolume;
        iNetwork.Schedule(MakeFunctorGeneric(*this, &ServiceVolumeMock::CallbackVolumeInc), nullptr);

    }
}

void ServiceVolumeMock::CallbackVolumeInc(void*)
{
    iValue->Update(iCurrentVolume);
}

void ServiceVolumeMock::Execute(ICommandTokens& aValue)
{
    Brn command = aValue.Next();
    if (Ascii::CaseInsensitiveEquals(command, Brn("balance")))
    {
        iBalance->Update(Ascii::Int(aValue.Next()));
    }
    else if (Ascii::CaseInsensitiveEquals(command, Brn("fade")))
    {
        iFade->Update(Ascii::Int(aValue.Next()));
    }
    else if (Ascii::CaseInsensitiveEquals(command, Brn("mute")))
    {
        iMute->Update((Ascii::CaseInsensitiveEquals(aValue.Next(), Brn("mute"))));
    }
    else if (Ascii::CaseInsensitiveEquals(command, Brn("value")))
    {
        iValue->Update(Ascii::Uint(aValue.Next()));
    }
    else if (Ascii::CaseInsensitiveEquals(command, Brn("volumeinc")))
    {
        VolumeInc();
    }
    else if (Ascii::CaseInsensitiveEquals(command, Brn("volumedec")))
    {
        VolumeDec();
    }
    else
    {
       // throw new NotSupportedException();
    }
}

//////////////////////////////////////////////////////


ProxyVolume::ProxyVolume(ServiceVolume& aService, IDevice& aDevice)
    :iService(aService)
    ,iDevice(aDevice)
{
}

IWatchable<TInt>& ProxyVolume::Balance()
{
    return iService.Balance();
}

IWatchable<TInt>& ProxyVolume::Fade()
{
    return iService.Fade();
}

IWatchable<TBool>& ProxyVolume::Mute()
{
    return iService.Mute();
}

IWatchable<TUint>& ProxyVolume::Value()
{
    return iService.Value();
}

IWatchable<TUint>& ProxyVolume::VolumeLimit()
{
    return iService.VolumeLimit();
}

TUint ProxyVolume::VolumeMilliDbPerStep()
{
    return iService.VolumeMilliDbPerStep();
}

TUint ProxyVolume::VolumeSteps()
{
    return iService.VolumeSteps();
}

TUint ProxyVolume::VolumeUnity()
{
    return iService.VolumeUnity();
}

TUint ProxyVolume::BalanceMax()
{
    return iService.BalanceMax();
}

TUint ProxyVolume::FadeMax()
{
    return iService.FadeMax();
}

TUint ProxyVolume::VolumeMax()
{
    return iService.VolumeMax();
}

void ProxyVolume::SetBalance(TInt aValue)
{
    return iService.SetBalance(aValue);
}

void ProxyVolume::SetFade(TInt aValue)
{
    return iService.SetFade(aValue);
}

void ProxyVolume::SetMute(TBool aValue)
{
    return iService.SetMute(aValue);
}

void ProxyVolume::SetVolume(TUint aValue)
{
    return iService.SetVolume(aValue);
}

void ProxyVolume::VolumeDec()
{
    return iService.VolumeDec();
}

void ProxyVolume::VolumeInc()
{
    return iService.VolumeInc();
}

void ProxyVolume::Dispose()
{
    iService.Unsubscribe();
}

IDevice& ProxyVolume::Device()
{
    return (iDevice);
}
