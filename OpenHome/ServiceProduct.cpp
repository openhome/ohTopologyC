#include <OpenHome/OhNetTypes.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/ServiceProduct.h>
#include <OpenHome/Network.h>
#include <OpenHome/Private/Ascii.h>
#include <OpenHome/Private/Debug.h>
#include <vector>

using namespace OpenHome;
using namespace OpenHome::Av;
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



//abstract Task SetSourceIndex(TUint aValue);
//abstract Task SetSourceIndexByName(string aValue);
//abstract Task SetStandby(TBool aValue);

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
        if (oldRoom != NULL)
        {
            delete oldRoom;
        }
    }
    else if (Ascii::CaseInsensitiveEquals(command, Brn("name")))
    {
        Bws<50>* oldName = iCurrentName;
        iCurrentName = new Bws<50>(aCommands.RemainingTrimmed());
        iName->Update(Brn(*iCurrentName));
        if (oldName != NULL)
        {
            delete oldName;
        }
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


/*
Task ServiceProductMock::SetSourceIndex(TUint aValue)
{
    Task task = Task.Factory.StartNew(() =>
    {
        iNetwork.Schedule(() =>
        {
            iSourceIndex.Update(aValue);
        });
    });
    return task;
}
*/

/*
void ServiceProductMock::SetSourceIndexCallback(void* aObj)
{
    iSourceIndex->Update(*((TUint*)aObj));

}
*/

/*
Task ServiceProductMock::SetSourceIndexByName(const Brx& aValue)
{
    THROW(NotSupportedException);
}
*/


/*
Task ServiceProductMock::SetStandby(TBool aValue)
{
    Task task = Task.Factory.StartNew(() =>
    {
        iNetwork.Schedule(() =>
        {
            iStandby.Update(aValue);
        });
    });
    return task;
}
*/

/*
void ServiceProductMock::SetStandbyCallback(void* aObj)
{
    iStandby->Update(*((TBool*)aObj));
}
*/



/////////////////////////////////////////////////////////////////////


ProxyProduct::ProxyProduct(ServiceProduct& aService, IDevice& aDevice)
    :iService(aService)
    ,iDevice(aDevice)
    //: Proxy<ServiceProduct>(aService, aDevice)
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
    return iService.Standby();
}


/*
Task ProxyProduct::SetSourceIndex(TUint aValue)
{
    return iService.SetSourceIndex(aValue);
}


Task ProxyProduct::SetSourceIndexByName(const Brx& aValue)
{
    return iService.SetSourceIndexByName(aValue);
}


Task ProxyProduct::SetStandby(TBool aValue)
{
    return iService.SetStandby(aValue);
}


*/

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



