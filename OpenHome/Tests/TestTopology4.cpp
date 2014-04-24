#include <OpenHome/Private/SuiteUnitTest.h>
#include <OpenHome/Topology4.h>
#include <OpenHome/Mockable.h>
#include <OpenHome/Injector.h>
#include <OpenHome/Tests/TestScriptHttpReader.h>
#include <OpenHome/Private/Ascii.h>
#include <exception>

#include <vector>


using namespace std;
using namespace OpenHome;
using namespace OpenHome::Av;
using namespace OpenHome::TestFramework;


namespace OpenHome {

namespace Av {


class SuiteTopology4: public SuiteUnitTest, public INonCopyable
{
public:
    SuiteTopology4(IReader& aReader);

private:
    // from SuiteUnitTest
    void Setup();
    void TearDown();
    void Test1();

private:
    void ExecuteCallback(void* aObj);
    void ScheduleCallback(void* aObj);

private:
    Topology4* iTopology4;
    IReader& iReader;
};



////////////////////////////////////////////////////////////////////////////////////

class RootWatcher : public IDisposable
{
public:

    RootWatcher(MockableScriptRunner* aRunner, ITopology4Root& aRoot)
    {
        iFactory = new ResultWatcherFactory(*aRunner);

        iFactory->Create<ITopology4Source*>(aRoot.Name(), aRoot.Source(), MakeFunctorGeneric(*this, &RootWatcher::CreateCallback1));
        iFactory->Create<vector<ITopology4Group*>>(aRoot.Name(), aRoot.Senders(), MakeFunctorGeneric(*this, &RootWatcher::CreateCallback2));
    }

    void CreateCallback1(ArgsTwo<ITopology4Source*, FunctorGeneric<const Brx&>>* aArgs)
    {
        ITopology4Source* s = aArgs->Arg1();
        FunctorGeneric<const Brx&> f = aArgs->Arg2();

        Bws<100> info;
        info.SetBytes(0);

        info.Append(Brn("Source"));
        info.Append(s->Index());
        info.Append(Brn(" "));
        info.Append(s->Group().Name());
        info.Append(Brn(" "));
        info.Append(s->Name());
        info.Append(Brn(" "));
        info.Append(s->Type());
        info.Append(Brn(" "));

        if (s->Visible())
        {
            info.Append(Brn("True"));
        }
        else
        {
            info.Append(Brn("False"));
        }


        info.Append(Brn(" "));
        if (s->HasInfo())
        {
            info.Append(Brn("True"));
        }
        else
        {
            info.Append(Brn("False"));
        }
        info.Append(Brn(" "));

        if (s->HasTime())
        {
            info.Append(Brn("True"));
        }
        else
        {
            info.Append(Brn("False"));
        }
        info.Append(Brn(" "));
        info.Append(s->Device().Udn());

        for(TUint i=0; i<s->Volumes().size(); i++)
        {
            info.Append(Brn(" "));
            info.Append(s->Volumes()[i]->Device().Udn());
            //info += " " +
            //g.Device.Udn;
        }

        f(info);




/*
        string info = "";
        info += string.Format("Source {0} {1} {2} {3} {4} {5} {6} {7} Volume",
            v.Index, v.Group.Name, v.Name, v.Type, v.Visible, v.HasInfo, v.HasTime, v.Device.Udn);
        foreach (var g in v.Volumes)
        {
            info += " " + g.Device.Udn;
        }
        w(info);
*/
    }

    void CreateCallback2(ArgsTwo<vector<ITopology4Group*>, FunctorGeneric<const Brx&>>* aArgs)
    {
        vector<ITopology4Group*> v = aArgs->Arg1();
        FunctorGeneric<const Brx&> f = aArgs->Arg2();

        Bws<100> buf;
        buf.Replace(Brn("\nSenders begin\n"));


        for(TUint i=0; i<v.size(); i++)
        {
            buf.Append(Brn("Sender "));
            buf.Append(v[i]->Name());
            buf.Append(Brn("\n"));
        }

        buf.Append(Brn("Senders end"));
        f(buf);
/*
        string info = "\nSenders begin\n";
        foreach (var g in v)
        {
            info += "Sender " + g.Name;
            info += "\n";
        }
        info += "Senders end";
        w(info);
*/
    }


    void Dispose()
    {
        iFactory->Dispose();
    }

private:
    ResultWatcherFactory* iFactory;
};

//////////////////////////////////////////////////////////////////////

class RoomWatcher : public IWatcher<vector<ITopology4Root*>>, public IDisposable
{
private:
    MockableScriptRunner* iRunner;
    ITopology4Room& iRoom;
    vector<RootWatcher*> iWatchers;

public:
    RoomWatcher(MockableScriptRunner* aRunner, ITopology4Room& aRoom)
        :iRunner(aRunner)
        ,iRoom(aRoom)
    {
        iRoom.Roots().AddWatcher(*this);
    }

