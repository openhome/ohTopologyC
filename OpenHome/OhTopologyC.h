#ifndef HEADER_OHTOPOLOGYC
#define HEADER_OHTOPOLOGYC

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/Command.h>


using namespace Linn;

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

////////////////////////////////////////////////////////////////////

typedef FunctorGeneric<void*> Action;

////////////////////////////////////////////////////////////////////

template<class T1, class T2>
class ArgsTwo
{
public:
    ArgsTwo(T1 aArg1, T2 aArg2) :iArg1(aArg1), iArg2(aArg2) {}
    T1 Arg1() const {return(iArg1);}
    T2 Arg2() const {return(iArg2);}
private:
    T1 iArg1;
    T2 iArg2;
};


////////////////////////////////////////////////////////////////////

class IExceptionReporter
{
public:
    virtual void Report(Exception& aException) = 0;
    virtual void Report(std::exception& aException) = 0;
    virtual ~IExceptionReporter() {}
};

////////////////////////////////////////////////////////////////////

class IJoinable
{
public:
    virtual void Join(Action aAction) = 0;
    virtual void Unjoin(Action aAction) = 0;
    virtual ~IJoinable() {}
};

////////////////////////////////////////////////////////////////////

class IMockable
{
public:
    virtual void Execute(ICommandTokens& aTokens) = 0;
    virtual ~IMockable() {}
};

////////////////////////////////////////////////////////////////////

class IDisposable
{
public:
    virtual void Dispose() = 0;
    virtual ~IDisposable() {}
};

////////////////////////////////////////////////////////////////////

class ILog
{
public:
    virtual ~ILog() {}
};


/*
class ServiceNotFoundException : public Exception
{
public:
    //ServiceNotFoundException();
    ServiceNotFoundException(const Brx& aMessage);
    ServiceNotFoundException(const Brx& aMessage, Exception& aInnerException);
};
*/

} // OpenHome

#endif // HEADER_OHTOPOLOGYC
