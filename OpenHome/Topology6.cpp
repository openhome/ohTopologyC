#include <OpenHome/Topology6.h>

#include <map>
#include <algorithm>


using namespace OpenHome;
using namespace OpenHome::Topology;
using namespace std;



MediaPresetExternal::MediaPresetExternal(IWatchableThread& aThread, Topology6Group& aGroup, TUint aIndex, IMediaMetadata* aMetadata, Topology6Source& aSource)
    :iIndex(aIndex)
    ,iMetadata(aMetadata)
    ,iSource(&aSource)
    ,iGroup(aGroup)
    ,iBuffering(new Watchable<TBool>(aThread, Brn("Buffering"), false))
    ,iPlaying(new Watchable<TBool>(aThread, Brn("Playing"), false))
    ,iSelected(new Watchable<TBool>(aThread, Brn("Selected"), false))
{
    iGroup.Source().AddWatcher(*this);
}


void MediaPresetExternal::Dispose()
{
    iGroup.Source().RemoveWatcher(*this);
    iBuffering->Dispose();
    iPlaying->Dispose();
    iSelected->Dispose();

}


TUint MediaPresetExternal::Index()
{
    return iIndex;
}


IMediaMetadata& MediaPresetExternal::Metadata()
{
    return *iMetadata;
}


IWatchable<TBool>& MediaPresetExternal::Buffering()
{
    return *iBuffering;
}


IWatchable<TBool>& MediaPresetExternal::Playing()
{
    return *iPlaying;
}


IWatchable<TBool>& MediaPresetExternal::Selected()
{
    return *iSelected;
}


void MediaPresetExternal::Play()
{
    if (!iPlaying->Value())
    {
        iBuffering->Update(true);
    }
    iSource->Select();
}


void MediaPresetExternal::ItemOpen(const Brx& /*aId*/, ITopologySource* aValue)
{
    iBuffering->Update(false);
    iPlaying->Update(aValue == iSource);
    iSelected->Update(aValue == iSource);
}


void MediaPresetExternal::ItemUpdate(const Brx& /*aId*/, ITopologySource* aValue, ITopologySource* /*aPrevious*/)
{
    iBuffering->Update(false);
    iPlaying->Update(aValue == iSource);
    iSelected->Update(aValue == iSource);
}


void MediaPresetExternal::ItemClose(const Brx& /*aId*/, ITopologySource* /*aValue*/)
{
    iBuffering->Update(false);
    iPlaying->Update(false);
    iSelected->Update(false);
}

///////////////////////////////////////////////////////
/*
ITopologyGroup& Topology6SourceNull::Group()
{
    THROW(NotImplementedException);
}


TUint Topology6SourceNull::Index()
{
    THROW(NotImplementedException);
}


Brn Topology6SourceNull::Name()
{
    THROW(NotImplementedException);
}


Brn Topology6SourceNull::Type()
{
    THROW(NotImplementedException);
}


TBool Topology6SourceNull::Visible()
{
    THROW(NotImplementedException);
}


IMediaPreset* Topology6SourceNull::CreatePreset()
{
    THROW(NotImplementedException);
}


std::vector<ITopologyGroup*>& Topology6SourceNull::Volumes()
{
    THROW(NotImplementedException);
}


IDevice& Topology6SourceNull::Device()
{
    THROW(NotImplementedException);
}


TBool Topology6SourceNull::HasInfo()
{
    THROW(NotImplementedException);
}


TBool Topology6SourceNull::HasTime()
{
    THROW(NotImplementedException);
}


void Topology6SourceNull::Select()
{
    THROW(NotImplementedException);
}
*/

///////////////////////////////////////////////////

Topology6Source::Topology6Source(INetwork& aNetwork, Topology6Group& aGroup, ITopology4Source& aSource, TBool aHasInfo, TBool aHasTime)
    :iNetwork(aNetwork)
    ,iGroup(aGroup)
    ,iSource(aSource)
		, iSourceName(aSource.Name())
		, iSourceType(aSource.Type())
    ,iVolumes(NULL)
    ,iHasInfo(aHasInfo)
    ,iHasTime(aHasTime)
{

}