    void Dispose()
    {
        iRoom.Roots().RemoveWatcher(*this);
        //iWatchers.ForEach(w => w.Dispose());
        for(TUint i=0; i<iWatchers.size(); i++)
        {
            iWatchers[i]->Dispose();
        }
    }

    void ItemOpen(const Brx& /*aId*/, vector<ITopology4Root*> aValue)
    {
        for(TUint i=0; i<aValue.size(); i++)
        {
            iWatchers.push_back(new RootWatcher(iRunner, *aValue[i]));
        }
    }

    void ItemUpdate(const Brx& /*aId*/, vector<ITopology4Root*> aValue, vector<ITopology4Root*> /*aPrevious*/)
    {
        //iWatchers.ForEach(w => w.Dispose());
        for(TUint i=0; i<iWatchers.size(); i++)
        {
            iWatchers[i]->Dispose();
        }

        iWatchers.clear();

        //foreach (var r in aValue)
        for(TUint i=0; i<aValue.size(); i++)
        {
            iWatchers.push_back(new RootWatcher(iRunner, *aValue[i]));
        }
    }

    void ItemClose(const Brx& /*aId*/, vector<ITopology4Root*> /*aValue*/)
    {
    }
};

///////////////////////////////////////////////////////////////////

class HouseWatcher : public IWatcherUnordered<ITopology4Room*>, public IDisposable
{
public:
    HouseWatcher(MockableScriptRunner* aRunner)
        :iRunner(aRunner)
        ,iFactory(new ResultWatcherFactory(*aRunner))
    {
    }

    void Dispose()
    {
        iFactory->Dispose();

        map<ITopology4Room*, RoomWatcher*>::iterator it;
        for(it=iWatcherLookup.begin(); it!=iWatcherLookup.end(); it++)
        {
            it->second->Dispose();
        }
    }

    void UnorderedOpen()
    {
    }

    void UnorderedInitialised()
    {
    }

    void UnorderedClose()
    {
    }

    void UnorderedAdd(ITopology4Room* aItem)
    {
        Bws<100> buf;
        buf.Replace(Brn("Room Added "));
        buf.Append(aItem->Name());
        Bwh* result = new Bwh(buf);
        iRunner->Result(result);

        iFactory->Create<EStandby>(aItem->Name(), aItem->Standby(), MakeFunctorGeneric(*this, &HouseWatcher::CreateCallback1)/* (v, w) => w("Standby " + v)*/);

        iFactory->Create<vector<ITopology4Source*>>(aItem->Name(), aItem->Sources(), MakeFunctorGeneric(*this, &HouseWatcher::CreateCallback2));
/*
        iFactory->Create<vector<ITopology4Source*>>(aItem.Name, aItem.Sources, (v, w) =>
        {
            string info = "\nSources begin\n";
            foreach (var s in v)
            {
                info += string.Format("Source {0} {1} {2} {3} {4} {5} {6} {7} Volume",
                    s.Index, s.Group.Name, s.Name, s.Type, s.Visible, s.HasInfo, s.HasTime, s.Device.Udn);
                foreach (var g in s.Volumes)
                {
                    info += " " + g.Device.Udn;
                }
                info += "\n";
            }
            info += "Sources end";
            w(info);
        });
*/
        iWatcherLookup[aItem] = new RoomWatcher(iRunner, *aItem);
    }

    void UnorderedRemove(ITopology4Room* aItem)
    {
        Bws<100> buf;
        buf.Replace(Brn("Room Removed "));
        buf.Append(aItem->Name());
        Bwh* result = new Bwh(buf);

        iRunner->Result(result);

        iFactory->Destroy(aItem->Name());
        iWatcherLookup[aItem]->Dispose();
        iWatcherLookup.erase(aItem);
    }

    void CreateCallback1(ArgsTwo<EStandby, FunctorGeneric<const Brx&>>* aArgs)
    {
        // (v, w) => w("Standby " + v)
        EStandby arg1 = aArgs->Arg1();
        FunctorGeneric<const Brx&> f = aArgs->Arg2();

        Bws<100> buf;
        buf.Replace(Brn("Standby "));

        Ascii::AppendDec(buf, arg1);
        f(buf);
        delete aArgs;
    }

