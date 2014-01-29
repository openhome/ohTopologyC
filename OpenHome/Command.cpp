#include <OpenHome/Private/Ascii.h>
#include <OpenHome/Command.h>

using namespace OpenHome;
using namespace Linn;

CommandTokens::CommandTokens(const Brx& aValue)
       : iValue(Ascii::Trim(aValue))
       , iCount(0)
       , iIndex(0)
{
   TUint index = 0;

   while (GetNextToken(iValue, index).Bytes() > 0)
   {
      iCount++;
   }
}

TUint CommandTokens::Count() const
{
   return (iCount);
}

const Brn CommandTokens::Next()
{
   ASSERT(iCount-- > 0);
   return (GetNextToken(iValue, iIndex));
}

const Brn CommandTokens::GetNextToken(const Brx& aValue, TUint& aIndex)
{
   const TByte* start = aValue.Ptr() + aIndex;
   TUint bytes = aValue.Bytes();
   ASSERT(aIndex <= bytes)

   while (true)
   {
      if (aIndex == bytes)
      {
         return (Brn(start, 0));
      }

      if (*start != ' ')
      {
         break;
      }

      aIndex++;
      start++;
   }

   const TByte* ptr = start;
   aIndex++;
   ptr++;
   TUint count = 1;

   while (true)
   {
      if (aIndex == bytes)
      {
         break;
      }

      if (*ptr == ' ')
      {
         break;
      }

      aIndex++;
      ptr++;
      count++;
   }

   return (Brn(start, count));
}


