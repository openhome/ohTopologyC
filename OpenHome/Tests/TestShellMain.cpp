#include <OpenHome/Net/Core/OhNet.h>
#include <OpenHome/Private/TestFramework.h>
#include <OpenHome/Tests/TestShell.h>
#include <OpenHome/Net/Private/CpiStack.h>
#include <OpenHome/Net/Private/DviStack.h>

using namespace OpenHome;
using namespace OpenHome::Net;

SIMPLE_TEST_DECLARATION(TestWatchableThread);
SIMPLE_TEST_DECLARATION(TestWatchable);



extern void TestTopology1(OpenHome::Environment& aEnv, const std::vector<Brn>& aArgs);
static void ShellTestTopology1(CpStack& aCpStack, DvStack& /*aDvStack*/, const std::vector<Brn>& aArgs)
{
    TestTopology1(aCpStack.Env(), aArgs);
}


extern void TestTopology2(OpenHome::Environment& aEnv, const std::vector<Brn>& aArgs);
static void ShellTestTopology2(CpStack& aCpStack, DvStack& /*aDvStack*/, const std::vector<Brn>& aArgs)
{
    TestTopology2(aCpStack.Env(), aArgs);
}

extern void TestTopology3(OpenHome::Environment& aEnv, const std::vector<Brn>& aArgs);
static void ShellTestTopology3(CpStack& aCpStack, DvStack& /*aDvStack*/, const std::vector<Brn>& aArgs)
{
    TestTopology3(aCpStack.Env(), aArgs);
}

//extern void TestTopology4(OpenHome::Environment& aEnv, const std::vector<Brn>& aArgs);
//static void ShellTestTopology4(CpStack& aCpStack, DvStack& /*aDvStack*/, const std::vector<Brn>& aArgs)
//{
//    TestTopology4(aCpStack.Env(), aArgs);
//}

extern void TestTopology5(OpenHome::Environment& aEnv, const std::vector<Brn>& aArgs);
static void ShellTestTopology5(CpStack& aCpStack, DvStack& /*aDvStack*/, const std::vector<Brn>& aArgs)
{
    TestTopology5(aCpStack.Env(), aArgs);
}

extern void TestTopology6(OpenHome::Environment& aEnv, const std::vector<Brn>& aArgs);
static void ShellTestTopology6(CpStack& aCpStack, DvStack& /*aDvStack*/, const std::vector<Brn>& aArgs)
{
    TestTopology6(aCpStack.Env(), aArgs);
}

SIMPLE_TEST_DECLARATION(TestServiceRadio);

void OpenHome::TestFramework::Runner::Main(TInt /*aArgc*/, TChar* /*aArgv*/[], Net::InitialisationParams* aInitParams)
{
    std::vector<ShellTest> shellTests;
    shellTests.push_back(ShellTest("TestWatchableThread", ShellTestWatchableThread));
    shellTests.push_back(ShellTest("TestWatchable", ShellTestWatchable));
    shellTests.push_back(ShellTest("TestTopology1", ShellTestTopology1));
    shellTests.push_back(ShellTest("TestTopology2", ShellTestTopology2));
    shellTests.push_back(ShellTest("TestTopology3", ShellTestTopology3));
    //shellTests.push_back(ShellTest("TestTopology4", ShellTestTopology4));
    shellTests.push_back(ShellTest("TestTopology5", ShellTestTopology5));
    shellTests.push_back(ShellTest("TestTopology6", ShellTestTopology6));
    shellTests.push_back(ShellTest("TestServiceRadio", ShellTestServiceRadio));

    OpenHome::ExecuteTestShell(aInitParams, shellTests);
}
