#include <OpenHome/Private/TestFramework.h>
#include <OpenHome/Private/OptionParser.h>
#include <OpenHome/Buffer.h>
#include <algorithm>

using namespace OpenHome;

extern void TestTopology3(OpenHome::Environment& aEnv, const std::vector<Brn>& aArgs);

void OpenHome::TestFramework::Runner::Main(TInt aArgc, TChar* aArgv[], Net::InitialisationParams* aInitParams)
{
    static const Brn kLocalhost("127.0.0.1");

    Net::Library* lib = new Net::Library(aInitParams);
    std::vector<Brn> args = OptionParser::ConvertArgs(aArgc, aArgv);

    Log::Print(
        "\n======================================================\n"
        "Overriding server name to '127.0.0.1'\n"
        "Temp hack to avoid DNS issues with OSX and Windows\n"
        "This won't be applied to Tests when run on Core platform"
        "\n======================================================\n\n"
        );
    // Note, getaddrinfo() in Windows OS port requires "127.0.0.1" be passed.
    // "localhost" is resolved to 0.0.0.0
    if(std::find(args.begin(), args.end(), Brn("-s")) != args.end() )  // only if found (ie already specified)
    {
        args.emplace_back("-s");
        args.emplace_back(kLocalhost);
    }
    TestTopology3(lib->Env(), args);
    delete lib;
}