TUint Topology6Source::Index() const
{
    return iSource.Index();
}


const Brx& Topology6Source::Name() const
{
    return iSourceName;
}


ITopologyGroup& Topology6Source::Group() const
{
    return iGroup;
}


const Brx& Topology6Source::Type() const
{
    return iSourceType;
}


TBool Topology6Source::Visible() const
{
    return iSource.Visible();
}


IMediaPreset* Topology6Source::CreatePreset()
{
    // create some metadata
    MediaMetadata* metadata = new MediaMetadata();
    // add a tag whose title matches my source name
    metadata->Add(iNetwork.GetTagManager().Audio().Title(), iSource.Name());

    // add a tag whose title matches my source name with an external:// prefix
    Bwh extSrcName;
    extSrcName.Replace(Brn("external://"));
    extSrcName.Append(iSource.Name());
    metadata->Add(iNetwork.GetTagManager().Audio().Artwork(), extSrcName);

    // get the root group of this group
    Topology6Group* group = &iGroup;
    while (group->Parent() != NULL)
    {
        group = group->Parent();
    }

    return (new MediaPresetExternal(iNetwork, *group, iSource.Index(), metadata, *this));
}


std::vector<ITopologyGroup*>& Topology6Source::Volumes() const
{
    return *iVolumes;
}


void Topology6Source::SetVolumes(std::vector<ITopologyGroup*>* aVolumes)
{
    iVolumes = aVolumes;
}


IDevice& Topology6Source::Device() const
{
    return(iGroup.Device());
}


TBool Topology6Source::HasInfo() const
{
    return iHasInfo;
}


TBool Topology6Source::HasTime() const
{
    return iHasTime;
}


void Topology6Source::Select()
{
    iGroup.SetSourceIndex(iSource.Index());
}

//////////////////////////////////////////////////////////////////

Topology6Group::Topology6Group(INetwork& aNetwork, const Brx& aRoomName, const Brx& aName, ITopology4Group& aGroup, std::vector<ITopology4Source*> aSources, ILog& /*aLog*/)
    :iNetwork(aNetwork)
    ,iRoomName(aRoomName)
    ,iName(aName)
    ,iGroup(aGroup)
    ,iCurrentSource(NULL)
    ,iCurrentGroupSource(NULL)
    ,iDisposed(false)
    ,iParent(NULL)
    ,iSenderService(NULL)
    ,iHasSender(false)
    ,iVectorSenders(new vector<ITopologyGroup*>())
    ,iCurrentVolumes(NULL)
    ,iWatchableSource(new Watchable<ITopologySource*>(iNetwork, Brn("source"), iCurrentSource))
    ,iWatchableGroupSource(new Watchable<ITopologySource*>(iNetwork, Brn("groupSource"), iCurrentGroupSource))
    ,iSenders(new Watchable<vector<ITopologyGroup*>*>(iNetwork, Brn("senders"), iVectorSenders))
{
    TBool hasInfo = Ascii::Contains(iGroup.Attributes(), Brn("Info"));
    TBool hasTime = (Ascii::Contains(iGroup.Attributes(), Brn("Time")) && hasInfo);

    for(TUint i=0; i<aSources.size(); i++)
    {
        auto source = new Topology6Source(aNetwork, *this, *(aSources[i]), hasInfo, hasTime);
        iSources.push_back(source);
        if (source->Visible())
        {
            iVisibleGroupSources.push_back(source);
        }
    }

    iGroup.SourceIndex().AddWatcher(*this);

    if (Ascii::Contains(iGroup.Attributes(), Brn("Sender")))
    {
        FunctorGeneric<ServiceCreateData*> f  = MakeFunctorGeneric(*this, &Topology6Group::CreateCallback);
        iGroup.Device().Create(f, eProxySender);
    }

}


