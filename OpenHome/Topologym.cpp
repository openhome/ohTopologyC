#include <OpenHome/Topologym.h>


using namespace OpenHome;
using namespace OpenHome::Av;
using namespace std;


TopologymSender* TopologymSender::Empty()
{
    return(new TopologymSender());
}


TopologymSender::TopologymSender()
    :iEnabled(false)
    ,iDevice(0)
{
}


TopologymSender::TopologymSender(IDevice& aDevice)
    :iEnabled(true)
    ,iDevice(&aDevice)
{
}


TBool TopologymSender::Enabled()
{
    return iEnabled;
}


IDevice& TopologymSender::Device()
{
    return *iDevice;
}

////////////////////////////////////////////


TopologymGroup::TopologymGroup(INetwork& aNetwork, ITopology2Group& aGroup)
    :iGroup(aGroup)
    ,iSender(new Watchable<ITopologymSender*>(aNetwork, Brn("Sender"), TopologymSender::Empty()))
{
}

void TopologymGroup::Dispose()
{
    if (iDisposed)
    {
        //throw new ObjectDisposedException("TopologymGroup.Dispose");
    }

    iDisposed = true;
}

Brn TopologymGroup::Id()
{
    return iGroup.Id();
}

Brn TopologymGroup::Attributes()
{
    return iGroup.Attributes();
}

Brn TopologymGroup::ModelName()
{
    return iGroup.ModelName();
}

Brn TopologymGroup::ManufacturerName()
{
    return iGroup.ManufacturerName();
}

Brn TopologymGroup::ProductId()
{
    return iGroup.ProductId();
}

IDevice& TopologymGroup::Device()
{
    return iGroup.Device();
}

IWatchable<Brn>& TopologymGroup::Room()
{
    return iGroup.Room();
}

IWatchable<Brn>& TopologymGroup::Name()
{
    return iGroup.Name();
}

IWatchable<TBool>& TopologymGroup::Standby()
{
    return iGroup.Standby();
}

IWatchable<TUint>& TopologymGroup::SourceIndex()
{
    return iGroup.SourceIndex();
}

//IEnumerable<IWatchable<ITopology2Source*>*>& TopologymGroup::Sources()
vector<Watchable<ITopology2Source*>*> TopologymGroup::Sources()
{
    return iGroup.Sources();
}

IWatchable<Brn>& TopologymGroup::Registration()
{
    return iGroup.Registration();
}

IWatchable<ITopologymSender*>& TopologymGroup::Sender()
{
    return *iSender;
}

void TopologymGroup::SetStandby(TBool aValue)
{
    iGroup.SetStandby(aValue);
}

void TopologymGroup::SetSourceIndex(TUint aValue)
{
    iGroup.SetSourceIndex(aValue);
}

void TopologymGroup::SetRegistration(const Brx& aValue)
{
    iGroup.SetRegistration(aValue);
}

void TopologymGroup::SetSender(ITopologymSender* aSender)
{
    iSender->Update(aSender);
}

////////////////////////////////////////////////////////////////////////////


ReceiverWatcher::ReceiverWatcher(Topologym& aTopology, TopologymGroup& aGroup)
    :iDisposed(false)
    ,iTopology(aTopology)
    ,iGroup(aGroup)
{
    for(TUint i=0; i<iGroup.Sources().size(); i++)
    {
        iGroup.Sources()[i]->AddWatcher(*this);

    }
/*
    foreach (IWatchable<ITopology2Source*> s in iGroup.Sources)
    {
        s.AddWatcher(this);
    }
*/
}

void ReceiverWatcher::Dispose()
{
    for(TUint i=0; i<iGroup.Sources().size(); i++)
    {
        iGroup.Sources()[i]->RemoveWatcher(*this);

    }
/*
    foreach (IWatchable<ITopology2Source*> s in iGroup.Sources)
    {
        s.RemoveWatcher(this);
    }
*/

    if (iReceiver != NULL)
    {
        iReceiver->TransportState().RemoveWatcher(*this);
        iReceiver->Metadata().RemoveWatcher(*this);

        iReceiver->Dispose();
        iReceiver = NULL;
    }

    SetSender(TopologymSender::Empty());

    //iGroup = null;
    //iTopology = null;

    iDisposed = true;
}

