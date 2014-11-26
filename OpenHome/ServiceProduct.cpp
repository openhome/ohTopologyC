#include <OpenHome/OhNetTypes.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/ServiceProduct.h>
#include <OpenHome/Network.h>
#include <OpenHome/Private/Ascii.h>
#include <OpenHome/Private/Debug.h>
#include <Generated/CpAvOpenhomeOrgProduct1.h>
#include <vector>

using namespace OpenHome;
using namespace OpenHome::Av;
using namespace OpenHome::Net;
using namespace std;


ServiceProduct::ServiceProduct(INetwork& aNetwork, IInjectorDevice& aDevice, ILog& aLog)
    :Service(aNetwork, aDevice, aLog)
    ,iRoom(new Watchable<Brn>(aNetwork, Brn("Room"), Brx::Empty()))
    ,iName(new Watchable<Brn>(aNetwork, Brn("Name"), Brx::Empty()))
    ,iSourceIndex(new Watchable<TUint>(aNetwork, Brn("SourceIndex"), 0))
    ,iSourceXml(new Watchable<Brn>(aNetwork, Brn("SourceXml"), Brx::Empty()))
    ,iStandby(new Watchable<TBool>(aNetwork, Brn("Standby"), false))
    ,iCurrentRoom(NULL)
    ,iCurrentName(NULL)
    ,iCurrentSourceXml(NULL)
{
}


ServiceProduct::~ServiceProduct()
{
    delete iCurrentRoom;
    delete iCurrentName;
    delete iCurrentSourceXml;
    delete iRoom;
    delete iName;
    delete iSourceIndex;
    delete iSourceXml;
    delete iStandby;
}



void ServiceProduct::Dispose()
{
    Service::Dispose();
    iRoom->Dispose();
    iName->Dispose();
    iSourceIndex->Dispose();
    iSourceXml->Dispose();
    iStandby->Dispose();
}


IProxy* ServiceProduct::OnCreate(IDevice& aDevice)
{
    return(new ProxyProduct(*this, aDevice));
}


IWatchable<Brn>& ServiceProduct::Room()
{
    return(*iRoom);
}


IWatchable<Brn>& ServiceProduct::Name()
{
    return(*iName);
}


IWatchable<TUint>& ServiceProduct::SourceIndex()
{
    return(*iSourceIndex);
}


IWatchable<Brn>& ServiceProduct::SourceXml()
{
    return(*iSourceXml);
}


IWatchable<TBool>& ServiceProduct::Standby()
{
    return(*iStandby);
}



//abstract Job SetSourceIndex(TUint aValue);
//abstract Job SetSourceIndexByName(string aValue);
//abstract Job SetStandby(TBool aValue);

// IProduct methods

Brn ServiceProduct::Attributes()
{
    return(Brn(iAttributes));
}


Brn ServiceProduct::ManufacturerImageUri()
{
    return(Brn(iManufacturerImageUri));
}


Brn ServiceProduct::ManufacturerInfo()
{
    return(Brn(iManufacturerInfo));
}


Brn ServiceProduct::ManufacturerName()
{
    return(Brn(iManufacturerName));
}


Brn ServiceProduct::ManufacturerUrl()
{
    return(Brn(iManufacturerUrl));
}


Brn ServiceProduct::ModelImageUri()
{
    return(Brn(iModelImageUri));
}


Brn ServiceProduct::ModelInfo()
{
    return(Brn(iModelInfo));
}


Brn ServiceProduct::ModelName()
{
    return(Brn(iModelName));
}


Brn ServiceProduct::ModelUrl()
{
    return(Brn(iModelUrl));
}


Brn ServiceProduct::ProductImageUri()
{
    return(Brn(iProductImageUri));
}


Brn ServiceProduct::ProductInfo()
{
    return(Brn(iProductInfo));
}


Brn ServiceProduct::ProductUrl()
{
    return(Brn(iProductUrl));
}


Brn ServiceProduct::ProductId()
{
    return(Brn(iProductId));
}

