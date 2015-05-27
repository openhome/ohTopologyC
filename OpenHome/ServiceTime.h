#ifndef HEADER_OHTOPC_SERVICE_TIME
#define HEADER_OHTOPC_SERVICE_TIME

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/Watchable.h>
#include <OpenHome/Service.h>
#include <OpenHome/Net/Core/CpDevice.h>
#include <OpenHome/Net/Core/FunctorAsync.h>


#include <vector>
#include <memory>

namespace OpenHome
{

namespace Net
{
class CpProxyAvOpenhomeOrgTime1;
}

namespace Topology
{

class ServiceTime : public Service
{
protected:
    ServiceTime(IInjectorDevice& aDevice, ILog& aLog);

public:
    void Dispose();
    IProxy* OnCreate(IDevice& aDevice);
    IWatchable<TUint>& Duration();

public:
    IWatchable<TUint>& Seconds();

protected :
    Watchable<TUint>* iDuration;
    Watchable<TUint>* iSeconds;
};

/////////////////////////////////////////////////

class ServiceTimeNetwork : public ServiceTime
{
public:
    ServiceTimeNetwork(IInjectorDevice& aDevice, Net::CpProxyAvOpenhomeOrgTime1* aService, ILog& aLog);
    void Dispose();

protected:
    TBool OnSubscribe();
    void OnCancelSubscribe();
    void OnUnsubscribe();

private:
    void HandleDurationChanged();
    void HandleSecondsChanged();
    void HandleInitialEvent();

    void DurationChangedCallback1(void*);
    void DurationChangedCallback2(void*);
    void SecondsChangedCallback1(void*);
    void SecondsChangedCallback2(void*);


private:
    Net::CpProxyAvOpenhomeOrgTime1* iService;
    TBool iSubscribed;
    //TBool iSubscribedSource;
};


/*
    class ServiceTimeMock : ServiceTime, IMockable
    {
        public ServiceTimeMock(IInjectorDevice aDevice, TUint aSeconds, TUint aDuration, ILog aLog)
            : base(aDevice, aLog)
        {
            iDuration.Update(aDuration);
            iSeconds.Update(aSeconds);
        }

        public override void Execute(IEnumerable<string> aValue)
        {
            string command = aValue.First().ToLowerInvariant();
            if (command == "duration")
            {
                IEnumerable<string> value = aValue.Skip(1);
                iDuration.Update(TUint.Parse(value.First()));
            }
            else if (command == "seconds")
            {
                IEnumerable<string> value = aValue.Skip(1);
                iSeconds.Update(TUint.Parse(value.First()));
            }
            else
            {
                throw new NotSupportedException();
            }
        }
    }
*/


class IProxyTime : public IProxy
{
public:
    IWatchable<TUint>& Duration(); // { get; }
    IWatchable<TUint>& Seconds(); // { get; }
};

//////////////////////////////////////

class ProxyTime : public IProxyTime, public INonCopyable
{
public:
    ProxyTime(ServiceTime& aService, IDevice& aDevice);
    IWatchable<TUint>& Duration();
    IWatchable<TUint>& Seconds();

    // IProxy
    virtual IDevice& Device();

    // IDisposable
    virtual void Dispose();

protected:
    ServiceTime& iService;

private:
    IDevice& iDevice;
};


} // Topology
} // OpeNHome

#endif // HEADER_OHTOPC_SERVICE_TIME

