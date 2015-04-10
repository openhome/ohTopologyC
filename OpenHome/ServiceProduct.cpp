#include <OpenHome/OhNetTypes.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/ServiceProduct.h>
#include <OpenHome/Network.h>
#include <OpenHome/Private/Ascii.h>
#include <OpenHome/Private/Debug.h>
#include <Generated/CpAvOpenhomeOrgProduct1.h>
#include <vector>

using namespace OpenHome;
using namespace OpenHome::Topology;
using namespace OpenHome::Net;
using namespace std;


ServiceProduct::ServiceProduct(IInjectorDevice& aDevice, ILog& aLog)
    :Service(aDevice, aLog)
    ,iRoomName(new Watchable<Brn>(iNetwork, Brn("RoomName"), Brx::Empty()))
    ,iName(new Watchable<Brn>(iNetwork, Brn("Name"), Brx::Empty()))
    ,iSourceIndex(new Watchable<TUint>(iNetwork, Brn("SourceIndex"), 0))
    ,iSourceXml(new Watchable<Brn>(iNetwork, Brn("SourceXml"), Brx::Empty()))
    ,iStandby(new Watchable<TBool>(iNetwork, Brn("Standby"), false))
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
    delete iRoomName;
    delete iName;
    delete iSourceIndex;
    delete iSourceXml;
    delete iStandby;
}



void ServiceProduct::Dispose()
{
    Service::Dispose();
    iRoomName->Dispose();
    iName->Dispose();
    iSourceIndex->Dispose();
    iSourceXml->Dispose();
    iStandby->Dispose();
}


IProxy* ServiceProduct::OnCreate(IDevice& aDevice)
{
    return(new ProxyProduct(*this, aDevice));
}