////////////////////////////////////////////////////////////

Source::Source(const Brx& aName, const Brx& aType, TBool aVisible)
    :iName(aName)
    ,iType(aType)
    ,iVisible(aVisible)
{

}


Brn Source::Name()
{
    return(Brn(iName));
}


void Source::SetName(const Brx& aName)
{
    iName.Replace(aName);
}


Brn Source::Type()
{
    return(Brn(iType));
}


TBool Source::Visible()
{
    return(iVisible);
}


void Source::SetVisible(TBool aValue)
{
    iVisible = aValue;
}

////////////////////////////////////////////////////////////////////////////

SrcXml::SrcXml()
{
    iSourceXml.SetBytes(0);
}


const Brx& SrcXml::ToString()
{
    ASSERT(iSources.size()>0);
    if (iSourceXml.Bytes()==0)
    {
        CreateSourceXml();
    }
    return(iSourceXml);
}


void SrcXml::UpdateName(TUint aIndex, const Brx& aName)
{
    iSources[aIndex]->SetName(aName);
    CreateSourceXml();
}


void SrcXml::UpdateVisible(TUint aIndex, TBool aVisible)
{
    iSources[aIndex]->SetVisible(aVisible);
    CreateSourceXml();
}


void SrcXml::Add(unique_ptr<Source> aSource)
{
    iSources.push_back(move(aSource));
}


void SrcXml::CreateSourceXml()
{
    iSourceXml.Replace(Brn("<SourceList>"));

    for(TUint i=0; i<iSources.size(); i++)
    {
        iSourceXml.Append(Brn("<Source>"));
        iSourceXml.Append(Brn("<Name>"));
        iSourceXml.Append(iSources[i]->Name());
        iSourceXml.Append(Brn("</Name>"));
        iSourceXml.Append(Brn("<Type>"));
        iSourceXml.Append(iSources[i]->Type());
        iSourceXml.Append(Brn("</Type>"));
        iSourceXml.Append(Brn("<Visible>"));

        if (iSources[i]->Visible())
        {
            iSourceXml.Append(Brn("true"));
        }
        else
        {
            iSourceXml.Append(Brn("false"));
        }

        iSourceXml.Append(Brn("</Visible>"));
        iSourceXml.Append(Brn("</Source>"));
    }

    iSourceXml.Append(Brn("</SourceList>"));
}


//////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////


ServiceProductNetwork::ServiceProductNetwork(INetwork& aNetwork, IInjectorDevice& aDevice, CpDevice& aCpDevice, ILog& aLog)
    :ServiceProduct(aNetwork, aDevice, aLog)
    ,iCpDevice(aCpDevice)
{
    iCpDevice.AddRef();

    iService = new CpProxyAvOpenhomeOrgProduct1(aCpDevice);

    Functor f1 = MakeFunctor(*this, &ServiceProductNetwork::HandleRoomChanged);
    iService->SetPropertyProductRoomChanged(f1);

    Functor f2 = MakeFunctor(*this, &ServiceProductNetwork::HandleNameChanged);
    iService->SetPropertyProductNameChanged(f2);

    Functor f3 = MakeFunctor(*this, &ServiceProductNetwork::HandleSourceIndexChanged);
    iService->SetPropertySourceIndexChanged(f3);

    Functor f4 = MakeFunctor(*this, &ServiceProductNetwork::HandleSourceXmlChanged);
    iService->SetPropertySourceXmlChanged(f4);

    Functor f5 = MakeFunctor(*this, &ServiceProductNetwork::HandleStandbyChanged);
    iService->SetPropertyStandbyChanged(f5);

    Functor f6 = MakeFunctor(*this, &ServiceProductNetwork::HandleInitialEvent);
    iService->SetPropertyInitialEvent(f6);

}


ServiceProductNetwork::~ServiceProductNetwork()
{
    delete iService;
}

void ServiceProductNetwork::Dispose()
{
    ServiceProduct::Dispose();
    iCpDevice.RemoveRef();
}

