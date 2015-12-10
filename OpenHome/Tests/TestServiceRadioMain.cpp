#include <OpenHome/Private/TestFramework.h>
#include <OpenHome/Private/OptionParser.h>
#include <OpenHome/Buffer.h>

using namespace OpenHome;

extern void TestServiceRadio();

void OpenHome::TestFramework::Runner::Main(TInt /*aArgc*/, TChar* /*aArgv*/[], Net::InitialisationParams* aInitParams)
{
  Net::UpnpLibrary::InitialiseMinimal(aInitParams);
  TestServiceRadio();
  delete aInitParams;
  Net::UpnpLibrary::Close();
}
