#ifndef HEADER_OHTOPC_JOB
#define HEADER_OHTOPC_JOB

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

class CallbackHandler;


class Job
{
public:
    Job(FunctorGeneric<void*> aAction, void* aObj); // can't be cancelled
    Job();

    void Start();
    void Wait();
    void Cancel();
    TBool IsCancelled();

    Job* ContinueWith(FunctorGeneric<void*> aAction, void* aObj);
    static Job* StartNew(FunctorGeneric<void*> aAction, void* aObj);

private:
    void Run();
    void Continue();

private:
    FunctorGeneric<void*> iAction;
    void* iActionArg;
    //Functor iCancellation;
    ThreadFunctor* iThread;
    Job* iJobContinue;
    Semaphore iSem;
    mutable Mutex iMutex;
    TBool iStarted;
    TBool iCompleted;
    TBool iCancelled;
};


////////////////////////////////////////////////////////////////////////////

class JobDone
{
public:
    JobDone();

    void SetResult(TBool aResult);
    void Cancel();
    Job* GetJob();  //  shouldn't this return a ref?

private:
    Job* iJob;
};

///////////////////////////////////////////////////////////////////////////

struct AsyncCbArg
{
    Net::IAsync* iAsync;
    void* iArg;
};

///////////////////////////////////////////////////////////////////////////

class Job2 : private Thread
{
public:
    Job2();
    void SetCallback(FunctorGeneric<AsyncCbArg*> aAction, void* aArg);
    void CallbackComplete();
    void Run();
    Net::FunctorAsync AsyncCb();
    void AsyncComplete(Net::IAsync& aAsync);
    void Cancel();

private:
    FunctorGeneric<AsyncCbArg*> iCallback;
    void* iArg;
    AsyncCbArg* iCbArg;
    TBool iCancelled;
    mutable Mutex iMutex;
};


//////////////////////////////////////////////////////////////


/**
 * Used to invoke an Action with particular Arguments on a Service
 *
 * Control point side clients must not create new Invocation instances or delete instances
 * returned by Service::GetInvocation() or passed to their invocation completed callback.
 *
 * Normal use will be
 * - create (possibly allocating) any data required in the callback
 * - use Service::GetInvocation() to retrieve a pointer from an existing pool
 *      the invocation will have Service and Action name set automatically
 *      the invocation will have the client-specified callback and data set automatically
 * - create one Argument-derived class for each InputParameter on the action
 *      these arguments must have their values set (i.e. use the c'tor taking two params)
 * - call AddInput() for each of these arguments
 * - create one Argument-derived class for each OutputParameter on the action
 *      these arguments should not have their values set (i.e. use the c'tor taking one param)
 * - call AddOutput() for each of these arguments
 * - call IInvocable::QueueInvocation()
 * ....
 * - The invocation completed callback will run.  Invocation::Error() will be
 *      true if the invocation failed.  If no error occurred, each member of
 *      OutputArguments() will be set.
 */