Job* ServiceProductNetwork::OnSubscribe()
{
    // Subscribe to (ohNet) Service and get informed later (on a separate thread) when its completed
    // Completion is signalled in HandleInitialEvent()
    iSubscribedSource = new JobDone();
    iService->Subscribe();

/*
    iSubscribedSource = new TaskCompletionSource<bool>();

    iService.Subscribe();


    return Task.Factory.ContinueWhenAll(
        new Task[] { iSubscribedSource.Task, iSubscribedConfigurationSource.Task, iSubscribedVolkanoSource.Task },
        (tasks) => { Task.WaitAll(tasks); });

    since we're not using ConfigurationSource and or VolkanoSource, theres only 1 task now so that would probably become...
    return(iSubscribedSource.Job);

*/
    return(iSubscribedSource->GetJob());
}

void ServiceProductNetwork::OnCancelSubscribe()
{
    if (iSubscribedSource != NULL)
    {
        //iSubscribedSource->TrySetCancelled();
        iSubscribedSource->Cancel();
    }
}

void ServiceProductNetwork::HandleInitialEvent()
{
    Brhz attributes;
    iService->PropertyAttributes(attributes);
    iAttributes.Replace(attributes);

    Brhz manufacturerImageUri;
    iService->PropertyManufacturerImageUri(manufacturerImageUri);
    iManufacturerImageUri.Replace(iManufacturerImageUri);

    Brhz manufacturerInfo;
    iService->PropertyManufacturerInfo(manufacturerInfo);
    iManufacturerInfo.Replace(manufacturerInfo);

    Brhz manufacturerName;
    iService->PropertyManufacturerName(manufacturerName);
    iManufacturerName.Replace(manufacturerName);

    Brhz manufacturerUrl;
    iService->PropertyManufacturerUrl(manufacturerUrl);
    iManufacturerUrl.Replace(manufacturerUrl);

    Brhz modelImageUri;
    iService->PropertyModelImageUri(modelImageUri);
    iModelImageUri.Replace(modelImageUri);

    Brhz modelInfo;
    iService->PropertyModelInfo(modelInfo);
    iModelInfo.Replace(modelInfo);

    Brhz modelName;
    iService->PropertyModelName(modelName);
    iModelName.Replace(modelName);

    Brhz modelUrl;
    iService->PropertyModelUrl(modelUrl);
    iModelUrl.Replace(modelUrl);

    Brhz productImageUri;
    iService->PropertyProductImageUri(productImageUri);
    iProductImageUri.Replace(productImageUri);

    Brhz productInfo;
    iService->PropertyProductInfo(productInfo);
    iProductInfo.Replace(productInfo);

    Brhz productUrl;
    iService->PropertyProductUrl(productUrl);
    iProductUrl.Replace(productUrl);

    if (!iSubscribedSource->GetJob()->IsCancelled())
    {
        iSubscribedSource->SetResult(true);
    }
}



void ServiceProductNetwork::OnUnsubscribe()
{
    if (iService != NULL)
    {
        iService->Unsubscribe();
    }

    iSubscribedSource = NULL;
}

void ServiceProductNetwork::SetSourceIndex(TUint aValue)
{
    FunctorAsync f;
    iService->BeginSetSourceIndex(aValue, f);

    //FunctorAsync f = MakeFunctorAsync(*this, &ServiceProductNetwork::BeginSetSourceIndexCallback);
    //iService->BeginSetSourceIndex(aValue, f);

/*
    iService->BeginSetSourceIndex(aValue, (ptr) =>
    {
        try
        {
            iService->EndSetSourceIndex(ptr);
            Callback();
            //taskSource.SetResult(true);
        }
        catch (Exception e)
        {
            taskSource.SetException(e);
        }
    });
*/

    //jobDone->Job().ContinueWith(t => { iLog.Write("Unobserved exception: {0}\n", t.Exception); }, TaskContinuationOptions.OnlyOnFaulted);
    //return (jobDone->GetJob());
}


