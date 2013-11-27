#ifndef HEADER_WATCHABLE_THREAD
#define HEADER_WATCHABLE_THREAD

#include <OpenHome/OhNetTypes.h>
//#include <OpenHome/Functor.h>
#include <OpenHome/Private/Thread.h>
#include <OpenHome/Private/Fifo.h>
#include <OpenHome/Exception.h>

#include <stddef.h>


namespace OpenHome
{


template<class Type>
class FunctorGeneric
{
public:
    void operator()(Type aType) const { iThunk(*this, aType); }
    typedef TAny (FunctorGeneric::*MemberFunction)();
    static const TUint kFudgeFactor = 2;

    FunctorGeneric() : iObject(0) {}

    TByte iCallbackMember[kFudgeFactor * sizeof(MemberFunction)];
    TAny* iObject;

protected:
    typedef void (*Thunk)(const FunctorGeneric&, Type);
    FunctorGeneric(Thunk aT, const TAny* aObject, const TAny* aCallback, TUint aBytes)
        : iThunk(aT)
    {
        iObject = (TAny*)aObject;
        memcpy(iCallbackMember, aCallback, aBytes);
    }

private:
    Thunk iThunk;
};

/////////////////////////////////////////////////////////////////

template<class Type, class Object, class MemFunc>
class MemberTranslatorGeneric : public FunctorGeneric<Type>
{
public:
    MemberTranslatorGeneric(Object& aC, const MemFunc& aM) :
        FunctorGeneric<Type>(Thunk,&aC,&aM,sizeof(MemFunc)) {}
    static void Thunk(const FunctorGeneric<Type>& aFb, Type aType)
    {
        Object* object = (Object*)aFb.iObject;
        MemFunc& memFunc(*(MemFunc*)(TAny*)(aFb.iCallbackMember));
        (object->*memFunc)(aType);
    }
};


////////////////////////////////////////////////////////////////////

/**
 * Create a FunctorGeneric around a non-const C++ member function
 */
template<class Type, class Object, class CallType>
inline MemberTranslatorGeneric<Type,Object,void (CallType::*)(Type)>
MakeFunctorGeneric(Object& aC, void(CallType::* const &aF)(Type))
    {
    typedef void(CallType::*MemFunc)(Type);
    return MemberTranslatorGeneric<Type,Object,MemFunc>(aC,aF);
    }



/////////////////////////////////////////////////////////

class IExceptionReporter
{
public:
    virtual ~IExceptionReporter() {}
    virtual void Report(Exception& aException) = 0;
    virtual void Report(std::exception& aException) = 0;
};

/////////////////////////////////////////////////////////

class IWatchableThread
{
public:
    virtual ~IWatchableThread() {};

    virtual void Assert() = 0;
    virtual void Schedule(FunctorGeneric<void*> aCallback, void* aObj) = 0;
    virtual void Execute(FunctorGeneric<void*> aCallback, void* aObj) = 0;
    virtual TBool IsWatchableThread() = 0;
};

///////////////////////////////////////////////


class SignalledCallback
{
public:
    SignalledCallback();
    void Set(FunctorGeneric<void*> aFunctor, void* aObj, Semaphore& aSem);
    void Set(FunctorGeneric<void*> aFunctor, void* aObj);
    void Callback();

private:
    FunctorGeneric<void*> iFunctor;
    void* iObj;
    Semaphore* iSem;
};

/////////////////////////////////////////////

class AutoSem
{
public:
    AutoSem(Semaphore* aSem);
    ~AutoSem();
private:
    Semaphore* iSem;
};

/////////////////////////////////////////////

class WatchableThread : public IWatchableThread
{
public:
    static const TUint kMaxFifoEntries = 10;

public:
    WatchableThread(IExceptionReporter& aReporter);
    virtual ~WatchableThread();
    virtual void Assert();
    virtual void Schedule(FunctorGeneric<void*> aCallback, void* aObj);
    virtual void Execute(FunctorGeneric<void*> aCallback, void* aObj);
    virtual TBool IsWatchableThread();

private:
    void Run();
    void Shutdown(void*);

private:
    IExceptionReporter& iExceptionReporter;
    Fifo<SignalledCallback*> iFree;
    Fifo<SignalledCallback*> iScheduled;
    ThreadFunctor* iThread;
};

//////////////////////////////////////////////




}




#endif //HEADER_WATCHABLE_THREAD