/*
class DllExportClass Invocation : public Async
{
public:
//
//    Intended for internal use only
//
    void SignalCompleted();

//
//    Returns true if the invocation failed; false if it successed
//
//    The Completed() callback will still be called if an invocation fails.  It should
//    not attempt to access OutputArguments() if it failed but must free any user data.
//
    DllExport TBool Error() const;

//
//    Returns true if the invocation failed; false if it successed
//
//    The Completed() callback will still be called if an invocation fails.  It should
//    not attempt to access OutputArguments() if it failed but must free any user data.
//
//    All args are output only and will only be set if the function returns true.
//
    DllExport TBool Error(Error::ELevel& aLevel, TUint& aCode, const TChar*& aDescription) const;

//
//    Intended for internal use only
//
    TBool Interrupt() const;

//
//    Intended for internal use only
//
    void Set(CpiService& aService, const OpenHome::Net::Action& aAction, CpiDevice& aDevice, FunctorAsync& aFunctor);

//
//    Add an input argument, passing ownership of aArgument
//
//    aArgument should have a value set (i.e. used the c'tor taking two params)
//
    DllExport void AddInput(Argument* aArgument);

//
//  Add an output argument, passing ownership of aArgument
//
//  aArgument should not have a value set (i.e. used the c'tor taking one param)
//
    DllExport void AddOutput(Argument* aArgument);

//
//  Set error details on this invocation.
//
//  The Completed() callback will assume that the invocation failed.
//  Intended for use by CpiDevice-derived classes.
//
    void SetError(Error::ELevel aLevel, TUint aCode, const Brx& aDescription);

//
//    Set the handler for interrupting this invocation
//
    void SetInterruptHandler(IInterruptHandler* aHandler);

//
//    Signal that this invocation should be interrupted if its Action is a member of aService
//    Intended for internal use only
//
    void Interrupt(const Service& aService);

    const OpenHome::Net::ServiceType& ServiceType() const;
    DllExport const OpenHome::Net::Action& Action() const;
    CpiDevice& Device();

//
//    Log details of this invocation + error status
//
    void Output(IAsyncOutput& aConsole);

    typedef std::vector<Argument*> VectorArguments;
//
//    Return a vector of input arguments (one per input parameter for the Action)
//
//    This can be called by any client but is assumed to be of interest to devices only
//
    const VectorArguments& InputArguments() const;

//
//    Return a vector of output arguments (one per output parameter for the Action)
//
//    This is assumed to be of interest to the device and the Completed() callback
//
    DllExport VectorArguments& OutputArguments();

    void SetInvoker(IInvocable& aInvocable);
    IInvocable& Invoker();
private:
    Invocation(CpStack& aCpStack, Fifo<OpenHome::Net::Invocation*>& aFree);
    Invocation& operator=(const Invocation& aInvocation);
    ~Invocation();
    void Clear();
    static void OutputArgument(IAsyncOutput& aConsole, const TChar* aKey, const Argument& aArgument);
    virtual TUint Type() const;
private:
    CpStack& iCpStack;
    OpenHome::Mutex iLock;
    Fifo<OpenHome::Net::Invocation*>& iFree;
    CpiService* iService;
    const OpenHome::Net::Action* iAction;
    CpiDevice* iDevice;
    FunctorAsync iFunctor;
    TUint iSequenceNumber;
    OpenHome::Net::Error iError;
    TBool iCompleted;
    VectorArguments iInput;
    VectorArguments iOutput;
    IInterruptHandler* iInterruptHandler;
    IInvocable* iInvoker;
private:
    friend class InvocationManager;
};
*/

///
// Dedicated thread which processes action invocations
//
// Intended for internal use only
//

/*
class Invoker : public Thread
{
public:
    Invoker(const TChar* aName, Fifo<Invoker*>& aFree);
    ~Invoker();

//
//    Signal that aInvocation should be invoked
//
    void Invoke(Invocation* aInvocation);

//
//    Interrupt any current invocation if its action is a member of aService
//
    void Interrupt(const Service& aService);
private:
    void SetError(Error::ELevel aLevel, TUint aCode, const Brx& aDescription, const TChar* aLogStr);
    void Run();
private:
    Fifo<Invoker*>& iFree;
    Invocation* iInvocation;
    OpenHome::Mutex iLock;
};
*/

/**
 * Singleton which manages the pools of Invocation and Invoker instances
 */

/*
class InvocationManager : public Thread
{
    friend class CpiService;
public:
    InvocationManager(CpStack& aCpStack);
    ~InvocationManager();
    void Invoke(OpenHome::Net::Invocation* aInvocation);
    void Interrupt(const Service& aService);
private:
    OpenHome::Net::Invocation* Invocation();
    void Run();
private:
    CpStack& iCpStack;
    OpenHome::Mutex iLock;
    Fifo<OpenHome::Net::Invocation*> iFreeInvocations;
    Fifo<OpenHome::Net::Invocation*> iWaitingInvocations;
    Fifo<Invoker*> iFreeInvokers;
    Invoker** iInvokers;
    TBool iActive;
};
*/


} // Av
} // OpenHome

#endif // HEADER_OHTOPC_JOB