void ServiceProductNetwork::SetSourceIndexByName(const Brx& aValue)
{
    FunctorAsync f;
    iService->BeginSetSourceIndexByName(aValue, f);

/*
    iService->BeginSetSourceIndexByName(aValue, (ptr) =>
    {
        try
        {
            iService->EndSetSourceIndexByName(ptr);
            taskSource.SetResult(true);
        }
        catch (Exception e)
        {
            taskSource.SetException(e);
        }
    });
*/
//    FunctorGeneric<void*> f2 = MakeFunctorGeneric(*this, &ServiceProductNetwork::DoNothing);
//    Job* job = jobDone->GetJob()->ContinueWith(f2, NULL);
//    return (job);
}



void ServiceProductNetwork::SetStandby(TBool aValue)
{
    FunctorAsync f;
    iService->BeginSetStandby(aValue, f);
/*
    iService->BeginSetStandby(aValue, (ptr) =>
    {
        try
        {
            iService->EndSetStandby(ptr);
            taskSource.SetResult(true);
        }
        catch (Exception e)
        {
            taskSource.SetException(e);
        }
    });
*/

    //return taskSource.Job.ContinueWith((t) => { });
    //FunctorGeneric<void*> f2 = MakeFunctorGeneric(*this, &ServiceProductNetwork::DoNothing);
    //Job* job = jobDone->GetJob()->ContinueWith(f2, NULL);
    //return (job);
}





void ServiceProductNetwork::HandleRoomChanged()
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceProductNetwork::RoomChangedCallback1);
    iNetwork.Schedule(f, NULL);
/*
    string room;
    iService->PropertyProductRoom(room);

    iNetwork.Schedule(() =>
    {
        iDisposeHandler.WhenNotDisposed(() =>
        {
            iRoom.Update(room);
        });
    });
*/
}


void ServiceProductNetwork::RoomChangedCallback1(void*)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceProductNetwork::RoomChangedCallback2);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}


void ServiceProductNetwork::RoomChangedCallback2(void*)
{
    Brhz room;
    iService->PropertyProductRoom(room);

    Bws<20>* oldRoom = iCurrentRoom;
    iCurrentRoom = new Bws<20>(room);
    iRoom->Update(Brn(*iCurrentRoom));
    delete oldRoom;
}

void ServiceProductNetwork::HandleNameChanged()
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceProductNetwork::NameChangedCallback1);
    iNetwork.Schedule(f, NULL);
}


void ServiceProductNetwork::NameChangedCallback1(void*)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceProductNetwork::NameChangedCallback2);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}


void ServiceProductNetwork::NameChangedCallback2(void*)
{
    Brhz name;
    iService->PropertyProductName(name);

    Bws<50>* oldName = iCurrentName;
    iCurrentName = new Bws<50>(name);

    iName->Update(Brn(*iCurrentName));
    delete oldName;
}



void ServiceProductNetwork::HandleSourceIndexChanged()
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceProductNetwork::SourceIndexChangedCallback1);
    iNetwork.Schedule(f, NULL);

/*
    TUint sourceIndex;
    iService->PropertySourceIndex(sourceIndex);

    iNetwork.Schedule(() =>
    {
        iDisposeHandler.WhenNotDisposed(() =>
        {
            iSourceIndex.Update(sourceIndex);
        });
    });
*/
}

void ServiceProductNetwork::SourceIndexChangedCallback1(void*)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceProductNetwork::SourceIndexChangedCallback2);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}


void ServiceProductNetwork::SourceIndexChangedCallback2(void*)
{
    TUint sourceIndex;
    iService->PropertySourceIndex(sourceIndex);
    iSourceIndex->Update(sourceIndex);
}


void ServiceProductNetwork::HandleSourceXmlChanged()
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceProductNetwork::SourceXmlChangedCallback1);
    iNetwork.Schedule(f, NULL);
