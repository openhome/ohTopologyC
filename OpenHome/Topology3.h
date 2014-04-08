#ifndef HEADER_TOPOLOGY3
#define HEADER_TOPOLOGY3

#include <OpenHome/IWatcher.h>
#include <OpenHome/IWatchable.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/Network.h>
#include <OpenHome/ServiceProduct.h>
#include <OpenHome/Device.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/Topologym.h>
#include <vector>
#include <map>


namespace OpenHome
{

namespace Av
{


class Topology3;


class ITopology3Room
{
public:
    virtual const Brx& Name() = 0;
    virtual IWatchableUnordered<ITopologymGroup*>& Groups() = 0;
    virtual void SetStandby(TBool aValue) = 0;
};

////////////////////////////////////////////////////////

class Topology3GroupWatcher : public IWatcher<Brn>, public IDisposable
{
public:
    Topology3GroupWatcher(Topology3& aTopology, ITopologymGroup& aGroup);

    // IWatcher<string>
    virtual void ItemOpen(const Brx& aId, Brn aValue);
    virtual void ItemUpdate(const Brx& aId, Brn aValue, Brn aPrevious);
    virtual void ItemClose(const Brx& aId, Brn aValue);

    // IDisposable
    virtual void Dispose();

private:
    Topology3& iTopology3;
    ITopologymGroup& iGroup;
};

/////////////////////////////////////////////////////////

class Topology3Room : public ITopology3Room
{
public:
    Topology3Room(IWatchableThread& aThread, const Brx& aName, ITopologymGroup& aGroup);

    // IDisposable
    virtual void Dispose();

    // ITopology3Room
    virtual const Brx& Name();
    virtual IWatchableUnordered<ITopologymGroup*>& Groups();
    virtual void SetStandby(TBool aValue);

    // Topology3Room
    virtual void Add(ITopologymGroup& aGroup);
    virtual TBool Remove(ITopologymGroup& aGroup);

private:
    Bws<100> iName; // FIXME: random capacity
    std::vector<ITopologymGroup*> iGroups;
    WatchableUnordered<ITopologymGroup*>* iWatchableGroups;
};

/////////////////////////////////////////////////////////

class ITopology3
{
public:
    virtual IWatchableUnordered<ITopology3Room*>& Rooms() = 0;
    virtual INetwork& Network() = 0;
};

/////////////////////////////////////////////////////////

class Topology3 : public ITopology3, public IWatcherUnordered<ITopologymGroup*>, public IDisposable
{
public:
    Topology3(ITopologym* aTopologym, ILog& aLog);

    // IDisposable
    virtual void Dispose();

    // ITopology3
    virtual IWatchableUnordered<ITopology3Room*>& Rooms();
    virtual INetwork& Network();

    // IWatcherUnordered<ITopologymGroup>
    virtual void UnorderedOpen();
    virtual void UnorderedInitialised();
    virtual void UnorderedClose();
    virtual void UnorderedAdd(ITopologymGroup* aItem);
    virtual void UnorderedRemove(ITopologymGroup* aItem);
    virtual void AddGroupToRoom(ITopologymGroup& aGroup, const Brx& aRoom);
    virtual void RemoveGroupFromRoom(ITopologymGroup& aGroup, const Brx& aRoom);

private:
    void ScheduleCallback(void*);
    void DisposeCallback(void*);

private:
    TBool iDisposed;
    INetwork& iNetwork;
    ITopologym* iTopologym;
    WatchableUnordered<ITopology3Room*>* iRooms;
    std::map<ITopologymGroup*, Topology3GroupWatcher*> iGroupWatcherLookup;
    std::map<Brn, Topology3Room*, BufferCmp> iRoomLookup;
};


} // Av

} // OpenHome

#endif //  HEADER_TOPOLOGY3

