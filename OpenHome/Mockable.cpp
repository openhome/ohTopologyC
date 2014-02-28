#include <OpenHome/Mockable.h>
#include <OpenHome/Private/Ascii.h>
#include <OpenHome/Private/Debug.h>


using namespace OpenHome;
using namespace Av;
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
void MockableScriptRunner::Run(Functor aWait, IReader& aStream, IMockable& aMockable)
{

    TBool wait = true;
    TBool eof = false;
    Brn lastline;

    try
    {
        lastline = aStream.ReadUntil('\n');
        //LOG(kTrace, "\n### start...\n");
        //LOG(kTrace, "lastline = \n");
        //LOG(kTrace, lastline);
        //LOG(kTrace, "\n");
    }
    catch(ReaderError)
    {
        ASSERTS(); // stream is empty!
    }


    for (;;)
    {
        LOG(kTrace, "\n\n");
        iLine.Replace(lastline);

        try
        {
            lastline = aStream.ReadUntil('\n');
            //LOG(kTrace, "\n### for loop\n");
            //LOG(kTrace, "lastline = ");
            //LOG(kTrace, lastline);
            //LOG(kTrace, "\n");
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

            //LOG(kTrace, "iLine = ");
            //LOG(kTrace, iLine);
            //LOG(kTrace, "\n");

            try
            {
                lastline = aStream.ReadUntil('\n');
                //LOG(kTrace, "lastline = ");
                //LOG(kTrace, lastline);
                //LOG(kTrace, "\n");
            }
            catch(ReaderError)
            {
                eof = true;
            }
        }


        //LOG(kTrace, "finshed search loop\n");

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
        //var commands = Tokeniser.Parse(line);

        //if (commands.Any())
        if (commands.Count()>0)
        {
            LOG(kTrace, "commands(%d): ", commands.Count());

            //LOG(kTrace, "\n");
            //LOG(kTrace, "commands.Remaining() = \n");
            LOG(kTrace, commands.Remaining());
            LOG(kTrace, "\n");


            Brn command = commands.Next();

            LOG(kTrace, "command = ");
            LOG(kTrace, command);
            LOG(kTrace, "\n");



            if (Ascii::CaseInsensitiveEquals(command, Brn("mock")))
            {
                LOG(kTrace, "mock... \n");
                //Console.WriteLine(line);
                //aMockable.Execute(commands.Skip(1));
                aMockable.Execute(commands);

                wait = true;
            }
            else if (Ascii::CaseInsensitiveEquals(command, Brn("expect")))
            {
                LOG(kTrace, "expect... \n");
                if (wait)
                {
                    //LOG(kTrace, "wait = true \n");
                    try
                    {
                        aWait();
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

                    LOG(kTrace, "expected = ");
                    LOG(kTrace, expected);
                    LOG(kTrace, "\nresult = ");
                    LOG(kTrace, result);

                    Assert(result, expected);
                }
                else
                {
                    THROW(QueueEmpty);
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
                    aWait();
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
                THROW(NotSupportedException);
            }
        }

        if (eof)
        {
            break;
        }
    }

}


/**

 */
void MockableScriptRunner::Result(const Brx& aValue)
{
    LOG(kTrace, "MockableScriptRunner::Result \n");
    iResultQueue.Write(Brn(aValue));
    //Console.WriteLine(aValue);
}


/**

 */
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

