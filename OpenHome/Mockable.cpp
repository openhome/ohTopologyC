#include <OpenHome/Mockable.h>
#include <OpenHome/Private/Ascii.h>


using namespace OpenHome;
using namespace Av;


//////////////////////////////////////////////////////////////////////////

Mockable::Mockable()
{
}


void Mockable::Add(const Brx& aId, IMockable& aMockable)
{
    iMockables[Brn(aId)] = &aMockable;
}


void Mockable::Remove(const Brx& aId)
{
    iMockables.erase(Brn(aId));
}


void Mockable::Execute(ICommandTokens& aTokens)
{
    Brn next(aTokens.Next());
    iMockables[next]->Execute(aTokens);
}

///////////////////////////////////////////////////////////////////

MockableStream::MockableStream(IReader& aReader, IMockable& aMockable)
    :iReader(aReader)
    ,iMockable(aMockable)
{
}


void MockableStream::Start()
{
    for (;;)
    {

        Brn line = iReader.ReadUntil('\n');

        if (line == Brx::Empty())
        {
            break;
        }

/*
        var commands = Tokeniser.Parse(line);
        if (commands.Any())
        {
            iMockable.Execute(commands);
        }
*/
        CommandTokens commands(line);
        if (commands.Count()>0)
        {
            iMockable.Execute(commands);
        }
    }
}

///////////////////////////////////////////////////

MockableScriptRunner::MockableScriptRunner()
    :iResultQueue(kMaxFifoEntries)
{
}


void MockableScriptRunner::Run(FunctorGeneric<void*> aWait, IReader& aStream, IMockable& aMockable)
{

    TBool wait = true;
    TBool eof = false;
    Brn lastline;

    try
    {
        lastline = aStream.ReadUntil('\n');
    }
    catch(ReaderError)
    {
        return;
    }


    for (;;)
    {
        iLine.Replace(lastline);

        try
        {
            lastline = aStream.ReadUntil('\n');
        }
        catch(ReaderError)
        {
            eof = true;
        }

        while ((!eof) &&
                (lastline != Brx::Empty()) &&
                (!lastline.BeginsWith(Brn("//"))) &&
                (!lastline.BeginsWith(Brn("mock"))) &&
                (!lastline.BeginsWith(Brn("expect"))) &&
                (!lastline.BeginsWith(Brn("empty"))) &&
                (!lastline.BeginsWith(Brn("break"))))
        {
            //line += "\n" + lastline;
            iLine.Append(Brn("\n"));
            iLine.Append(lastline);

            try
            {
                lastline = aStream.ReadUntil('\n');
            }
            catch(ReaderError)
            {
                eof = true;
            }
        }

        if (iLine == Brx::Empty())
        {
            break;
        }


        if (iLine.BeginsWith(Brn("//")))
        {
            continue;
        }

        CommandTokens commands(iLine);
        //var commands = Tokeniser.Parse(line);

        //if (commands.Any())
        if (commands.Count()>0)
        {
            //string command = commands.First().ToLowerInvariant();
            Brn command = commands.Next();

            if (Ascii::CaseInsensitiveEquals(command, Brn("mock")))
            {
                //Console.WriteLine(line);
                //aMockable.Execute(commands.Skip(1));
                aMockable.Execute(commands);



                wait = true;
            }
            else if (Ascii::CaseInsensitiveEquals(command, Brn("expect")))
            {
                if (wait)
                {
                    try
                    {
                        TUint dummy;
                        aWait(&dummy);
                    }
                    catch (Exception e)
                    {
                        //Console.WriteLine(e);
                    }

                    wait = false;
                }

                //string expected = line.Substring("expect".Length + 1);
                Brn expected = iLine.Split(Brn("expect").Bytes() + 1);


                if (iResultQueue.SlotsUsed()>0)
                {
                    Brn result = iResultQueue.Read();
                    Assert(result, expected);
                }
                else
                {
                    //THROW(queue empty);
                }

/*
                try
                {
                    Brn result = iResultQueue.Read();

                    Assert(result, expected);
                }
                catch (InvalidOperationException)
                {
                    //Console.WriteLine(string.Format("Failed\nExpected: {0} but queue was empty", expected));
                    throw;
                }
*/
            }
            else if (Ascii::CaseInsensitiveEquals(command, Brn("empty")))
            {
                try
                {
                    TUint dummy;
                    aWait(&dummy);
                }
                catch (Exception e)
                {
                    //Console.WriteLine(e);
                }

                ASSERT(iResultQueue.SlotsUsed() == 0);
            }
            else if (Ascii::CaseInsensitiveEquals(command, Brn("break")))
            {
                //Debugger.Break();
            }
            else
            {
                //THROW(NotSupportedException);
            }
        }

        if (eof)
        {
            break;
        }
    }

}


