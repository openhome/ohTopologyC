#include <OpenHome/Service.h>
#include <OpenHome/Network.h>
#include <OpenHome/Private/Thread.h>
#include <memory>
using namespace OpenHome;
using namespace OpenHome::Topology;
using namespace OpenHome::Net;
using namespace std;


// ServiceCreateData

ServiceCreateData::ServiceCreateData(FunctorGeneric<IProxy*>& aCallback, IDevice& aDevice)
    : iCallback(aCallback)
    , iDevice(&aDevice)
{
    ASSERT(iDevice != NULL);
}

IDevice& ServiceCreateData::Device()
{
    return *iDevice;
}

void ServiceCreateData::RunCallback(IProxy* aProxy)
{
    iCallback(aProxy);
}


// Service

Service::Service(IInjectorDevice& aDevice)
    :iNetwork(aDevice.Network())
    ,iDisposeHandler(new DisposeHandler())
    //,iCancelSubscribe(new CancellationTokenSource())
    //,iSubscribeTask(NULL)
    ,iDevice(aDevice)
    ,iUdn(iDevice.Udn())
    ,iRefCount(0)
    ,iMutexSubscribe("SVS")
    ,iMockSubscribe(false)
    ,iSubscribed(false)
    ,iMutexSemas("SSMX")
{
}

Service::~Service()
{
    delete iDisposeHandler;
}


void Service::Dispose()
{
    Assert();

    iDisposeHandler->Dispose();
    //iCancelSubscribe.Cancel();
    OnCancelSubscribe();

    // wait for any inflight subscriptions to complete
//    if (iSubscribeTask != NULL)
//    {
//        iSubscribeTask->Wait();
/*
        try
        {
            iSubscribeTask->Wait();
        }
        catch (AggregateException e)
        {
            HandleAggregate(e);
        }
*/
//    }

    OnUnsubscribe();

    iMutexSubscribe.Wait();
    //std::vector<ServiceCreateData*> subscriptionsData(iSubscriptionsData);
    iSubscriptionsData.clear();
    iMutexSubscribe.Signal();

    iRefCount = 0;

    // for (TUint i=0; i<subscriptionsData.size(); i++)
    // {
    //     delete subscriptionsData[i];
    // }



    //iCancelSubscribe.Dispose();

    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &Service::DisposeCallback);
    Execute(f, NULL);
}


void Service::DisposeCallback(void*)
{
    if (iRefCount != 0)
    {
        ASSERTS();
    }
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
    DisposeLock lock(*iDisposeHandler);
    return(iDevice);
}


void Service::Create(FunctorGeneric<IProxy*> aCallback, IDevice& aDevice)
{
    Assert(); // check we're on watchable thread

    DisposeLock lock(*iDisposeHandler);

    if (iRefCount == 0)
    {
        ASSERT(!iMockSubscribe);
        iMockSubscribe = OnSubscribe();
    }

    iRefCount++;

    if (iMockSubscribe)
    {
        // mock - callback immediately
        aCallback(OnCreate(aDevice));
    }
    else
    {
        iMutexSubscribe.Wait();
        if (iSubscribed)
        {
            // non mock, already subscribed - callback immediately
            iMutexSubscribe.Signal();
            aCallback(OnCreate(aDevice));
        }
        else
        {
            // non mock, not yet subscribed - callback later when subscribe completes
            ServiceCreateData serviceCreateData(aCallback, aDevice);
            iSubscriptionsData.push_back(serviceCreateData);
            iMutexSubscribe.Signal();
        }
    }

/*
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

*/
}



void Service::SubscribeCompleted()
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &Service::SubscribeCompletedCallback);
    Schedule(f, NULL);
}


void Service::SubscribeCompletedCallback(void* /*aArgs*/)
{
    ASSERT(!iMockSubscribe);
    iMutexSubscribe.Wait();
    //std::vector<ServiceCreateData*> subscriptionsData(iSubscriptionsData);
    //iSubscriptionsData.clear();
    iSubscribed = true;
    iMutexSubscribe.Signal();

    for (auto it = iSubscriptionsData.begin(); it != iSubscriptionsData.end(); ++it)
    {
        //auto data = subscriptionsData[i];
        auto proxy = OnCreate((*it).Device());
        (*it).RunCallback(proxy);
        //delete data;
    }
    iSubscriptionsData.clear();
/*
    if (t.Exception == NULL && !iCancelSubscribe.IsCancellationRequested)
    {
        callback(OnCreate(device));
    }
    else
    {
        --iRefCount;
        if (iRefCount == 0)
        {
            iSubscribeTask = NULL;
        }
    }
*/
}


TBool Service::OnSubscribe()
{
    return(true);
}


void Service::OnCancelSubscribe()
{
}


void Service::OnUnsubscribe()
{
}


void Service::Unsubscribe()
{
    Assert();

    ASSERT(iRefCount != 0);
    iRefCount--;
    if (iRefCount == 0)
    {
        iMockSubscribe = false;
        OnUnsubscribe();

/*
        if (iSubscribeTask != NULL)
        {
            iSubscribeTask->Wait();
            iSubscribeTask = NULL;
        }
*/
    }
}

void Service::Start(FunctorGeneric<void*> aCallback, void* aArg)
{
    Semaphore* sema = new Semaphore("SVCS", 0);
    iMutexSemas.Wait();
    iSemas.push_back(sema);
    iMutexSemas.Signal();

    auto startData = new StartData();
    startData->iCallback = aCallback;
    startData->iArg = aArg;
    startData->iSema = sema;

    auto f = MakeFunctorGeneric(*this, &Service::StartCallback);
    Schedule(f, startData);

/*
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
*/
}


void Service::StartCallback(void* aArg)
{
    auto data = (StartData*)aArg;
    data->iCallback(data->iArg);
    data->iSema->Signal();
    delete data;
}


void Service::AddTask(Semaphore* aSema)
{
    iMutexSemas.Wait();
    iSemas.push_back(aSema);
    iMutexSemas.Signal();

/*
    lock (iTasks)
    {
        iTasks.Add(aTask);
    }
*/
}




/*
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
    iMutexSemas.Wait();
    vector<Semaphore*> semas(iSemas);
    iSemas.clear();
    iMutexSemas.Signal();

    for(TUint i=0; i<semas.size(); i++)
    {
        semas[i]->Wait();
    }

    return(true);


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
}


void Service::Assert()
{
    iNetwork.Assert();
}


void Service::Schedule(FunctorGeneric<void*> aCallback, void* aObj)
{
    iNetwork.Schedule(aCallback, aObj);
}


void Service::Execute(FunctorGeneric<void*> aCallback, void* aObj)
{
    iNetwork.Execute(aCallback, aObj);
}


void Service::Execute()
{
    iNetwork.Execute();
}


void Service::Execute(ICommandTokens& /*aCommand*/)
{
}