/*
    string sourceXml;
    iService->PropertySourceXml(sourceXml);

    iNetwork.Schedule(() =>
    {
        iDisposeHandler.WhenNotDisposed(() =>
        {
            iSourceXml.Update(sourceXml);
        });
    });
*/
}


void ServiceProductNetwork::SourceXmlChangedCallback1(void*)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceProductNetwork::SourceXmlChangedCallback2);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}

void ServiceProductNetwork::SourceXmlChangedCallback2(void*)
{
    Brhz sourceXml;
    iService->PropertySourceXml(sourceXml);

    Bws<2048>* oldSourceXml = iCurrentSourceXml;
    iCurrentSourceXml = new Bws<2048>(sourceXml);

    Brn xml(*iCurrentSourceXml);

    iSourceXml->Update(xml);

    delete oldSourceXml;
}

void ServiceProductNetwork::HandleStandbyChanged()
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceProductNetwork::StandbyChangedCallback1);
    iNetwork.Schedule(f, NULL);
/*
    TBool standby;
    iService->PropertyStandby(standby);

    iNetwork.Schedule(() =>
    {
        iDisposeHandler.WhenNotDisposed(() =>
        {
            iStandby.Update(standby);
        });
    });
*/
}

void ServiceProductNetwork::StandbyChangedCallback1(void*)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceProductNetwork::StandbyChangedCallback2);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}

