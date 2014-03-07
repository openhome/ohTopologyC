#include <OpenHome/Private/TestFramework.h>

extern void TestTopology2(OpenHome::Environment& aEnv);

void OpenHome::TestFramework::Runner::Main(TInt /*aArgc*/, TChar* /*aArgv*/[], Net::InitialisationParams* aInitParams)
{
    Net::Library* lib = new Net::Library(aInitParams);
    TestTopology2(lib->Env());
    delete lib;
}