Brn ReceiverWatcher::ListeningToUri()
{
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

void ReceiverWatcher::SetSender(ITopologymSender* aSender)
{
    iGroup.SetSender(aSender);
}

void ReceiverWatcher::ItemOpen(const Brx& /*aId*/, Brn aValue)
{
//    iTransportState = Brn(aValue);
    iTransportState.Set(aValue);
}

void ReceiverWatcher::ItemUpdate(const Brx& /*aId*/, Brn aValue, Brn /*aPrevious*/)
{
    iTransportState.Set(aValue);
    iTopology.ReceiverChanged(*this);
}

void ReceiverWatcher::ItemClose(const Brx& /*aId*/, Brn /*aValue*/)
{
    iTransportState.Set(Brx::Empty());
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
    }
}


void ReceiverWatcher::ItemUpdate(const Brx& /*aId*/, ITopology2Source* /*aPrevious*/, ITopology2Source* /*aPrevious*/)
{
}

void ReceiverWatcher::ItemClose(const Brx& /*aId*/, ITopology2Source* /*aValue*/)
{
}


///////////////////////////////////////////////////////////////////

SenderWatcher::SenderWatcher(Topologym& aTopology, ITopology2Group& aGroup)
    :iTopology(aTopology)
    ,iDisposeHandler(new DisposeHandler())
    ,iDevice(aGroup.Device())
    ,iMetadata(SenderMetadata::Empty())
    ,iDisposed(false)
{
    aGroup.Device().Create(MakeFunctorGeneric(*this, &SenderWatcher::CreateCallback), eProxySender);

/*
    aGroup.Device.Create<IProxySender*>((sender) =>
    {
        if (!iDisposed)
        {
            iSender = sender;
            iSender.Metadata.AddWatcher(this);
            iTopology.SenderChanged(iDevice, iMetadata.Uri, Brx::Empty());
        }
        else
        {
            sender.Dispose();
        }
    });
*/
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
}


void SenderWatcher::Dispose()
{
    iDisposeHandler->Dispose();

    ISenderMetadata* previous = iMetadata;

    if (iSender != NULL)
    {
        iSender->Metadata().RemoveWatcher(*this);

        iSender->Dispose();
        iSender = NULL;
    }

    iTopology.SenderChanged(iDevice, iMetadata->Uri(), previous->Uri());

    iDisposed = true;
}

Brn SenderWatcher::Uri()
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


Topologym::Topologym(ITopology2* aTopology2, ILog& /*aLog*/)
    :iTopology2(aTopology2)
    ,iNetwork(aTopology2->Network())
    ,iGroups(new WatchableUnordered<ITopologymGroup*>(iNetwork))
    ,iDisposed(false)
{
    iNetwork.Schedule(MakeFunctorGeneric(*this, &Topologym::ScheduleCallback), 0);
}


void Topologym::ScheduleCallback(void*)
{
    iTopology2->Groups().AddWatcher(*this);
}


void Topologym::Dispose()
{
    if (iDisposed)
    {
        //throw new ObjectDisposedException("Topologym.Dispose");
    }

    iNetwork.Execute(MakeFunctorGeneric(*this, &Topologym::DisposeCallback), 0);

    iTopology2 = NULL;
    //iGroupLookup = null;
    //iReceiverLookup = null;
    //iSenderLookup = null;

    iGroups->Dispose();
    iGroups = NULL;

    iDisposed = true;
}


void Topologym::DisposeCallback(void*)
{
    iTopology2->Groups().RemoveWatcher(*this);

    map<ITopology2Group*, TopologymGroup*>::iterator it1;
    for(it1 = iGroupLookup.begin(); it1!=iGroupLookup.end(); it1++)
    {
        it1->second->Dispose();
    }

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
}