IWatchable<Brn>& ServiceProduct::RoomName()
{
    return(*iRoomName);
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

TopologySource::TopologySource(const Brx& aName, const Brx& aType, TBool aVisible)
    :iName(aName)
    ,iType(aType)
    ,iVisible(aVisible)
{

}


Brn TopologySource::Name()
{
    return(Brn(iName));
}


void TopologySource::SetName(const Brx& aName)
{
    iName.Replace(aName);
}


Brn TopologySource::Type()
{
    return(Brn(iType));
}


TBool TopologySource::Visible()
{
    return(iVisible);
}


void TopologySource::SetVisible(TBool aValue)
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


void SrcXml::Add(unique_ptr<TopologySource> aSource)
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


ServiceProductNetwork::ServiceProductNetwork(IInjectorDevice& aDevice, CpProxyAvOpenhomeOrgProduct1* aService, ILog& aLog)
    :ServiceProduct(aDevice, aLog)
    ,iService(aService)
    ,iSubscribed(false)
{

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
}

TBool ServiceProductNetwork::OnSubscribe()
{
    // Subscribe to (ohNet) Service and get informed later (on a separate thread) when its completed
    // Completion is signalled in HandleInitialEvent()
    iService->Subscribe();
    iSubscribed = true;
    return(false); // false = not mock
}

void ServiceProductNetwork::OnCancelSubscribe()
{
/*
    if (iSubscribedSource != NULL)
    {
        //iSubscribedSource->TrySetCancelled();
        iSubscribedSource->iCancelled = true;
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

    //if (!iSubscribedSource->iCancelled)
    //{
        SubscribeCompleted();
    //}
}



void ServiceProductNetwork::OnUnsubscribe()
{
    if (iService != NULL)
    {
        iService->Unsubscribe();
    }
    iSubscribed = false;
}

void ServiceProductNetwork::SetSourceIndex(TUint aValue)
{
    FunctorAsync f;
    iService->BeginSetSourceIndex(aValue, f);
}


void ServiceProductNetwork::SetSourceIndexByName(const Brx& aValue)
{
    FunctorAsync f;
    iService->BeginSetSourceIndexByName(aValue, f);
}



void ServiceProductNetwork::SetStandby(TBool aValue)
{
    FunctorAsync f;
    iService->BeginSetStandby(aValue, f);
}


void ServiceProductNetwork::HandleRoomChanged()
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceProductNetwork::RoomChangedCallback1);
    Schedule(f, NULL);
}


void ServiceProductNetwork::RoomChangedCallback1(void*)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceProductNetwork::RoomChangedCallback2);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}


void ServiceProductNetwork::RoomChangedCallback2(void*)
{
    if (iSubscribed)
    {
        Brhz room;
        iService->PropertyProductRoom(room);

        Bws<20>* oldRoom = iCurrentRoom;
        iCurrentRoom = new Bws<20>(room);
        iRoomName->Update(Brn(*iCurrentRoom));
        delete oldRoom;
    }
}

void ServiceProductNetwork::HandleNameChanged()
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceProductNetwork::NameChangedCallback1);
    Schedule(f, NULL);
}


void ServiceProductNetwork::NameChangedCallback1(void*)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceProductNetwork::NameChangedCallback2);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}


void ServiceProductNetwork::NameChangedCallback2(void*)
{
    if (iSubscribed)
    {
        Brhz name;
        iService->PropertyProductName(name);

        Bws<50>* oldName = iCurrentName;
        iCurrentName = new Bws<50>(name);

        iName->Update(Brn(*iCurrentName));
        delete oldName;
    }
}



void ServiceProductNetwork::HandleSourceIndexChanged()
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceProductNetwork::SourceIndexChangedCallback1);
    Schedule(f, NULL);
}

void ServiceProductNetwork::SourceIndexChangedCallback1(void*)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceProductNetwork::SourceIndexChangedCallback2);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}


void ServiceProductNetwork::SourceIndexChangedCallback2(void*)
{
    if (iSubscribed)
    {
        TUint sourceIndex;
        iService->PropertySourceIndex(sourceIndex);
        iSourceIndex->Update(sourceIndex);
    }
}


void ServiceProductNetwork::HandleSourceXmlChanged()
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceProductNetwork::SourceXmlChangedCallback1);
    Schedule(f, NULL);
}


void ServiceProductNetwork::SourceXmlChangedCallback1(void*)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceProductNetwork::SourceXmlChangedCallback2);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}

void ServiceProductNetwork::SourceXmlChangedCallback2(void*)
{
    if (iSubscribed)
    {
        Brhz sourceXml;
        iService->PropertySourceXml(sourceXml);

        Bws<2048>* oldSourceXml = iCurrentSourceXml;
        iCurrentSourceXml = new Bws<2048>(sourceXml);

        Brn xml(*iCurrentSourceXml);

        iSourceXml->Update(xml);

        delete oldSourceXml;
    }
}

void ServiceProductNetwork::HandleStandbyChanged()
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceProductNetwork::StandbyChangedCallback1);
    Schedule(f, NULL);
}

void ServiceProductNetwork::StandbyChangedCallback1(void*)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceProductNetwork::StandbyChangedCallback2);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}

void ServiceProductNetwork::StandbyChangedCallback2(void*)
{
    if (iSubscribed)
    {
        TBool standby;
        iService->PropertyStandby(standby);
        iStandby->Update(standby);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

ServiceProductMock::ServiceProductMock(IInjectorDevice& aDevice, const Brx& aRoom, const Brx& aName, TUint aSourceIndex, unique_ptr<SrcXml> aSourceXmlFactory, TBool aStandby,
    const Brx& aAttributes, const Brx& aManufacturerImageUri, const Brx& aManufacturerInfo, const Brx& aManufacturerName, const Brx& aManufacturerUrl, const Brx& aModelImageUri, const Brx& aModelInfo, const Brx& aModelName,
    const Brx& aModelUrl, const Brx& aProductImageUri, const Brx& aProductInfo, const Brx& aProductUrl, const Brx& aProductId, ILog& aLog)
    : ServiceProduct(aDevice, aLog)
    ,iSourceXmlFactory(move(aSourceXmlFactory))
{
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

    iRoomName->Update(Brn(*iCurrentRoom));
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
        iRoomName->Update(Brn(*iCurrentRoom));
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
    u->iValue = aIndex;
    Schedule(f, u);
}


void ServiceProductMock::SetSourceIndexCallback(void* aIndex)
{
    auto u = ((UintValue*)aIndex);
    TUint index = u->iValue;
    delete u;
    iSourceIndex->Update(index);
}


void ServiceProductMock::SetSourceIndexByName(const Brx& /*aValue*/)
{
    THROW(NotSupportedException);
}


void ServiceProductMock::SetStandby(TBool aValue)
{
    auto f = MakeFunctorGeneric(*this, &ServiceProductMock::SetStandbyCallback);

    if (aValue)
    {
        Schedule(f, (void*)1);
    }
    else
    {
        Schedule(f, (void*)0);
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


IWatchable<Brn>& ProxyProduct::RoomName()
{
    return iService.RoomName();
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

