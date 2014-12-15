#ifndef HEADER_OHTOPC_ASYNC_ADAPTOR
#define HEADER_OHTOPC_ASYNC_ADAPTOR

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/Private/Thread.h>
#include <OpenHome/Private/Fifo.h>
#include <OpenHome/Net/Core/FunctorAsync.h>
#include <vector>


namespace OpenHome
{
namespace Av
{

struct AsyncCbArg
{
    Net::IAsync* iAsync;
    void* iArg;
};

///////////////////////////////////////////////////////////////////////////

class AsyncAdaptor;

class AsyncAdaptorManager
{
private:
    static const TUint kAdaptorCount = 10;

public:
    AsyncAdaptorManager();
    ~AsyncAdaptorManager();
    AsyncAdaptor& GetAdaptor();
    void ReleaseJob(AsyncAdaptor& aAdaptor);

private:
    Fifo<AsyncAdaptor*> iAsyncAdaptors;
};

///////////////////////////////////////////////////////////////////////////

class AsyncAdaptor : public INonCopyable
{
    friend class AsyncAdaptorManager;

private:
    AsyncAdaptor(AsyncAdaptorManager& aMan);
    ~AsyncAdaptor();

public:
    void SetCallback(FunctorGeneric<AsyncCbArg*> aCallback, void* aArg);
    Net::FunctorAsync AsyncCb();

private:
    void AsyncComplete(Net::IAsync& aAsync);
    void Reset();

private:
    AsyncAdaptorManager& iMan;
    FunctorGeneric<AsyncCbArg*> iCallback;
    AsyncCbArg* iCombinedArgs;
};

} // Av
} // OpenHome

#endif // HEADER_OHTOPC_ASYNC_ADAPTOR
