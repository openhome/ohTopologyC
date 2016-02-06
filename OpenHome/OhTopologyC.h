#pragma once

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/Command.h>
#include <OpenHome/Functor.h>

EXCEPTION(NotSupportedException);
EXCEPTION(NotImplementedException);

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

} // OpenHome
