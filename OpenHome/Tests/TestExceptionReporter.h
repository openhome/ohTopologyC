#ifndef HEADER_TEST_EXCEPTION_REPORTER
#define HEADER_TEST_EXCEPTION_REPORTER

#include <OpenHome/Private/OptionParser.h>
#include <OpenHome/OsWrapper.h>


namespace OpenHome {

class TestExceptionReporter : public IExceptionReporter
{
public:
    TestExceptionReporter() :iCount(0) ,iCountStd(0) {}
    virtual void Report(Exception& /*aException*/) { iCount++;}
    virtual void Report(std::exception& /*aException*/) { iCountStd++;}

    TUint Count() const { return(iCount); }
    TUint CountStd() const { return(iCountStd); }

private:
    TUint iCount;
    TUint iCountStd;
};


} // namespace OpenHome


/////////////////////////////////////////////////////////////////////

#endif // HEADER_TEST_EXCEPTION_REPORTER
