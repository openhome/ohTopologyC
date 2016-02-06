#include <OpenHome/Topology2.h>
#include <OpenHome/Net/Private/XmlParser.h>
#include <OpenHome/Private/Ascii.h>


using namespace OpenHome;
using namespace OpenHome::Topology;
using namespace OpenHome::Net;
using namespace std;


Topology2Source::Topology2Source(TUint aIndex, const Brx& aName, const Brx& aType, TBool aVisible)
    :iIndex(aIndex)
    ,iName(aName)
    ,iType(aType)
    ,iVisible(aVisible)
{
}


TUint Topology2Source::Index()
{
    return(iIndex);
}


Brn Topology2Source::Name()
{
    return(Brn(iName));
}


Brn Topology2Source::Type()
{
    return(Brn(iType));
}


TBool Topology2Source::Visible()
{
    return(iVisible);
}

///////////////////////////////////////////////////////////////////////////////////


Topology2Group::Topology2Group(IWatchableThread& aThread, const Brx& aId, IProxyProduct& aProduct)
    :iThread(aThread)
    ,iDisposed(false)
    ,iId(aId)
    ,iProduct(aProduct)
{
    iProduct.SourceXml().AddWatcher(*this);
}


Topology2Group::~Topology2Group()
{
    for(TUint i=0; i<iWatchableSources.size(); i++)
    {
        delete iWatchableSources[i];
    }

    for(TUint i=0; i<iSources.size(); i++)
    {
        delete iSources[i];
    }
}

void Topology2Group::Dispose()
{
    if (iDisposed)
    {
        //throw new ObjectDisposedException("Topology2Group.Dispose");
    }

    iProduct.SourceXml().RemoveWatcher(*this);

    for(TUint i=0; i<iWatchableSources.size(); i++)
    {
        iWatchableSources[i]->Dispose();
    }

    iDisposed = true;
}


Brn Topology2Group::Id()
{
    return(Brn(iId));
}


Brn Topology2Group::Attributes()
{
    return(iProduct.Attributes());
}


Brn Topology2Group::ModelName()
{
    return(iProduct.ModelName());
}


Brn Topology2Group::ManufacturerName()
{
    return(iProduct.ManufacturerName());
}


Brn Topology2Group::ProductId()
{
    return(iProduct.ProductId());
}

Brn Topology2Group::ProductImageUri()
{
    return(iProduct.ProductImageUri());
}

IDevice& Topology2Group::Device()
{
    return(iProduct.Device());
}


void Topology2Group::ItemOpen(const Brx& /*aId*/, Brn aValue)
{
    ProcessSourceXml(aValue, true);
}


void Topology2Group::ItemClose(const Brx& /*aId*/, Brn /*aValue*/)
{
}


void Topology2Group::ItemUpdate(const Brx& /*aId*/, Brn aValue, Brn /*aPrevious*/)
{

    //try
    //{
        ProcessSourceXml(aValue, false);
    //}
    //catch (XmlException)
    //{
        // TO DO: find some way to write XML to log file
    //}

}


IWatchable<Brn>& Topology2Group::RoomName()
{
    return(iProduct.RoomName());
}


IWatchable<Brn>& Topology2Group::Name()
{
    return(iProduct.Name());
}


IWatchable<TBool>& Topology2Group::Standby()
{
    return(iProduct.Standby());
}


IWatchable<TUint>& Topology2Group::SourceIndex()
{
    return(iProduct.SourceIndex());
}


std::vector<Watchable<ITopology2Source*>*>& Topology2Group::Sources()
{
    return(iWatchableSources);
}


void Topology2Group:: SetStandby(TBool aValue)
{
    iProduct.SetStandby(aValue);
}

void Topology2Group::SetSourceIndex(TUint aValue)
{
    iProduct.SetSourceIndex(aValue);
}


