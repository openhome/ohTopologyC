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

class Topology4Group;
class Topology4Room;

class IMediaPreset : public IDisposable
{
public:
    virtual TUint Index() = 0;
    virtual IMediaMetadata& Metadata() = 0;
    virtual IWatchable<TBool>& Buffering() = 0;
    virtual IWatchable<TBool>& Playing() = 0;
    virtual IWatchable<TBool>& Selected() = 0;
    virtual void Play() = 0;
};

/////////////////////////////////////////////////////////////////////

class ITopology4Group
{
public:
    virtual Brn Name() = 0;
    virtual IDevice& Device() = 0;
    virtual IWatchable<ITopologymSender*>& Sender() = 0;
};

/////////////////////////////////////////////////////////////////////

class  ITopology4Source
{
public:
    virtual ITopology4Group& Group() = 0;
    virtual TUint Index() = 0;
    virtual Brn Name() = 0;
    virtual Brn Type() = 0;
    virtual TBool Visible() = 0;
    virtual IMediaPreset& CreatePreset() = 0;
    virtual const std::vector<ITopology4Group*> Volumes() = 0;
    virtual IDevice& Device() = 0;
    virtual TBool HasInfo() = 0;
    virtual TBool HasTime() = 0;
};

/////////////////////////////////////////////////////////////////////

class Topology4SourceNull : public ITopology4Source
{
public:
    virtual ITopology4Group& Group();
    virtual TUint Index();
    virtual Brn Name();
    virtual Brn Type();
    virtual TBool Visible();
    virtual IMediaPreset& CreatePreset();
    virtual const std::vector<ITopology4Group*> Volumes();
    virtual IDevice& Device();
    virtual TBool HasInfo();
    virtual TBool HasTime();

};

/////////////////////////////////////////////////////////////////////

class Topology4Source : public ITopology4Source, public INonCopyable
{
public:
    Topology4Source(INetwork& aNetwork, Topology4Group& aGroup, ITopology2Source* aSource);
    virtual TUint Index();
    virtual Brn Name();
    virtual ITopology4Group& Group();
    virtual Brn Type();
    virtual TBool Visible();
    virtual IMediaPreset& CreatePreset();
    virtual const std::vector<ITopology4Group*> Volumes();
    virtual IDevice& Device();
    virtual TBool HasInfo();
    virtual TBool HasTime();
    virtual void SetDevice(IDevice& aDevice);
    virtual void SetHasInfo(TBool aHasInfo);
    virtual void SetHasTime(TBool aHasTime);
    virtual void SetVolumes(const std::vector<ITopology4Group*> aVolumes);

    virtual void Select();

private:
    INetwork& iNetwork;
    Topology4Group& iGroup;
    ITopology2Source* iSource;

    const std::vector<ITopology4Group*>* iVolumes;
    IDevice* iDevice;
    TBool iHasInfo;
    TBool iHasTime;
};

///////////////////////////////////////////////////////////////

class MediaPresetExternal : public IMediaPreset, public IWatcher<ITopology4Source*>, public INonCopyable
{
public:
    MediaPresetExternal(IWatchableThread& aThread, Topology4Group& aGroup, TUint aIndex, IMediaMetadata* aMetadata, Topology4Source& aSource);

    virtual void Dispose();
    virtual TUint Index();
    virtual IMediaMetadata& Metadata();
    virtual IWatchable<TBool>& Buffering();
    virtual IWatchable<TBool>& Playing();
    virtual IWatchable<TBool>& Selected();
    virtual void Play();
    virtual void ItemOpen(const Brx& aId, ITopology4Source* aValue);
    virtual void ItemUpdate(const Brx& aId, ITopology4Source* aValue, ITopology4Source* aPrevious);
    virtual void ItemClose(const Brx& aId, ITopology4Source* aValue);

private:
    TUint iIndex;
    IMediaMetadata* iMetadata;
    Topology4Source& iSource;
    Topology4Group& iGroup;
    Watchable<TBool>* iBuffering;
    Watchable<TBool>* iPlaying;
    Watchable<TBool>* iSelected;
};

/////////////////////////////////////////////////////////////////////

