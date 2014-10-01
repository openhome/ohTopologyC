#ifndef HEADER_SERVICE_PRODUCT
#define HEADER_SERVICE_PRODUCT


#include <OpenHome/OhNetTypes.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/Watchable.h>
#include <OpenHome/Service.h>
#include <OpenHome/Net/Core/CpDevice.h>
#include <OpenHome/Net/Core/FunctorAsync.h>
#include <Generated/CpAvOpenhomeOrgProduct1.h>
#include <OpenHome/Job.h>


#include <vector>
#include <memory>



namespace OpenHome
{

namespace Av
{

class Source
{
public:
    static const TUint kMaxNameBytes = 50;
    static const TUint kMaxTypeBytes = 50;

public:
    Source(const Brx& aName, const Brx& aType, TBool aVisible);
    virtual Brn Name();
    virtual Brn Type();
    virtual TBool Visible();
    virtual void SetName(const Brx& aName);
    virtual void SetVisible(TBool aValue);

private:
    Bws<kMaxNameBytes> iName;
    Bws<kMaxTypeBytes> iType;
    TBool iVisible;
};

////////////////////////////////////////////////////////////

class SrcXml
{
public:
    static const TUint kMaxXmlBytes = 5000;
public:
    SrcXml();

    virtual void Add(std::unique_ptr<Source>);
    virtual const Brx& ToString();
    virtual void UpdateName(TUint aIndex, const Brx& aName);
    virtual void UpdateVisible(TUint aIndex, TBool aVisible);

private:
    void CreateSourceXml();

private:
    //std::vector<Source*>* iSources;
    std::vector<std::unique_ptr<Source>> iSources;
    Bws<kMaxXmlBytes> iSourceXml;
};

/////////////////////////////////////////////////////////////////

class IProxyProduct : public IProxy
{
public:
    virtual IWatchable<Brn>& Room() = 0;
    virtual IWatchable<Brn>& Name() = 0;
    virtual IWatchable<TUint>& SourceIndex() = 0;
    virtual IWatchable<Brn>& SourceXml() = 0;
    virtual IWatchable<TBool>& Standby() = 0;

    virtual void SetSourceIndex(TUint aValue) = 0;
    virtual void SetSourceIndexByName(const Brx& aValue) = 0;
    virtual void SetStandby(TBool aValue) = 0;

    virtual Brn Attributes() = 0;
    virtual Brn ManufacturerImageUri() = 0;
    virtual Brn ManufacturerInfo() = 0;
    virtual Brn ManufacturerName() = 0;
    virtual Brn ManufacturerUrl() = 0;
    virtual Brn ModelImageUri() = 0;
    virtual Brn ModelInfo() = 0;
    virtual Brn ModelName() = 0;
    virtual Brn ModelUrl() = 0;
    virtual Brn ProductImageUri() = 0;
    virtual Brn ProductInfo() = 0;
    virtual Brn ProductUrl() = 0;
    virtual Brn ProductId() = 0;
};


/////////////////////////////////////////////////////////////////

class ServiceProduct : public Service
{
protected:
    ServiceProduct(INetwork& aNetwork, IInjectorDevice& aDevice, ILog& aLog);

public:
    virtual void Dispose();
    virtual IProxy* OnCreate(IDevice* aDevice);

    // IServiceProduct methods
    virtual IWatchable<Brn>& Room();
    virtual IWatchable<Brn>& Name();
    virtual IWatchable<TUint>& SourceIndex();
    virtual IWatchable<Brn>& SourceXml();
    virtual IWatchable<TBool>& Standby();

    // abstract
    virtual void SetSourceIndex(TUint aValue) = 0;
    virtual void SetSourceIndexByName(const Brx& aValue) = 0;
    virtual void SetStandby(TBool aValue) = 0;


    // IProduct methods
    virtual Brn Attributes();
    virtual Brn ManufacturerImageUri();
    virtual Brn ManufacturerInfo();
    virtual Brn ManufacturerName();
    virtual Brn ManufacturerUrl();
    virtual Brn ModelImageUri();
    virtual Brn ModelInfo();
    virtual Brn ModelName();
    virtual Brn ModelUrl();
    virtual Brn ProductImageUri();
    virtual Brn ProductInfo();
    virtual Brn ProductUrl();
    virtual Brn ProductId();


protected:
    Bws<100> iAttributes;
    Bws<100> iManufacturerImageUri;
    Bws<100> iManufacturerInfo;
    Bws<100> iManufacturerName;
    Bws<100> iManufacturerUrl;
    Bws<100> iModelImageUri;
    Bws<100> iModelInfo;
    Bws<100> iModelName;
    Bws<100> iModelUrl;
    Bws<100> iProductImageUri;
    Bws<100> iProductInfo;
    Bws<100> iProductUrl;
    Bws<100> iProductId;
    Watchable<Brn>* iRoom;
    Watchable<Brn>* iName;
    Watchable<TUint>* iSourceIndex;
    Watchable<Brn>* iSourceXml;
    Watchable<TBool>* iStandby;

