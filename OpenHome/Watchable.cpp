#include <OpenHome/Watchable.h>
#include <vector>
#include <algorithm>


using namespace OpenHome;
using namespace OpenHome::Topology;
using namespace std;



///////////////////////////////////////////////////////

/**

 */
WatchableBase::WatchableBase(IWatchableThread& aWatchableThread)
    :iWatchableThread(aWatchableThread)
{
}


/**

 */
void WatchableBase::Assert()
{
    iWatchableThread.Assert();
}


/**

 */
void WatchableBase::Schedule(FunctorGeneric<void*> aCallback, void* aObj)
{
    iWatchableThread.Schedule(aCallback, aObj);
}


/**

 */
void WatchableBase::Execute(FunctorGeneric<void*> aCallback, void* aObj)
{
    iWatchableThread.Execute(aCallback, aObj);
}


void WatchableBase::Execute()
{
    iWatchableThread.Execute();
}