Topology6Group::~Topology6Group()
{
    delete iWatchableGroupSource;
    delete iWatchableSource;
    delete iSenderService;
    delete iSenders;
    for(TUint i=0; i<iSources.size(); i++)
    {
        delete iSources[i];
    }

    delete iVectorSenders;
    delete iCurrentVolumes;

}


void Topology6Group::CreateCallback(ServiceCreateData* aData)
{
    IProxySender* senderService = (IProxySender*)aData->iProxy;
    delete aData;

    if (!iDisposed)
    {
        iSenderService = senderService;
        iSenderService->Status().AddWatcher(*this);
    }
    else
    {
        senderService->Dispose();
        delete senderService;
    }
}


void Topology6Group::Dispose()
{
    iGroup.SourceIndex().RemoveWatcher(*this);

    iWatchableSource->Dispose();

    iWatchableGroupSource->Dispose();

    if (iSenderService != NULL)
    {
        iSenderService->Status().RemoveWatcher(*this);
        iSenderService->Dispose();
        iHasSender = false;
    }

    iSenders->Dispose();

    iDisposed = true;
}


const Brx& Topology6Group::Name() const
{
    return iName;
}


IDevice& Topology6Group::Device() const
{
    return iGroup.Device();
}


Brn Topology6Group::RoomName()
{
    return iRoomName;
}


Brn Topology6Group::ModelName()
{
    return iGroup.ModelName();
}


Brn Topology6Group::ManufacturerName()
{
    return iGroup.ManufacturerName();
}


Brn Topology6Group::ProductId()
{
    return iGroup.ProductId();
}

Brn Topology6Group::ProductImageUri()
{
    return iGroup.ProductImageUri();
}

Brn Topology6Group::Attributes()
{
    return iGroup.Attributes();
}

IWatchable<ISender*>& Topology6Group::Sender() const
{
    return iGroup.Sender();
}

IWatchable<ITopologySource*>& Topology6Group::Source() const
{
    ASSERT(!iDisposed);
    return *iWatchableSource;
}


TBool Topology6Group::HasVolume() const
{
    return Ascii::Contains(iGroup.Attributes(), Brn("Volume"));
}

TBool Topology6Group::HasInfo() const
{
    return Ascii::Contains(iGroup.Attributes(), Brn("Info"));
}

TBool Topology6Group::HasTime() const
{
    return Ascii::Contains(iGroup.Attributes(), Brn("Time"));
}

ITopology4Group& Topology6Group::Group()
{
    return(iGroup);
}


void Topology6Group::EvaluateSources()
{
    // build list of all volume groups (groups with Volume service attribute)
    auto volumes = new vector<ITopologyGroup*>();
    // traverse UP the tree adding(insert) any groups that have Volume
    // starting with this group, then my parent group, then my parent's group etc
    Topology6Group* group = this;

    while (group != NULL)
    {
        if(Ascii::Contains(group->Group().Attributes(), Brn("Volume")))
        {
            volumes->insert(volumes->begin(), group);
        }

        group = group->Parent();
    }
    auto oldVolumes = iCurrentVolumes;
    iCurrentVolumes = volumes;
    delete oldVolumes;


    // trigger source evaluation on all my child groups
    for(TUint i=0; i<iChildren.size(); i++)
    {
        auto group = iChildren[i];
        group->EvaluateSources();
    }


    // Build list of Visible sources:
    // Expand sources to those of any child groups
    // For non expanded sources: remove if not Visible
    for (TUint i= 0; i<iSources.size(); i++)
    {
        Topology6Source* s = iSources[i];
        s->SetVolumes(volumes); // update all my sources with my Volume attribute status

        TBool expanded = false;

        for(TUint j=0; j<iChildren.size(); j++)
        {
            auto g = iChildren[j];

            // if group is connected to source expand source to group sources
            if (s->Name() == g->Name())
            {
                iVisibleSources.insert(iVisibleSources.end(), g->Sources().begin(), g->Sources().end());
                expanded = true;
            }
        }

        if (!expanded)
        {
            // only include if source is visible
            if (s->Visible())
            {
                iVisibleSources.push_back(s);
            }
        }
    }


    ITopologySource* source = EvaluateSource();
    auto oldSource = iCurrentSource;
    iCurrentSource = source;
    iWatchableSource->Update(iCurrentSource);
    delete oldSource;
}


