#include <OpenHome/Service.h>
#include <OpenHome/Network.h>

using namespace OpenHome;
using namespace OpenHome::Av;
using namespace std;


Service::Service(INetwork& aNetwork, IInjectorDevice* aDevice, ILog& aLog)
    :iNetwork(aNetwork)
    ,iLog(aLog)
    ,iDevice(aDevice)
    //,iDisposeHandler(new DisposeHandler())
    //,iCancelSubscribe(new CancellationTokenSource())
    ,iRefCount(0)
//    ,iSubscribeTask(null);
{
}


void Service::Dispose()
{
    Assert();

    //iDisposeHandler.Dispose();
    //iCancelSubscribe.Cancel();
    //OnCancelSubscribe();

    // wait for any inflight subscriptions to complete

/*
    if (iSubscribeTask != null)
    {
        try
        {
            iSubscribeTask.Wait();
        }
        catch (AggregateException e)
        {
            HandleAggregate(e);
        }
    }
*/
    //OnUnsubscribe();

    //iCancelSubscribe.Dispose();

    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &Service::DisposeCallback);

    TUint dummy;
    iNetwork.Schedule(f, &dummy);
}


void Service::DisposeCallback(void*)
{
    ASSERT(iRefCount == 0);
}

/*
void Service::HandleAggregate(AggregateException aException)
{
    aException.Handle((x) =>
    {
        if (x is ProxyError)
        {
            return(true);
        }
        if (x is TaskCanceledException)
        {
            return(true);
        }
        if (x is AggregateException)
        {
            // will throw if aggregate contains an unhandled case
            HandleAggregate(x as AggregateException);
            return(true);
        }

        return(false);
    });
}
*/

IInjectorDevice& Service::Device()
{
    //using (iDisposeHandler.Lock())
    //{
        return(*iDevice);
    //}
}


void Service::Create(FunctorGeneric<void*> aCallback, EServiceType aServiceType, IDevice* aDevice)
{
    Assert();

/*
    //using (iDisposeHandler.Lock())
    //{
        if (iRefCount == 0)
        {
            ASSERT(iSubscribeTask == null);
            iSubscribeTask = OnSubscribe();
        }

        iRefCount++;

        if (iSubscribeTask != null)
        {
            iSubscribeTask = iSubscribeTask.ContinueWith((t) =>
            {
                iNetwork.Schedule(() =>
                {
                    // we must access t.Exception property to supress finalized task exceptions
                    if (t.Exception == null && !iCancelSubscribe.IsCancellationRequested)
                    {
                        aCallback((T)OnCreate(aDevice));
                    }
                    else
                    {
                        --iRefCount;
                        if (iRefCount == 0)
                        {
                            iSubscribeTask = null;
                        }
                    }
                });
            });
        }
        else
        {
            aCallback((T)OnCreate(aDevice));
        }
    //}

*/
}


void Service::CreateCallback(void*)
{

}


/*
Task Service::OnSubscribe()
{
    return null;
}
*/

void Service::OnCancelSubscribe()
{
}

void Service::OnUnsubscribe()
{
}


void Service::Unsubscribe()
{
    ASSERT(iRefCount != 0);
    iRefCount--;
    if (iRefCount == 0)
    {
/*
        OnUnsubscribe();
        if (iSubscribeTask != null)
        {
            try
            {
                iSubscribeTask.Wait();
            }
            catch (AggregateException ex)
            {
                HandleAggregate(ex);
            }
            iSubscribeTask = null;
        }
*/
    }
}


/*
Task Service::Start(FunctorGeneric<void*> aAction)
{

    var task = Task.Factory.StartNew(() =>
    {
        iNetwork.Schedule(() =>
        {
            aAction();
        });
    });

    lock (iTasks)
    {
        iTasks.Add(task);
    }


    return (task);
}

Task<T> Service::Start<T>(Func<T> aFunction)
{

    var task = Task.Factory.StartNew<T>(() =>
    {
        T value = default(T);

        iNetwork.Execute(() =>
        {
            value = aFunction();
        });

        return (value);
    });

    lock (iTasks)
    {
        iTasks.Add(task);
    }

    return (task);
}
*/

TBool Service::Wait()
{
/*
    Task[] tasks;

    lock (iTasks)
    {
        tasks = iTasks.ToArray();
        iTasks.Clear();
    }

    Task.WaitAll(tasks);

    return (tasks.Length == 0);
*/
    return(false);
}

// IWatchableThread

void Service::Assert()
{
    iNetwork.Assert();
}


void Service::Schedule(FunctorGeneric<void*> aCallback, void* aObj)
{
    TBool dummy;
    iNetwork.Schedule(aCallback, &dummy);
}


void Service::Execute(FunctorGeneric<void*> aCallback, void* aObj)
{
    TBool dummy;
    iNetwork.Execute(aCallback, &dummy);
}


TBool Service::IsWatchableThread()
{
    return(iNetwork.IsWatchableThread());
}


// IMockable

void Service::Execute(ICommandTokens& aCommand)
{
}



/////////////////////////////////////////////////////


template <class T>
Proxy<T>::Proxy(T aService, IDevice& aDevice)
    :iService(aService)
    ,iDevice(aDevice)
{
}


template <class T>
IDevice& Proxy<T>::Device()
{
    return (iDevice);
}


template <class T>
void Proxy<T>::Dispose()
{
    iService.Unsubscribe();
}


