#include <OpenHome/ServiceReceiver.h>
#include <OpenHome/Private/Ascii.h>
#include <OpenHome/Private/Debug.h>
#include <OpenHome/Service.h>
#include <OpenHome/Network.h>

using namespace OpenHome;
using namespace OpenHome::Av;


ServiceReceiver::ServiceReceiver(INetwork& aNetwork, IInjectorDevice& aDevice, ILog& aLog)
    :Service(aNetwork, &aDevice, aLog)
    ,iMetadata(new Watchable<IInfoMetadata*>(aNetwork, Brn("Metadata"), InfoMetadata::Empty()))
    ,iTransportState(new Watchable<Brn>(aNetwork, Brn("TransportState"), Brx::Empty()))
    ,iCurrentMetadata(NULL)
{
}

void ServiceReceiver::Dispose()
{
    Service::Dispose();
    iMetadata->Dispose();
    iTransportState->Dispose();
    delete iMetadata;
    delete iTransportState;

    if (iCurrentMetadata!=NULL)
    {
        delete iCurrentMetadata;
    }

    iMetadata = NULL;
    iTransportState = NULL;
}

IProxy* ServiceReceiver::OnCreate(IDevice* aDevice)
{
    return(new ProxyReceiver(*this, *aDevice));
}

IWatchable<IInfoMetadata*>& ServiceReceiver::Metadata()
{
    return(*iMetadata);
}

IWatchable<Brn>& ServiceReceiver::TransportState()
{
    return(*iTransportState);
}

const Brx& ServiceReceiver::ProtocolInfo()
{
    return iProtocolInfo;
}



////////////////////////////////////////////////////////////////
/*

class ServiceReceiverNetwork : ServiceReceiver
{
ServiceReceiverNetwork(INetwork aNetwork, IInjectorDevice aDevice, CpDevice aCpDevice, ILog aLog)
    : base(aNetwork, aDevice, aLog)
{
    iCpDevice = aCpDevice;
    iCpDevice.AddRef();

    iService = new CpProxyAvOpenhomeOrgReceiver1(aCpDevice);

    iService.SetPropertyMetadataChanged(HandleMetadataChanged);
    iService.SetPropertyTransportStateChanged(HandleTransportStateChanged);

    iService.SetPropertyInitialEvent(HandleInitialEvent);
}

void Dispose()
{
    base.Dispose();

    iService.Dispose();
    iService = null;

    iCpDevice.RemoveRef();
}

Task OnSubscribe()
{
    Do.Assert(iSubscribedSource == null);

    iSubscribedSource = new TaskCompletionSource<bool>();

    iService.Subscribe();

    return iSubscribedSource.Task.ContinueWith((t) => { });
}

void OnCancelSubscribe()
{
    if (iSubscribedSource != null)
    {
        iSubscribedSource.TrySetCanceled();
    }
}

void HandleInitialEvent()
{
    iProtocolInfo = iService.PropertyProtocolInfo();

    if (!iSubscribedSource.Task.IsCanceled)
    {
        iSubscribedSource.SetResult(true);
    }
}

void OnUnsubscribe()
{
    if (iService != null)
    {
        iService.Unsubscribe();
    }

    iSubscribedSource = null;
}

Task Play()
{
    TaskCompletionSource<bool> taskSource = new TaskCompletionSource<bool>();
    iService.BeginPlay((ptr) =>
    {
        try
        {
            iService.EndPlay(ptr);
            taskSource.SetResult(true);
        }
        catch (Exception e)
        {
            taskSource.SetException(e);
        }
    });
    taskSource.Task.ContinueWith(t => { iLog.Write("Unobserved exception: {0}\n", t.Exception); }, TaskContinuationOptions.OnlyOnFaulted);
    return taskSource.Task;
}

Task Play(ISenderMetadata aMetadata)
{
    TaskCompletionSource<bool> taskSource = new TaskCompletionSource<bool>();
    iService.BeginSetSender(aMetadata.Uri, aMetadata.ToString(), (ptr1) =>
    {
        try
        {
            iService.EndSetSender(ptr1);
            iService.BeginPlay((ptr2) =>
            {
                try
                {
                    iService.EndPlay(ptr2);
                    taskSource.SetResult(true);
                }
                catch (Exception e)
                {
                    taskSource.SetException(e);
                }
            });
        }
        catch (Exception e)
        {
            taskSource.SetException(e);
        }
    });
    taskSource.Task.ContinueWith(t => { iLog.Write("Unobserved exception: {0}\n", t.Exception); }, TaskContinuationOptions.OnlyOnFaulted);
    return taskSource.Task;
}

Task Stop()
{
    TaskCompletionSource<bool> taskSource = new TaskCompletionSource<bool>();
    iService.BeginStop((ptr) =>
    {
        try
        {
            iService.EndStop(ptr);
            taskSource.SetResult(true);
        }
        catch (Exception e)
        {
            taskSource.SetException(e);
        }
    });
    taskSource.Task.ContinueWith(t => { iLog.Write("Unobserved exception: {0}\n", t.Exception); }, TaskContinuationOptions.OnlyOnFaulted);
    return taskSource.Task;
}

void HandleMetadataChanged()
{
    IMediaMetadata metadata = iNetwork.TagManager.FromDidlLite(iService.PropertyMetadata());
    string uri = iService.PropertyUri();
    iNetwork.Schedule(() =>
    {
        iDisposeHandler.WhenNotDisposed(() =>
        {
            iMetadata.Update(new InfoMetadata(metadata, uri));
        });
    });
}

void HandleTransportStateChanged()
{
    string transportState = iService.PropertyTransportState();
    iNetwork.Schedule(() =>
    {
        iDisposeHandler.WhenNotDisposed(() =>
        {
            iTransportState.Update(transportState);
        });
    });
}

readonly CpDevice iCpDevice;
TaskCompletionSource<bool> iSubscribedSource;
CpProxyAvOpenhomeOrgReceiver1 iService;
}

*/

