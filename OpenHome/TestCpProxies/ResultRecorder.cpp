#include <OpenHome/TestCpProxies/ResultRecorder.h>
#include <OpenHome/Private/Printer.h>
#include <set>
#include <algorithm>
#include <iterator>
using namespace OpenHome::Topology;
using namespace OpenHome;

ResultRecorderBase::ResultRecorderBase()
{

}

ResultRecorderBase::~ResultRecorderBase()
{

}

TBool ResultRecorderBase::AsExpected(const std::vector<TUint>& aExpected)
{
  if(aExpected.size() != iResults.size())
  {
    return false;
  }
  /**Uncomment if order doesn't matter for equality*/
  //TBool same;
  //std::set<TUint> expected(aExpected.begin(), aExpected.end());
  //std::set<TUint> actual(iResults.begin(), iResults.end());
  //std::vector<TUint> similar;
  //std::set_intersection(expected.begin(),expected.end(),actual.begin(),actual.end(), std::back_inserter(similar));
  //similar.size() == aExpected.size() ? same = true : same = false;
  //return same;
  TBool result = std::equal(aExpected.begin(), aExpected.end(), iResults.begin());
  return result;

}

void ResultRecorderBase::Record(TUint aResult)
{
  iResults.push_back(aResult);
}

void ResultRecorderBase::ClearResults()
{
    iResults.clear();
}