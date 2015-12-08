#include <OpenHome/Private/TestFramework.h>
#include <OpenHome/Private/OptionParser.h>
#include <OpenHome/Buffer.h>

using namespace OpenHome;

extern void TestServiceRadio(std::vector<Brn>& aArgs);

void OpenHome::TestFramework::Runner::Main(TInt aArgc, TChar* aArgv[], Net::InitialisationParams* aInitParams)
{
  Net::UpnpLibrary::InitialiseMinimal(aInitParams);
  std::vector<Brn> args = OptionParser::ConvertArgs(aArgc, aArgv);
  TestServiceRadio(args);
  delete aInitParams;
  Net::UpnpLibrary::Close();
}
