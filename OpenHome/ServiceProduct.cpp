#include <OpenHome/OhNetTypes.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/ServiceProduct.h>
#include <OpenHome/Network.h>
#include <OpenHome/Private/Ascii.h>
#include <OpenHome/Private/Debug.h>
#include <vector>

using namespace OpenHome;
using namespace OpenHome::Av;
using namespace OpenHome::Net;
using namespace std;


ServiceProduct::ServiceProduct(INetwork& aNetwork, IInjectorDevice& aDevice, ILog& aLog)
    :Service(aNetwork, &aDevice, aLog)
    ,iRoom(new Watchable<Brn>(aNetwork, Brn("Room"), Brx::Empty()))
    ,iName(new Watchable<Brn>(aNetwork, Brn("Name"), Brx::Empty()))
    ,iSourceIndex(new Watchable<TUint>(aNetwork, Brn("SourceIndex"), 0))
    ,iSourceXml(new Watchable<Brn>(aNetwork, Brn("SourceXml"), Brx::Empty()))
    ,iStandby(new Watchable<TBool>(aNetwork, Brn("Standby"), false))
{
}


void ServiceProduct::Dispose()
{
    Service::Dispose();
    iRoom->Dispose();
    iName->Dispose();
    iSourceIndex->Dispose();
    iSourceXml->Dispose();
    iStandby->Dispose();

    delete iCurrentRoom;
    delete iCurrentName;
    delete iRoom;
    delete iName;
    delete iSourceIndex;
    delete iSourceXml;
    delete iStandby;
}


