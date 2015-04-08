#include <OpenHome/Topology5.h>

#include <map>
#include <algorithm>


using namespace OpenHome;
using namespace OpenHome::Topology;
using namespace std;



Topology5GroupWatcher::Topology5GroupWatcher(Topology5& aTopology5, ITopology4Group& aGroup)
    :iTopology5(aTopology5)
    ,iGroup(aGroup)
{
    iGroup.RoomName().AddWatcher(*this);
}

// IWatcher<string>

void Topology5GroupWatcher::ItemOpen(const Brx& /*aId*/, Brn aRoom)
{
    iTopology5.AddGroupToRoom(iGroup, aRoom);
}

void Topology5GroupWatcher::ItemUpdate(const Brx& /*aId*/, Brn aRoom, Brn aPreviousRoom)
{
    iTopology5.RemoveGroupFromRoom(iGroup, aPreviousRoom);
    iTopology5.AddGroupToRoom(iGroup, aRoom);
}

void Topology5GroupWatcher::ItemClose(const Brx& /*aId*/, Brn aRoom)
{
    iTopology5.RemoveGroupFromRoom(iGroup, aRoom);
}

// IDisposable

void Topology5GroupWatcher::Dispose()
{
    iGroup.RoomName().RemoveWatcher(*this);
}

///////////////////////////////////////////////////////////////

Topology5Room::Topology5Room(IWatchableThread& aThread, const Brx& aName, ITopology4Group& aGroup)
    :iName(aName)
    ,iWatchableGroups(new WatchableUnordered<ITopology4Group*>(aThread))
{
    Add(aGroup);
}


Topology5Room::~Topology5Room()
{
    delete iWatchableGroups;
    iWatchableGroups = NULL;
    iGroups.clear();
}


void Topology5Room::Dispose()
{
    iWatchableGroups->Dispose();
}

// ITopology5Room

const Brx& Topology5Room::Name()
{
    return(iName);
}

IWatchableUnordered<ITopology4Group*>& Topology5Room::Groups()
{
    return(*iWatchableGroups);
}

void Topology5Room::SetStandby(TBool aValue)
{
    for(TUint i=0; i<iGroups.size(); i++)
    {
        iGroups[i]->SetStandby(aValue);
    }
}

// Topology5Room

void Topology5Room::Add(ITopology4Group& aGroup)
{
    iWatchableGroups->Add(&aGroup);
    iGroups.push_back(&aGroup);
}

TBool Topology5Room::Remove(ITopology4Group& aGroup)
{
    iWatchableGroups->Remove(&aGroup);

    auto it =  find(iGroups.begin(), iGroups.end(), &aGroup);;
    ASSERT(it!=iGroups.end());

    iGroups.erase(it);

    return (iGroups.size() == 0);
}

///////////////////////////////////////////////////////

Topology5::Topology5(ITopology4* aTopology4, ILog& /*aLog*/)
    :iDisposed(false)
    ,iNetwork(aTopology4->Network())
    ,iTopology4(aTopology4)
    ,iRooms(new WatchableUnordered<ITopology5Room*>(iNetwork))
{
    iNetwork.Schedule(MakeFunctorGeneric(*this, &Topology5::WatchT4Groups), NULL);
}


Topology5::~Topology5()
{
    delete iRooms;

    for(auto it=iGroupWatcherLookup.begin(); it!=iGroupWatcherLookup.end(); it++)
    {
        delete it->second;
    }

    delete iTopology4;
}


void Topology5::WatchT4Groups(void*)
{
    iTopology4->Groups().AddWatcher(*this);
}


void Topology5::Dispose()
{
    if (iDisposed)
    {
        //throw new ObjectDisposedException("Topology5.Dispose");
    }

    iNetwork.Execute(MakeFunctorGeneric(*this, &Topology5::DisposeCallback), NULL);

    iRooms->Dispose();

    iDisposed = true;
    iTopology4->Dispose();
}


void Topology5::DisposeCallback(void*)
{
    iTopology4->Groups().RemoveWatcher(*this);

    // removing these watchers should cause all rooms to be detached and disposed
    for(auto it=iGroupWatcherLookup.begin(); it!=iGroupWatcherLookup.end(); it++)
    {
        it->second->Dispose();
    }
}


IWatchableUnordered<ITopology5Room*>& Topology5::Rooms()
{
    return(*iRooms);
}

INetwork& Topology5::Network()
{
    return(iNetwork);
}


void Topology5::UnorderedOpen()
{
}

void Topology5::UnorderedInitialised()
{
}

void Topology5::UnorderedClose()
{
}

void Topology5::UnorderedAdd(ITopology4Group* aItem)
{
    iGroupWatcherLookup[aItem] = new Topology5GroupWatcher(*this, *aItem);
}

void Topology5::UnorderedRemove(ITopology4Group* aItem)
{
    ASSERT(iGroupWatcherLookup.count(aItem)>0);
    auto watcher = iGroupWatcherLookup[aItem];
    watcher->Dispose();
    delete watcher;
    iGroupWatcherLookup.erase(aItem);
}


void Topology5::AddGroupToRoom(ITopology4Group& aGroup, const Brx& aRoomName)
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
        Topology5Room* room = new Topology5Room(iNetwork, aRoomName, aGroup);
        iRoomLookup[roomName] = room;
        iRooms->Add(room);
    }
}


void Topology5::RemoveGroupFromRoom(ITopology4Group& aGroup, const Brx& aRoomName)
{
    Brn roomName(aRoomName);

    if (iRoomLookup.count(roomName) > 0)
    {
        Topology5Room* room = iRoomLookup[roomName];

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


