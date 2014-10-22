#include <OpenHome/Topology4.h>

#include <map>
#include <algorithm>


using namespace OpenHome;
using namespace OpenHome::Av;
using namespace std;



Topology4GroupWatcher::Topology4GroupWatcher(Topology4& aTopology4, ITopology3Group& aGroup)
    :iTopology4(aTopology4)
    ,iGroup(aGroup)
{
    iGroup.Room().AddWatcher(*this);
}

// IWatcher<string>

void Topology4GroupWatcher::ItemOpen(const Brx& /*aId*/, Brn aValue)
{
    iTopology4.AddGroupToRoom(iGroup, aValue);
}

void Topology4GroupWatcher::ItemUpdate(const Brx& /*aId*/, Brn aValue, Brn aPrevious)
{
    iTopology4.RemoveGroupFromRoom(iGroup, aPrevious);
    iTopology4.AddGroupToRoom(iGroup, aValue);
}

void Topology4GroupWatcher::ItemClose(const Brx& /*aId*/, Brn aValue)
{
    iTopology4.RemoveGroupFromRoom(iGroup, aValue);
}

// IDisposable

void Topology4GroupWatcher::Dispose()
{
    iGroup.Room().RemoveWatcher(*this);
}

///////////////////////////////////////////////////////////////

Topology4Room::Topology4Room(IWatchableThread& aThread, const Brx& aName, ITopology3Group& aGroup)
    :iName(aName)
    ,iWatchableGroups(new WatchableUnordered<ITopology3Group*>(aThread))
{
    Add(aGroup);
}


Topology4Room::~Topology4Room()
{
    delete iWatchableGroups;
    iWatchableGroups = NULL;
    iGroups.clear();
}


void Topology4Room::Dispose()
{
    iWatchableGroups->Dispose();
}

// ITopology4Room

const Brx& Topology4Room::Name()
{
    return(iName);
}

IWatchableUnordered<ITopology3Group*>& Topology4Room::Groups()
{
    return(*iWatchableGroups);
}

void Topology4Room::SetStandby(TBool aValue)
{
    for(TUint i=0; i<iGroups.size(); i++)
    {
        iGroups[i]->SetStandby(aValue);
    }
}

// Topology4Room

void Topology4Room::Add(ITopology3Group& aGroup)
{
    iWatchableGroups->Add(&aGroup);
    iGroups.push_back(&aGroup);
}

TBool Topology4Room::Remove(ITopology3Group& aGroup)
{
    iWatchableGroups->Remove(&aGroup);

    vector<ITopology3Group*>::iterator it =  find(iGroups.begin(), iGroups.end(), &aGroup);;
    ASSERT(it!=iGroups.end());

    iGroups.erase(it);

    return (iGroups.size() == 0);
}

///////////////////////////////////////////////////////

Topology4::Topology4(Topology3* aTopology3, ILog& /*aLog*/)
    :iDisposed(false)
    ,iNetwork(aTopology3->Network())
    ,iTopology3(aTopology3)
    ,iRooms(new WatchableUnordered<ITopology4Room*>(iNetwork))
{
    iNetwork.Schedule(MakeFunctorGeneric(*this, &Topology4::ScheduleCallback), NULL);
}


Topology4::~Topology4()
{
    delete iTopology3;
    delete iRooms;

    map<ITopology3Group*, Topology4GroupWatcher*>::iterator it;
    for(it=iGroupWatcherLookup.begin(); it!=iGroupWatcherLookup.end(); it++)
    {
        delete it->second;
    }

}


void Topology4::ScheduleCallback(void*)
{
    iTopology3->Groups().AddWatcher(*this);
}


void Topology4::Dispose()
{
    if (iDisposed)
    {
        //throw new ObjectDisposedException("Topology4.Dispose");
    }

    iNetwork.Execute(MakeFunctorGeneric(*this, &Topology4::DisposeCallback), NULL);

    iRooms->Dispose();

    iDisposed = true;
    iTopology3->Dispose();
}


void Topology4::DisposeCallback(void*)
{
    iTopology3->Groups().RemoveWatcher(*this);

    // removing these watchers should cause all rooms to be detached and disposed
    map<ITopology3Group*, Topology4GroupWatcher*>::iterator it;
    for(it=iGroupWatcherLookup.begin(); it!=iGroupWatcherLookup.end(); it++)
    {
        it->second->Dispose();
    }
}


IWatchableUnordered<ITopology4Room*>& Topology4::Rooms()
{
    return(*iRooms);
}

INetwork& Topology4::Network()
{
    return(iNetwork);
}

// IWatcherUnordered<ITopology3Group>

void Topology4::UnorderedOpen()
{
}

void Topology4::UnorderedInitialised()
{
}

void Topology4::UnorderedClose()
{
}

void Topology4::UnorderedAdd(ITopology3Group* aItem)
{
    iGroupWatcherLookup[aItem] = new Topology4GroupWatcher(*this, *aItem);
}

void Topology4::UnorderedRemove(ITopology3Group* aItem)
{
    ASSERT(iGroupWatcherLookup.count(aItem)>0);
    auto watcher = iGroupWatcherLookup[aItem];
    watcher->Dispose();
    delete watcher;
    iGroupWatcherLookup.erase(aItem);
}


void Topology4::AddGroupToRoom(ITopology3Group& aGroup, const Brx& aRoomName)
{
    Brn roomName(aRoomName);

    if(iRoomLookup.count(roomName)>0)
    {
        // room already exists
        iRoomLookup[roomName]->Add(aGroup);
    }
    else
    {
        // need to create a new room
        Topology4Room* room = new Topology4Room(iNetwork, aRoomName, aGroup);
        iRoomLookup[roomName] = room;
        iRooms->Add(room);
    }
}


void Topology4::RemoveGroupFromRoom(ITopology3Group& aGroup, const Brx& aRoomName)
{
    Brn roomName(aRoomName);

    if (iRoomLookup.count(roomName) > 0)
    {
        Topology4Room* room = iRoomLookup[roomName];

        if (room->Remove(aGroup))
        {
            // no more groups in room - remove it
            iRooms->Remove(room);
            iRoomLookup.erase(roomName);
            room->Dispose();
            delete room;
        }
    }
}