void MockableScriptRunner::Result(const Brx& aValue)
{
    iResultQueue.Write(Brn(aValue));
    //Console.WriteLine(aValue);
}


void MockableScriptRunner::Assert(const Brx& aActual, const Brx& aExpected)
{
    if (aActual != aExpected)
    {
        //Console.WriteLine(string.Format("Failed\nExpected: {0}\nReceived: {1}", aExpected, aActual));
        throw new AssertError();
    }
    else
    {
        //Console.Write('.');
    }
}


void MockableScriptRunner::Assert(TBool aExpression)
{
    if (!aExpression)
    {
        //Console.WriteLine("Failed");
        throw new AssertError();
    }
    else
    {
        //Console.Write('.');
    }
}

////////////////////////////////////////////////////////////////////////

template <class T>
ResultWatcherFactory<T>::ResultWatcherFactory(MockableScriptRunner& aRunner)
    :iRunner(aRunner)
{
    //iWatchers = new Dictionary<string, List<IDisposable>>();
}


template <class T>
void ResultWatcherFactory<T>::Create(const Brx& aId, IWatchable<T>& aWatchable, FunctorGeneric<ArgsTwo<T, FunctorGeneric<const Brx&>>> aAction)
{
    //List<IDisposable> watchers;
    std::vector<IDisposable*> watchers;
/*
    if (!iWatchers.TryGetValue(aId, watchers))
    {
        //watchers = new List<IDisposable>();
        iWatchers.Add(aId, watchers);
    }
*/
    watchers.push_back(new ResultWatcher<T>(iRunner, aId, aWatchable, aAction));
}


template <class T>
void ResultWatcherFactory<T>::Create(const Brx& aId, IWatchableUnordered<T>& aWatchable, FunctorGeneric<ArgsTwo<T, FunctorGeneric<const Brx&>>> aAction)
{
    std::vector<IDisposable*> watchers;

/*
    if (!iWatchers.TryGetValue(aId, watchers))
    {
        //watchers = new List<IDisposable>();
        iWatchers.Add(aId, watchers);
    }
*/
    watchers.push_back(new ResultUnorderedWatcher<T>(iRunner, aId, aWatchable, aAction));
}


template <class T>
void ResultWatcherFactory<T>::Create(const Brx& aId, IWatchableOrdered<T>& aWatchable, FunctorGeneric<ArgsTwo<T, FunctorGeneric<const Brx&>>> aAction)
{
    std::vector<IDisposable*> watchers;
/*
    if (!iWatchers.TryGetValue(aId, out watchers))
    {
        //watchers = new List<IDisposable>();
        iWatchers.Add(aId, watchers);
    }
*/
    watchers.push_back(new ResultOrderedWatcher<T>(iRunner, aId, aWatchable, aAction));
}


template <class T>
void ResultWatcherFactory<T>::Destroy(const Brx& aId)
{
    //iWatchers[aId].ForEach(w => w.Dispose());
    iWatchers.erase(Brn(aId));
}


template <class T>
void ResultWatcherFactory<T>::Dispose()
{
/*
    foreach (var entry in iWatchers)
    {
        entry.Value.ForEach(w => w.Dispose());
    }
*/
}

/////////////////////////////////////////////////////////////////////////////////////