const std::vector<ITopologySource*>& Topology6Group::Sources() const
{
    return iVisibleSources;
}



IWatchable<ITopologySource*>& Topology6Group::GroupSource() const
{
    return (*iWatchableGroupSource);
}

const std::vector<ITopologySource*>& Topology6Group::GroupSources() const
{
    return iVisibleGroupSources;
}

void Topology6Group::EvaluateSenders()
{
    for(TUint i=0; i<iChildren.size(); i++)
    {
        auto group = iChildren[i];
        group->EvaluateSenders();
    }

    vector<ITopologyGroup*>* vectorSenders = new vector<ITopologyGroup*>();

    for(TUint i=0; i<iChildren.size(); i++)
    {
        auto* group = iChildren[i];
        auto v = group->Senders().Value();
        vectorSenders->insert(vectorSenders->end(), v->begin(), v->end());
    }

    if (iHasSender)
    {
        vectorSenders->insert(vectorSenders->begin(), this);
    }

    auto oldVectorSenders = iVectorSenders;
    iVectorSenders = vectorSenders;
    iSenders->Update(iVectorSenders);
    delete oldVectorSenders;
}


IWatchable<std::vector<ITopologyGroup*>*>& Topology6Group::Senders() const
{
    return(*iSenders);
}


Topology6Group* Topology6Group::Parent() const
{
    return iParent;
}


TBool Topology6Group::AddIfIsChild(Topology6Group& aGroup)
{
    // aGroup.Name = name of one of my sources

    // If one of my sources has the same name as specified group:
    // I am the parent of that group, that group is a child of mine
    // My children are my sources

    for(TUint i=0; i<iSources.size(); i++)
    {
        Topology6Source* s = iSources[i];
        if (aGroup.Name() == s->Name())
        {
            aGroup.SetParent(*this, s->Index());
            iChildren.push_back(&aGroup);
            return true;
        }
    }

    return false;
}


void Topology6Group::SetParent(Topology6Group& aGroup)
{
    iParent = &aGroup;
}


void Topology6Group::SetParent(Topology6Group& aGroup, TUint aIndex)
{
    SetParent(aGroup);
    iParentSourceIndex = aIndex;
}


void Topology6Group::ItemOpen(const Brx& /*aId*/, TUint aValue)
{
    iSourceIndex = aValue;
}


void Topology6Group::ItemUpdate(const Brx& /*aId*/, TUint aValue, TUint /*aPrevious*/)
{
   iSourceIndex = aValue;
   EvaluateSourceFromChild();
}


void Topology6Group::ItemClose(const Brx& /*aId*/, TUint /*aValue*/)
{
}


ITopologySource* Topology6Group::EvaluateSource()
{

    // return the currently active source object.
    Topology6Source* source = iSources[iSourceIndex];

    // if the source has a child get the child's active source instead
    for(TUint i=0; i<iChildren.size(); i++)
    {
        Topology6Group* g = iChildren[i];
        if (g->Name() == source->Name())
        {
            return g->EvaluateSource();
        }
    }

    return(source);
}


void Topology6Group::EvaluateSourceFromChild()
{
    if (iParent != NULL)
    {
        iParent->EvaluateSourceFromChild();
    }

    ITopologySource* source = EvaluateSource();
    iWatchableSource->Update(source);
}


