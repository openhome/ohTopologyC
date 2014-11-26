#ifndef HEADER_OHTOPOLOGYC
#define HEADER_OHTOPOLOGYC

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/Command.h>
#include <OpenHome/Functor.h>


EXCEPTION(NotSupportedException);
EXCEPTION(NotImplementedException);

using namespace Linn;

namespace OpenHome
{


enum EStandby
{
    eOn,
    eMixed,
    eOff
};

///////////////////////

enum EServiceType
{
    eProxyProduct,
    eProxyInfo,
    eProxyTime,
    eProxySender,
    eProxyVolume,
    eProxyPlaylist,
    eProxyRadio,
    eProxyReceiver,
};

////////////////////////////////////////////////////////////////////

template<class T1, class T2>
class ArgsTwo
{
public:
    ArgsTwo(T1 aArg1, T2 aArg2) : iArg1(aArg1), iArg2(aArg2) {}
    T1 Arg1() const {return(iArg1);}
    T2 Arg2() const {return(iArg2);}
private:
    T1 iArg1;
    T2 iArg2;
};

////////////////////////////////////////////////////////////////////

struct UintValue
{
   TUint iUintValue;
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
    virtual void Join(Functor aAction) = 0;
    virtual void Unjoin(Functor aAction) = 0;
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
    virtual void Write(const Brx& aInfo) = 0;
    virtual ~ILog() {}
};


////////////////////////////////////////////////////////////////////

class LogDummy : public ILog
{
public:
    virtual void Write(const Brx&) {}
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
