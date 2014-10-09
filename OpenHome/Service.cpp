#include <OpenHome/Service.h>
#include <OpenHome/Network.h>

using namespace OpenHome;
using namespace OpenHome::Av;
using namespace std;


Service::Service(INetwork& aNetwork, IInjectorDevice* aDevice, ILog& aLog)
    :iNetwork(aNetwork)
    ,iLog(aLog)
    ,iDisposeHandler(new DisposeHandler())
    //,iCancelSubscribe(new CancellationTokenSource())
    ,iSubscribeTask(NULL)
    ,iDevice(aDevice)
    ,iRefCount(0)
    ,iMutexJobs("JOBX")
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
    if (iSubscribeTask != NULL)
    {
        iSubscribeTask->Wait();
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
    }

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
    return(*iDevice);
}


void Service::Create(FunctorGeneric<void*> aCallback, EServiceType /*aServiceType*/, IDevice* aDevice)
{
    Assert(); // check we're on watchable thread

    DisposeLock lock(*iDisposeHandler);

    if (iRefCount == 0)
    {
        ASSERT(iSubscribeTask == NULL);
        iSubscribeTask = OnSubscribe();
    }

    iRefCount++;


    if (iSubscribeTask != NULL)
    {
/*
        iSubscribeTask = iSubscribeTask.ContinueWith((t) =>
        {
            iNetwork.Schedule(() =>
            {
                // we must access t.Exception property to supress finalized task exceptions
                if (t.Exception == NULL && !iCancelSubscribe.IsCancellationRequested)
                {
                    aCallback(OnCreate(aDevice));
                }
                else
                {
                    --iRefCount;
                    if (iRefCount == 0)
                    {
                        iSubscribeTask = NULL;
                    }
                }
            });
        });
*/

        ArgsTwo<FunctorGeneric<void*>, IDevice*>* args = new ArgsTwo<FunctorGeneric<void*>, IDevice*>(aCallback, aDevice);

        FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &Service::CreateCallback);

        iSubscribeTask = iSubscribeTask->ContinueWith(f, args);

    }
    else
    {
        IProxy* product = OnCreate(aDevice);
        ArgsTwo<IDevice*, IProxy*>* args = new ArgsTwo<IDevice*, IProxy*>(aDevice, product);
        aCallback(args);
    }
}



void Service::CreateCallback(void* aArgs)
//void Service::CreateCallback(ArgsTwo<FunctorGeneric<void*>, IDevice*>* aArgs)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &Service::CreateCallbackCallback);
    iNetwork.Schedule(f, aArgs);
}


void Service::CreateCallbackCallback(void* aArgs)
//void Service::CreateCallbackCallback(ArgsTwo<FunctorGeneric<void*>, IDevice*>* aArgs)
{
    ArgsTwo<FunctorGeneric<void*>, IDevice*>* args = (ArgsTwo<FunctorGeneric<void*>, IDevice*>*)aArgs;

    FunctorGeneric<void*> callback = args->Arg1();
    IDevice* device = args->Arg2();
    delete args;

    IProxy* product = OnCreate(device);
    ArgsTwo<IDevice*, IProxy*>* cbArgs = new ArgsTwo<IDevice*, IProxy*>(device, product);

    callback(cbArgs);
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


Job* Service::OnSubscribe()
{
    return(NULL);
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

        OnUnsubscribe();
        if (iSubscribeTask != NULL)
        {

            //try
            //{
                iSubscribeTask->Wait();
            //}
            //catch (AggregateException ex)
            //{
            //    HandleAggregate(ex);
            //}

            iSubscribeTask = NULL;

        }

    }
}



Job* Service::Start(FunctorGeneric<void*> aAction)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &Service::StartCallback);
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


void Service::StartCallback(void* aArg)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &Service::StartCallbackCallback);
    iNetwork.Schedule(f, aArg);
}


void Service::StartCallbackCallback(void* aArg)
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