void Topology6Group::ItemOpen(const Brx& /*aId*/, Brn aValue)
{
    iHasSender = (aValue == Brn("Enabled"));
    EvaluateSendersFromChild();
}


void Topology6Group::ItemUpdate(const Brx& /*aId*/, Brn aValue, Brn /*aPrevious*/)
{
    iHasSender = (aValue == Brn("Enabled"));
    EvaluateSendersFromChild();
}


void Topology6Group::ItemClose(const Brx& /*aId*/, Brn /*aValue*/)
{
}


void Topology6Group::EvaluateSendersFromChild()
{
    if (iParent != NULL)
    {
        iParent->EvaluateSendersFromChild();
    }
    else
    {
        EvaluateSenders();
    }
}


void Topology6Group::SetSourceIndex(TUint aValue)
{
    if (iParent != NULL)
    {
        iParent->SetSourceIndex(iParentSourceIndex);
    }

    iGroup.SetSourceIndex(aValue);
}


//////////////////////////////////////////////////////////////////////////////

Topology6GroupWatcher::Topology6GroupWatcher(Topology6Room& aRoom, ITopology4Group& aGroup)
    :iRoom(aRoom)
    ,iGroup(aGroup)
{
    iGroup.Name().AddWatcher(*this); // watch group Name

    vector<Watchable<ITopology4Source*>*>& s = iGroup.Sources();

    for(TUint i=0; i<s.size(); i++)
    {
        s[i]->AddWatcher(*this); // watch each source
    }
}

void Topology6GroupWatcher::Dispose()
{
    // detach from all properties we are currently watching
    iGroup.Name().RemoveWatcher(*this);
    vector<Watchable<ITopology4Source*>*>& s = iGroup.Sources();

    for(TUint i=0; i<s.size(); i++)
    {
        s[i]->RemoveWatcher(*this);
    }
}


Brn Topology6GroupWatcher::Name()
{
    return iName;
}

ITopology4Group& Topology6GroupWatcher::Group()
{
    return(iGroup);
}


std::vector<ITopology4Source*>& Topology6GroupWatcher::Sources()
{
    return iSources;
}


void Topology6GroupWatcher::ItemOpen(const Brx& /*aId*/, Brn aName)
{
   iName.Set(aName);
}


void Topology6GroupWatcher::ItemUpdate(const Brx& /*aId*/, Brn aName, Brn /*aPreviousName*/)
{
    iName.Set(aName);
    iRoom.CreateTree();
}

void Topology6GroupWatcher::ItemClose(const Brx& /*aId*/, Brn /*aName*/)
{
}


void Topology6GroupWatcher::ItemOpen(const Brx& /*aId*/, ITopology4Source* aSource)
{
    iSources.push_back(aSource);
}


void Topology6GroupWatcher::ItemUpdate(const Brx& /*aId*/, ITopology4Source* aSource, ITopology4Source* aPreviousSource)
{
    auto it = find(iSources.begin(), iSources.end(), aPreviousSource);
    if (it!=iSources.end())
    {
        iSources[it-iSources.begin()] = aSource;
    }

    iRoom.CreateTree();
}


void Topology6GroupWatcher::ItemClose(const Brx& /*aId*/, ITopology4Source* /*aSource*/)
{
}


///////////////////////////////////////////////////////////////////


Topology6Room::Topology6Room(INetwork& aNetwork, ITopology5Room& aRoom, ILog& aLog)
    :iNetwork(aNetwork)
    ,iRoom(aRoom)
    ,iLog(aLog)
    ,iName(iRoom.Name())
    ,iStandbyCount(0)
    ,iStandby(eOff)
    ,iCurrentRoots(new vector<ITopologyRoot*>())
    ,iCurrentSources(new vector<ITopologySource*>())
    ,iCurrentGroups(new vector<ITopologyGroup*>())
    ,iWatchableStandby(new Watchable<EStandby>(iNetwork, Brn("standby"), eOff))
    ,iWatchableRoots(new Watchable<vector<ITopologyRoot*>*>(iNetwork, Brn("roots"), iCurrentRoots))
    ,iWatchableSources(new Watchable<vector<ITopologySource*>*>(iNetwork, Brn("sources"), iCurrentSources))
    ,iWatchableGroups(new Watchable<vector<ITopologyGroup*>*>(iNetwork, Brn("groups"), iCurrentGroups))
{
    iRoom.Groups().AddWatcher(*this);  // T4Groups with matching room name
}

