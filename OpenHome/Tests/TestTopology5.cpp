#include <OpenHome/Private/SuiteUnitTest.h>
#include <OpenHome/Topology5.h>
#include <OpenHome/Mockable.h>
#include <OpenHome/Injector.h>
#include <OpenHome/Tests/TestScriptHttpReader.h>
#include <OpenHome/Private/Ascii.h>
#include <OpenHome/MetaData.h>
#include <exception>
#include <vector>


using namespace std;
using namespace OpenHome;
using namespace OpenHome::Av;
using namespace OpenHome::TestFramework;


namespace OpenHome
{
namespace Av
{
namespace TestTopology5
{


class SuiteTopology5: public SuiteUnitTest, public INonCopyable
{
public:
    SuiteTopology5(IReader& aReader);

private:
    // from SuiteUnitTest
    void Setup();
    void TearDown();
    void Test1();

private:
    void ExecuteCallback(void* aObj);
    void ScheduleCallback(void* aObj);

private:
    Topology5* iTopology5;
    IReader& iReader;
};



////////////////////////////////////////////////////////////////////////////////////

class RootWatcher : public IDisposable, public INonCopyable
{
public:
    RootWatcher(MockableScriptRunner& aRunner, ITopology5Root& aRoot)
        :iFactory(new ResultWatcherFactory(aRunner))
    {
        //Topology5Group::LogVolumes(aRoot);
        iFactory->Create<ITopology5Source*>(aRoot.Name(), aRoot.Source(), MakeFunctorGeneric(*this, &RootWatcher::CreateCallback1));
        iFactory->Create<vector<ITopology5Group*>*>(aRoot.Name(), aRoot.Senders(), MakeFunctorGeneric(*this, &RootWatcher::CreateCallback2));
    }

    void CreateCallback1(MockCbData<ITopology5Source*>* aArgs)
    {
        ITopology5Source* s = aArgs->iData;
        auto f = aArgs->iCallback;

        Bwh* buf = new Bwh(6000);

        buf->Replace(Brn("Source "));

        Ascii::AppendDec(*buf, s->Index());

        buf->Append(Brn(" "));
        buf->Append(s->Group().Name());
        buf->Append(Brn(" "));
        buf->Append(s->Name());
        buf->Append(Brn(" "));
        buf->Append(s->Type());
        buf->Append(Brn(" "));

        if (s->Visible())
        {
            buf->Append(Brn("True "));
        }
        else
        {
            buf->Append(Brn("False "));
        }


        if (s->HasInfo())
        {
            buf->Append(Brn("True "));
        }
        else
        {
            buf->Append(Brn("False "));
        }

        if (s->HasTime())
        {
            buf->Append(Brn("True "));
        }
        else
        {
            buf->Append(Brn("False "));
        }
        //iBuf.Append(udn);
        buf->Append(s->Device().Udn());

        buf->Append(Brn(" Volume"));

        for(TUint i=0; i<s->Volumes().size(); i++)
        {
            buf->Append(Brn(" "));
            buf->Append(s->Volumes()[i]->Device().Udn());
        }


        f(*buf);
        delete aArgs;
        delete buf;
    }

    void CreateCallback2(MockCbData<vector<ITopology5Group*>*>* aArgs)
    {
        vector<ITopology5Group*>* v = aArgs->iData;
        FunctorGeneric<const Brx&> f = aArgs->iCallback;

        Bwh* buf = new Bwh(6000);
        buf->Replace(Brn("\nSenders begin\n"));

        for(TUint i=0; i<v->size(); i++)
        {
            buf->Append(Brn("Sender "));
            buf->Append((*v)[i]->Name());
            buf->Append(Brn("\n"));
        }

        buf->Append(Brn("Senders end"));
        f(*buf);
        delete aArgs;
        delete buf;
    }


    virtual void Dispose()
    {
        iFactory->Dispose();
        delete iFactory;
    }

private:
    ResultWatcherFactory* iFactory;
    //Bws<6000> iBuf;
};

//////////////////////////////////////////////////////////////////////

class RoomWatcher : public IWatcher<vector<ITopology5Root*>*>, public IDisposable, public INonCopyable
{
private:
    MockableScriptRunner& iRunner;
    ITopology5Room& iRoom;
    vector<RootWatcher*> iWatchers;

public:
    RoomWatcher(MockableScriptRunner& aRunner, ITopology5Room& aRoom)
        :iRunner(aRunner)
        ,iRoom(aRoom)
    {
        Log::Print("RoomWatcher()  room = ");
        Log::Print(aRoom.Name());
        Log::Print("\n");

        auto roots = iRoom.Roots().Value();
        for(TUint i=0; i<roots->size(); i++)
        {
            auto root = (*roots)[i];
            Topology5Group::LogVolumes(*root);
        }

        Thread::Sleep(100);
        iRoom.Roots().AddWatcher(*this);
    }

