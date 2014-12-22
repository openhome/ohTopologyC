#ifndef HEADER_TOPOLOGY4
#define HEADER_TOPOLOGY4

#include <OpenHome/IWatcher.h>
#include <OpenHome/IWatchable.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/Network.h>
#include <OpenHome/ServiceProduct.h>
#include <OpenHome/Device.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/Topology3.h>
#include <vector>
#include <map>


namespace OpenHome
{

namespace Av
{


class Topology4;


class ITopology4Room
{
public:
    virtual const Brx& Name() = 0;
    virtual IWatchableUnordered<ITopology3Group*>& Groups() = 0;
    virtual void SetStandby(TBool aValue) = 0;
    virtual ~ITopology4Room() {}
};

////////////////////////////////////////////////////////

class Topology4GroupWatcher : public IWatcher<Brn>, public IDisposable, public INonCopyable
{
public:
    Topology4GroupWatcher(Topology4& aTopology, ITopology3Group& aGroup);

    // IWatcher<string>
    virtual void ItemOpen(const Brx& aId, Brn aValue);
    virtual void ItemUpdate(const Brx& aId, Brn aValue, Brn aPrevious);
    virtual void ItemClose(const Brx& aId, Brn aValue);

    // IDisposable
    virtual void Dispose();

private:
    Topology4& iTopology4;
    ITopology3Group& iGroup;
};

/////////////////////////////////////////////////////////

class Topology4Room : public ITopology4Room
{
public:
    Topology4Room(IWatchableThread& aThread, const Brx& aName, ITopology3Group& aGroup);
    ~Topology4Room();

    // IDisposable
    virtual void Dispose();

    // ITopology4Room
    virtual const Brx& Name();
    virtual IWatchableUnordered<ITopology3Group*>& Groups();
    virtual void SetStandby(TBool aValue);

    // Topology4Room
    virtual void Add(ITopology3Group& aGroup);
    virtual TBool Remove(ITopology3Group& aGroup);

private:
    Bws<100> iName; // FIXME: random capacity
    std::vector<ITopology3Group*> iGroups;
    WatchableUnordered<ITopology3Group*>* iWatchableGroups;
};

/////////////////////////////////////////////////////////

class ITopology4 : public IDisposable
{
public:
    virtual IWatchableUnordered<ITopology4Room*>& Rooms() = 0;
    virtual INetwork& Network() = 0;
    virtual void Dispose() = 0;
    virtual ~ITopology4() {}
};

/////////////////////////////////////////////////////////

class Topology4 : public ITopology4, public IWatcherUnordered<ITopology3Group*>, public INonCopyable
{
public:
    Topology4(ITopology3* aTopology3, ILog& aLog);
    ~Topology4();

    virtual void AddGroupToRoom(ITopology3Group& aGroup, const Brx& aRoom);
    virtual void RemoveGroupFromRoom(ITopology3Group& aGroup, const Brx& aRoom);

    // IDisposable
    virtual void Dispose();

    // ITopology4
    virtual IWatchableUnordered<ITopology4Room*>& Rooms();
    virtual INetwork& Network();

    // IWatcherUnordered<ITopology3Group>
    virtual void UnorderedOpen();
    virtual void UnorderedInitialised();
    virtual void UnorderedClose();
    virtual void UnorderedAdd(ITopology3Group* aItem);
    virtual void UnorderedRemove(ITopology3Group* aItem);

private:
    void WatchT3Groups(void*);
    void DisposeCallback(void*);

private:
    TBool iDisposed;
    INetwork& iNetwork;
    ITopology3* iTopology3;
    WatchableUnordered<ITopology4Room*>* iRooms;
    std::map<ITopology3Group*, Topology4GroupWatcher*> iGroupWatcherLookup;
    std::map<Brn, Topology4Room*, BufferCmp> iRoomLookup;
};


} // Av

} // OpenHome

#endif //  HEADER_TOPOLOGY4

