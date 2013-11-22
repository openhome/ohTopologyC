#include <OpenHome/Net/Core/OhNet.h>
#include <OpenHome/Private/TestFramework.h>
//#include <OpenHome/Tests/WatchableThread.h>
#include <OpenHome/Tests/TestShell.h>
#include <OpenHome/Net/Private/CpiStack.h>
#include <OpenHome/Net/Private/DviStack.h>

using namespace OpenHome;

SIMPLE_TEST_DECLARATION(TestWatchableThread);



void OpenHome::TestFramework::Runner::Main(TInt /*aArgc*/, TChar* /*aArgv*/[], Net::InitialisationParams* aInitParams)
{
    std::vector<ShellTest> shellTests;
    shellTests.push_back(ShellTest("TestWatchableThread", ShellTestWatchableThread));

    OpenHome::ExecuteTestShell(aInitParams, shellTests);
}
