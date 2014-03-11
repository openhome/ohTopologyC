#include <OpenHome/Private/TestFramework.h>

extern void TestWatchable();

void OpenHome::TestFramework::Runner::Main(TInt /*aArgc*/, TChar* /*aArgv*/[], Net::InitialisationParams* aInitParams)
{
    Net::UpnpLibrary::InitialiseMinimal(aInitParams);
    TestWatchable();
    delete aInitParams;
    Net::UpnpLibrary::Close();
}
