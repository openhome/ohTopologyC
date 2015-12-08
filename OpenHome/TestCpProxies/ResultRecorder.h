#pragma once

#include <OpenHome/Types.h>
#include <OpenHome/Buffer.h>
#include <vector>
#include <algorithm>
namespace OpenHome{
namespace Topology{



class ResultRecorderBase
{
  public:
    ResultRecorderBase();
    ~ResultRecorderBase();
    void ClearResults();
  protected:
    TBool AsExpected(const std::vector<TUint>& aExpected);
    void Record(TUint aResult);
  private:
    std::vector<TUint> iResults;
};

template<class T>
class ResultRecorder : public ResultRecorderBase
{
public:
  TBool AsExpected(const std::vector<T>& aExpected)
  {
    std::vector<TUint> expected;
    std::for_each(aExpected.begin(), aExpected.end(), [&expected](T aElem){expected.push_back(static_cast<TUint>(aElem));});
    return ResultRecorderBase::AsExpected(expected);
  }
  void Record(T aResult)
  {
    ResultRecorderBase::Record(static_cast<TUint>(aResult));
  }
};

} //Topology
} //OpenHome