    virtual void Dispose()
    {
        iRoom.Roots().RemoveWatcher(*this);
        for(TUint i=0; i<iWatchers.size(); i++)
        {
            iWatchers[i]->Dispose();
            delete iWatchers[i];
        }
    }

    virtual void ItemOpen(const Brx& /*aId*/, vector<ITopology5Root*>* aValue)
    {
        Log::Print("RoomWatcher  ItemOpen()  room = ");
        Log::Print(iRoom.Name());
        Log::Print("\n");

        for(TUint i=0; i<aValue->size(); i++)
        {
            auto root = (*aValue)[i];
            //Topology5Group::LogVolumes(*root);
            iWatchers.push_back(new RootWatcher(iRunner, *root));
        }
    }

    virtual void ItemUpdate(const Brx& /*aId*/, vector<ITopology5Root*>* aValue, vector<ITopology5Root*>* /*aPrevious*/)
    {
        Log::Print("RoomWatcher  ItemUpdate()  room = ");
        Log::Print(iRoom.Name());
        Log::Print("\n");
        //auto value = new vector<ITopology5Root*>(*aValue);

        for(TUint i=0; i<iWatchers.size(); i++)
        {
            iWatchers[i]->Dispose();
            delete iWatchers[i];
        }

        iWatchers.clear();

        for(TUint i=0; i<aValue->size(); i++)
        {
            Log::Print("RoomWatcher  ItemUpdate() creating watcher %d \n", i);
            auto root = (*aValue)[i];
            //Topology5Group::LogVolumes(*root);

            iWatchers.push_back(new RootWatcher(iRunner, *root));
        }
    }

    virtual void ItemClose(const Brx& /*aId*/, vector<ITopology5Root*>* /*aValue*/)
    {
    }
};

///////////////////////////////////////////////////////////////////

class HouseWatcher : public IWatcherUnordered<ITopology5Room*>, public IDisposable, public INonCopyable
{
public:
    HouseWatcher(MockableScriptRunner& aRunner)
        :iRunner(aRunner)
        ,iFactory(new ResultWatcherFactory(aRunner))
    {
    }

    virtual void Dispose()
    {
        iFactory->Dispose();
        delete iFactory;

        for(auto it=iWatcherLookup.begin(); it!=iWatcherLookup.end(); it++)
        {
            it->second->Dispose();
            delete it->second;
        }
    }

    virtual void UnorderedOpen()
    {
    }

    virtual void UnorderedInitialised()
    {
    }

    virtual void UnorderedClose()
    {
    }

    virtual void UnorderedAdd(ITopology5Room* aItem)
    {

        Bwh* result = new Bwh(10000);
        result->Replace(Brn("Room Added "));
        result->Append(aItem->Name());
        iRunner.Result(result);

        iFactory->Create<EStandby>(aItem->Name(), aItem->Standby(), MakeFunctorGeneric(*this, &HouseWatcher::CreateCallback1));
        iFactory->Create<vector<ITopology5Source*>*>(aItem->Name(), aItem->Sources(), MakeFunctorGeneric(*this, &HouseWatcher::CreateCallback2));
        iWatcherLookup[aItem] = new RoomWatcher(iRunner, *aItem);
    }

    virtual void UnorderedRemove(ITopology5Room* aItem)
    {
        Bwh* result = new Bwh(6000);
        result->Replace(Brn("Room Removed "));
        result->Append(aItem->Name());

        iRunner.Result(result);

        iFactory->Destroy(aItem->Name());
        iWatcherLookup[aItem]->Dispose();
        delete iWatcherLookup[aItem];
        iWatcherLookup.erase(aItem);
    }

    void CreateCallback1(MockCbData<EStandby>* aArgs)
    {
        // (v, w) => w("Standby " + v)
        EStandby arg1 = aArgs->iData;
        FunctorGeneric<const Brx&> f = aArgs->iCallback;

        Bwh* buf = new Bwh(6000);

        buf->Replace(Brn("Standby "));

        if (arg1==eOff)
        {
            buf->Append(Brn("eOff"));
        }
        else if (arg1==eOn)
        {
            buf->Append(Brn("eOn"));
        }
        else if (arg1==eMixed)
        {
            buf->Append(Brn("eMixed"));
        }
        else
        {
            ASSERTS();
        }

        f(*buf);
        delete aArgs;
        delete buf;
    }

