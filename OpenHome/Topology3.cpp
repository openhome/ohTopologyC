#include <OpenHome/Topology3.h>


using namespace OpenHome;
using namespace OpenHome::Av;
using namespace std;

Topology3Sender* Topology3Sender::iEmpty = new Topology3Sender();

Topology3Sender* Topology3Sender::Empty()
{
    return(iEmpty);
}

void Topology3Sender::DestroyStatics()
{
    delete iEmpty;
}

Topology3Sender::Topology3Sender()
    :iEnabled(false)
    ,iDevice(0)
{
}


Topology3Sender::Topology3Sender(IDevice& aDevice)
    :iEnabled(true)
    ,iDevice(&aDevice)
{
}


TBool Topology3Sender::Enabled()
{
    return iEnabled;
}


IDevice& Topology3Sender::Device()
{
    return *iDevice;
}

////////////////////////////////////////////


Topology3Group::Topology3Group(INetwork& aNetwork, ITopology2Group& aGroup)
    :iGroup(aGroup)
    ,iSender(new Watchable<ITopology3Sender*>(aNetwork, Brn("Sender"), Topology3Sender::Empty()))
    ,iDisposed(false)
    ,iCurrentSender(NULL)
{
}

void Topology3Group::Dispose()
{
    if (iDisposed)
    {
        //throw new ObjectDisposedException("Topology3Group.Dispose");
    }

    delete iSender;

    if (iCurrentSender!=Topology3Sender::Empty())
    {
        delete iCurrentSender;
    }

    iDisposed = true;
}

Brn Topology3Group::Id()
{
    return iGroup.Id();
}

Brn Topology3Group::Attributes()
{
    return iGroup.Attributes();
}

Brn Topology3Group::ModelName()
{
    return iGroup.ModelName();
}

Brn Topology3Group::ManufacturerName()
{
    return iGroup.ManufacturerName();
}

Brn Topology3Group::ProductId()
{
    return iGroup.ProductId();
}

IDevice& Topology3Group::Device()
{
    return iGroup.Device();
}

IWatchable<Brn>& Topology3Group::Room()
{
    return iGroup.Room();
}

IWatchable<Brn>& Topology3Group::Name()
{
    return iGroup.Name();
}

IWatchable<TBool>& Topology3Group::Standby()
{
    return iGroup.Standby();
}

IWatchable<TUint>& Topology3Group::SourceIndex()
{
    return iGroup.SourceIndex();
}

vector<Watchable<ITopology2Source*>*>& Topology3Group::Sources()
{
    return iGroup.Sources();
}


IWatchable<ITopology3Sender*>& Topology3Group::Sender()
{
    return *iSender;
}

void Topology3Group::SetStandby(TBool aValue)
{
    iGroup.SetStandby(aValue);
}

void Topology3Group::SetSourceIndex(TUint aValue)
{
    iGroup.SetSourceIndex(aValue);
}


void Topology3Group::SetSender(ITopology3Sender* aSender)
{
    iSender->Update(aSender);

    if ((iCurrentSender!=NULL)&&(iCurrentSender!=Topology3Sender::Empty()))
    {
        delete iCurrentSender;
    }

    iCurrentSender = aSender;
}

////////////////////////////////////////////////////////////////////////////


ReceiverWatcher::ReceiverWatcher(Topology3& aTopology, Topology3Group& aGroup)
    :iDisposed(false)
    ,iTopology(aTopology)
    ,iGroup(aGroup)
    ,iReceiver(NULL)
    ,iTransportState(Brx::Empty())
    ,iMetadata(NULL)
{
    for(TUint i=0; i<iGroup.Sources().size(); i++)
    {
        iGroup.Sources()[i]->AddWatcher(*this);

    }
}

void ReceiverWatcher::Dispose()
{
    for(TUint i=0; i<iGroup.Sources().size(); i++)
    {
        iGroup.Sources()[i]->RemoveWatcher(*this);

    }

    if (iReceiver != NULL)
    {
        iReceiver->TransportState().RemoveWatcher(*this);
        iReceiver->Metadata().RemoveWatcher(*this);
        iReceiver->Dispose();
        delete iReceiver;
        iReceiver = NULL;
    }

    SetSender(Topology3Sender::Empty());

    //iGroup = null;
    //iTopology = null;

    iDisposed = true;
}

const Brx& ReceiverWatcher::ListeningToUri()
{
    //ASSERT(iMetadata!=NULL);

    if (iTransportState.Equals(Brx::Empty()) || iTransportState.Equals(Brn("Stopped")))
    {
        return(Brx::Empty());
    }
/*
    if (string.IsNullOrEmpty(iTransportState) || iTransportState == "Stopped")
    {
        return null;
    }
*/
    return iMetadata->Uri();
}

void ReceiverWatcher::SetSender(ITopology3Sender* aSender)
{
    iGroup.SetSender(aSender);
}

void ReceiverWatcher::ItemOpen(const Brx& /*aId*/, Brn aValue)
{
//    iTransportState = Brn(aValue);
    iTransportState.Replace(aValue);
}

