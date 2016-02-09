#pragma once

#include <OpenHome/IWatcher.h>
#include <OpenHome/IWatchable.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/Network.h>
#include <OpenHome/ServiceProduct.h>
#include <OpenHome/Device.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/Topology5.h>
#include <OpenHome/Media.h>

#include <vector>
#include <map>

namespace OpenHome
{
namespace Topology
{
class ITopologySource;
class Topology6Group;
class Topology6Room;


/////////////////////////////////////////////////////////////////////

class ITopologyGroup
{
public:
    virtual const Brx& Name() const = 0;
    virtual IDevice& Device() const = 0;
    virtual IWatchable<ISender*>& Sender() const = 0;
    virtual IWatchable<ITopologySource*>& GroupSource() const = 0;
    virtual const std::vector<ITopologySource*>& GroupSources() const = 0;

    virtual TBool HasVolume() const = 0;
    virtual TBool HasInfo() const = 0;
    virtual TBool HasTime() const = 0;

    virtual ~ITopologyGroup() {}
};

/////////////////////////////////////////////////////////////////////


/*
  public interface ITopologySource
     {
         ITopologyGroup Group { get; }

         uint Index { get; }
         string Name { get; }
         string Type { get; }

         IMediaPreset CreatePreset();
         void Create(string aId, Action<ICredentialsSubscription> aCallback);

         IEnumerable<ITopologyGroup> Volumes { get; }
         //IDevice InfoDevice { get; }
         //IDevice TimeDevice { get; }
         IDevice Device { get; }
         bool HasInfo { get; }
         bool HasTime { get;
*/


class  ITopologySource
{
public:

    virtual TUint Index() const = 0;
    virtual const Brx& Name() const = 0;
    virtual const Brx& Type() const = 0;
    virtual std::vector<ITopologyGroup*>& Volumes() const = 0;
    virtual IDevice& Device() const = 0;
    virtual TBool HasInfo() const = 0;
    virtual TBool HasTime() const = 0;
    virtual ITopologyGroup& Group() const = 0;
    virtual IMediaPreset* CreatePreset() = 0;

    virtual ~ITopologySource() {}
};

/////////////////////////////////////////////////////////////////////

class Topology6Source : public ITopologySource, public INonCopyable
{
    friend class Topology6Group;

public:
    Topology6Source(INetwork& aNetwork, Topology6Group& aGroup, ITopology4Source& aSource, TBool aHasInfo, TBool aHasTime);

    // ITopologySource
    TUint Index() const override;
    const Brx& Name() const override;
    const Brx& Type() const override;
    std::vector<ITopologyGroup*>& Volumes() const override;
    IDevice& Device() const override;
    TBool HasInfo() const override;
    TBool HasTime() const override;
    IMediaPreset* CreatePreset() override;
    ITopologyGroup& Group() const override;

    TBool Visible() const;
    void Select();


private:
    virtual void SetVolumes(std::vector<ITopologyGroup*>* aVolumes);

private:
    INetwork& iNetwork;
    Topology6Group& iGroup;
    ITopology4Source& iSource;
    Brn iSourceName;
    Brn iSourceType;
    std::vector<ITopologyGroup*>* iVolumes;
    TBool iHasInfo;
    TBool iHasTime;
};

///////////////////////////////////////////////////////////////

class MediaPresetExternal : public IMediaPreset, public IWatcher<ITopologySource*>, public INonCopyable
{
    friend class IWatcher<ITopologySource*>;

public:
    MediaPresetExternal(IWatchableThread& aThread, Topology6Group& aGroup, TUint aIndex, IMediaMetadata* aMetadata, Topology6Source& aSource);

    void Dispose() override;
    TUint Index() override;
    IMediaMetadata& Metadata() override;
    IWatchable<TBool>& Buffering() override;
    IWatchable<TBool>& Playing() override;
    IWatchable<TBool>& Selected() override;
    void Play() override;

private:
    void ItemOpen(const Brx& aId, ITopologySource* aValue) override;
    void ItemUpdate(const Brx& aId, ITopologySource* aValue, ITopologySource* aPrevious) override;
    void ItemClose(const Brx& aId, ITopologySource* aValue) override;

private:
    TUint iIndex;
    IMediaMetadata* iMetadata;
    Topology6Source* iSource;
    Topology6Group& iGroup;
    Watchable<TBool>* iBuffering;
    Watchable<TBool>* iPlaying;
    Watchable<TBool>* iSelected;
};


/////////////////////////////////////////////////////////////////////

class ITopologyRoot : public ITopologyGroup
{
public:
    virtual IWatchable<ITopologySource*>& Source() const = 0;
    virtual const std::vector<ITopologySource*>& Sources() const = 0;
    virtual IWatchable<std::vector<ITopologyGroup*>*>& Senders() const = 0;
    virtual ~ITopologyRoot() {}
};

/////////////////////////////////////////////////////////////////////

class Topology6Group : public ITopologyRoot, public IWatcher<TUint>, public IWatcher<Brn>, public IDisposable, public INonCopyable
{
    friend class Topology6Room;
    friend class IWatcher<TUint>;
    friend class IWatcher<Brn>;

public:
    Topology6Group(INetwork& aNetwork, const Brx& aRoom, const Brx& aName, ITopology4Group& aGroup, std::vector<ITopology4Source*> aSources);
    ~Topology6Group();