void ServiceProductNetwork::StandbyChangedCallback2(void*)
{
    TBool standby;
    iService->PropertyStandby(standby);
    iStandby->Update(standby);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

ServiceProductMock::ServiceProductMock(INetwork& aNetwork, IInjectorDevice& aDevice, const Brx& aRoom, const Brx& aName, TUint aSourceIndex, unique_ptr<SrcXml> aSourceXmlFactory, TBool aStandby,
    const Brx& aAttributes, const Brx& aManufacturerImageUri, const Brx& aManufacturerInfo, const Brx& aManufacturerName, const Brx& aManufacturerUrl, const Brx& aModelImageUri, const Brx& aModelInfo, const Brx& aModelName,
    const Brx& aModelUrl, const Brx& aProductImageUri, const Brx& aProductInfo, const Brx& aProductUrl, const Brx& aProductId, ILog& aLog)
    : ServiceProduct(aNetwork, aDevice, aLog)
    ,iSourceXmlFactory(move(aSourceXmlFactory))
{
//    Brn x(aSourceXmlFactory->ToString());

    iAttributes.Replace(aAttributes);
    iManufacturerImageUri.Replace(aManufacturerImageUri);
    iManufacturerInfo.Replace(aManufacturerInfo);
    iManufacturerName.Replace(aManufacturerName);
    iManufacturerUrl.Replace(aManufacturerUrl);
    iModelImageUri.Replace(aModelImageUri);
    iModelInfo.Replace(aModelInfo);
    iModelName.Replace(aModelName);
    iModelUrl.Replace(aModelUrl);
    iProductImageUri.Replace(aProductImageUri);
    iProductInfo.Replace(aProductInfo);
    iProductUrl.Replace(aProductUrl);
    iProductId.Replace(aProductId);

    iCurrentRoom = new Bws<20>(aRoom);
    iCurrentName = new Bws<50>(aName);
    iCurrentSourceXml = new Bws<2048>(iSourceXmlFactory->ToString());

    iRoom->Update(Brn(*iCurrentRoom));
    iName->Update(Brn(*iCurrentName));
    iSourceXml->Update(Brn(*iCurrentSourceXml));
    iSourceIndex->Update(aSourceIndex);
    iStandby->Update(aStandby);
}


void ServiceProductMock::Execute(ICommandTokens& aCommands)
{
    Brn command = aCommands.Next();

    if (Ascii::CaseInsensitiveEquals(command, Brn("attributes")))
    {
        iAttributes.Replace(aCommands.RemainingTrimmed());
    }
    else if (Ascii::CaseInsensitiveEquals(command, Brn("manufacturerimageuri")))
    {
        iManufacturerImageUri.Replace(aCommands.RemainingTrimmed());
    }
    else if (Ascii::CaseInsensitiveEquals(command, Brn("manufacturerinfo")))
    {
        iManufacturerInfo.Replace(aCommands.RemainingTrimmed());
    }
    else if (Ascii::CaseInsensitiveEquals(command, Brn("manufacturername")))
    {
        iManufacturerName.Replace(aCommands.RemainingTrimmed());
    }
    else if (Ascii::CaseInsensitiveEquals(command, Brn("manufacturerurl")))
    {
        iManufacturerUrl.Replace(aCommands.RemainingTrimmed());
    }
    else if (Ascii::CaseInsensitiveEquals(command, Brn("modelimageuri")))
    {
        iModelImageUri.Replace(aCommands.RemainingTrimmed());
    }
    else if (Ascii::CaseInsensitiveEquals(command, Brn("modelinfo")))
    {
        iModelInfo.Replace(aCommands.RemainingTrimmed());
    }
    else if (Ascii::CaseInsensitiveEquals(command, Brn("modelname")))
    {
        iManufacturerName.Replace(aCommands.RemainingTrimmed());
    }
    else if (Ascii::CaseInsensitiveEquals(command, Brn("modelurl")))
    {
        iModelUrl.Replace(aCommands.RemainingTrimmed());
    }
    else if (Ascii::CaseInsensitiveEquals(command, Brn("productimageuri")))
    {
        iProductImageUri.Replace(aCommands.RemainingTrimmed());
    }
    else if (Ascii::CaseInsensitiveEquals(command, Brn("productinfo")))
    {
        iProductInfo.Replace(aCommands.RemainingTrimmed());
    }
    else if (Ascii::CaseInsensitiveEquals(command, Brn("producturl")))
    {
        iProductUrl.Replace(aCommands.RemainingTrimmed());
    }
    else if (Ascii::CaseInsensitiveEquals(command, Brn("productid")))
    {
        iProductId.Replace(aCommands.RemainingTrimmed());
    }
    else if (Ascii::CaseInsensitiveEquals(command, Brn("room")))
    {
        Bws<20>* oldRoom = iCurrentRoom;
        iCurrentRoom = new Bws<20>(aCommands.RemainingTrimmed());
        iRoom->Update(Brn(*iCurrentRoom));
        delete oldRoom;
    }
    else if (Ascii::CaseInsensitiveEquals(command, Brn("name")))
    {
        Bws<50>* oldName = iCurrentName;
        iCurrentName = new Bws<50>(aCommands.RemainingTrimmed());
        iName->Update(Brn(*iCurrentName));
        delete oldName;
    }
    else if (Ascii::CaseInsensitiveEquals(command, Brn("sourceindex")))
    {
        iSourceIndex->Update(Ascii::Uint(aCommands.Next()));
    }
    else if (Ascii::CaseInsensitiveEquals(command, Brn("standby")))
    {
        Brn val = aCommands.Next();
        TBool standby = Ascii::CaseInsensitiveEquals(val, Brn("true"));
        iStandby->Update(standby);
    }
    else if (Ascii::CaseInsensitiveEquals(command, Brn("source")))
    {
        TUint index = Ascii::Uint(aCommands.Next());
        Brn property = aCommands.Next();

        if (Ascii::CaseInsensitiveEquals(property, Brn("name")))
        {
            iSourceXmlFactory->UpdateName(index, aCommands.RemainingTrimmed());
            iSourceXml->Update(Brn(iSourceXmlFactory->ToString()));
        }
        else if (Ascii::CaseInsensitiveEquals(property, Brn("visible")))
        {
            Brn val = aCommands.Next();
            TBool visible = Ascii::CaseInsensitiveEquals(val, Brn("true"));
            iSourceXmlFactory->UpdateVisible(index, visible);
            iSourceXml->Update(Brn(iSourceXmlFactory->ToString()));
        }

        else
        {
            THROW(NotSupportedException);
        }

    }

    else
    {
        THROW(NotSupportedException);
    }


}



void ServiceProductMock::SetSourceIndex(TUint aIndex)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, & ServiceProductMock::SetSourceIndexCallback);

    auto u = new UintValue();
    u->iUintValue = aIndex;

    iNetwork.Schedule(f, u);


