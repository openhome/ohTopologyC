#include <OpenHome/Topology2.h>
#include <OpenHome/Net/Private/XmlParser.h>
#include <OpenHome/Private/Ascii.h>


using namespace OpenHome;
using namespace OpenHome::Av;
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


void Topology2Group::Dispose()
{
    if (iDisposed)
    {
        //throw new ObjectDisposedException("Topology2Group.Dispose");
    }

    iProduct.SourceXml().RemoveWatcher(*this);
    //iProduct = null;

    for(TUint i=0; i<iWatchableSources.size(); i++)
    {
        Watchable<ITopology2Source*>* x = (Watchable<ITopology2Source*>*) iWatchableSources[i];
        x->Dispose();
        delete x;

    }

    for(TUint i=0; i<iSources.size(); i++)
    {
        delete iSources[i];
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


IWatchable<Brn>& Topology2Group::Room()
{
    return(iProduct.Room());
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


//IEnumerable<IWatchable<ITopology2Source>>& Topology2Group::Sources()
std::vector<Watchable<ITopology2Source*>*>& Topology2Group::Sources()
{
    return(iWatchableSources);
}


void Topology2Group:: SetStandby(TBool /*aValue*/)
{
    //if (iProduct != null)
    //{
        //iProduct.SetStandby(aValue);
    //}
}

void Topology2Group::SetSourceIndex(TUint /*aValue*/)
{
    //if (iProduct != null)
    //{
        //iProduct.SetSourceIndex(aValue);
    //}
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

Topology2::Topology2(ITopology1* aTopology1, ILog& /*aLog*/)
    :iTopology1(aTopology1)
    ,iNetwork(iTopology1->Network())
    //,iLog(aLog)
    ,iGroups(new WatchableUnordered<ITopology2Group*>(iNetwork))
    ,iDisposed(false)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &Topology2::ScheduleCallback);
    iNetwork.Schedule(f, NULL);
}


Topology2::~Topology2()
{
    delete iTopology1;
}

void Topology2::ScheduleCallback(void*)
{
    iTopology1->Products().AddWatcher(*this);
}


void Topology2::Dispose()
{
    if (iDisposed)
    {
        //throw new ObjectDisposedException("Topology2.Dispose");
    }

    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &Topology2::ExecuteCallback);
    iNetwork.Execute(f, NULL);

    //iTopology1 = null;
    //iGroupLookup = NULL;

    iGroups->Dispose();
    delete iGroups;
    iGroups = NULL;

    iDisposed = true;
}


void Topology2::ExecuteCallback(void*)
{
    iTopology1->Products().RemoveWatcher(*this);

    map<IProxyProduct*, Topology2Group*>::iterator it;
    for(it=iGroupLookup.begin(); it!=iGroupLookup.end(); it++)
    {
        it->second->Dispose();
        delete it->second;
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