    Topology6Group* Parent() const;
    void SetSourceIndex(TUint aValue);

    // ITopologyGroup
    const Brx& Name() const override;
    IDevice& Device() const override;
    IWatchable<ISender*>& Sender() const override;
    IWatchable<ITopologySource*>& GroupSource() const override;
    const std::vector<ITopologySource*>& GroupSources() const override;
    TBool HasVolume() const override;
    TBool HasInfo() const override;
    TBool HasTime() const override;

    // IDisposable
    void Dispose() override;

    // ITopologyRoot
    IWatchable<ITopologySource*>& Source() const override;
    const std::vector<ITopologySource*>& Sources() const override;
    IWatchable<std::vector<ITopologyGroup*>*>& Senders() const override;

private:
    Brn RoomName();
    Brn ModelName();
    Brn ManufacturerName();
    Brn ProductId();
    Brn ProductImageUri();
    Brn Attributes();

    void EvaluateSenders();
    void EvaluateSources();
    TBool AddIfIsChild(Topology6Group& aGroup);
    ITopology4Group& Group();

    void SetParent(Topology6Group& aGroup);
    void SetParent(Topology6Group& aGroup, TUint aIndex);
    ITopologySource* EvaluateSource();
    void EvaluateSourceFromChild();
    void EvaluateSendersFromChild();
    void CreateCallback(IProxy* aProxy);

    // IWatcher<TUint>
    void ItemOpen(const Brx& aId, TUint aValue) override;
    void ItemUpdate(const Brx& aId, TUint aValue, TUint aPrevious) override;
    void ItemClose(const Brx& aId, TUint aValue) override;

    // IWatcher<Brn>
    void ItemOpen(const Brx& aId, Brn aValue) override;
    void ItemUpdate(const Brx& aId, Brn aValue, Brn aPrevious) override;
    void ItemClose(const Brx& aId, Brn aValue) override;

private:
    INetwork& iNetwork;
    Brn iRoomName;
    Brn iName;
    ITopology4Group& iGroup;
    ITopologySource* iCurrentSource;
    ITopologySource* iCurrentGroupSource;
    TBool iDisposed;

    TUint iParentSourceIndex;
    Topology6Group* iParent;
    IProxySender* iSenderService;
    TBool iHasSender;
    TUint iSourceIndex;

    std::vector<Topology6Group*> iChildren;
    std::vector<Topology6Source*> iSources;
    std::vector<ITopologySource*> iVisibleSources;
    std::vector<ITopologySource*> iVisibleGroupSources;
    std::vector<ITopologyGroup*>* iVectorSenders; // added in ohTopologyC
    std::vector<ITopologyGroup*>* iCurrentVolumes;

    Watchable<ITopologySource*>* iWatchableSource;
    Watchable<ITopologySource*>* iWatchableGroupSource;
    Watchable<std::vector<ITopologyGroup*>*>* iSenders;
};

/////////////////////////////////////////////////////////////////////

class ITopology4GroupWatcher : public IWatcher<Brn>, public IWatcher<ITopology4Source*>, public IDisposable
{
public:
    virtual void Dispose() override = 0;
    virtual Brn Name() = 0;
    virtual ITopology4Group& Group() = 0;

    virtual const std::vector<ITopology4Source*>& Sources() = 0 ;

    // IWatcher<Brn>
    virtual void ItemOpen(const Brx& aId, Brn aValue) override = 0;
    virtual void ItemUpdate(const Brx& aId, Brn aValue, Brn aPrevious) override = 0;
    virtual void ItemClose(const Brx& aId, Brn aValue) override = 0;