class ITopology4Registration
{
public:
    virtual Brn Room() = 0;
    virtual Brn ModelName() = 0;
    virtual Brn ManufacturerName() = 0;
    virtual Brn ProductId() = 0;
};


/////////////////////////////////////////////////////////////////////

class ITopology4Root : public ITopology4Group
{
public:
    virtual IWatchable<ITopology4Source*>& Source() = 0;
    virtual const std::vector<ITopology4Source*> Sources() = 0;
    virtual IWatchable<std::vector<ITopology4Group*>>& Senders() = 0;
};

/////////////////////////////////////////////////////////////////////

class Topology4Group : public ITopology4Root, public ITopology4Registration, public IWatcher<TUint>, public IWatcher<Brn>, public IDisposable, public INonCopyable
{
public:
    Topology4Group(INetwork& aNetwork, const Brx& aRoom, const Brx& aName, ITopologymGroup& aGroup, const std::vector<ITopology2Source*> aSources, ILog& aLog);
    virtual void Dispose();
    virtual Brn Name();
    virtual IDevice& Device();
    virtual Brn Room();
    virtual Brn ModelName();
    virtual Brn ManufacturerName();
    virtual Brn ProductId();
    virtual IWatchable<ITopologymSender*>& Sender();
    virtual IWatchable<ITopology4Source*>& Source();
    virtual void EvaluateSources();
    virtual const std::vector<ITopology4Source*> Sources();
    virtual void EvaluateSenders();
    virtual IWatchable<std::vector<ITopology4Group*>>& Senders();
    virtual Topology4Group* Parent();
    virtual TBool AddIfIsChild(Topology4Group& aGroup);
    virtual void ItemOpen(const Brx& aId, TUint aValue);
    virtual void ItemUpdate(const Brx& aId, TUint aValue, TUint aPrevious);
    virtual void ItemClose(const Brx& aId, TUint aValue);
    virtual void ItemOpen(const Brx& aId, Brn aValue);
    virtual void ItemUpdate(const Brx& aId, Brn aValue, Brn aPrevious);
    virtual void ItemClose(const Brx& aId, Brn aValue);
    virtual void SetSourceIndex(TUint aValue);

    ITopologymGroup& Group();


private:
    void SetParent(Topology4Group& aGroup);
    void SetParent(Topology4Group& aGroup, TUint aIndex);
    ITopology4Source* EvaluateSource();
    void EvaluateSourceFromChild();
    void EvaluateSendersFromChild();
    void CreateCallback(void* aSender);


private:
    INetwork& iNetwork;
    Brn iRoom;
    Brn iName;
    ITopologymGroup* iGroup;
    ILog& iLog;
    TBool iDisposed;

    TUint iParentSourceIndex;
    Topology4Group* iParent;
    IProxySender* iSender;
    TBool iHasSender;
    TUint iSourceIndex;

    std::vector<Topology4Group*> iChildren;
    std::vector<Topology4Source*> iSources;
    std::vector<ITopology4Source*> iVisibleSources;

    Watchable<std::vector<ITopology4Group*>>* iSenders;
    Watchable<ITopology4Source*>* iWatchableSource;
};

/////////////////////////////////////////////////////////////////////

class Topology4GroupWatcher : public IWatcher<Brn>, public IWatcher<ITopology2Source*>, public IDisposable, public INonCopyable
{
public:
    Topology4GroupWatcher(Topology4Room& aRoom, ITopologymGroup& aGroup);
    virtual void Dispose();
    virtual Brn Room();
    virtual Brn Name();
    virtual const std::vector<ITopology2Source*> Sources();

    virtual void ItemOpen(const Brx& aId, Brn aValue);
    virtual void ItemUpdate(const Brx& aId, Brn aValue, Brn aPrevious);
    virtual void ItemClose(const Brx& aId, Brn aValue);
    virtual void ItemOpen(const Brx& aId, ITopology2Source* aValue);
    virtual void ItemUpdate(const Brx& aId, ITopology2Source* aValue, ITopology2Source* aPrevious);
    virtual void ItemClose(const Brx& aId, ITopology2Source* aValue);

private:
    Topology4Room& iRoom;
    ITopologymGroup& iGroup;
    Brn iRoomName;
    Brn iName;
    std::vector<ITopology2Source*> iSources;

};