Topology6Room::~Topology6Room()
{
    for(TUint i=0; i<iGroups.size() ;i++)
    {
        delete iGroups[i];
    }
    iGroups.clear();

    for(auto it=iGroupWatchers.begin(); it!=iGroupWatchers.end(); it++)
    {
        delete *it;
    }

    delete iCurrentRoots;
    delete iCurrentSources;
    delete iCurrentGroups;

    delete iWatchableRoots;
    delete iWatchableSources;
    delete iWatchableStandby;
    delete iWatchableGroups;

    iWatchableStandby = NULL;
    iWatchableRoots = NULL;
    iWatchableSources = NULL;
    iWatchableGroups = NULL;
}

void Topology6Room::Dispose()
{
    iRoom.Groups().RemoveWatcher(*this);

    for(auto it=iGroupWatchers.begin(); it!=iGroupWatchers.end(); it++)
    {
        auto groupWatcher = *it;
        ITopology4Group& t4Group = groupWatcher->Group();
        t4Group.Standby().RemoveWatcher(*this);
        groupWatcher->Dispose();

    }

    for(TUint i=0; i<iGroups.size() ;i++)
    {
        iGroups[i]->Dispose();
    }

    iWatchableStandby->Dispose();
    iWatchableRoots->Dispose();
    iWatchableSources->Dispose();
    iWatchableGroups->Dispose();
}


const Brx& Topology6Room::Name() const
{
    return iName;
}


IWatchable<EStandby>& Topology6Room::Standby() const
{
    return *iWatchableStandby;
}


IWatchable<std::vector<ITopologyRoot*>*>& Topology6Room::Roots() const
{
    return *iWatchableRoots;
}


IWatchable<std::vector<ITopologySource*>*>& Topology6Room::Sources() const
{
    return *iWatchableSources;
}

IWatchable<std::vector<ITopologyGroup*>*>& Topology6Room::Groups() const
{
    return *iWatchableGroups;
}

void Topology6Room::SetStandby(TBool aValue)
{
    iRoom.SetStandby(aValue);
}


void Topology6Room::UnorderedOpen()
{
}


void Topology6Room::UnorderedInitialised()
{
}


void Topology6Room::UnorderedClose()
{
}


void Topology6Room::UnorderedAdd(ITopology4Group* aT4Group)
{
    auto groupWatcher = new Topology6GroupWatcher(*this, *aT4Group);
    iGroupWatchers.push_back(groupWatcher);
    aT4Group->Standby().AddWatcher(*this);

    CreateTree();
}


void Topology6Room::UnorderedRemove(ITopology4Group* aT4Group)
{
    TBool groupFound = false;

    auto it = iGroupWatchers.begin();
    for(it = iGroupWatchers.begin(); it != iGroupWatchers.end(); it++)
    {
        auto groupWatcher = *it;
        ITopology4Group& t4Group = groupWatcher->Group();
        if (&t4Group == aT4Group)
        {
            iGroupWatchers.erase(it);
            groupWatcher->Dispose();
            delete groupWatcher;
            t4Group.Standby().RemoveWatcher(*this);
            groupFound = true;
            break;
        }
    }

    ASSERT(groupFound);

    if (iGroupWatchers.size() > 0)
    {
        CreateTree();
    }
}


