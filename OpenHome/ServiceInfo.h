#ifndef HEADER_SERVICE_INFO
#define HEADER_SERVICE_INFO


#include <OpenHome/OhNetTypes.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/Watchable.h>
#include <OpenHome/Service.h>
#include <OpenHome/Net/Core/CpDevice.h>
#include <OpenHome/Net/Core/FunctorAsync.h>
#include <Generated/CpAvOpenhomeOrgInfo1.h>
#include <OpenHome/Job.h>
#include <OpenHome/MetaData.h>


#include <vector>
#include <memory>

namespace OpenHome
{
namespace Av
{

class IInfoDetails
{
public:
    virtual TUint BitDepth() = 0;
    virtual TUint BitRate() = 0;
    virtual const Brx& CodecName() = 0;
    virtual TUint Duration() = 0;
    virtual TBool Lossless() = 0;
    virtual TUint SampleRate() = 0;
};

/////////////////////////////////////////

class InfoDetails : public IInfoDetails
{
public:
    InfoDetails();
    InfoDetails(TUint aBitDepth, TUint aBitRate, const Brx& aCodecName, TUint aDuration, TBool aLossless, TUint aSampleRate);

    virtual TUint BitDepth();
    virtual TUint BitRate();
    virtual const Brx& CodecName();
    virtual TUint Duration();
    virtual TBool Lossless();
    virtual TUint SampleRate();

private:
    TUint iBitDepth;
    TUint iBitRate;
    Bws<100> iCodecName; // FIXME: random capacity value
    TUint iDuration;
    TBool iLossless;
    TUint iSampleRate;
};

/////////////////////////////////////////////////////////

class ServiceInfo : public Service
{
protected:
    ServiceInfo(INetwork& aNetwork, IInjectorDevice& aDevice, ILog& aLog);

public:
    void Dispose();
    IProxy* OnCreate(IDevice& aDevice);

    virtual IWatchable<IInfoDetails*>& Details();
    virtual IWatchable<IInfoMetadata*>& Metadata();
    virtual IWatchable<IInfoMetatext*>& Metatext();

protected:
    Watchable<IInfoDetails*>* iDetails;
    Watchable<IInfoMetadata*>* iMetadata;
    Watchable<IInfoMetatext*>* iMetatext;
};

/////////////////////////////////////////////

class ServiceInfoNetwork : public ServiceInfo
{
public:
    ServiceInfoNetwork(INetwork& aNetwork, IInjectorDevice& aDevice, Net::CpDevice& aCpDevice, ILog& aLog);
    ~ServiceInfoNetwork();

    virtual void Dispose();

protected:
    virtual Job* OnSubscribe();
    virtual void OnCancelSubscribe();
    virtual void OnUnsubscribe();

private:
    void HandleInitialEvent();
    void HandleDetailsChanged();
    void HandleMetadataChanged();
    void HandleMetatextChanged();

    void HandleDetailsChangedCallback(void*);
    void HandleDetailsChangedCallbackCallback(void*);
    void HandleMetadataChangedCallback(void*);
    void HandleMetadataChangedCallbackCallback(void*);
    void HandleMetatextChangedCallback(void*);
    void HandleMetatextChangedCallbackCallback(void*);

private:
    Net::CpDevice& iCpDevice;
    JobDone* iSubscribedSource;
    Net::CpProxyAvOpenhomeOrgInfo1* iService;
};

////////////////////////////////////////////////////////

class IProxyInfo : public IProxy
{
public:
    virtual IWatchable<IInfoDetails*>& Details() = 0;
    virtual IWatchable<IInfoMetadata*>& Metadata() = 0;
    virtual IWatchable<IInfoMetatext*>& Metatext() = 0;
};

////////////////////////////////////////////////////////

class ProxyInfo : public IProxyInfo, public INonCopyable
{
public:
    ProxyInfo(ServiceInfo& aService, IDevice& aDevice);

    virtual IWatchable<IInfoDetails*>& Details();
    virtual IWatchable<IInfoMetadata*>& Metadata();
    virtual IWatchable<IInfoMetatext*>& Metatext();


    // IProxy
    virtual IDevice& Device();

    // IDisposable
    virtual void Dispose();

protected:
    ServiceInfo& iService;

private:
    IDevice& iDevice;
};


} // Av
} // OpenHome

#endif
