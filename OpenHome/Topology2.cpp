#include <OpenHome/Topology2.h>



using namespace OpenHome;
using namespace OpenHome::Av;
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

    //vector<Watchable<ITopology2Source*>*>::iterator it;
    for(TUint i=0; i<iWatchableSources.size(); i++)
    {
        ((Watchable<ITopology2Source*>*)iWatchableSources[i])->Dispose();
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


void Topology2Group::ItemOpen(const Brx& aId, Brn aValue)
{
    ProcessSourceXml(aValue, true);
}


void Topology2Group::ItemClose(const Brx& aId, Brn aValue)
{
}


void Topology2Group::ItemUpdate(const Brx& aId, Brn aValue, Brn aPrevious)
{
/*
    try
    {
        ProcessSourceXml(aValue, false);
    }
    catch (XmlException)
    {
        // TO DO: find some way to write XML to log file
    }
*/
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


IWatchable<Brn>& Topology2Group::Registration()
{
    return(iProduct.Registration());
}


//IEnumerable<IWatchable<ITopology2Source>>& Topology2Group::Sources()
std::vector<IWatchable<ITopology2Source*>*> Topology2Group::Sources()
{
    return(iWatchableSources);
}


void Topology2Group:: SetStandby(TBool aValue)
{
    //if (iProduct != null)
    //{
        //iProduct.SetStandby(aValue);
    //}
}

void Topology2Group::SetSourceIndex(TUint aValue)
{
    //if (iProduct != null)
    //{
        //iProduct.SetSourceIndex(aValue);
    //}
}

void Topology2Group::SetRegistration(const Brx& aValue)
{
    //if (iProduct != null)
    //{
        //iProduct.SetRegistration(aValue);
    //}
}

void Topology2Group::ProcessSourceXml(const Brx& aSourceXml, TBool aInitial)
{
/*
    TUint index = 0;

    XmlDocument document = new XmlDocument();
    document.LoadXml(aSourceXml);

    XmlNodeList sources = document.SelectNodes("SourceList/Source");
    foreach (XmlNode s in sources)
    {
        XmlNode nameNode = s.SelectSingleNode("Name");
        XmlNode typeNode = s.SelectSingleNode("Type");
        XmlNode visibleNode = s.SelectSingleNode("Visible");

        string name = string.Empty;
        string type = string.Empty;
        TBool visible = false;
        if (nameNode != null && nameNode.FirstChild != null)
        {
            name = nameNode.FirstChild.Value;
        }
        if (typeNode != null && typeNode.FirstChild != null)
        {
            type = typeNode.FirstChild.Value;
        }
        if (visibleNode != null && visibleNode.FirstChild != null)
        {
            string value = visibleNode.FirstChild.Value;
            try
            {
                visible = TBool.Parse(value);
            }
            catch (FormatException)
            {
                try
                {
                    visible = TUint.Parse(value) > 0;
                }
                catch (FormatException)
                {
                    visible = false;
                }
            }
        }

        ITopology2Source source = new Topology2Source(index, name, type, visible);

        if (aInitial)
        {
            iSources.Add(source);
            iWatchableSources.Add(new Watchable<ITopology2Source>(iThread, string.Format("{0}({1})", iId, index.ToString()), source));
        }
        else
        {
            ITopology2Source oldSource = iSources[(int)index];
            if (oldSource.Name != source.Name ||
                oldSource.Visible != source.Visible ||
                oldSource.Index != source.Index ||
                oldSource.Type != source.Type)
            {
                iSources[(int)index] = source;
                iWatchableSources[(int)index].Update(source);
            }
        }

        ++index;
    }
*/
}



/////////////////////////////////////////////////////////////

Topology2::Topology2(ITopology1* aTopology1, ILog& aLog)
    :iTopology1(aTopology1)
    ,iNetwork(iTopology1->Network())
    //,iLog(aLog)
    ,iGroups(new WatchableUnordered<ITopology2Group*>(iNetwork))
    ,iDisposed(false)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &Topology2::ScheduleCallback);
    iNetwork.Schedule(f, NULL);
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
    //iGroupLookup = null;

    iGroups->Dispose();
    //iGroups = null;

    iDisposed = true;
}


void Topology2::ExecuteCallback(void*)
{
    iTopology1->Products().RemoveWatcher(*this);

    map<IProxyProduct*, Topology2Group*>::iterator it;
    for(it=iGroupLookup.begin(); it!=iGroupLookup.end(); it++)
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
    }
}


void Topology2::UnorderedClose()
{
}