    void CreateCallback2(ArgsTwo<vector<ITopology4Source*>, FunctorGeneric<const Brx&>>* aArgs)
    {
        vector<ITopology4Source*> v = aArgs->Arg1();
        FunctorGeneric<const Brx&> f = aArgs->Arg2();


        Bws<100> info;
        info.Replace(Brn("\nSources begin\n"));

        for(TUint i=0; i<v.size(); i++)
        {
            ITopology4Source* s = v[i];

            info.Append(Brn("Source"));
            info.Append(s->Index());
            info.Append(Brn(" "));
            info.Append(s->Group().Name());
            info.Append(Brn(" "));
            info.Append(s->Name());
            info.Append(Brn(" "));
            info.Append(s->Type());
            info.Append(Brn(" "));

            if (s->Visible())
            {
                info.Append(Brn("True"));
            }
            else
            {
                info.Append(Brn("False"));
            }


            info.Append(Brn(" "));
            if (s->HasInfo())
            {
                info.Append(Brn("True"));
            }
            else
            {
                info.Append(Brn("False"));
            }
            info.Append(Brn(" "));

            if (s->HasTime())
            {
                info.Append(Brn("True"));
            }
            else
            {
                info.Append(Brn("False"));
            }
            info.Append(Brn(" "));
            info.Append(s->Device().Udn());
            info.Append(Brn(" "));
            info.Append(Brn("Volume"));

            //string.Format("Source {0} {1} {2} {3} {4} {5} {6} {7} Volume",
            //    s.Index, s.Group.Name, s.Name, s.Type, s.Visible, s.HasInfo, s.HasTime, s.Device.Udn);

            //foreach (var g in s.Volumes)

            for(TUint i=0; i<s->Volumes().size(); i++)
            {
                info.Append(Brn(" "));
                info.Append(s->Volumes()[i]->Device().Udn());
                //info += " " +
                //g.Device.Udn;
            }

            info.Append(Brn("\n"));

            //info += "\n";
        }
        //info += "Sources end";
        info.Append(Brn("Sources end"));
        f(info);
    }

private:
    MockableScriptRunner* iRunner;
    ResultWatcherFactory* iFactory;
    map<ITopology4Room*, RoomWatcher*> iWatcherLookup;
};


} // Av

} // OpenHome


//////////////////////////////////////////////////////////////////////////

SuiteTopology4::SuiteTopology4(IReader& aReader)
    :SuiteUnitTest("SuiteTopology4")
    ,iReader(aReader)
{
    AddTest(MakeFunctor(*this, &SuiteTopology4::Test1));
}


void SuiteTopology4::Setup()
{
}


void SuiteTopology4::TearDown()
{
    delete iTopology4;
}


void SuiteTopology4::Test1()
{
    Mockable* mocker = new Mockable();
    ILog* log = new LogDummy();
    Network* network = new Network(50, *log);

    InjectorMock* mockInjector = new InjectorMock(*network, Brx::Empty(), *log);
    mocker->Add(Brn("network"), *mockInjector);

    Topology1* topology1 = new Topology1(network, *log);
    Topology2* topology2 = new Topology2(topology1, *log);
    Topologym* topologym = new Topologym(topology2, *log);
    Topology3* topology3 = new Topology3(topologym, *log);
    iTopology4 = new Topology4(topology3, *log);


    MockableScriptRunner* runner = new MockableScriptRunner();
    HouseWatcher* watcher = new HouseWatcher(runner);

    FunctorGeneric<void*> fs = MakeFunctorGeneric(*this, &SuiteTopology4::ScheduleCallback);
    network->Schedule(fs, watcher);

    Functor f = MakeFunctor(*network, &Network::Wait);

    TEST(runner->Run(f, iReader, *mocker));
    FunctorGeneric<void*> fe = MakeFunctorGeneric(*this, &SuiteTopology4::ExecuteCallback);
    network->Execute(fe, watcher);

    iTopology4->Dispose();
    topology3->Dispose();
    topologym->Dispose();
    topology2->Dispose();
    topology1->Dispose();
    mockInjector->Dispose();
    network->Dispose();
}


void SuiteTopology4::ExecuteCallback(void* aObj)
{
    LOG(kTrace, "SuiteTopology4::ExecuteCallback() \n");
    HouseWatcher* watcher = (HouseWatcher*)aObj;
    iTopology4->Rooms().RemoveWatcher(*watcher);
    watcher->Dispose();
}


void SuiteTopology4::ScheduleCallback(void* aObj)
{
    LOG(kTrace, "SuiteTopology4::ScheduleCallback() \n");
    HouseWatcher* watcher = (HouseWatcher*)aObj;
    iTopology4->Rooms().AddWatcher(*watcher);
}

//////////////////////////////////////////////////////////////

void TestTopology4(Environment& aEnv, std::vector<Brn>& aArgs)
{
    if(aArgs.size()<2)
    {
        aArgs.push_back(Brn("--path"));
        aArgs.push_back(Brn("~eamonnb/Topology4TestScript.txt"));
    }

    TestScriptHttpReader reader(aEnv, aArgs);

    Runner runner("Topology4 tests\n");
    runner.Add(new SuiteTopology4(reader));
    runner.Run();
}