/*
    Job task = Job.Factory.StartNew(() =>
    {
        iNetwork.Schedule(() =>
        {
            iSourceIndex.Update(aValue);
        });
    });
    return task;
*/
}


void ServiceProductMock::SetSourceIndexCallback(void* aIndex)
{
    TUint index = ((UintValue*)aIndex)->iUintValue;
    delete aIndex;
    iSourceIndex->Update(index);
}


void ServiceProductMock::SetSourceIndexByName(const Brx& /*aValue*/)
{
    THROW(NotSupportedException);
}


void ServiceProductMock::SetStandby(TBool aValue)
{
/*
    Job task = Job.Factory.StartNew(() =>
    {
        iNetwork.Schedule(() =>
        {
            iStandby.Update(aValue);
        });
    });
    return task;
*/
    auto f = MakeFunctorGeneric(*this, &ServiceProductMock::SetStandbyCallback);

    if (aValue)
    {
        iNetwork.Schedule(f, (void*)1);
    }
    else
    {
        iNetwork.Schedule(f, (void*)0);
    }
}

void ServiceProductMock::SetStandbyCallback(void* aValue)
{
    iStandby->Update(aValue>0);
}

/////////////////////////////////////////////////////////////////////


ProxyProduct::ProxyProduct(ServiceProduct& aService, IDevice& aDevice)
    :iService(aService)
    ,iDevice(aDevice)
{
}


IWatchable<Brn>& ProxyProduct::Room()
{
    return iService.Room();
}


IWatchable<Brn>& ProxyProduct::Name()
{
    return iService.Name();
}


IWatchable<TUint>& ProxyProduct::SourceIndex()
{
    return iService.SourceIndex();
}


IWatchable<Brn>& ProxyProduct::SourceXml()
{
    return iService.SourceXml();
}


IWatchable<TBool>& ProxyProduct::Standby()
{
    return (iService.Standby());
}



void ProxyProduct::SetSourceIndex(TUint aValue)
{
    iService.SetSourceIndex(aValue);
}


void ProxyProduct::SetSourceIndexByName(const Brx& aValue)
{
    iService.SetSourceIndexByName(aValue);
}


void ProxyProduct::SetStandby(TBool aValue)
{
    iService.SetStandby(aValue);
}


Brn ProxyProduct::Attributes()
{
    return iService.Attributes();
}


Brn ProxyProduct::ManufacturerImageUri()
{
    return iService.ManufacturerImageUri();
}


Brn ProxyProduct::ManufacturerInfo()
{
    return iService.ManufacturerInfo();
}


Brn ProxyProduct::ManufacturerName()
{
    return iService.ManufacturerName();
}


Brn ProxyProduct::ManufacturerUrl()
{
    return iService.ManufacturerUrl();
}


Brn ProxyProduct::ModelImageUri()
{
    return iService.ModelImageUri();
}


Brn ProxyProduct::ModelInfo()
{
    return iService.ModelInfo();
}


Brn ProxyProduct::ModelName()
{
    return iService.ModelName();
}


Brn ProxyProduct::ModelUrl()
{
    return iService.ModelUrl();
}


Brn ProxyProduct::ProductImageUri()
{
    return iService.ProductImageUri();
}


Brn ProxyProduct::ProductInfo()
{
    return iService.ProductInfo();
}


Brn ProxyProduct::ProductUrl()
{
    return iService.ProductUrl();
}


Brn ProxyProduct::ProductId()
{
    return iService.ProductId();
}

void ProxyProduct::Dispose()
{
    iService.Unsubscribe();
}

IDevice& ProxyProduct::Device()
{
    return (iDevice);
}