void ReceiverWatcher::ItemUpdate(const Brx& /*aId*/, Brn aValue, Brn /*aPrevious*/)
{
    iTransportState.Replace(aValue);
    iTopology.ReceiverChanged(*this);
}

void ReceiverWatcher::ItemClose(const Brx& /*aId*/, Brn /*aValue*/)
{
    iTransportState.Replace(Brx::Empty());
}

void ReceiverWatcher::ItemOpen(const Brx& /*aId*/, IInfoMetadata* aValue)
{
    iMetadata = aValue;
}

void ReceiverWatcher::ItemUpdate(const Brx& /*aId*/, IInfoMetadata* aValue, IInfoMetadata* /*aPrevious*/)
{
    iMetadata = aValue;
    iTopology.ReceiverChanged(*this);
}

void ReceiverWatcher::ItemClose(const Brx& /*aId*/, IInfoMetadata* /*aValue*/)
{
    iMetadata = NULL;
}

void ReceiverWatcher::ItemOpen(const Brx& /*aId*/, ITopology2Source* aValue)
{
    if (aValue->Type().Equals(Brn("Receiver")))
    {
        iGroup.Device().Create(MakeFunctorGeneric(*this, &ReceiverWatcher::CreateCallback), eProxyReceiver);
/*
        iGroup.Device.Create<IProxyReceiver*>((receiver) =>
        {
            if (!iDisposed)
            {
                iReceiver = receiver;
                iReceiver.TransportState.AddWatcher(this);
                iReceiver.Metadata.AddWatcher(this);
                iTopology.ReceiverChanged(this);
            }
            else
            {
                receiver.Dispose();
            }
        });
*/
    }
}


void ReceiverWatcher::CreateCallback(void* aArgs)
{
    ArgsTwo<IDevice*, IProxy*>* args = (ArgsTwo<IDevice*, IProxy*>*)aArgs;
    IProxyReceiver* receiver = (IProxyReceiver*)(args->Arg2());

    if (!iDisposed)
    {
        iReceiver = receiver;
        iReceiver->TransportState().AddWatcher(*this);
        iReceiver->Metadata().AddWatcher(*this);
        iTopology.ReceiverChanged(*this);
    }
    else
    {
        receiver->Dispose();
        delete receiver;
    }
    delete args;
}


void ReceiverWatcher::ItemUpdate(const Brx& /*aId*/, ITopology2Source* /*aPrevious*/, ITopology2Source* /*aPrevious*/)
{
}

void ReceiverWatcher::ItemClose(const Brx& /*aId*/, ITopology2Source* /*aValue*/)
{
}


///////////////////////////////////////////////////////////////////

SenderWatcher::SenderWatcher(Topology3& aTopology, ITopology2Group& aGroup)
    :iTopology(aTopology)
    ,iDisposeHandler(new DisposeHandler())
    ,iDevice(aGroup.Device())
    ,iMetadata(SenderMetadata::Empty())
    ,iDisposed(false)
{
    aGroup.Device().Create(MakeFunctorGeneric(*this, &SenderWatcher::CreateCallback), eProxySender);
}


void SenderWatcher::CreateCallback(void* aArgs)
{
    ArgsTwo<IDevice*, IProxy*>* args = (ArgsTwo<IDevice*, IProxy*>*)aArgs;
    IProxySender* sender = (IProxySender*)(args->Arg2());

    if (!iDisposed)
    {
        iSender = sender;
        iSender->Metadata().AddWatcher(*this);
        iTopology.SenderChanged(iDevice, iMetadata->Uri(), Brx::Empty());
    }
    else
    {
        sender->Dispose();
    }
    delete args;
}


void SenderWatcher::Dispose()
{
    iDisposeHandler->Dispose();
    delete iDisposeHandler;

    ISenderMetadata* previous = iMetadata;

    if (iSender != NULL)
    {
        iSender->Metadata().RemoveWatcher(*this);
        iSender->Dispose();
        delete iSender;
        iSender = NULL;
    }

    iTopology.SenderChanged(iDevice, iMetadata->Uri(), previous->Uri());

    iDisposed = true;
}

const Brx& SenderWatcher::Uri()
{
    DisposeLock lock(*iDisposeHandler);
    return iMetadata->Uri();
}

IDevice& SenderWatcher::Device()
{
    DisposeLock lock(*iDisposeHandler);
    return iDevice;
}

void SenderWatcher::ItemOpen(const Brx& /*aId*/, ISenderMetadata* aValue)
{
    iMetadata = aValue;
}

void SenderWatcher::ItemUpdate(const Brx& /*aId*/, ISenderMetadata* aValue, ISenderMetadata* aPrevious)
{
    iMetadata = aValue;
    iTopology.SenderChanged(iDevice, iMetadata->Uri(), aPrevious->Uri());
}

void SenderWatcher::ItemClose(const Brx& /*aId*/, ISenderMetadata* /*aValue*/)
{
    iMetadata = SenderMetadata::Empty();
}


/////////////////////////////////////////////////////////////