/*
    class ServiceProductNetwork : ServiceProduct
    {
        ServiceProductNetwork(INetwork aNetwork, IInjectorDevice aDevice, CpDevice aCpDevice, ILog aLog  )
            : base(aNetwork, aDevice, aLog)
        {
            iCpDevice = aCpDevice;
            iCpDevice.AddRef();

            iService = new CpProxyAvOpenhomeOrgProduct1(aCpDevice);
            iServiceConfiguration = new CpProxyLinnCoUkConfiguration1(aCpDevice);

            string value;
            if (aCpDevice.GetAttribute("Upnp.Service.linn-co-uk.Volkano", out value))
            {
                if (uint.Parse(value) == 1)
                {
                    iServiceVolkano = new CpProxyLinnCoUkVolkano1(aCpDevice);
                }
            }

            iService.SetPropertyProductRoomChanged(HandleRoomChanged);
            iService.SetPropertyProductNameChanged(HandleNameChanged);
            iService.SetPropertySourceIndexChanged(HandleSourceIndexChanged);
            iService.SetPropertySourceXmlChanged(HandleSourceXmlChanged);
            iService.SetPropertyStandbyChanged(HandleStandbyChanged);

            iServiceConfiguration.SetPropertyParameterXmlChanged(HandleParameterXmlChanged);

            iService.SetPropertyInitialEvent(HandleInitialEvent);
            iServiceConfiguration.SetPropertyInitialEvent(HandleInitialEventConfiguration);
        }

        void Dispose()
        {
            base.Dispose();

            iService.Dispose();
            iService = null;

            iServiceConfiguration.Dispose();
            iServiceConfiguration = null;

            if (iServiceVolkano != null)
            {
                iServiceVolkano.Dispose();
                iServiceVolkano = null;
            }

            iCpDevice.RemoveRef();
        }

        protected Task OnSubscribe()
        {
            Do.Assert(iSubscribedSource == null);
            Do.Assert(iSubscribedConfigurationSource == null);
            Do.Assert(iSubscribedVolkanoSource == null);

            TaskCompletionSource<TBool> volkano = new TaskCompletionSource<TBool>();
            iSubscribedSource = new TaskCompletionSource<TBool>();
            iSubscribedConfigurationSource = new TaskCompletionSource<TBool>();
            iSubscribedVolkanoSource = volkano;

            iService.Subscribe();
            iServiceConfiguration.Subscribe();

            if (iServiceVolkano != null)
            {
                iServiceVolkano.BeginProductId((ptr) =>
                {
                    try
                    {
                        iServiceVolkano.EndProductId(ptr, out iProductId);
                        if (!volkano.Task.IsCanceled)
                        {
                            volkano.SetResult(true);
                        }
                    }
                    catch (ProxyError e)
                    {
                        if (!volkano.Task.IsCanceled)
                        {
                            volkano.SetException(e);
                        }
                    }
                });
            }
            else
            {
                if (!volkano.Task.IsCanceled)
                {
                    volkano.SetResult(true);
                }
            }

            return Task.Factory.ContinueWhenAll(
                new Task[] { iSubscribedSource.Task, iSubscribedConfigurationSource.Task, iSubscribedVolkanoSource.Task },
                (tasks) => { Task.WaitAll(tasks); });
        }

        protected void OnCancelSubscribe()
        {
            if (iSubscribedSource != null)
            {
                iSubscribedSource.TrySetCanceled();
            }
            if (iSubscribedConfigurationSource != null)
            {
                iSubscribedConfigurationSource.TrySetCanceled();
            }
            if (iSubscribedVolkanoSource != null)
            {
                iSubscribedVolkanoSource.TrySetCanceled();
            }
        }

        void HandleInitialEvent()
        {
            iAttributes = iService.PropertyAttributes();
            iManufacturerImageUri = iService.PropertyManufacturerImageUri();
            iManufacturerInfo = iService.PropertyManufacturerInfo();
            iManufacturerName = iService.PropertyManufacturerName();
            iManufacturerUrl = iService.PropertyManufacturerUrl();
            iModelImageUri = iService.PropertyModelImageUri();
            iModelInfo = iService.PropertyModelInfo();
            iModelName = iService.PropertyModelName();
            iModelUrl = iService.PropertyModelUrl();
            iProductImageUri = iService.PropertyProductImageUri();
            iProductInfo = iService.PropertyProductInfo();
            iProductUrl = iService.PropertyProductUrl();

            if (!iSubscribedSource.Task.IsCanceled)
            {
                iSubscribedSource.SetResult(true);
            }
        }

        void HandleInitialEventConfiguration()
        {
            string propertyXml = iServiceConfiguration.PropertyParameterXml();
            iNetwork.Schedule(() =>
            {
                iDisposeHandler.WhenNotDisposed(() =>
                {
                    ParseParameterXml(propertyXml);
                });
            });

            if (!iSubscribedConfigurationSource.Task.IsCanceled)
            {
                iSubscribedConfigurationSource.SetResult(true);
            }
        }

        protected void OnUnsubscribe()
        {
            if (iService != null)
            {
                iService.Unsubscribe();
            }
            if (iServiceConfiguration != null)
            {
                iServiceConfiguration.Unsubscribe();
            }

            iSubscribedSource = null;
            iSubscribedConfigurationSource = null;
            iSubscribedVolkanoSource = null;
        }

        Task SetSourceIndex(TUint aValue)
        {
            TaskCompletionSource<TBool> taskSource = new TaskCompletionSource<TBool>();
            iService.BeginSetSourceIndex(aValue, (ptr) =>
            {
                try
                {
                    iService.EndSetSourceIndex(ptr);
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

        Task SetSourceIndexByName(string aValue)
        {
            TaskCompletionSource<TBool> taskSource = new TaskCompletionSource<TBool>();
            iService.BeginSetSourceIndexByName(aValue, (ptr) =>
            {
                try
                {
                    iService.EndSetSourceIndexByName(ptr);
                    taskSource.SetResult(true);
                }
                catch (Exception e)
                {
                    taskSource.SetException(e);
                }
            });
            return taskSource.Task.ContinueWith((t) => { });
        }

        Task SetStandby(TBool aValue)
        {
            TaskCompletionSource<TBool> taskSource = new TaskCompletionSource<TBool>();
            iService.BeginSetStandby(aValue, (ptr) =>
            {
                try
                {
                    iService.EndSetStandby(ptr);
                    taskSource.SetResult(true);
                }
                catch (Exception e)
                {
                    taskSource.SetException(e);
                }
            });
            return taskSource.Task.ContinueWith((t) => { });
        }

        Task SetRegistration(string aValue)
        {
            TaskCompletionSource<TBool> taskSource = new TaskCompletionSource<TBool>();
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
            return taskSource.Task.ContinueWith((t) => { });
        }

        void HandleRoomChanged()
        {
            string room = iService.PropertyProductRoom();
            iNetwork.Schedule(() =>
            {
                iDisposeHandler.WhenNotDisposed(() =>
                {
                    iRoom.Update(room);
                });
            });
        }

        void HandleNameChanged()
        {
            string name = iService.PropertyProductName();
            iNetwork.Schedule(() =>
            {
                iDisposeHandler.WhenNotDisposed(() =>
                {
                    iName.Update(name);
                });
            });
        }

        void HandleSourceIndexChanged()
        {
            TUint sourceIndex = iService.PropertySourceIndex();
            iNetwork.Schedule(() =>
            {
                iDisposeHandler.WhenNotDisposed(() =>
                {
                    iSourceIndex.Update(sourceIndex);
                });
            });
        }

        void HandleSourceXmlChanged()
        {
            string sourceXml = iService.PropertySourceXml();
            iNetwork.Schedule(() =>
            {
                iDisposeHandler.WhenNotDisposed(() =>
                {
                    iSourceXml.Update(sourceXml);
                });
            });
        }

        void HandleStandbyChanged()
        {
            TBool standby = iService.PropertyStandby();
            iNetwork.Schedule(() =>
            {
                iDisposeHandler.WhenNotDisposed(() =>
                {
                    iStandby.Update(standby);
                });
            });
        }

        void HandleParameterXmlChanged()
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

        void ParseParameterXml(string aParameterXml)
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
            if (registration != null && registration.FirstChild != null)
            {
                iRegistration.Update(registration.FirstChild.Value);
            }
            else
            {
                iRegistration.Update(Brx::Empty());
            }
        }

        readonly CpDevice iCpDevice;
        TaskCompletionSource<TBool> iSubscribedSource;
        TaskCompletionSource<TBool> iSubscribedConfigurationSource;
        TaskCompletionSource<TBool> iSubscribedVolkanoSource;
        CpProxyAvOpenhomeOrgProduct1 iService;
        CpProxyLinnCoUkConfiguration1 iServiceConfiguration;
        CpProxyLinnCoUkVolkano1 iServiceVolkano;
    }
*/
