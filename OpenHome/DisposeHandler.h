#ifndef HEADER_OHTOPOLOGYC_DISPOSE_HANDLER
#define HEADER_OHTOPOLOGYC_DISPOSE_HANDLER

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Functor.h>
#include <OpenHome/Private/Thread.h>
#include <OpenHome/OhTopologyC.h>


namespace OpenHome
{
namespace Av
{


class DisposeHandler : IDisposable
{
public:
    DisposeHandler();
    //IDisposable* Lock();
    TBool WhenNotDisposed(Functor aAction);
    // IDisposable
    void Dispose();

    void Enter();
    void Leave();


private:
    TInt iCount;
    TBool iDisposed;
    Mutex iMutex;
};

////////////////////////////////////////////////

class DisposeLock : public IDisposable
{
public:
    DisposeLock(DisposeHandler& aHandler);

    // IDisposable
    void Dispose();

private:
    DisposeHandler& iHandler;
};




} // Av
} // OpenHome

#endif // HEADER_OHTOPOLOGYC_DISPOSE_HANDLER