IWatchableUnordered<ITopologymGroup*>& Topologym::Groups()
{
    return *iGroups;
}

INetwork& Topologym::Network()
{
    return iNetwork;
}

void Topologym::UnorderedOpen()
{
}

void Topologym::UnorderedInitialised()
{
}

void Topologym::UnorderedAdd(ITopology2Group* aItem)
{
    TopologymGroup* group = new TopologymGroup(iNetwork, *aItem);
    iReceiverLookup[aItem] = new ReceiverWatcher(*this, *group);

    if (Ascii::Contains(aItem->Attributes(), Brn("Sender")))
    {
        iSenderLookup[aItem] = new SenderWatcher(*this, *aItem);
    }

    iGroupLookup[aItem] = group;
    iGroups->Add(group);
}

void Topologym::UnorderedRemove(ITopology2Group* aItem)
{
    if (iGroupLookup.count(aItem)>0)
    {
        TopologymGroup* group = iGroupLookup[aItem];

        if (Ascii::Contains(aItem->Attributes(), Brn("Sender")))
        {
            iSenderLookup[aItem]->Dispose();
            iSenderLookup.erase(aItem);
        }

        // schedule higher layer notification
        iGroups->Remove(group);
        iGroupLookup.erase(aItem);

        iReceiverLookup[aItem]->Dispose();
        iReceiverLookup.erase(aItem);

        group->Dispose();
    }

/*
    TopologymGroup group;
    if (iGroupLookup.TryGetValue(aItem, out group))
    {
        if (aItem.Attributes.Contains("Sender"))
        {
            iSenderLookup[aItem].Dispose();
            iSenderLookup.Remove(aItem);
        }

        // schedule higher layer notification
        iGroups.Remove(group);
        iGroupLookup.Remove(aItem);

        iReceiverLookup[aItem].Dispose();
        iReceiverLookup.Remove(aItem);

        group.Dispose();
    }
*/
}

void Topologym::UnorderedClose()
{
}

void Topologym::ReceiverChanged(ReceiverWatcher& aReceiver)
{
    map<ITopology2Group*, SenderWatcher*>::iterator it;
    for(it = iSenderLookup.begin(); it!=iSenderLookup.end(); it++)
    {
        SenderWatcher* watcher = it->second;

        if (aReceiver.ListeningToUri().Equals(Brx::Empty()))
        {
            aReceiver.SetSender(TopologymSender::Empty());
        }
        else if(aReceiver.ListeningToUri().Equals(watcher->Uri()))
        {
            aReceiver.SetSender(new TopologymSender(watcher->Device()));
        }
    }

/*
    foreach (SenderWatcher s in iSenderLookup.Values)
    {
        if (string.IsNullOrEmpty(aReceiver.ListeningToUri))
        {
            aReceiver.SetSender(TopologymSender::Empty);
        }
        else if (aReceiver.ListeningToUri == s.Uri)
        {
            // set TopologymGroup sender
            aReceiver.SetSender(new TopologymSender(s.Device));
        }
    }
*/
}

void Topologym::SenderChanged(IDevice& aDevice, const Brx& aUri, const Brx& aPreviousUri)
{
    map<ITopology2Group*, ReceiverWatcher*>::iterator it;
    for(it = iReceiverLookup.begin(); it!=iReceiverLookup.end(); it++)
    {
        ReceiverWatcher* watcher = it->second;

        if (aPreviousUri.Equals(watcher->ListeningToUri()))
        {
            watcher->SetSender(TopologymSender::Empty());
        }
        else if (aUri.Equals(watcher->ListeningToUri()))
        {
            // set TopologymGroup sender
            watcher->SetSender(new TopologymSender(aDevice));
        }
    }
/*
    foreach (ReceiverWatcher r in iReceiverLookup.Values)
    {
        if (aPreviousUri == r.ListeningToUri)
        {
            r.SetSender(TopologymSender::Empty);
        }
        else if (aUri == r.ListeningToUri)
        {
            // set TopologymGroup sender
            r.SetSender(new TopologymSender(aDevice));
        }
    }
*/
}



