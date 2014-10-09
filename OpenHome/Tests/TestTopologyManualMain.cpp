#include <OpenHome/Private/TestFramework.h>
#include <OpenHome/Private/OptionParser.h>
#include <OpenHome/Buffer.h>

using namespace OpenHome;

extern void TestTopologyManual(Net::Library& aLib, const std::vector<Brn>& aArgs);

void OpenHome::TestFramework::Runner::Main(TInt aArgc, TChar* aArgv[], Net::InitialisationParams* aInitParams)
{
    Net::Library* lib = new Net::Library(aInitParams);

    std::vector<Brn> args = OptionParser::ConvertArgs(aArgc, aArgv);
    TestTopologyManual(*lib, args);
    delete lib;
}
