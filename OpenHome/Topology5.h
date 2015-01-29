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
#include <OpenHome/Media.h>
#include <vector>
#include <map>


namespace OpenHome
{

namespace Av
{

class Topology5Group;
class Topology5Room;

/////////////////////////////////////////////////////////////////////

class ITopology5Group
{
public:
    virtual Brn Name() = 0;
    virtual IDevice& Device() = 0;
    virtual IWatchable<ITopology3Sender*>& Sender() = 0;
    virtual ~ITopology5Group() {}
};

/////////////////////////////////////////////////////////////////////

class  ITopology5Source
{
public:
    virtual TUint Index() = 0;
    virtual Brn Name() = 0;
    virtual Brn Type() = 0;
    virtual TBool Visible() = 0;
    virtual ITopology5Group& Group() = 0;
    virtual IMediaPreset* CreatePreset() = 0;
    virtual std::vector<ITopology5Group*>& Volumes() = 0;
    virtual IDevice& Device() = 0;
    virtual TBool HasInfo() = 0;
    virtual TBool HasTime() = 0;
    virtual void Select() = 0;

    virtual ~ITopology5Source() {}
};

/////////////////////////////////////////////////////////////////////

class Topology5SourceNull : public ITopology5Source
{
public:
    virtual TUint Index();
    virtual Brn Name();
    virtual Brn Type();
    virtual TBool Visible();
    virtual ITopology5Group& Group();
    virtual IMediaPreset* CreatePreset();
    virtual std::vector<ITopology5Group*>& Volumes();
    virtual IDevice& Device();
    virtual TBool HasInfo();
    virtual TBool HasTime();
    virtual void Select();
};

/////////////////////////////////////////////////////////////////////

class Topology5Source : public ITopology5Source, public INonCopyable
{
public:
    Topology5Source(INetwork& aNetwork, Topology5Group& aGroup, IDevice& aDevice, ITopology2Source& aSource);

    // ITopology5Source
    virtual TUint Index();
    virtual Brn Name();
    virtual Brn Type();
    virtual TBool Visible();
    virtual ITopology5Group& Group();
    virtual IMediaPreset* CreatePreset();
    virtual std::vector<ITopology5Group*>& Volumes();
    virtual IDevice& Device();
    virtual TBool HasInfo();
    virtual TBool HasTime();
    virtual void Select();

    virtual void SetHasInfo(TBool aHasInfo);
    virtual void SetHasTime(TBool aHasTime);
    virtual void SetVolumes(std::vector<ITopology5Group*>* aVolumes);


private:
    INetwork& iNetwork;
    Topology5Group& iGroup;
    ITopology2Source& iSource;
    std::vector<ITopology5Group*>* iVolumes;
    IDevice& iDevice;
    TBool iHasInfo;
    TBool iHasTime;
};

///////////////////////////////////////////////////////////////

class MediaPresetExternal : public IMediaPreset, public IWatcher<ITopology5Source*>, public INonCopyable
{
public:
    MediaPresetExternal(IWatchableThread& aThread, Topology5Group& aGroup, TUint aIndex, IMediaMetadata* aMetadata, Topology5Source& aSource);

    virtual void Dispose();
    virtual TUint Index();
    virtual IMediaMetadata& Metadata();
    virtual IWatchable<TBool>& Buffering();
    virtual IWatchable<TBool>& Playing();
    virtual IWatchable<TBool>& Selected();
    virtual void Play();
    virtual void ItemOpen(const Brx& aId, ITopology5Source* aValue);
    virtual void ItemUpdate(const Brx& aId, ITopology5Source* aValue, ITopology5Source* aPrevious);
    virtual void ItemClose(const Brx& aId, ITopology5Source* aValue);

private:
    TUint iIndex;
    IMediaMetadata* iMetadata;
    Topology5Source& iSource;
    Topology5Group& iGroup;
    Watchable<TBool>* iBuffering;
    Watchable<TBool>* iPlaying;
    Watchable<TBool>* iSelected;
};


/////////////////////////////////////////////////////////////////////

class ITopology5Root : public ITopology5Group
{
public:
    virtual IWatchable<ITopology5Source*>& Source() = 0;
    virtual std::vector<ITopology5Source*>& Sources() = 0;
    virtual IWatchable<std::vector<ITopology5Group*>*>& Senders() = 0;
    virtual ~ITopology5Root() {}
};

/////////////////////////////////////////////////////////////////////

class Topology5Group : public ITopology5Root, public IWatcher<TUint>, public IWatcher<Brn>, public IDisposable, public INonCopyable
{
public:
    Topology5Group(INetwork& aNetwork, const Brx& aRoom, const Brx& aName, ITopology3Group& aGroup, std::vector<ITopology2Source*> aSources, ILog& aLog);
    ~Topology5Group();

