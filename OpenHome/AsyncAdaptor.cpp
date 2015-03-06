#include <OpenHome/OhNetTypes.h>
#include <OpenHome/AsyncAdaptor.h>
#include <vector>


using namespace OpenHome;
using namespace OpenHome::Topology;


AsyncAdaptor::AsyncAdaptor(AsyncAdaptorManager& aMan)
    :iMan(aMan)
    ,iCombinedArgs(new AsyncCbArg())
{
    Reset();
}


AsyncAdaptor::~AsyncAdaptor()
{
    delete iCombinedArgs;
}


void AsyncAdaptor::SetCallback(FunctorGeneric<AsyncCbArg*> aCallback, void* aArg)
{
    iCallback = aCallback;
    iCombinedArgs->iArg = aArg;
}


Net::FunctorAsync AsyncAdaptor::AsyncCb()
{
    return (MakeFunctorAsync(*this, &AsyncAdaptor::AsyncComplete));
}


void AsyncAdaptor::AsyncComplete(Net::IAsync& aAsync)
{
    iCombinedArgs->iAsync = &aAsync;
    iCallback(iCombinedArgs);
    Reset();
    iMan.Release(*this); // add "this" back into fifo of available jobs
}


void AsyncAdaptor::Reset()
{
    iCallback = FunctorGeneric<AsyncCbArg*>();  // reset to null functor
    iCombinedArgs->iAsync = NULL;
    iCombinedArgs->iArg = NULL;
}

/////////////////////////////////////////////////////

AsyncAdaptorManager::AsyncAdaptorManager()
    :iAsyncAdaptors(kAdaptorCount)
{
    for (TUint i=0; i<kAdaptorCount; i++)
    {
        iAsyncAdaptors.Write(new AsyncAdaptor(*this));
    }
}


AsyncAdaptorManager::~AsyncAdaptorManager()
{
    for (TUint i=0; i<kAdaptorCount; i++)
    {
        delete (iAsyncAdaptors.Read());
    }
}


AsyncAdaptor& AsyncAdaptorManager::GetAdaptor()
{
    return(*iAsyncAdaptors.Read());
}


void AsyncAdaptorManager::Release(AsyncAdaptor& aAsyncAdaptor)
{
    return(iAsyncAdaptors.Write(&aAsyncAdaptor));
}



