#include <OpenHome/OhNetTypes.h>
#include<OpenHome/DisposeHandler.h>
#include <OpenHome/Private/Thread.h>


using namespace OpenHome;
using namespace OpenHome::Topology;



DisposeHandler::DisposeHandler()
    :iCount(0)
    ,iDisposed(false)
    ,iMutex("DISP")
{
}


TBool DisposeHandler::WhenNotDisposed(Functor aCallback)
{
    AutoMutex mutex(iMutex);
    if (!iDisposed)
    {
        aCallback();
        return (true);
    }

    return (false);
}


TBool DisposeHandler::WhenNotDisposed(FunctorGeneric<void*> aCallback, void* aObj)
{
    AutoMutex mutex(iMutex);
    if (!iDisposed)
    {
        aCallback(aObj);
        return (true);
    }

    return (false);
}



void DisposeHandler::Enter()
{
    AutoMutex mutex(iMutex);
    ASSERT(!iDisposed);
    iCount++;
}


void DisposeHandler::Leave()
{
    AutoMutex mutex(iMutex);
    iCount--;
}


void DisposeHandler::Dispose()
{
    AutoMutex mutex(iMutex);
    ASSERT(!iDisposed);
    iDisposed = true;
    ASSERT(iCount == 0);
}


///////////////////////////////////////////////////////////

DisposeLock::DisposeLock(DisposeHandler& aHandler)
    :iHandler(aHandler)
{
    iHandler.Enter();
}


DisposeLock::~DisposeLock()
{
    iHandler.Leave();
}

