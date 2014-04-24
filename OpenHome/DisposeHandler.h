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
    TBool WhenNotDisposed(Functor aAction);
    TBool WhenNotDisposed(FunctorGeneric<void*> aCallback, void* aObj);
    void Enter();
    void Leave();

    // IDisposable
    virtual void Dispose();

private:
    TInt iCount;
    TBool iDisposed;
    Mutex iMutex;
};

////////////////////////////////////////////////

class DisposeLock : public INonCopyable
{
public:
    DisposeLock(DisposeHandler& aHandler);
    ~DisposeLock();

private:
    DisposeHandler& iHandler;
};




} // Av
} // OpenHome

#endif // HEADER_OHTOPOLOGYC_DISPOSE_HANDLER