    // IWatcher<ITopology4Source*>
    virtual void ItemOpen(const Brx& aId, ITopology4Source* aValue) override = 0;
    virtual void ItemUpdate(const Brx& aId, ITopology4Source* aValue, ITopology4Source* aPrevious) override = 0;
    virtual void ItemClose(const Brx& aId, ITopology4Source* aValue) override = 0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////

class Topology6GroupWatcher : public ITopology4GroupWatcher,  public INonCopyable
{
public:
    Topology6GroupWatcher(Topology6Room& aRoom, ITopology4Group& aGroup);
    virtual void Dispose() override;
    virtual Brn Name() override;
    virtual ITopology4Group& Group() override;
    virtual const std::vector<ITopology4Source*>& Sources() override;

private:
    // ITopology4GroupWatcher
    void ItemOpen(const Brx& aId, Brn aName) override;
    void ItemUpdate(const Brx& aId, Brn aName, Brn aPreviousName) override;
    void ItemClose(const Brx& aId, Brn aName) override;
    void ItemOpen(const Brx& aId, ITopology4Source* aSource) override;
    void ItemUpdate(const Brx& aId, ITopology4Source* aSource, ITopology4Source* aPreviousSource) override;
    void ItemClose(const Brx& aId, ITopology4Source* aSource) override;

private:
    Topology6Room& iRoom;
    ITopology4Group& iGroup;
    Brn iName;
    std::vector<ITopology4Source*> iSources;

};

/////////////////////////////////////////////////////////////////////

class ITopologyRoom
{
public:
    virtual const Brx& Name() const = 0;
    virtual IWatchable<EStandby>& Standby() const = 0;
    virtual IWatchable<std::vector<ITopologyRoot*>*>& Roots() const = 0;
    virtual IWatchable<std::vector<ITopologySource*>*>& Sources() const = 0;
    virtual IWatchable<std::vector<ITopologyGroup*>*>& Groups() const = 0;
    virtual void SetStandby(TBool aValue) = 0;
    virtual ~ITopologyRoom() {}
};

/////////////////////////////////////////////////////////////////////

class Topology6Room : public ITopologyRoom, public IWatcherUnordered<ITopology4Group*>, public IWatcher<TBool>, public IDisposable, public INonCopyable
{
    friend class Topology6GroupWatcher;
    friend class IWatcherUnordered<ITopology4Group*>;
    friend class IWatcher<TBool>;

public:
    Topology6Room(INetwork& aNetwork, ITopology5Room& aRoom);
    ~Topology6Room();

    virtual void Dispose() override;
    virtual const Brx& Name() const override;
    virtual void SetStandby(TBool aValue) override;

    virtual IWatchable<EStandby>& Standby() const override;
    virtual IWatchable<std::vector<ITopologyRoot*>*>& Roots() const override;
    virtual IWatchable<std::vector<ITopologySource*>*>& Sources() const override;
    virtual IWatchable<std::vector<ITopologyGroup*>*>& Groups() const override;

private:
    // IWatcherUnordered<ITopology4Group*>
    void UnorderedOpen() override;
    void UnorderedInitialised() override;
    void UnorderedClose() override;
    void UnorderedAdd(ITopology4Group* aItem) override;
    void UnorderedRemove(ITopology4Group* aItem) override;

    // IWatcher<TBool>
    void ItemOpen(const Brx& aId, TBool aValue) override;
    void ItemUpdate(const Brx& aId, TBool aValue, TBool aPrevious) override;
    void ItemClose(const Brx& aId, TBool aValue) override;

    void EvaluateStandby();
    void InsertIntoTree(Topology6Group& aGroup);
    void CreateTree();   // made private in ohTopologyC
    void WatchT5Rooms(void*);

private:
    INetwork& iNetwork;
    ITopology5Room& iRoom;

    Brn iName;
    TUint iStandbyCount;
    EStandby iStandby;

    std::vector<ITopologyRoot*>* iCurrentRoots; // added in ohTopologyC
    std::vector<ITopologySource*>* iCurrentSources; // added in ohTopologyC
    std::vector<ITopologyGroup*>* iCurrentGroups; // added in ohTopologyC

    Watchable<EStandby>* iWatchableStandby;
    Watchable<std::vector<ITopologyRoot*>*>* iWatchableRoots;
    Watchable<std::vector<ITopologySource*>*>* iWatchableSources;
    Watchable<std::vector<ITopologyGroup*>*>* iWatchableGroups;

    std::vector<Topology6Group*> iGroups;
    std::vector<Topology6Group*> iRoots;

    std::vector<ITopology4GroupWatcher*> iGroupWatchers;
};

/////////////////////////////////////////////////////////////////////

class ITopology6 : public IDisposable
{
public:
    virtual IWatchableUnordered<ITopologyRoom*>& Rooms() const = 0;
    virtual INetwork& Network() const = 0;
    virtual void Dispose()  override = 0;
    virtual ~ITopology6() {}
};

/////////////////////////////////////////////////////////////////////

class Topology6 : public ITopology6, public IWatcherUnordered<ITopology5Room*>, public INonCopyable
{
    friend class IWatcherUnordered<ITopology5Room*>;

public:
    Topology6(ITopology5* aTopology5);
    ~Topology6();

    virtual IWatchableUnordered<ITopologyRoom*>& Rooms() const override;
    virtual void Dispose() override;
    virtual INetwork& Network() const override;

    static Topology6* CreateTopology(INetwork& aNetwork);

private:
    // IWatcherUnordered<ITopology5Room*>
    virtual void UnorderedOpen() override;
    virtual void UnorderedInitialised() override;
    virtual void UnorderedClose() override;
    virtual void UnorderedAdd(ITopology5Room* aT5Room) override;
    virtual void UnorderedRemove(ITopology5Room* aT5Room) override;

    void WatchT5Rooms(void*);
    void DisposeCallback(void*);
    void UnorderedAddCallback(void* aT5Room);
    void UnorderedRemoveCallback(void* aT5Room);

private:
    ITopology5* iTopology5;
    INetwork& iNetwork;
    DisposeHandler* iDisposeHandler;
    WatchableUnordered<ITopologyRoom*>* iRooms;
    std::map<ITopology5Room*, Topology6Room*> iRoomLookup;
};

} // Topology
} // OpenHome