template <class T>
ResultWatcher<T>::ResultWatcher(MockableScriptRunner& aRunner, const Brx& aId, IWatchable<T>& aWatchable, FunctorGeneric<ArgsTwo<T, FunctorGeneric<const Brx&>>> aAction)
    :iRunner(aRunner)
    ,iId(aId)
    ,iWatchable(aWatchable)
    ,iAction(aAction)
{
    iWatchable.AddWatcher(this);
}


template <class T>
void ResultWatcher<T>::ItemOpen(const Brx& aId, T aValue)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ResultWatcher::ItemOpenCallback);
    iAction(aValue, f);
}


template <class T>
void ResultWatcher<T>::ItemOpenCallback(const Brx& aValue)
{
    iResultOpenBuf.Replace(iId);
    iResultOpenBuf.Append(Brn(" open "));
    iResultOpenBuf.Append(aValue);
    iRunner.Result(iResultOpenBuf);
}


template <class T>
void ResultWatcher<T>::ItemUpdate(const Brx& aId, T aValue, T aPrevious)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ResultWatcher::ItemUpdateCallback);
    iAction(aValue, f);
}


template <class T>
void ResultWatcher<T>::ItemUpdateCallback(const Brx& aValue)
{
    iResultUpdateBuf.Replace(iId);
    iResultUpdateBuf.Append(Brn(" update "));
    iResultUpdateBuf.Append(aValue);
    iRunner.Result(iResultUpdateBuf);
}


template <class T>
void ResultWatcher<T>::ItemClose(const Brx& aId, T aValue)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ResultWatcher::ItemCloseCallback);
    iAction(aValue, f);
}


template <class T>
void ResultWatcher<T>::ItemCloseCallback(const Brx& aValue)
{
    iResultCloseBuf.Replace(iId);
    iResultCloseBuf.Append(Brn(" close "));
    iResultCloseBuf.Append(aValue);
    iRunner.Result(iResultCloseBuf);
}


template <class T>
void ResultWatcher<T>::Dispose()
{
    iWatchable.RemoveWatcher(this);
}

////////////////////////////////////////////////////////////

template <class T>
ResultUnorderedWatcher<T>::ResultUnorderedWatcher(MockableScriptRunner& aRunner, const Brx& aId, IWatchableUnordered<T>& aWatchable, FunctorGeneric<ArgsTwo<T, FunctorGeneric<const Brx&>>> aAction)
    :iRunner(aRunner)
    ,iId(aId)
    ,iWatchable(aWatchable)
    ,iAction(aAction)
{
    iWatchable.AddWatcher(this);
}


template <class T>
void ResultUnorderedWatcher<T>::UnorderedOpen()
{
}


template <class T>
void ResultUnorderedWatcher<T>::UnorderedInitialised()
{
}


template <class T>
void ResultUnorderedWatcher<T>::UnorderedAdd(T aItem)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ResultUnorderedWatcher::UnorderedAddCallback);
    iAction(aItem, f);
/*
    iAction(aItem, (s) =>
    {
        iRunner.Result(iId + " add " + s);
    });
*/
}
template <class T>
void ResultUnorderedWatcher<T>::UnorderedAddCallback(const Brx& aValue)
{
    iResultAddBuf.Replace(iId);
    iResultAddBuf.Append(Brn(" add "));
    iResultAddBuf.Append(aValue);
    iRunner.Result(iResultAddBuf);
/*
    iAction(aItem, (s) =>
    {
        iRunner.Result(iId + " add " + s);
    });
*/
}


template <class T>
void ResultUnorderedWatcher<T>::UnorderedRemove(T aItem)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ResultUnorderedWatcher::UnorderedRemoveCallback);
    iAction(aItem, f);
/*
    iAction(aItem, (s) =>
    {
        iRunner.Result(iId + " remove " + s);
    });
*/
}
template <class T>
void ResultUnorderedWatcher<T>::UnorderedRemoveCallback(const Brx& aValue)
{
    iResultRemoveBuf.Replace(iId);
    iResultRemoveBuf.Append(Brn(" remove "));
    iResultRemoveBuf.Append(aValue);
    iRunner.Result(iResultRemoveBuf);


/*
    iAction(aItem, (s) =>
    {
        iRunner.Result(iId + " remove " + s);
    });
*/
}


