#include <OpenHome/Topology3.h>

#include <map>
#include <algorithm>


using namespace OpenHome;
using namespace OpenHome::Av;
using namespace std;



Topology3GroupWatcher::Topology3GroupWatcher(Topology3& aTopology3, ITopologymGroup& aGroup)
    :iTopology3(aTopology3)
    ,iGroup(aGroup)
{
    iGroup.Room().AddWatcher(*this);
}

// IWatcher<string>

void Topology3GroupWatcher::ItemOpen(const Brx& /*aId*/, Brn aValue)
{
    iTopology3.AddGroupToRoom(iGroup, aValue);
}

void Topology3GroupWatcher::ItemUpdate(const Brx& /*aId*/, Brn aValue, Brn aPrevious)
{
    iTopology3.RemoveGroupFromRoom(iGroup, aPrevious);
    iTopology3.AddGroupToRoom(iGroup, aValue);
}

void Topology3GroupWatcher::ItemClose(const Brx& /*aId*/, Brn aValue)
{
    iTopology3.RemoveGroupFromRoom(iGroup, aValue);
}

// IDisposable

void Topology3GroupWatcher::Dispose()
{
    iGroup.Room().RemoveWatcher(*this);
}

///////////////////////////////////////////////////////////////

Topology3Room::Topology3Room(IWatchableThread& aThread, const Brx& aName, ITopologymGroup& aGroup)
    :iName(aName)
    ,iWatchableGroups(new WatchableUnordered<ITopologymGroup*>(aThread))
{
    Add(aGroup);
}

void Topology3Room::Dispose()
{
    iWatchableGroups->Dispose();
    iWatchableGroups = NULL;

    iGroups.clear();
    //iGroups = null;
}

// ITopology3Room

const Brx& Topology3Room::Name()
{
    return(iName);
}

IWatchableUnordered<ITopologymGroup*>& Topology3Room::Groups()
{
    return(*iWatchableGroups);
}

void Topology3Room::SetStandby(TBool aValue)
{
    for(TUint i=0; i<iGroups.size(); i++)
    {
        iGroups[i]->SetStandby(aValue);
    }
}

// Topology3Room

void Topology3Room::Add(ITopologymGroup& aGroup)
{
    iWatchableGroups->Add(&aGroup);
    iGroups.push_back(&aGroup);
}

TBool Topology3Room::Remove(ITopologymGroup& aGroup)
{
    iWatchableGroups->Remove(&aGroup);

    vector<ITopologymGroup*>::iterator it =  find(iGroups.begin(), iGroups.end(), &aGroup);;
    ASSERT(it!=iGroups.end());

    iGroups.erase(it);

    return (iGroups.size() == 0);
}

///////////////////////////////////////////////////////

Topology3::Topology3(ITopologym* aTopologym, ILog& /*aLog*/)
    :iDisposed(false)
    ,iNetwork(aTopologym->Network())
    ,iTopologym(aTopologym)
    ,iRooms(new WatchableUnordered<ITopology3Room*>(iNetwork))
{
    iNetwork.Schedule(MakeFunctorGeneric(*this, &Topology3::ScheduleCallback), NULL);
}


void Topology3::ScheduleCallback(void*)
{
    iTopologym->Groups().AddWatcher(*this);
}


void Topology3::Dispose()
{
    if (iDisposed)
    {
        //throw new ObjectDisposedException("Topology3.Dispose");
    }


    iNetwork.Execute(MakeFunctorGeneric(*this, &Topology3::DisposeCallback), NULL);
    iTopologym = NULL;

    //iRoomLookup = NULL;
    //iGroupWatcherLookup = NULL;

    iRooms->Dispose();
    iRooms = NULL;

    iDisposed = true;
}


void Topology3::DisposeCallback(void*)
{
    iTopologym->Groups().RemoveWatcher(*this);

    map<ITopologymGroup*, Topology3GroupWatcher*>::iterator it;

    // removing these watchers should cause all rooms to be detached and disposed
    for(it=iGroupWatcherLookup.begin(); it!=iGroupWatcherLookup.end(); it++)
    {
        it->second->Dispose();
    }

/*
    // removing these watchers should cause all rooms to be detached and disposed
    foreach (var group in iGroupWatcherLookup.Values)
    {
        group.Dispose();
    }
*/
}


IWatchableUnordered<ITopology3Room*>& Topology3::Rooms()
{
    return(*iRooms);
}

INetwork& Topology3::Network()
{
    return(iNetwork);
}

// IWatcherUnordered<ITopologymGroup>

void Topology3::UnorderedOpen()
{
}

void Topology3::UnorderedInitialised()
{
}

void Topology3::UnorderedClose()
{
}

void Topology3::UnorderedAdd(ITopologymGroup* aItem)
{
    iGroupWatcherLookup[aItem] = new Topology3GroupWatcher(*this, *aItem);
}

void Topology3::UnorderedRemove(ITopologymGroup* aItem)
{
    iGroupWatcherLookup[aItem]->Dispose();
    ASSERT(iGroupWatcherLookup.count(aItem)>0);
    iGroupWatcherLookup.erase(aItem);
}


void Topology3::AddGroupToRoom(ITopologymGroup& aGroup, const Brx& aRoom)
{
    if(iRoomLookup.count(Brn(aRoom))>0)
    {
        // room already exists
        iRoomLookup[Brn(aRoom)]->Add(aGroup);
    }
    else
    {
        // need to create a new room
        Topology3Room* room = new Topology3Room(iNetwork, aRoom, aGroup);
        iRoomLookup[Brn(aRoom)] = room;
        iRooms->Add(room);
    }
}


void Topology3::RemoveGroupFromRoom(ITopologymGroup& aGroup, const Brx& aRoom)
{
    Brn roomName(aRoom);

    if (iRoomLookup.count(Brn(aRoom)) > 0)
    {
        Topology3Room* room = iRoomLookup[Brn(aRoom)];

        if (room->Remove(aGroup))
        {
            // no more groups in room - remove it
            iRooms->Remove(room);
            iRoomLookup.erase(Brn(aRoom));
            room->Dispose();
        }
    }
}


