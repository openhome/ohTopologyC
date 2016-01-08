#pragma once

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/Watchable.h>
#include <OpenHome/Service.h>
#include <OpenHome/Net/Core/CpDevice.h>
#include <OpenHome/Net/Core/FunctorAsync.h>
#include <OpenHome/MetaData.h>

#include <vector>
#include <memory>

namespace OpenHome
{
namespace Net
{
    class CpProxyAvOpenhomeOrgInfo1;
}

namespace Topology
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
    virtual ~IInfoDetails(){}
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
    ServiceInfo(IInjectorDevice& aDevice, ILog& aLog);
    ~ServiceInfo();

public:
    void Dispose();
    IProxy* OnCreate(IDevice& aDevice);

    virtual IWatchable<IInfoDetails*>& Details();
    virtual IWatchable<IInfoMetadata*>& Metadata();
    virtual IWatchable<IInfoMetatext*>& Metatext();

protected:
    IInfoDetails* iCurrentDetails;
    IInfoMetadata* iCurrentMetadata;
    IInfoMetatext* iCurrentMetatext;
    Watchable<IInfoDetails*>* iDetails;
    Watchable<IInfoMetadata*>* iMetadata;
    Watchable<IInfoMetatext*>* iMetatext;
};

/////////////////////////////////////////////

class ServiceInfoNetwork : public ServiceInfo
{
public:
    ServiceInfoNetwork(IInjectorDevice& aDevice, Net::CpProxyAvOpenhomeOrgInfo1* aService, ILog& aLog);
    ~ServiceInfoNetwork();

    virtual void Dispose();

protected:
    virtual TBool OnSubscribe();
    virtual void OnCancelSubscribe();
    virtual void OnUnsubscribe();

private:
    void HandleInitialEvent();
    void HandleDetailsChanged();
    void HandleMetadataChanged();
    void HandleMetatextChanged();
    void HandlePendingDetailsChanged();

    void HandleDetailsChangedCallback1(void*);
    void HandleDetailsChangedCallback2(void*);
    void HandleMetadataChangedCallback1(void*);
    void HandleMetadataChangedCallback2(void*);
    void HandleMetatextChangedCallback1(void*);
    void HandleMetatextChangedCallback2(void*);

private:
    Net::CpProxyAvOpenhomeOrgInfo1* iService;
    TBool iSubscribed;
    TBool iDetailsChanged;
};

////////////////////////////////////////////////////////

class ServiceInfoMock : public ServiceInfo
{
public:
    ServiceInfoMock(IInjectorDevice& aDevice, IInfoDetails* aDetails, IInfoMetadata* aMetadata, IInfoMetatext* aMetatext, ILog& aLog);
    ~ServiceInfoMock();
public:
    void Execute(ICommandTokens& aValue) override; //from IMockable
private:
    IInfoDetails* iCurrentDetails;
    IInfoMetadata* iCurrentMetadata;
    IInfoMetatext* iCurrentMetatext;
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


} // Topology
} // OpenHome