void Topology6Room::CreateTree()
{
    vector<ITopologyGroup*>* newGroups = new vector<ITopologyGroup*>();
    vector<Topology6Group*> prevGroups(iGroups); // take snapshot for deletion

    // Rebuild iGroups and iRoots
    iGroups.clear();
    iRoots.clear();

    for(auto it=iGroupWatchers.begin(); it!=iGroupWatchers.end(); it++)
    {
        auto groupWatcher = *it;
        Topology6Group* t6Group = new Topology6Group(iNetwork, iName, groupWatcher->Name(), groupWatcher->Group(), groupWatcher->Sources(), iLog);

        InsertIntoTree(*t6Group);
        if (t6Group->ProductId() != Brx::Empty())
        {
            newGroups->push_back(t6Group);
        }
    }

    // Copy roots list to new list of ITopologyRoot
    // and build single list of sources from all roots.
    auto newRoots = new vector<ITopologyRoot*>();
    auto newSources = new vector<ITopologySource*>();

    for(TUint i=0; i<iRoots.size(); i++)
    {
        Topology6Group* t6Group = iRoots[i];
        newRoots->push_back(t6Group); // copy into ITopologyRoot list

        t6Group->EvaluateSources(); // refresh group's source list
        t6Group->EvaluateSenders(); // refresh group's sender list

        // Append all sources from this group to sources list
        auto gSources = t6Group->Sources();
        newSources->insert(newSources->end(), gSources.begin(), gSources.end());
    }

    auto oldRoots = iCurrentRoots;
    iCurrentRoots = newRoots;

    auto oldSources = iCurrentSources;
    iCurrentSources = newSources;

    auto oldGroups = iCurrentGroups;
    iCurrentGroups = newGroups;


    iWatchableRoots->Update(iCurrentRoots);
    iWatchableSources->Update(iCurrentSources);
    iWatchableGroups->Update(iCurrentGroups);

    delete oldRoots;
    delete oldSources;
    delete oldGroups;

    for(TUint i=0; i<prevGroups.size(); i++)
    {
        auto group = prevGroups[i];
        group->Dispose();
        delete group;
    }
}


void Topology6Room::InsertIntoTree(Topology6Group& aGroup)
{
    // if group is the first group found
    if (iGroups.size() == 0)
    {
        iGroups.push_back(&aGroup); // add as a group
        iRoots.push_back(&aGroup); // add as a root
        return;
    }

    // check for an existing parent
    //(aGroup is a child (source) of another group)
    for(TUint i=0; i<iGroups.size(); i++)
    {
        if (iGroups[i]->AddIfIsChild(aGroup))
        {
            iGroups.push_back(&aGroup); // add as a group
            return;
        }
    }

    // remove any roots that are children of mine
    // (root = not a child (source) of any other group)
    for(TUint i=0; i<iRoots.size(); i++)
    {
        Topology6Group* g = iRoots[i];
        if ( aGroup.AddIfIsChild(*g) )
        {
            iRoots.erase(iRoots.begin()+i);
            break;
        }
    }

    iGroups.push_back(&aGroup); // add as a group
    iRoots.push_back(&aGroup); // add as a root
}


void Topology6Room::ItemOpen(const Brx& /*aId*/, TBool aStandby)
{
    if (aStandby)
    {
        iStandbyCount++;
    }

    EvaluateStandby();
}


void Topology6Room::ItemUpdate(const Brx& /*aId*/, TBool aStandby, TBool /*aPreviousStandby*/)
{
    if (aStandby)
    {
        iStandbyCount++;
    }
    else
    {
        iStandbyCount--;
    }

    EvaluateStandby();
}


void Topology6Room::ItemClose(const Brx& /*aId*/, TBool aStandby)
{
    if (aStandby)
    {
        iStandbyCount--;
    }

    if (iGroupWatchers.size() > 0)
    {
        EvaluateStandby();
    }
}


