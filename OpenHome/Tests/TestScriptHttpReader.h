#ifndef HEADER_TESTSCRIPTHTTPREADER
#define HEADER_TESTSCRIPTHTTPREADER

#include <OpenHome/Private/OptionParser.h>
#include <OpenHome/OsWrapper.h>
#include <OpenHome/Private/Http.h>


using namespace OpenHome;
using namespace OpenHome::TestFramework;


namespace OpenHome {

class TestScriptHttpReader : public HttpReader
{
public:
    TestScriptHttpReader(Environment& aEnv, std::vector<Brn>& aArgs)
        :HttpReader(aEnv)
    {
        OptionParser parser;
        OptionString optionServer("-s", "--server", Brn("eng"), "address of server to connect to");
        parser.AddOption(&optionServer);
        OptionUint optionPort("-p", "--port", 80, "server port to connect on");
        parser.AddOption(&optionPort);
        OptionString optionPath("", "--path", Brn(""), "path to use on server");
        parser.AddOption(&optionPath);

        if (!parser.Parse(aArgs) || parser.HelpDisplayed()) {
            return;
        }
        ASSERT(optionPort.Value() <= 65535);


        std::vector<NetworkAdapter*>* ifs = Os::NetworkListAdapters(aEnv, Net::InitialisationParams::ELoopbackExclude, "TestTopology1");


        ASSERT(ifs->size() > 0);
        TIpAddress addr = (*ifs)[0]->Address(); // assume we are only on one subnet (or using loopback)
        for (TUint i=0; i<ifs->size(); i++) {
            TIpAddress addrTmp = (*ifs)[i]->Address();
            Endpoint endpt(optionPort.Value(), addrTmp);
            Endpoint::AddressBuf buf;
            endpt.AppendAddress(buf);
            (*ifs)[i]->RemoveRef("TestTopology1");
        }
        delete ifs;

        Endpoint endptClient(0, addr);
        Endpoint::AddressBuf buf;
        endptClient.AppendAddress(buf);
        Log::Print("Using network interface %s\n", buf.Ptr());

        // set up server uri
        Endpoint endptServer = Endpoint(optionPort.Value(), optionServer.Value());
        iUriBuf.Replace(Brn("http://"));
        endptServer.AppendEndpoint(iUriBuf);
        iUriBuf.Append(Brn("/"));
        iUriBuf.Append(optionPath.Value());

        Uri uri(iUriBuf);

        Log::Print("HttpReader setup \n");

        Log::Print("Uri:");
        Log::Print(iUriBuf);
        Log::Print("\n");


        if (!Connect(uri))
        {
            Log::Print("Failed to connect \n");

            ASSERTS();
        }
    }

private:
    Bws<500> iUriBuf;

};


} // OpenHome

#endif // HEADER_TESTSCRIPTHTTPREADER
