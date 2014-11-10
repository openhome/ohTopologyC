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



        Net::InitialisationParams::ELoopback loopback;
        if (optionServer.Value().Equals(Brn("127.0.0.1"))) { // using loopback
            loopback = Net::InitialisationParams::ELoopbackUse;
        }
        else {
            loopback = Net::InitialisationParams::ELoopbackExclude;
        }
        std::vector<NetworkAdapter*>* ifs = Os::NetworkListAdapters(aEnv, loopback, "TestTopology");



        ASSERT(ifs->size() > 0);
        TIpAddress addr = (*ifs)[0]->Address(); // assume we are only on one subnet (or using loopback)

        for (TUint i=0; i<ifs->size(); i++) {

            //Log::Print("ifs adr:%x \n", (*ifs)[i]->Address());
            (*ifs)[i]->RemoveRef("TestTopology");

        }

        delete ifs;

        Endpoint endptClient(0, addr);
        Endpoint::AddressBuf buf;
        endptClient.AppendAddress(buf);
        Log::Print("Using network interface %s\n", buf.Ptr());
        Log::Print("Client IP:%x \n", addr);
        Log::Print("Server IP:");
        Log::Print(optionServer.Value());
        Log::Print("\n");

        // set up server uri

        Endpoint endptServer = Endpoint(optionPort.Value(), optionServer.Value());

        iUriBuf.Replace(Brn("http://"));
        endptServer.AppendEndpoint(iUriBuf);
        iUriBuf.Append(Brn("/"));
        iUriBuf.Append(optionPath.Value());

        Uri uri(iUriBuf);

        Log::Print("HttpReader setup \n");
        Log::Print("Uri:\n");
        Log::Print(iUriBuf);
        Log::Print("\n");


        if (!Connect(uri))
        {
            Log::Print("Failed to connect \n");
            ASSERTS();
        }
    }

private:
    Bws<100> iUriBuf;

};


} // OpenHome

#endif // HEADER_TESTSCRIPTHTTPREADER