void Topology6Room::EvaluateStandby()
{
    EStandby standby = eOff;

    if (iStandbyCount > 0)
    {
        standby = eMixed;

        if (iStandbyCount == iGroupWatchers.size())
        {
            standby = eOn;
        }
    }

    if (standby != iStandby)
    {
        iStandby = standby;
        iWatchableStandby->Update(standby);
    }
}

///////////////////////////////////////////////////////////////////////////////////

Topology6::Topology6(ITopology5* aTopology5, ILog& aLog)
    :iTopology5(aTopology5)
    ,iLog(aLog)
    ,iNetwork(iTopology5->Network())
    ,iDisposeHandler(new DisposeHandler())
    ,iRooms(new WatchableUnordered<ITopologyRoom*>(iNetwork))
{
    iNetwork.Schedule(MakeFunctorGeneric(*this, &Topology6::WatchT5Rooms), NULL);
}

Topology6::~Topology6()
{
    delete iDisposeHandler;
    delete iRooms;

    for(auto it=iRoomLookup.begin(); it!=iRoomLookup.end(); it++)
    {
        delete it->second;
    }

    iRoomLookup.clear();
    delete iTopology5;
}

Topology6* Topology6::CreateTopology(INetwork* aNetwork, ILog& aLog)
{
    Topology1* topology1 = new Topology1(aNetwork, aLog);
    Topology2* topology2 = new Topology2(topology1, aLog);
    Topology3* topology3 = new Topology3(topology2, aLog);
    Topology4* topology4 = new Topology4(topology3, aLog);
    Topology5* topology5 = new Topology5(topology4, aLog);
    Topology6* top6 = new Topology6(topology5, aLog);
    return(top6);
}

void Topology6::WatchT5Rooms(void*)
{
     iTopology5->Rooms().AddWatcher(*this);
}


void Topology6::Dispose()
{
    iDisposeHandler->Dispose();
    iNetwork.Execute(MakeFunctorGeneric(*this, &Topology6::DisposeCallback), NULL);
    iRooms->Dispose();

    iTopology5->Dispose();
}


void Topology6::DisposeCallback(void*)
{
    iTopology5->Rooms().RemoveWatcher(*this);

    map<ITopology5Room*, Topology6Room*>::iterator it;
    for(it=iRoomLookup.begin(); it!=iRoomLookup.end(); it++)
    {
        it->second->Dispose();
    }
}


IWatchableUnordered<ITopologyRoom*>& Topology6::Rooms() const
{
    DisposeLock lock(*iDisposeHandler);
    return(*iRooms);
}


INetwork& Topology6::Network() const
{
    DisposeLock lock(*iDisposeHandler);
    return iNetwork;
}


void Topology6::UnorderedOpen()
{
}

void Topology6::UnorderedInitialised()
{
}


void Topology6::UnorderedClose()
{
}


void Topology6::UnorderedAdd(ITopology5Room* aT5Room)
{
    iDisposeHandler->WhenNotDisposed(MakeFunctorGeneric(*this, &Topology6::UnorderedAddCallback), aT5Room);
}


void Topology6::UnorderedAddCallback(void* aT5Room)
{
    ITopology5Room* t5Room = (ITopology5Room*)aT5Room;
    Topology6Room* t6Room = new Topology6Room(iNetwork, *t5Room, iLog);
    iRooms->Add(t6Room);
    iRoomLookup[t5Room] = t6Room;
}


void Topology6::UnorderedRemove(ITopology5Room* aT5Room)
{
    iDisposeHandler->WhenNotDisposed(MakeFunctorGeneric(*this, &Topology6::UnorderedRemoveCallback), aT5Room);
}


void Topology6::UnorderedRemoveCallback(void* aT5Room)
{
    // schedule notification of L4 room removal
    ITopology5Room* t5Room = (ITopology5Room*)aT5Room;
    Topology6Room* t6Room = iRoomLookup[t5Room];
    iRooms->Remove(t6Room);
    iRoomLookup.erase(t5Room);
    t6Room->Dispose();
    delete t6Room;
}