IProxy* ServiceProduct::OnCreate(IDevice* aDevice)
{
    return(new ProxyProduct(*this, *aDevice));
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

    iRoom->Update(Brn(*iCurrentRoom));
    iName->Update(Brn(*iCurrentName));
    iSourceIndex->Update(aSourceIndex);
    iSourceXml->Update(Brn(iSourceXmlFactory->ToString()));
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



Job* ServiceProductMock::SetSourceIndex(TUint aValue)
{
    return(0);

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


void ServiceProductMock::SetSourceIndexCallback(void* aObj)
{
    iSourceIndex->Update(*((TUint*)aObj));

}



Job* ServiceProductMock::SetSourceIndexByName(const Brx& aValue)
{
    THROW(NotSupportedException);
}


Job* ServiceProductMock::SetStandby(TBool aValue)
{
    return(0);

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



Job* ProxyProduct::SetSourceIndex(TUint aValue)
{
    return (iService.SetSourceIndex(aValue));
}


Job* ProxyProduct::SetSourceIndexByName(const Brx& aValue)
{
    return (iService.SetSourceIndexByName(aValue));
}


Job* ProxyProduct::SetStandby(TBool aValue)
{
    return (iService.SetStandby(aValue));
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

///////////////////////////////////////////////////////////////////////////////////////////////




ServiceProductNetwork::ServiceProductNetwork(INetwork& aNetwork, IInjectorDevice& aDevice, CpDevice& aCpDevice, ILog& aLog)
    :ServiceProduct(aNetwork, aDevice, aLog)
    ,iCpDevice(aCpDevice)
{
    iCpDevice.AddRef();

    iService = new CpProxyAvOpenhomeOrgProduct1(aCpDevice);
    //iServiceConfiguration = new CpProxyLinnCoUkConfiguration1(aCpDevice);

/*
    Brh value;
    if (aCpDevice.GetAttribute(Brn("Upnp.Service.linn-co-uk.Volkano"), value))
    {
        if (uint.Parse(value) == 1)
        {
            iServiceVolkano = new CpProxyLinnCoUkVolkano1(aCpDevice);
        }
    }
*/
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

    //iServiceConfiguration.SetPropertyParameterXmlChanged(HandleParameterXmlChanged);
    //iServiceConfiguration.SetPropertyInitialEvent(HandleInitialEventConfiguration);
}

void ServiceProductNetwork::Dispose()
{
    ServiceProduct::Dispose();

    delete iService;
    iService = NULL;
/*
    iServiceConfiguration.Dispose();
    iServiceConfiguration = NULL;

    if (iServiceVolkano != NULL)
    {
        iServiceVolkano.Dispose();
        iServiceVolkano = NULL;
    }
*/
    iCpDevice.RemoveRef();
}

Job* ServiceProductNetwork::OnSubscribe()
{
    ASSERT(iSubscribedSource == NULL);
    //ASSERT(iSubscribedConfigurationSource == NULL);
    //ASSERT(iSubscribedVolkanoSource == NULL);

    //JobDone volkano = new JobDone();
    iSubscribedSource = new JobDone();
    //iSubscribedConfigurationSource = new JobDone();
    //iSubscribedVolkanoSource = volkano;

    iService->Subscribe();
    //iServiceConfiguration.Subscribe();

/*
    if (iServiceVolkano != NULL)
    {
        iServiceVolkano.BeginProductId((ptr) =>
        {
            try
            {
                iServiceVolkano.EndProductId(ptr, out iProductId);
                if (!volkano.Job.IsCancelled)
                {
                    volkano.SetResult(true);
                }
            }
            catch (ProxyError e)
            {
                if (!volkano.Job.IsCancelled)
                {
                    volkano.SetException(e);
                }
            }
        });
    }
    else
    {
        if (!volkano.Job.IsCancelled)
        {
            volkano.SetResult(true);
        }
    }
*/


/*
    return Job.Factory.ContinueWhenAll(
        new Job[] { iSubscribedSource.Job , iSubscribedConfigurationSource.Job, iSubscribedVolkanoSource.Job },
        (tasks) => { Job.WaitAll(tasks); });

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
/*
    if (iSubscribedConfigurationSource != NULL)
    {
        iSubscribedConfigurationSource.TrySetCanceled();
    }
    if (iSubscribedVolkanoSource != NULL)
    {
        iSubscribedVolkanoSource.TrySetCanceled();
    }
*/
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

/*
void ServiceProductNetwork::HandleInitialEventConfiguration()
{

    string propertyXml = iServiceConfiguration.PropertyParameterXml();

    iNetwork.Schedule(() =>
    {
        iDisposeHandler.WhenNotDisposed(() =>
        {
            ParseParameterXml(propertyXml);
        });
    });

    if (!iSubscribedConfigurationSource.Job.IsCancelled)
    {
        iSubscribedConfigurationSource.SetResult(true);
    }

}
*/

void ServiceProductNetwork::OnUnsubscribe()
{
    if (iService != NULL)
    {
        iService->Unsubscribe();
    }
/*
    if (iServiceConfiguration != NULL)
    {
        iServiceConfiguration.Unsubscribe();
    }
*/
    iSubscribedSource = NULL;
    //iSubscribedConfigurationSource = NULL;
    //iSubscribedVolkanoSource = NULL;
}

Job* ServiceProductNetwork::SetSourceIndex(TUint aValue)
{
    JobDone* jobDone = new JobDone();

    FunctorAsync f = MakeFunctorAsync(*this, &ServiceProductNetwork::BeginSetSourceIndexCallback);
    iService->BeginSetSourceIndex(aValue, f);

/*
    iService->BeginSetSourceIndex(aValue, (ptr) =>
    {
        try
        {
            iService->EndSetSourceIndex(ptr);
            taskSource.SetResult(true);
        }
        catch (Exception e)
        {
            taskSource.SetException(e);
        }
    });
*/

    //jobDone->Job().ContinueWith(t => { iLog.Write("Unobserved exception: {0}\n", t.Exception); }, TaskContinuationOptions.OnlyOnFaulted);
    return (jobDone->GetJob());
}

void ServiceProductNetwork::BeginSetSourceIndexCallback(IAsync& aValue)
{
    JobDone* jobDone = new JobDone(); // FIXME: this should be the object created in the original method

    try
    {
        iService->EndSetSourceIndex(aValue);
        jobDone->SetResult(true);
    }
    catch (Exception e)
    {
        jobDone->SetException(e);
    }
}


Job* ServiceProductNetwork::SetSourceIndexByName(const Brx& aValue)
{
    JobDone* jobDone = new JobDone();

    //FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceProductNetwork::BeginSetSourceIndexByNameCallback);
    FunctorAsync f = MakeFunctorAsync(*this, &ServiceProductNetwork::BeginSetSourceIndexByNameCallback);
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
    FunctorGeneric<void*> f2 = MakeFunctorGeneric(*this, &ServiceProductNetwork::DoNothing);
    Job* job = jobDone->GetJob()->ContinueWith(f2, NULL);
    return (job);
}


void ServiceProductNetwork::BeginSetSourceIndexByNameCallback(IAsync& aAsync)
{
    JobDone* jobDone = new JobDone(); // FIXME: this should be the object created in the original method

    try
    {
        iService->EndSetSourceIndexByName(aAsync);
        jobDone->SetResult(true);
    }
    catch (Exception e)
    {
        jobDone->SetException(e);
    }
}


void ServiceProductNetwork::DoNothing(void* /*aObj*/)
{

}


Job* ServiceProductNetwork::SetStandby(TBool aValue)
{
    JobDone* jobDone = new JobDone();

    //FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceProductNetwork::BeginSetStandbyCallback);
    FunctorAsync f = MakeFunctorAsync(*this, &ServiceProductNetwork::BeginSetStandbyCallback);
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
    FunctorGeneric<void*> f2 = MakeFunctorGeneric(*this, &ServiceProductNetwork::DoNothing);
    Job* job = jobDone->GetJob()->ContinueWith(f2, NULL);
    return (job);
}


void ServiceProductNetwork::BeginSetStandbyCallback(IAsync& aAsync)
{
    JobDone* jobDone = new JobDone(); // FIXME: this should be the object created in the original method

    try
    {
        iService->EndSetStandby(aAsync);
        jobDone->SetResult(true);
    }
    catch (Exception e)
    {
        jobDone->SetException(e);
    }
}



/*
Job ServiceProductNetwork::SetRegistration(string aValue)
{

    JobDone taskSource = new JobDone();
    iServiceConfiguration.BeginSetParameter("TuneIn Radio", "Test Mode", "true", (ptr) =>
    {
        try
        {
            iServiceConfiguration.EndSetParameter(ptr);
            iServiceConfiguration.BeginSetParameter("TuneIn Radio", "Password", aValue, (ptr2) =>
            {
                try
                {
                    iServiceConfiguration.EndSetParameter(ptr2);
                    iServiceConfiguration.BeginSetParameter("TuneIn Radio", "Test Mode", "false", (ptr3) =>
                    {
                        try
                        {
                            iServiceConfiguration.EndSetParameter(ptr3);
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
        }
        catch (Exception e)
        {
            taskSource.SetException(e);
        }
    });
    return taskSource.Job.ContinueWith((t) => { });
}
*/

void ServiceProductNetwork::HandleRoomChanged()
{
    Brhz room;
    iService->PropertyProductRoom(room);
/*
    iNetwork.Schedule(() =>
    {
        iDisposeHandler.WhenNotDisposed(() =>
        {
            iRoom.Update(room);
        });
    });
*/
}

void ServiceProductNetwork::HandleNameChanged()
{
    Brhz name;
    iService->PropertyProductName(name);
/*
    iNetwork.Schedule(() =>
    {
        iDisposeHandler.WhenNotDisposed(() =>
        {
            iName.Update(name);
        });
    });
*/
}

void ServiceProductNetwork::HandleSourceIndexChanged()
{
    TUint sourceIndex;
    iService->PropertySourceIndex(sourceIndex);
/*
    iNetwork.Schedule(() =>
    {
        iDisposeHandler.WhenNotDisposed(() =>
        {
            iSourceIndex.Update(sourceIndex);
        });
    });
*/
}

void ServiceProductNetwork::HandleSourceXmlChanged()
{
    Brhz sourceXml;
    iService->PropertySourceXml(sourceXml);
/*
    iNetwork.Schedule(() =>
    {
        iDisposeHandler.WhenNotDisposed(() =>
        {
            iSourceXml.Update(sourceXml);
        });
    });
*/
}

void ServiceProductNetwork::HandleStandbyChanged()
{
    TBool standby;
    iService->PropertyStandby(standby);
/*
    iNetwork.Schedule(() =>
    {
        iDisposeHandler.WhenNotDisposed(() =>
        {
            iStandby.Update(standby);
        });
    });
*/
}

/*
void ServiceProductNetwork::HandleParameterXmlChanged()
{
    string paramXml = iServiceConfiguration.PropertyParameterXml();
    iNetwork.Schedule(() =>
    {
        iDisposeHandler.WhenNotDisposed(() =>
        {
            ParseParameterXml(paramXml);
        });
    });
}

void ServiceProductNetwork::ParseParameterXml(string aParameterXml)
{
    XmlDocument document = new XmlDocument();
    document.LoadXml(aParameterXml);

    //<ParameterList>
    // ...
    //  <Parameter>
    //    <Target>TuneIn Radio</Target>
    //    <Name>Password</Name>
    //    <Type>string</Type>
    //    <Value></Value>
    //  </Parameter>
    // ...
    //</ParameterList>

    System.Xml.XmlNode registration = document.SelectSingleNode("/ParameterList/Parameter[Target=\"TuneIn Radio\" and Name=\"Password\"]/Value");
    if (registration != NULL && registration.FirstChild != NULL)
    {
        iRegistration.Update(registration.FirstChild.Value);
    }
    else
    {
        iRegistration.Update(Brx::Empty());
    }
}
*/