    virtual void Dispose();
    virtual Brn Name();
    virtual IDevice& Device();
    virtual Brn RoomName();
    virtual Brn ModelName();
    virtual Brn ManufacturerName();
    virtual Brn ProductId();

    virtual IWatchable<ITopology3Sender*>& Sender();
    virtual IWatchable<std::vector<ITopology5Group*>*>& Senders();

    virtual IWatchable<ITopology5Source*>& Source();
    virtual std::vector<ITopology5Source*>& Sources();
    virtual Topology5Group* Parent();
    ITopology3Group& Group();

    virtual void EvaluateSenders();
    virtual void EvaluateSources();
    virtual TBool AddIfIsChild(Topology5Group& aGroup);
    virtual void SetSourceIndex(TUint aValue);

    // IWatcher<TUint>
    virtual void ItemOpen(const Brx& aId, TUint aValue);
    virtual void ItemUpdate(const Brx& aId, TUint aValue, TUint aPrevious);
    virtual void ItemClose(const Brx& aId, TUint aValue);

    // IWatcher<Brn>
    virtual void ItemOpen(const Brx& aId, Brn aValue);
    virtual void ItemUpdate(const Brx& aId, Brn aValue, Brn aPrevious);
    virtual void ItemClose(const Brx& aId, Brn aValue);


private:
    void SetParent(Topology5Group& aGroup);
    void SetParent(Topology5Group& aGroup, TUint aIndex);
    ITopology5Source* EvaluateSource();
    void EvaluateSourceFromChild();
    void EvaluateSendersFromChild();
    void CreateCallback(ServiceCreateData* aSender);

private:
    INetwork& iNetwork;
    Brn iRoomName;
    Brn iName;
    ITopology3Group* iGroup;
    ITopology5Source* iCurrentSource;
    //ILog& iLog;
    TBool iDisposed;

    TUint iParentSourceIndex;
    Topology5Group* iParent;
    IProxySender* iSender;
    TBool iHasSender;
    TUint iSourceIndex;

    std::vector<Topology5Group*> iChildren;
    std::vector<Topology5Source*> iSources;
    std::vector<ITopology5Source*> iVisibleSources;
    std::vector<ITopology5Group*>* iVectorSenders; // added in ohTopologyC
    std::vector<ITopology5Group*>* iCurrentVolumes;

    Watchable<ITopology5Source*>* iWatchableSource;
    Watchable<std::vector<ITopology5Group*>*>* iSenders;
};

/////////////////////////////////////////////////////////////////////

class Topology5GroupWatcher : public ITopology3GroupWatcher,  public INonCopyable
{
public:
    Topology5GroupWatcher(Topology5Room& aRoom, ITopology3Group& aGroup);
    virtual void Dispose();
    virtual Brn Name();
    virtual std::vector<ITopology2Source*>& Sources();

    // ITopology3GroupWatcher
    virtual void ItemOpen(const Brx& aId, Brn aName);
    virtual void ItemUpdate(const Brx& aId, Brn aName, Brn aPreviousName);
    virtual void ItemClose(const Brx& aId, Brn aName);
    virtual void ItemOpen(const Brx& aId, ITopology2Source* aSource);
    virtual void ItemUpdate(const Brx& aId, ITopology2Source* aSource, ITopology2Source* aPreviousSource);
    virtual void ItemClose(const Brx& aId, ITopology2Source* aSource);

private:
    Topology5Room& iRoom;
    ITopology3Group& iGroup;
    Brn iName;
    std::vector<ITopology2Source*> iSources;

};

/////////////////////////////////////////////////////////////////////

class ITopology5Room
{
public:
    virtual Brn Name() = 0;
    virtual IWatchable<EStandby>& Standby() = 0;
    virtual IWatchable<std::vector<ITopology5Root*>*>& Roots() = 0;
    virtual IWatchable<std::vector<ITopology5Source*>*>& Sources() = 0;
    virtual void SetStandby(TBool aValue) = 0;
    virtual ~ITopology5Room() {}
};

/////////////////////////////////////////////////////////////////////

class Topology5Room : public ITopology5Room, public IWatcherUnordered<ITopology3Group*>, public IWatcher<TBool>, public IDisposable, public INonCopyable
{
friend class Topology5GroupWatcher;

public:
    Topology5Room(INetwork& aNetwork, ITopology4Room& aRoom, ILog& aLog);
    ~Topology5Room();

