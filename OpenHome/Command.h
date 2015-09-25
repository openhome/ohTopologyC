#pragma once

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Private/Stream.h>
#include <OpenHome/Buffer.h>

namespace OpenHome {


class ICommandTokens
{
public:
   virtual TUint Count() const = 0;
   virtual const Brn Next() = 0;
   virtual const Brn Remaining() = 0;
   virtual const Brn RemainingTrimmed() = 0;
   virtual ~ICommandTokens() {}
};

///////////////////////////////////////////////////////////////////////////

class ICommandHandler
{
public:
   virtual void Execute(ICommandTokens& aTokens, IWriter& aWriter) = 0;
   virtual ~ICommandHandler() {}
};

///////////////////////////////////////////////////////////////////////////

class CommandTokens : public ICommandTokens
{
public:
   CommandTokens(const Brx& aValue);
   virtual TUint Count() const;
   virtual const Brn Next();
   virtual const Brn Remaining();
   virtual const Brn RemainingTrimmed();
private:
   const Brn GetNextToken(const Brx& aValue, TUint& aIndex);
private:
   Brn iValue;
   TUint iCount;
   TUint iIndex;
};

///////////////////////////////////////////////////////////////////////////


} // namespace Linn




