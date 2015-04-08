#ifndef HEADER_TOPOLOGY5
#define HEADER_TOPOLOGY5

#include <OpenHome/IWatcher.h>
#include <OpenHome/IWatchable.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/Network.h>
#include <OpenHome/ServiceProduct.h>
#include <OpenHome/Device.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/Topology4.h>
#include <vector>
#include <map>


namespace OpenHome
{

namespace Topology
{


class Topology5;


class ITopology5Room
{
public:
    virtual const Brx& Name() = 0;
    virtual IWatchableUnordered<ITopology4Group*>& Groups() = 0;
    virtual void SetStandby(TBool aValue) = 0;
    virtual ~ITopology5Room() {}
};

////////////////////////////////////////////////////////

class Topology5GroupWatcher : public IWatcher<Brn>, public IDisposable, public INonCopyable
{
    friend class IWatcher<Brn>;

public:
    Topology5GroupWatcher(Topology5& aTopology, ITopology4Group& aGroup);

    // IDisposable
    virtual void Dispose();

private:
    // IWatcher<Brn>
    virtual void ItemOpen(const Brx& aId, Brn aRoom);
    virtual void ItemUpdate(const Brx& aId, Brn aRoom, Brn aPreviousRoom);
    virtual void ItemClose(const Brx& aId, Brn aRoom);

private:
    Topology5& iTopology5;
    ITopology4Group& iGroup;
};

/////////////////////////////////////////////////////////

class Topology5Room : public ITopology5Room
{
    friend class Topology5;
public:
    Topology5Room(IWatchableThread& aThread, const Brx& aName, ITopology4Group& aGroup);
    ~Topology5Room();

    // IDisposable
    virtual void Dispose();

    // ITopology5Room
    virtual const Brx& Name();
    virtual IWatchableUnordered<ITopology4Group*>& Groups();
    virtual void SetStandby(TBool aValue);

private:
    void Add(ITopology4Group& aGroup);
    TBool Remove(ITopology4Group& aGroup);

private:
    Bws<100> iName; // FIXME: random capacity
    std::vector<ITopology4Group*> iGroups;
    WatchableUnordered<ITopology4Group*>* iWatchableGroups;
};

/////////////////////////////////////////////////////////

class ITopology5 : public IDisposable
{
public:
    virtual IWatchableUnordered<ITopology5Room*>& Rooms() = 0;
    virtual INetwork& Network() = 0;
    virtual void Dispose() = 0;
    virtual ~ITopology5() {}
};

/////////////////////////////////////////////////////////

class Topology5 : public ITopology5, public IWatcherUnordered<ITopology4Group*>, public INonCopyable
{
friend class Topology5GroupWatcher;

public:
    Topology5(ITopology4* aTopology4, ILog& aLog);
    ~Topology5();

    // IDisposable
    virtual void Dispose();

    // ITopology4
    virtual IWatchableUnordered<ITopology5Room*>& Rooms();
    virtual INetwork& Network();

    // IWatcherUnordered<ITopology3Group>
    virtual void UnorderedOpen();
    virtual void UnorderedInitialised();
    virtual void UnorderedClose();
    virtual void UnorderedAdd(ITopology4Group* aItem);
    virtual void UnorderedRemove(ITopology4Group* aItem);

private:
    void WatchT4Groups(void*);
    void DisposeCallback(void*);
    void AddGroupToRoom(ITopology4Group& aGroup, const Brx& aRoom);
    void RemoveGroupFromRoom(ITopology4Group& aGroup, const Brx& aRoom);

private:
    TBool iDisposed;
    INetwork& iNetwork;
    ITopology4* iTopology4;
    WatchableUnordered<ITopology5Room*>* iRooms;
    std::map<ITopology4Group*, Topology5GroupWatcher*> iGroupWatcherLookup;
    std::map<Brn, Topology5Room*, BufferCmp> iRoomLookup;
};


} // Topology

} // OpenHome

#endif //  HEADER_TOPOLOGY4

