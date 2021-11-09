// lab04.cpp
/* Simple process scheduler simulator program */
/* include c header files */
#include <stdlib.h>
#include <unistd.h> // for function fork()
#include <stdio.h>
#include <time.h> //for generate random seed
// include c++ header files
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <queue>
#include <list>
#define N 3 //N is the number of the worker processes. You may increase N to 100 when your program runs correctly
#define M 3 //M is the number of jobs. You may increase M to 50 when your program runs correctly
#define debug 1
using namespace std;

// TODO: DELETE
void showq(queue<string> gq);

string queueListPop(list<queue<string>> &queueList, int at);
void jobQueueAppend(int n, string queueString, string jobToProcess);
string genJobProcess(int n);
int retrieveJobFromStream(fstream &stream);
void queueListPush(list<queue<string>> &queueList, string element, int at);
void setJobQueues();
void jobGenerator();
void jobScheduler();
bool isFileEmpty(fstream &queue);
string selectJob();
void executeJob(int n);
const string SERVER_QUEUE = "queueServerFile";
const string POW_USER_QUEUE = "queuePUserFile";
const string USER_QUEUE = "queueRUserFile";

fstream queueServer;
fstream queuePUser;
fstream queueRUser;
list<queue<string>> queueList;
vector<string> jobProcessVector;

int main()
{
    int pid = 0;
    setJobQueues(); /* Set up the priority job queues with chosen file and/or data structure */
    if (pid = fork() > 0)
    {                   /* jobGenerator process */
        jobGenerator(); /* generate random jobs and put them into the priority queues. The priority queues must be protected in a critical region */
        exit(0);
    }
    else
    {                   /* job scheduler process */
        jobScheduler(); /* schedule and execute the jobs. */
        exit(0);
    }
    return (1);
}

// These create files for three different types of queues, a high priority server queue, a power user queue
// And a normal user queue
void setJobQueues()
{
    //Remove any potential leftover queues (in case of fatal error preventing cleanup)
    queue<string> server;
    queue<string> pUser;
    queue<string> rUser;
    queueList.assign({server, pUser, rUser});

    // TODO: DELETE DEBUG INFO
    for (queue<string> &a : queueList)
        showq(a);
    cout << endl;
}

void showq(queue<string> gq)
{
    queue<string> g = gq;
    while (!g.empty())
    {
        cout << g.front();
        cout << " ";
        g.pop();
    }
    cout << '\n';
}

void queueListPush(list<queue<string>> &queueList, string element, int at)
{
    list<queue<string>>::iterator pos = queueList.begin();
    for (size_t i = 0; i < at; i++)
    {
        ++pos;
    }
    pos->push(element);
}
string genJobProcess(int n)
{
    string ret = n + "|";
    ret += "TODO";
    ret += ";";
    return ret;
}

void jobGenerator()
{
    int i = 0, n = 0;
    string jobToProcess;
    cout
        << "jobGenerator: Use a loop to generate M jobs in the priority queue: \n";
    // initialize random seed
    srand(time(0));
    while (i < M)
    {
        // generate a random number between 1-100
        n = rand() % 100 + 1;
        jobToProcess = "1|TODO";
        cout << "jobGenerator: Job number is : " << n << endl;
        cout << "jobGenerator: Job is : " << jobToProcess << endl;
        if (n >= 1 && n <= 30)
        {
            cout << "jobGenerator: job placed in server queue " << n << endl;
            queueListPush(queueList, jobToProcess, 0);
        }
        else if (n >= 31 && n <= 60)
        {
            cout << "jobGenerator: job placed in power user queue " << n << endl;
            queueListPush(queueList, jobToProcess, 1);
        }
        else if (n >= 61 && n <= 100)
        {
            cout << "executeJob: job placed in user queue " << n << endl;
            queueListPush(queueList, jobToProcess, 2);
        }

        // TODO: Remove after debug
        if (queueList.front().empty())
        {
            cout << "server queue empty!" << endl;
        }
        /*
        list<queue<string>>::iterator pos;
        for (pos = queueList.begin(); pos != queueList.end(); pos++)
        {
            showq(*pos);
        }
        */
        usleep(100); //100 can be adjusted to synchronize the job generation and job scheduling processes.
        i++;
    }
}

void jobScheduler()
{
    string jobString;
    string jobToken;
    int i = 0, n = 0, pid = 0;
    while (i < N)
    {                            /* schedule and run maximum N jobs */
        jobString = selectJob(); /* pick a job from the job priority queues */
        n = stoi(jobString.substr(0, jobString.find('|')));
        jobToken = jobString.substr(jobString.find('|') + 1, jobString.find(';') - jobString.find('|') - 1);
        cout << "N INDEX: " << n << endl;
        cout << "TOKEN: " << jobToken << endl;

        if (n > 0)
        { /* valid job id */
            if (pid = fork() == 0)
            {                  /* child worker process */
                executeJob(n); /* execute the job */
                exit(0);
            }
        }
        i++;
    }
}

string selectJob()
{
    queueServer.close();
    queuePUser.close();
    queueRUser.close();
    string n = "0|NULL";

    if (queueList.front().empty())
    {
        cout << "server queue empty!" << endl;
    }
    if (queueList.back().empty())
    {
        cout << "user queue empty!" << endl;
    }
    cout << "selectJob: Select a highest priority job from the priority queue: \n";

    // TODO: Rewrite the function

    n = queueListPop(queueList, 0);

    cout << "output n is:" << n << "\n";
    return n;
}

string queueListPop(list<queue<string>> &queueList, int at)
{
    string ret = "-1|NULL";
    list<queue<string>>::iterator pos;
    for (pos = queueList.begin(); pos != queueList.end(); pos++)
    {
        if (pos->empty())
        {
            cout << "QUEUE EMPTY!" << endl;
        }
        else
        {
            ret = pos->front();
            pos->pop();
            break;
        }
    }
    return ret;
}

int retrieveJobFromStream(fstream &stream)
{
    return stream.peek();
}

bool isFileEmpty(fstream &queue)
{
    return queue.peek() == ifstream::traits_type::eof();
}

void executeJob(int n)
{
    if (n >= 1 && n <= 30)
    {
        cout << "executeJob: execute server job " << n << endl;
        /* ... */
    }
    else if (n >= 31 && n <= 60)
    {
        cout << "executeJob: execute power user job " << n << endl;
        /* ... */
    }
    else if (n >= 61 && n <= 100)
    {
        cout << "executeJob: execute user job " << n << endl;
        /* ... */
    }
    else
    {
        cout << "Invalid job #: " << n << endl;
    }
}