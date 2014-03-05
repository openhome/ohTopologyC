#include <OpenHome/OhNetTypes.h>
#include<OpenHome/DisposeHandler.h>
#include <OpenHome/Private/Thread.h>


using namespace OpenHome;
using namespace OpenHome::Av;



DisposeHandler::DisposeHandler()
    :iCount(0)
    ,iDisposed(false)
    ,iMutex("DISP")
{
}


TBool DisposeHandler::WhenNotDisposed(Functor aAction)
{
    AutoMutex mutex(iMutex);
    if (!iDisposed)
    {
        aAction();
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


void DisposeLock::Dispose()
{
    iHandler.Leave();
}

