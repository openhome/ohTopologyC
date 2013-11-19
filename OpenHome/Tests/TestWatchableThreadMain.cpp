#include <OpenHome/Private/TestFramework.h>

extern void TestWatchableThread();

void OpenHome::TestFramework::Runner::Main(TInt /*aArgc*/, TChar* /*aArgv*/[], Net::InitialisationParams* aInitParams)
{
    Net::UpnpLibrary::InitialiseMinimal(aInitParams);
    TestWatchableThread();
    delete aInitParams;
    Net::UpnpLibrary::Close();
}