    Bws<20>* iCurrentRoom;
    Bws<50>* iCurrentName;
};

/////////////////////////////////////////////////////////////////

class ServiceProductMock : public ServiceProduct
{
public:
    ServiceProductMock(INetwork& aNetwork, IInjectorDevice& aDevice, const Brx& aRoom, const Brx& aName, TUint aSourceIndex, std::unique_ptr<SrcXml> aSourceXmlFactory, TBool aStandby,
        const Brx& aAttributes, const Brx& aManufacturerImageUri, const Brx& aManufacturerInfo, const Brx& aManufacturerName, const Brx& aManufacturerUrl, const Brx& aModelImageUri,
        const Brx& aModelInfo, const Brx& aModelName, const Brx& aModelUrl, const Brx& aProductImageUri, const Brx& aProductInfo, const Brx& aProductUrl, const Brx& aProductId, ILog& aLog);

    virtual void Execute(ICommandTokens& aValue);
    virtual void SetSourceIndex(TUint aValue);
    virtual void SetSourceIndexByName(const Brx& aValue);
    virtual void SetStandby(TBool aValue);

private:
    virtual void SetSourceIndexCallback(void* aValue);
    //virtual void SetStandbyCallback(void* aValue);


private:
    std::unique_ptr<SrcXml> iSourceXmlFactory;
};


/////////////////////////////////////////////////////////////////

class ProxyProduct : public IProxyProduct, public INonCopyable//, public Proxy<ServiceProduct>
{
public:
    ProxyProduct(ServiceProduct& aService, IDevice& aDevice);

    // IProduct
    virtual IWatchable<Brn>& Room();
    virtual IWatchable<Brn>& Name();
    virtual IWatchable<TUint>& SourceIndex();
    virtual IWatchable<Brn>& SourceXml();
    virtual IWatchable<TBool>& Standby();

    virtual void SetSourceIndex(TUint aValue);
    virtual void SetSourceIndexByName(const Brx& aValue);
    virtual void SetStandby(TBool aValue);

    virtual Brn Attributes();
    virtual Brn ManufacturerImageUri();
    virtual Brn ManufacturerInfo();
    virtual Brn ManufacturerName();
    virtual Brn ManufacturerUrl();
    virtual Brn ModelImageUri();
    virtual Brn ModelInfo();
    virtual Brn ModelName();
    virtual Brn ModelUrl();
    virtual Brn ProductImageUri();
    virtual Brn ProductInfo();
    virtual Brn ProductUrl();
    virtual Brn ProductId();


    // IProxy
    virtual IDevice& Device();

    // IDisposable
    virtual void Dispose();

//protected:
//    Proxy(T aService, IDevice& aDevice);

protected:
    ServiceProduct& iService;

private:
    IDevice& iDevice;
};

////////////////////////////////////////////////////////////

class ServiceProductNetwork : public ServiceProduct
{
public:
    ServiceProductNetwork(INetwork& aNetwork, IInjectorDevice& aDevice, Net::CpDevice& aCpDevice, ILog& aLog);

    virtual void Dispose();
    virtual void SetSourceIndex(TUint aValue);
    virtual void SetSourceIndexByName(const Brx& aValue);
    virtual void SetStandby(TBool aValue);

protected:
    virtual Job* OnSubscribe();
    virtual void OnCancelSubscribe();
    virtual void OnUnsubscribe();

private:
    void HandleInitialEvent();
    void HandleRoomChanged();
    void HandleNameChanged();
    void HandleSourceIndexChanged();
    void HandleSourceXmlChanged();
    void HandleStandbyChanged();

    void BeginSetSourceIndexCallback(Net::IAsync& aValue);
    void BeginSetStandbyCallback(Net::IAsync& aAsync);
    void BeginSetSourceIndexByNameCallback(Net::IAsync& aAsync);
    void DoNothing(void* aObj);

private:
    Net::CpDevice& iCpDevice;
    JobDone* iSubscribedSource;
    Net::CpProxyAvOpenhomeOrgProduct1* iService;
};




} // Av

} // OpenHome


#endif //HEADER_SERVICE_PRODUCT