/////////////////////////////////////////////////////////////////////

class ITopology4Room
{
public:
    virtual Brn Name() = 0;
    virtual IWatchable<EStandby>& Standby() = 0;
    virtual IWatchable<std::vector<ITopology4Root*>>& Roots() = 0;
    virtual IWatchable<std::vector<ITopology4Source*>>& Sources() = 0;
    //virtual IWatchable<const std::vector<ITopology4Registration*>> Registrations() = 0;
    virtual void SetStandby(TBool aValue) = 0;
};

/////////////////////////////////////////////////////////////////////

class Topology4Room : public ITopology4Room, public IWatcherUnordered<ITopologymGroup*>, public IWatcher<TBool>, public IDisposable, public INonCopyable
{
public:
    Topology4Room(INetwork& aNetwork, ITopology3Room& aRoom, ILog& aLog);
    virtual void Dispose();
    virtual Brn Name();
    virtual IWatchable<EStandby>& Standby();
    virtual IWatchable<std::vector<ITopology4Root*>>& Roots();
    virtual IWatchable<std::vector<ITopology4Source*>>& Sources();
    virtual IWatchable<std::vector<ITopology4Registration*>>& Registrations();
    virtual void SetStandby(TBool aValue);
    virtual void UnorderedOpen();
    virtual void UnorderedInitialised();
    virtual void UnorderedClose();
    virtual void UnorderedAdd(ITopologymGroup* aItem);
    virtual void UnorderedRemove(ITopologymGroup* aItem);
    virtual void CreateTree();
    virtual void ItemOpen(const Brx& aId, TBool aValue);
    virtual void ItemUpdate(const Brx& aId, TBool aValue, TBool aPrevious);
    virtual void ItemClose(const Brx& aId, TBool aValue);


private:
    void EvaluateStandby();
    void EvaluateStandby(TBool aLastGroup);
    void InsertIntoTree(Topology4Group& aGroup);

private:
    INetwork& iNetwork;
    ITopology3Room& iRoom;

    ILog& iLog;

    Brn iName;
    TUint iStandbyCount;
    EStandby iStandby;

    Watchable<EStandby>* iWatchableStandby;
    Watchable<std::vector<ITopology4Root*>>* iWatchableRoots;
    Watchable<std::vector<ITopology4Source*>>* iWatchableSources;
    Watchable<std::vector<ITopology4Registration*>>* iWatchableRegistrations;

    std::map<ITopologymGroup*, Topology4GroupWatcher*> iGroupLookup;
    std::vector<Topology4Group*> iGroups;
    std::vector<Topology4Group*> iRoots;
};

/////////////////////////////////////////////////////////////////////

class ITopology4
{
public:
    virtual IWatchableUnordered<ITopology4Room*>& Rooms() = 0;
    virtual INetwork& Network() = 0;
};

/////////////////////////////////////////////////////////////////////

class Topology4 : public ITopology4, public IWatcherUnordered<ITopology3Room*>, public IDisposable, public INonCopyable
{
public:
    Topology4(ITopology3* aTopology3, ILog& aLog);
    virtual void Dispose();
    virtual IWatchableUnordered<ITopology4Room*>& Rooms();
    virtual INetwork& Network();
    virtual void UnorderedOpen();
    virtual void UnorderedInitialised();
    virtual void UnorderedClose();
    virtual void UnorderedAdd(ITopology3Room* aItem);
    virtual void UnorderedRemove(ITopology3Room* aItem);

private:
    void ScheduleCallback(void*);
    void DisposeCallback(void*);
    void UnorderedAddCallback(void* aT3Room);
    void UnorderedRemoveCallback(void* aT3Room);


private:
    ITopology3* iTopology3;
    ILog& iLog;
    INetwork& iNetwork;
    DisposeHandler* iDisposeHandler;
    WatchableUnordered<ITopology4Room*>* iRooms;
    std::map<ITopology3Room*, Topology4Room*> iRoomLookup;
};

} // Av
} // OpenHome

#endif // HEADER_TOPOLOGY4

