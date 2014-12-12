#include <OpenHome/Service.h>
#include <OpenHome/Network.h>

using namespace OpenHome;
using namespace OpenHome::Av;
using namespace std;


Service::Service(INetwork& aNetwork, IInjectorDevice& aDevice, ILog& aLog)
    :iNetwork(aNetwork)
    ,iLog(aLog)
    ,iDisposeHandler(new DisposeHandler())
    //,iCancelSubscribe(new CancellationTokenSource())
    //,iSubscribeTask(NULL)
    ,iDevice(aDevice)
    ,iRefCount(0)
    ,iMutexJobs("SVJ")
    ,iMutexSubscribe("SVS")
    ,iMockSubscribe(false)
    ,iSubscribed(false)
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
    //delete iDisposeHandler;
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

    //iCancelSubscribe.Dispose();

    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &Service::DisposeCallback);
    iNetwork.Schedule(f, NULL);
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


void Service::Create(FunctorGeneric<ServiceCreateData*> aCallback, IDevice* aDevice)
{
    Assert(); // check we're on watchable thread

    DisposeLock lock(*iDisposeHandler);

    auto serviceCreateData = new ServiceCreateData();
    serviceCreateData->iDevice = aDevice;

    if (iRefCount == 0)
    {
        ASSERT(!iMockSubscribe);
        iMockSubscribe = OnSubscribe();
    }

    iRefCount++;

    if (iMockSubscribe)
    {
        // mock - callback immediately
        serviceCreateData->iProxy = OnCreate(*aDevice);
        aCallback(serviceCreateData);
    }
    else
    {
        iMutexSubscribe.Wait();
        if (iSubscribed)
        {
            // non mock, already subscribed - callback immediately
            iMutexSubscribe.Signal();
            serviceCreateData->iProxy = OnCreate(*aDevice);
            aCallback(serviceCreateData);
        }
        else
        {
            // non mock, not yet subscribed - callback later when subscribe completes
            serviceCreateData->iCallback = aCallback;
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
    iNetwork.Schedule(f, NULL);
}


void Service::SubscribeCompletedCallback(void* /*aArgs*/)
{
    ASSERT(!iMockSubscribe);
    iMutexSubscribe.Wait();
    auto subscriptionsData = new vector<ServiceCreateData*>(iSubscriptionsData);
    iSubscriptionsData.clear();
    iSubscribed = true;
    iMutexSubscribe.Signal();

    for (TUint i=0; i<subscriptionsData->size(); i++)
    {
        auto data = (*subscriptionsData)[i];
        data->iProxy = OnCreate(*(data->iDevice));
        data->iCallback(data);
    }

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


Job* Service::Start(FunctorGeneric<void*> aAction)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &Service::StartCallback1);
    Job* job = Job::StartNew(f, &aAction);

    AutoMutex mutex(iMutexJobs);
    iJobs.push_back(job);
    return(job);


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


void Service::StartCallback1(void* aArg)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &Service::StartCallback2);
    iNetwork.Schedule(f, aArg);
}


void Service::StartCallback2(void* aArg)
{
    FunctorGeneric<void*> action = *((FunctorGeneric<void*>*)aArg);
    action(NULL);
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
    iMutexJobs.Wait();
    vector<Job*> jobs(iJobs);
    iJobs.clear();
    iMutexJobs.Signal();

    for(TUint i=0; i<jobs.size(); i++)
    {
        jobs[i]->Wait();
    }

    return(jobs.size()==0);

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


void Service::Schedule(FunctorGeneric<void*> aCallback, void* /*aObj*/)
{
    iNetwork.Schedule(aCallback, NULL);
}


void Service::Execute(FunctorGeneric<void*> aCallback, void* /*aObj*/)
{
    iNetwork.Execute(aCallback, NULL);
}


void Service::Execute(ICommandTokens& /*aCommand*/)
{
}


