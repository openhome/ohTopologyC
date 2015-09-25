#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Net/Private/Shell.h>
#include <OpenHome/Net/Private/ShellCommandRun.h>

#define SIMPLE_TEST_DECLARATION(x)  \
    extern void x();                \
    static void Shell##x(CpStack&, DvStack&, const std::vector<Brn>&) { x(); } \

#define ENV_TEST_DECLARATION(x) \
    extern void x(Environment&);            \
    static void Shell##x(CpStack&, DvStack& aDvStack, const std::vector<Brn>&) { x(aDvStack.Env()); } \

namespace OpenHome {
namespace Net {
    class InitialisationParams;
}

//namespace Media {

static const TUint kTestShellTimeout = 60; // initial timeout for TestShell. This is increased  by testharness once running.

void ExecuteTestShell(OpenHome::Net::InitialisationParams* aInitParams, std::vector<OpenHome::Net::ShellTest>& aTests);

//} // namespace Media
} // namespace OpenHome
