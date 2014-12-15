#include <OpenHome/Mockable.h>
#include <OpenHome/Private/Ascii.h>
#include <OpenHome/Private/Debug.h>

using namespace OpenHome;
using namespace OpenHome::Av;
using namespace std;


EXCEPTION(QueueEmpty);

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
    Brn remaining(aTokens.Remaining());
    iMockables[next]->Execute(aTokens);
}

///////////////////////////////////////////////////////////////////


/**

 */
MockableScriptRunner::MockableScriptRunner()
    :iResultQueue(kMaxFifoEntries)
{
}


/**

 */
TBool MockableScriptRunner::Run(Functor aWait, IReader& aStream, IMockable& aMockable)
{
    LOG(kTrace, " MockableScriptRunner::Run \n");

    TBool wait = true;
    TBool eof = false;
    Brn lastline;

    try
    {
        lastline = aStream.ReadUntil('\n');
    }
    catch(ReaderError)
    {
        ASSERTS(); // stream is empty!
    }

    TUint count = 0;

    for (;;)
    {
        count++;
        //OpenHome::Log::Print("Count = %d\n", count);

        LOG(kTrace, "\n\n");
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
            count++;
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


        if (eof)
        {
            LOG(kTrace, "empty line - break out\n");
            break;
        }


        if (iLine.BeginsWith(Brn("//")))
        {
            LOG(kTrace, "skipping line : ");
            LOG(kTrace, iLine);
            LOG(kTrace, "\n");
            continue;
        }

        CommandTokens commands(iLine);

        if (commands.Count()>0)
        {
            LOG(kTrace, "commands(%d): ", commands.Count());

            LOG(kTrace, commands.Remaining());
            LOG(kTrace, "\n");


            Brn command = commands.Next();

            LOG(kTrace, "command = ");
            LOG(kTrace, command);
            LOG(kTrace, "\n");



            if (Ascii::CaseInsensitiveEquals(command, Brn("mock")))
            {
                LOG(kTrace, "mock... \n");
                aMockable.Execute(commands);

                wait = true;
            }
            else if (Ascii::CaseInsensitiveEquals(command, Brn("expect")))
            {
                LOG(kTrace, "expect... \n");
                if (wait)
                {
                    try
                    {
                        aWait();
                    }
                    catch (Exception e)
                    {
                        OpenHome::Log::Print("exception\n");
                    }

                    wait = false;
                }

                Brn expected = iLine.Split(Brn("expect").Bytes() + 1);

                if (iResultQueue.SlotsUsed()>0)
                {
                    Bwh* result = iResultQueue.Read();

                    if (!Test(*result, expected))
                    {
                        OpenHome::Log::Print("\n######################################################\n");
                        OpenHome::Log::Print("Count = %d\n", count);
                        OpenHome::Log::Print("########################################################\n");
                        OpenHome::Log::Print("########################################################\n");
                        OpenHome::Log::Print("########################################################\n");
                        OpenHome::Log::Print("########################################################\n");
                        OpenHome::Log::Print("########################################################\n");

                        delete result;
                        return(false);
                    }
                    delete result;
                }
                else
                {
                    THROW(QueueEmpty);
                }

            }
            else if (Ascii::CaseInsensitiveEquals(command, Brn("empty")))
            {
                try
                {
                    aWait();
                }
                catch (Exception e)
                {
                    //Console.WriteLine(e);
                     LOG(kTrace, " exception \n");
                }

                if (iResultQueue.SlotsUsed() != 0)
                {
                    ASSERTS();
                }
            }
            else if (Ascii::CaseInsensitiveEquals(command, Brn("break")))
            {
                 LOG(kTrace, "\n");
                //Debugger.Break();
            }
            else
            {
                THROW(NotSupportedException);
            }
        }

        if (eof)
        {
            break;
        }
    }


    return(true);
}


/**

 */
void MockableScriptRunner::Result(Bwh* aValue)
{
    LOG(kTrace, "\nMockableScriptRunner::Result: \n");
    LOG(kTrace, *aValue);
    //Brn actual(*aValue);
    iResultQueue.Write(aValue);
}


/**

 */
TBool MockableScriptRunner::Test(const Brx& aActual, const Brx& aExpected)
{
/*
    OpenHome::Log::Print("\n\nActual/Expected:\n");
    OpenHome::Log::Print(aActual);
    OpenHome::Log::Print("\n");
    OpenHome::Log::Print(aExpected);
*/
    TBool success = aActual.Equals(aExpected);

    if (!success)
    {
        OpenHome::Log::Print("\n\nActual:\n");
        OpenHome::Log::Print(aActual);
        OpenHome::Log::Print("\n");
        OpenHome::Log::Print("\nExpected:\n");
        OpenHome::Log::Print(aExpected);
    }

    return(success);
}


/**

 */
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


ResultWatcherFactory::ResultWatcherFactory(MockableScriptRunner& aRunner)
    :iRunner(aRunner)
{
}



void ResultWatcherFactory::Destroy(const Brx& aId)
{
    std::vector<IDisposable*> v = iWatchers[Brn(aId)];
    for(TUint i=0; i<v.size(); i++)
    {
        v[i]->Dispose();
        delete v[i];
    }

    iWatchers.erase(Brn(aId));
}


void ResultWatcherFactory::Dispose()
{
    for(auto it=iWatchers.begin();it!=iWatchers.end(); it++)
    {
        std::vector<IDisposable*> v = it->second;
        for(TUint i=0; i<v.size(); i++)
        {
            auto x = v[i];
            x->Dispose();
            delete x;
        }

    }

}