template <class T>
void ResultUnorderedWatcher<T>::UnorderedClose()
{
}


template <class T>
void ResultUnorderedWatcher<T>::Dispose()
{
    iWatchable.RemoveWatcher(this);
}

//////////////////////////////////////////////////////////////////////////////


template <class T>
ResultOrderedWatcher<T>::ResultOrderedWatcher(MockableScriptRunner& aRunner, const Brx& aId, IWatchableOrdered<T>& aWatchable, FunctorGeneric<ArgsTwo<T, FunctorGeneric<void*>>> aAction)
    :iRunner(aRunner)
    ,iId(aId)
    ,iWatchable(aWatchable)
    ,iAction(aAction)
{
    iWatchable.AddWatcher(this);
}


template <class T>
void ResultOrderedWatcher<T>::OrderedOpen()
{
}


template <class T>
void ResultOrderedWatcher<T>::OrderedInitialised()
{
}


template <class T>
void ResultOrderedWatcher<T>::OrderedAdd(T aItem, TUint aIndex)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ResultOrderedWatcher::OrderedAddCallback);
    iAction(aItem, f);
/*
    iAction(aItem, (s) =>
    {
        iRunner.Result(iId + " add " + s);
    });
*/
}
template <class T>
void ResultOrderedWatcher<T>::OrderedAddCallback(const Brx& aValue)
{
    iResultAddBuf.Replace(iId);
    iResultAddBuf.Append(Brn(" add "));
    iResultAddBuf.Append(aValue);
    iRunner.Result(iResultAddBuf);
/*
    iAction(aItem, (s) =>
    {
        iRunner.Result(iId + " add " + s);
    });
*/
}


template <class T>
void ResultOrderedWatcher<T>::OrderedMove(T aItem, TUint aFrom, TUint aTo)
{
    iResultMoveBuf.Replace(iId);
    iResultMoveBuf.Append(Brn(" moved from "));
    Ascii::AppendDec(iResultMoveBuf, aFrom);
    iResultMoveBuf.Append(Brn(" to "));
    Ascii::AppendDec(iResultMoveBuf, aFrom);
    iResultMoveBuf.Append(Brn(" "));
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ResultOrderedWatcher::OrderedMoveCallback);
    iAction(aItem, f);
/*    iAction(aItem, (s) =>
    {
        iRunner.Result(iId + " moved from " + aFrom + " to " + aTo + " " + s);
    });
*/
}
template <class T>
void ResultOrderedWatcher<T>::OrderedMoveCallback(const Brx& aValue)
{
    iResultMoveBuf.Append(aValue);
    iRunner.Result(iResultMoveBuf);
/*    iAction(aItem, (s) =>
    {
        iRunner.Result(iId + " moved from " + aFrom + " to " + aTo + " " + s);
    });
*/
}


template <class T>
void ResultOrderedWatcher<T>::OrderedRemove(T aItem, TUint aIndex)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ResultOrderedWatcher::OrderedRemoveCallback);
    iAction(aItem, f);
/*
    iAction(aItem, (s) =>
    {
        iRunner.Result(iId + " remove " + s);
    });
*/
}
template <class T>
void ResultOrderedWatcher<T>::OrderedRemoveCallback(const Brx& aValue)
{
    iResultRemoveBuf.Replace(iId);
    iResultRemoveBuf.Append(Brn(" remove "));
    iResultRemoveBuf.Append(aValue);
    iRunner.Result(iResultRemoveBuf);
/*
    iAction(aItem, (s) =>
    {
        iRunner.Result(iId + " remove " + s);
    });
*/
}


template <class T>
void ResultOrderedWatcher<T>::OrderedClose()
{
}


template <class T>
void ResultOrderedWatcher<T>::Dispose()
{
    iWatchable.RemoveWatcher(this);
}

//////////////////////////////////////////////////////////////////////////////////

/* not used
void MockableExtensions::Execute(IMockable aMockable, const Brx& aValue)
{
    aMockable.Execute(Tokeniser.Parse(aValue));
}
*/