void Topology2Group::ProcessSourceXml(const Brx& aSourceXml, TBool aInitial)
{
    Brn xmlDoc = Brn(aSourceXml);
    Brn remaining = xmlDoc;
    Brn sourceTag;
    Brn name;
    Brn type;
    Brn visibleStr;
    TBool visible;
    TUint index = 0;

    while(!remaining.Equals(Brx::Empty()))
    {
        try
        {
            sourceTag = XmlParserBasic::Find(Brn("Source"), xmlDoc, remaining);
            xmlDoc = remaining;
            name = XmlParserBasic::Find(Brn("Name"), sourceTag);
            type = XmlParserBasic::Find(Brn("Type"), sourceTag);
            visibleStr = XmlParserBasic::Find(Brn("Visible"), sourceTag);
        }
        catch(XmlError)
        {
            remaining.Set(Brx::Empty());
            break;
        }

        if(Ascii::CaseInsensitiveEquals(visibleStr, Brn("True")))
        {
            visible = true;
        }
        else
        {
            visible = false;
        }

        if (type == Brn("Disc"))
        {
            name = iProduct.Name().Value();
        }

        ITopology2Source* source = new Topology2Source(index, name, type, visible);

        if (aInitial)
        {
            iSources.push_back(source);

            Bws<100> id;
            id.Replace(iId);
            id.Append(Brn("("));
            Ascii::AppendDec(id, index);
            id.Append(Brn(")"));
            iWatchableSources.push_back(new Watchable<ITopology2Source*>(iThread, id, source));
        }
        else
        {
            ITopology2Source* oldSource = iSources[index];
            if ((!oldSource->Name().Equals(source->Name())) ||
                (oldSource->Visible() != source->Visible()) ||
                (oldSource->Index() != source->Index()) ||
                (!oldSource->Type().Equals(source->Type())) )
            {
                iSources[index] = source;
                iWatchableSources[index]->Update(source);
                delete oldSource;
            }
            else
            {
                delete source;
            }
        }

        index++;
    }

}



/////////////////////////////////////////////////////////////

Topology2::Topology2(ITopology1* aTopology1)
    :iTopology1(aTopology1)
    ,iNetwork(iTopology1->Network())
    ,iGroups(new WatchableUnordered<ITopology2Group*>(iNetwork))
    ,iDisposed(false)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &Topology2::WatchT1Products);
    iNetwork.Schedule(f, NULL);
}


Topology2::~Topology2()
{
    delete iTopology1;
    delete iGroups;

    for(auto it=iGroupLookup.begin(); it!=iGroupLookup.end(); it++)
    {
        delete it->second;
    }

    iGroupLookup.clear();
}

void Topology2::WatchT1Products(void*)
{
    iTopology1->Products().AddWatcher(*this);
}


void Topology2::Dispose()
{
    if (iDisposed)
    {
        //throw new ObjectDisposedException("Topology2.Dispose");
    }

    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &Topology2::DisposeCallback);
    iNetwork.Execute(f, NULL);
    iGroups->Dispose();
    iTopology1->Dispose();

    iDisposed = true;
}


void Topology2::DisposeCallback(void*)
{
    iTopology1->Products().RemoveWatcher(*this);

    for(auto it=iGroupLookup.begin(); it!=iGroupLookup.end(); it++)
    {
        it->second->Dispose();
    }
}


IWatchableUnordered<ITopology2Group*>& Topology2::Groups()
{
    return *iGroups;
}


INetwork& Topology2::Network()
{
    return iNetwork;
}


void Topology2::UnorderedOpen()
{
}


void Topology2::UnorderedInitialised()
{
}


void Topology2::UnorderedAdd(IProxyProduct* aProduct)
{
    //try
    //{
        Topology2Group* group = new Topology2Group(iNetwork, aProduct->Device().Udn(), *aProduct);
        iGroupLookup[aProduct] = group;
        iGroups->Add(group);
    //}
    //catch (XmlException)
    //{
        // TO DO: find some way to write XML to log file
    //}
}


void Topology2::UnorderedRemove(IProxyProduct* aProduct)
{
    if (iGroupLookup.count(aProduct)>0)  // found in map
    {
        // schedule higher layer notification
        Topology2Group* group = iGroupLookup[aProduct];

        iGroups->Remove(group);
        iGroupLookup.erase(aProduct);
        group->Dispose();
        delete group;
    }
}


void Topology2::UnorderedClose()
{
}