    void CreateCallback2(MockCbData<vector<ITopology5Source*>*>* aArgs)
    {
        vector<ITopology5Source*>* v = aArgs->iData;
        FunctorGeneric<const Brx&> f = aArgs->iCallback;

        Bwh* buf = new Bwh(6000);

        buf->Replace(Brn("\nSources begin\n"));

        for(TUint i=0; i<v->size(); i++)
        {
            ITopology5Source* s = (*v)[i];

            buf->Append(Brn("Source "));

            Ascii::AppendDec(*buf, s->Index());

            buf->Append(Brn(" "));
            buf->Append(s->Group().Name());
            buf->Append(Brn(" "));
            buf->Append(s->Name());
            buf->Append(Brn(" "));
            buf->Append(s->Type());
            buf->Append(Brn(" "));

            if (s->Visible())
            {
                buf->Append(Brn("True "));
            }
            else
            {
                buf->Append(Brn("False "));
            }

            Brn udn(s->Device().Udn());

            if (s->HasInfo())
            {
                buf->Append(Brn("True "));
            }
            else
            {
                buf->Append(Brn("False "));
            }

            if (s->HasTime())
            {
                buf->Append(Brn("True "));
            }
            else
            {
                buf->Append(Brn("False "));
            }


            buf->Append(udn);
            buf->Append(Brn(" Volume"));


            for(TUint i=0; i<s->Volumes().size(); i++)
            {
                buf->Append(Brn(" "));
                buf->Append(s->Volumes()[i]->Device().Udn());
            }

            buf->Append(Brn("\n"));

        }
        buf->Append(Brn("Sources end"));

        f(*buf);
        delete aArgs;
        delete buf;
    }

private:
    MockableScriptRunner& iRunner;
    ResultWatcherFactory* iFactory;
    map<ITopology5Room*, RoomWatcher*> iWatcherLookup;
    //Bws<6000> iBuf;
};

} // TestTopology5
} // Av
} // OpenHome


using namespace OpenHome::Av::TestTopology5;

//////////////////////////////////////////////////////////////////////////

SuiteTopology5::SuiteTopology5(IReader& aReader)
    :SuiteUnitTest("SuiteTopology5")
    ,iReader(aReader)
{
    AddTest(MakeFunctor(*this, &SuiteTopology5::Test1));
}


void SuiteTopology5::Setup()
{
}


void SuiteTopology5::TearDown()
{
}


void SuiteTopology5::Test1()
{
    Mockable* mocker = new Mockable();
    ILog* log = new LogDummy();
    Network* network = new Network(50, *log);

    InjectorMock* mockInjector = new InjectorMock(*network, Brx::Empty(), *log);
    mocker->Add(Brn("network"), *mockInjector);


    Topology1* topology1 = new Topology1(network, *log);
    Topology2* topology2 = new Topology2(topology1, *log);
    Topology3* topology3 = new Topology3(topology2, *log);
    Topology4* topology4 = new Topology4(topology3, *log);
    iTopology5 = new Topology5(topology4, *log);

    MockableScriptRunner* runner = new MockableScriptRunner();
    HouseWatcher* watcher = new HouseWatcher(*runner);

    FunctorGeneric<void*> fs = MakeFunctorGeneric(*this, &SuiteTopology5::ScheduleCallback);
    network->Schedule(fs, watcher);

    Functor f = MakeFunctor(*network, &Network::Wait);

    TBool test = runner->Run(f, iReader, *mocker);
    //OpenHome::Log::Print("test = %d\n", test);
    TEST(test);

    FunctorGeneric<void*> fe = MakeFunctorGeneric(*this, &SuiteTopology5::ExecuteCallback);
    network->Execute(fe, watcher);

    iTopology5->Dispose();
    mockInjector->Dispose();
    network->Dispose();

    delete watcher;
    delete runner;

    delete iTopology5;
    delete mockInjector;
    delete network;

    delete log;
    delete mocker;

    Topology3Sender::DestroyStatics();
    InfoMetadata::DestroyStatics();
    SenderMetadata::DestroyStatics();
}


void SuiteTopology5::ExecuteCallback(void* aObj)
{
    HouseWatcher* watcher = (HouseWatcher*)aObj;
    iTopology5->Rooms().RemoveWatcher(*watcher);
    watcher->Dispose();
}


void SuiteTopology5::ScheduleCallback(void* aObj)
{
    HouseWatcher* watcher = (HouseWatcher*)aObj;
    iTopology5->Rooms().AddWatcher(*watcher);
}

//////////////////////////////////////////////////////////////

void TestTopology5(Environment& aEnv, const std::vector<Brn>& aArgs)
{
    std::vector<Brn> args(aArgs);

    if( find(args.begin(), args.end(), Brn("--path")) == args.end() )
    {
        args.push_back(Brn("--path"));
        args.push_back(Brn("~eamonnb/Topology5TestScript.txt"));
    }

    TestScriptHttpReader* reader = new TestScriptHttpReader(aEnv, args);

    Runner runner("Topology5 tests\n");
    runner.Add(new SuiteTopology5(*reader));
    runner.Run();

    delete reader;
}