ServiceReceiverMock::ServiceReceiverMock(INetwork& aNetwork, IInjectorDevice& aDevice, const Brx& aMetadata, const Brx& aProtocolInfo,
                                         const Brx& aTransportState, const Brx& aUri, ILog& aLog)
    :ServiceReceiver(aNetwork, aDevice, aLog)
{
    iProtocolInfo.Replace(aProtocolInfo);

    iCurrentMetadata = new InfoMetadata(aNetwork.TagManager().FromDidlLite(aMetadata), aUri);
    iMetadata->Update(iCurrentMetadata);
    iTransportState->Update(Brn(aTransportState));
}

/*
Task ServiceReceiverMock::Play()
{
    return Start(() =>
    {
        iTransportState.Update(Brn("Playing"));
    });
}

Task ServiceReceiverMock::Play(ISenderMetadata aMetadata)
{
    return Start(() =>
    {
        iMetadata.Update(new InfoMetadata(iNetwork.TagManager.FromDidlLite(aMetadata.ToString()), aMetadata.Uri));
        iTransportState.Update(Brn("Playing"));
    });
}

Task ServiceReceiverMock::Stop()
{
    return Start(() =>
    {
        iTransportState.Update(Brn("Stopped"));
    });
}
*/

void ServiceReceiverMock::Execute(ICommandTokens& aValue)
{
    Brn command = aValue.Next();

    if (Ascii::CaseInsensitiveEquals(command, Brn("protocolinfo")))
    {
        iProtocolInfo.Replace(aValue.RemainingTrimmed());
/*
        IEnumerable<string> value = aValue.Skip(1);
        iProtocolInfo = string.Join(" ", value);
*/
    }
    else if (Ascii::CaseInsensitiveEquals(command, Brn("metadata")))
    {
        if (aValue.Count() < 2)
        {
            //throw new NotSupportedException();
        }

        // Get the remaining tokens
        Brn remaining(aValue.RemainingTrimmed());

        // Get the last token
        TUint count = aValue.Count()-1;

        for(TUint i=0; i<count; i++)
        {
            aValue.Next();
        }

        Brn lastToken(aValue.Next());

        // Remove last token from remaining tokens
        TUint allButLastTokenBytes = remaining.Bytes()-lastToken.Bytes();
        Brn allButLastToken(remaining.Split(0, allButLastTokenBytes));

        // FIXME : use a static function to do the new here (allowing implementation of a fixed size pool of objects in future)
        IInfoMetadata* metadata = new InfoMetadata(iNetwork.TagManager().FromDidlLite(allButLastToken), lastToken);
        iMetadata->Update(metadata);

        if (iCurrentMetadata!=NULL)
        {
            delete iCurrentMetadata;
        }

        iCurrentMetadata = metadata;
    }
    else if (Ascii::CaseInsensitiveEquals(command, Brn("transportstate")))
    {
        Brn state(aValue.Next());

        if (state.Equals(kTransportStatePlaying))
        {
            iTransportState->Update(kTransportStatePlaying);
        }
        else if (state.Equals(kTransportStateStopped))
        {
            iTransportState->Update(kTransportStateStopped);
        }
        else if (state.Equals(kTransportStatePaused))
        {
            iTransportState->Update(kTransportStatePaused);
        }
    }
    else
    {
        //throw new NotSupportedException();
    }
}


///////////////////////////////////////////////////////////////////////


ProxyReceiver::ProxyReceiver(ServiceReceiver& aService, IDevice& aDevice)
    :iService(aService)
    ,iDevice(aDevice)
{
}

const Brx& ProxyReceiver::ProtocolInfo()
{
    return iService.ProtocolInfo();
}

IWatchable<IInfoMetadata*>& ProxyReceiver::Metadata()
{
    return iService.Metadata();
}

IWatchable<Brn>& ProxyReceiver::TransportState()
{
    return iService.TransportState();
}

/*
Task Play()
{
    return iService.Play();
}

Task Play(ISenderMetadata aMetadata)
{
    return iService.Play(aMetadata);
}

Task Stop()
{
    return iService.Stop();
}
*/


void ProxyReceiver::Dispose()
{
    iService.Unsubscribe();
}


IDevice& ProxyReceiver::Device()
{
    return(iDevice);
}