    virtual void Dispose();
    virtual Brn Name();
    virtual void SetStandby(TBool aValue);

    virtual IWatchable<EStandby>& Standby();
    virtual IWatchable<std::vector<ITopology5Root*>*>& Roots();
    virtual IWatchable<std::vector<ITopology5Source*>*>& Sources();

    // IWatcherUnordered<ITopology3Group*>
    virtual void UnorderedOpen();
    virtual void UnorderedInitialised();
    virtual void UnorderedClose();
    virtual void UnorderedAdd(ITopology3Group* aItem);
    virtual void UnorderedRemove(ITopology3Group* aItem);

    // IWatcher<TBool>
    virtual void ItemOpen(const Brx& aId, TBool aValue);
    virtual void ItemUpdate(const Brx& aId, TBool aValue, TBool aPrevious);
    virtual void ItemClose(const Brx& aId, TBool aValue);

private:
    void EvaluateStandby();
    void InsertIntoTree(Topology5Group& aGroup);
    void CreateTree();   // made private in ohTopologyC

private:
    INetwork& iNetwork;
    ITopology4Room& iRoom;

    ILog& iLog;

    Brn iName;
    TUint iStandbyCount;
    EStandby iStandby;

    std::vector<ITopology5Root*>* iCurrentRoots; // added in ohTopologyC
    std::vector<ITopology5Source*>* iCurrentSources; // added in ohTopologyC

    Watchable<EStandby>* iWatchableStandby;
    Watchable<std::vector<ITopology5Root*>*>* iWatchableRoots;
    Watchable<std::vector<ITopology5Source*>*>* iWatchableSources;

    std::vector<ITopology3Group*> iT3Groups;
    std::vector<Topology5Group*> iGroups;
    std::vector<Topology5Group*> iRoots;
};

/////////////////////////////////////////////////////////////////////

class ITopology5
{
public:
    virtual IWatchableUnordered<ITopology5Room*>& Rooms() = 0;
    virtual INetwork& Network() = 0;
    virtual ~ITopology5() {}
};

/////////////////////////////////////////////////////////////////////

class Topology5 : public ITopology5, public IWatcherUnordered<ITopology4Room*>, public IDisposable, public INonCopyable
{
public:
    Topology5(ITopology4* aTopology4, ILog& aLog);
    ~Topology5();

    static Topology5* CreateTopology5(INetwork* aNetwork, ILog& aLog);

    virtual IWatchableUnordered<ITopology5Room*>& Rooms();

    virtual void Dispose();
    virtual INetwork& Network();

    // IWatcherUnordered<ITopology4Room*>
    virtual void UnorderedOpen();
    virtual void UnorderedInitialised();
    virtual void UnorderedClose();
    virtual void UnorderedAdd(ITopology4Room* aT4Room);
    virtual void UnorderedRemove(ITopology4Room* aT4Room);

private:
    void WatchT4Rooms(void*);
    void DisposeCallback(void*);
    void UnorderedAddCallback(void* aT4Room);
    void UnorderedRemoveCallback(void* aT4Room);


private:
    ITopology4* iTopology4;
    ILog& iLog;
    INetwork& iNetwork;
    DisposeHandler* iDisposeHandler;
    WatchableUnordered<ITopology5Room*>* iRooms;
    std::map<ITopology4Room*, Topology5Room*> iRoomLookup;
};

} // Av
} // OpenHome

#endif // HEADER_TOPOLOGY5