Topology3::Topology3(ITopology2* aTopology2, ILog& /*aLog*/)
    :iTopology2(aTopology2)
    ,iNetwork(aTopology2->Network())
    ,iGroups(new WatchableUnordered<ITopology3Group*>(iNetwork))
    ,iDisposed(false)
{
    iNetwork.Schedule(MakeFunctorGeneric(*this, &Topology3::ScheduleCallback), 0);
}


Topology3::~Topology3()
{
    Topology3Sender::DestroyStatics();
}


void Topology3::ScheduleCallback(void*)
{
    iTopology2->Groups().AddWatcher(*this);
}


void Topology3::Dispose()
{
    if (iDisposed)
    {
        //throw new ObjectDisposedException("Topology3.Dispose");
    }

    iNetwork.Execute(MakeFunctorGeneric(*this, &Topology3::DisposeCallback), 0);

    iTopology2->Dispose();
    delete iTopology2;
    iTopology2 = NULL;


    iGroups->Dispose();
    delete iGroups;
    iGroups = NULL;

    iDisposed = true;
}


void Topology3::DisposeCallback(void*)
{
    iTopology2->Groups().RemoveWatcher(*this);

    map<ITopology2Group*, ReceiverWatcher*>::iterator it2;
    for(it2 = iReceiverLookup.begin(); it2!=iReceiverLookup.end(); it2++)
    {
        it2->second->Dispose();
    }

    map<ITopology2Group*, SenderWatcher*>::iterator it3;
    for(it3 = iSenderLookup.begin(); it3!=iSenderLookup.end(); it3++)
    {
        it3->second->Dispose();
    }

    map<ITopology2Group*, Topology3Group*>::iterator it1;
    for(it1 = iGroupLookup.begin(); it1!=iGroupLookup.end(); it1++)
    {
        it1->second->Dispose();
    }

    for(it2 = iReceiverLookup.begin(); it2!=iReceiverLookup.end(); it2++)
    {
        delete it2->second;
    }

    for(it3 = iSenderLookup.begin(); it3!=iSenderLookup.end(); it3++)
    {
        delete it3->second;
    }

    for(it1 = iGroupLookup.begin(); it1!=iGroupLookup.end(); it1++)
    {
        delete it1->second;
    }


}


IWatchableUnordered<ITopology3Group*>& Topology3::Groups()
{
    return *iGroups;
}

INetwork& Topology3::Network()
{
    return iNetwork;
}

void Topology3::UnorderedOpen()
{
}

void Topology3::UnorderedInitialised()
{
}

void Topology3::UnorderedAdd(ITopology2Group* aItem)
{
    Topology3Group* group = new Topology3Group(iNetwork, *aItem);
    iReceiverLookup[aItem] = new ReceiverWatcher(*this, *group);

    if (Ascii::Contains(aItem->Attributes(), Brn("Sender")))
    {
        iSenderLookup[aItem] = new SenderWatcher(*this, *aItem);
    }

    iGroupLookup[aItem] = group;
    iGroups->Add(group);
}

void Topology3::UnorderedRemove(ITopology2Group* aItem)
{
    if (iGroupLookup.count(aItem)>0)
    {
        if (Ascii::Contains(aItem->Attributes(), Brn("Sender")))
        {
            auto sender = iSenderLookup[aItem];
            sender->Dispose();
            delete sender;
            iSenderLookup.erase(aItem);
        }

        // schedule higher layer notification
        Topology3Group* group = iGroupLookup[aItem];

        iGroups->Remove(group);
        iGroupLookup.erase(aItem);

        auto receiver = iReceiverLookup[aItem];
        receiver->Dispose();
        delete receiver;
        iReceiverLookup.erase(aItem);

        group->Dispose();
        delete group;
    }
}

void Topology3::UnorderedClose()
{
}

void Topology3::ReceiverChanged(ReceiverWatcher& aReceiver)
{
    Brn receiverUri(aReceiver.ListeningToUri());

    map<ITopology2Group*, SenderWatcher*>::iterator it;
    for(it = iSenderLookup.begin(); it!=iSenderLookup.end(); it++)
    {
        SenderWatcher* watcher = it->second;
        Brn watcherUri(watcher->Uri());

        if (receiverUri.Equals(Brx::Empty()))
        {
            aReceiver.SetSender(Topology3Sender::Empty());
        }
        else if(receiverUri.Equals(watcher->Uri()))
        {
            // set Topology3Group sender
            aReceiver.SetSender(new Topology3Sender(watcher->Device()));
        }
    }
}

void Topology3::SenderChanged(IDevice& aDevice, const Brx& aUri, const Brx& aPreviousUri)
{
    map<ITopology2Group*, ReceiverWatcher*>::iterator it;
    for(it = iReceiverLookup.begin(); it!=iReceiverLookup.end(); it++)
    {
        ReceiverWatcher* watcher = it->second;

        if (aPreviousUri.Equals(watcher->ListeningToUri()))
        {
            watcher->SetSender(Topology3Sender::Empty());
        }
        else if (aUri.Equals(watcher->ListeningToUri()) && (!aUri.Equals(Brx::Empty())))
        {
            // set Topology3Group sender
            watcher->SetSender(new Topology3Sender(aDevice));
        }
    }
}



