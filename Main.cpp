// lab04.cpp
/* Simple process scheduler simulator program */
/* include c header files */
#include <stdlib.h>
#include <unistd.h> // for function fork()
#include <stdio.h>
#include <time.h> //for generate random seed
// include c++ header files
#include <string>
#include <iostream>
#include <fstream>
#include <queue>
#include <forward_list>
#define N 3 //N is the number of the worker processes. You may increase N to 100 when your program runs correctly
#define M 3 //M is the number of jobs. You may increase M to 50 when your program runs correctly
#define debug 1
using namespace std;

// TODO: DELETE
void showq(queue<string> gq);

void jobQueueAppend(int n, string queueString, string jobToProcess);
string genJobProcess(int n);
int retrieveJobFromStream(fstream &stream);
void setJobQueues();
void jobGenerator();
void jobScheduler();
bool isFileEmpty(fstream &queue);
int selectJob();
void executeJob(int n);
const string SERVER_QUEUE = "queueServerFile";
const string POW_USER_QUEUE = "queuePUserFile";
const string USER_QUEUE = "queueRUserFile";

fstream queueServer;
fstream queuePUser;
fstream queueRUser;
forward_list<queue<string>> queueList;

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
        cout << '\t' << g.front();
        g.pop();
    }
    cout << '\n';
}

void jobQueueAppend(int n, string queueString, string jobToProcess)
{
    if (queueString == SERVER_QUEUE)
        queueServer << n << '|' << jobToProcess << ";";
    if (queueString == POW_USER_QUEUE)
        queuePUser << n << '|' << jobToProcess << ";";
    if (queueString == USER_QUEUE)
        queueRUser << n << '|' << jobToProcess << ";";
}

string genJobProcess(int n)
{
    // TODO: Finish job process generator
    return "TODO";
}

void jobGenerator()
{
    int i = 0, n = 0;
    cout << "jobGenerator: Use a loop to generate M jobs in the priority queue: \n";
    // initialize random seed
    srand(time(0));
    while (i < M)
    {
        // generate a random number between 1-100
        n = rand() % 100 + 1;
        string jobToProcess = genJobProcess(n);
        cout << "jobGenerator: Job number is : " << n << endl;
        
        /*
        if (n >= 1 && n <= 30)
        {
            cout << "jobGenerator: job placed in server queue " << n << endl;
            jobQueueAppend(n, SERVER_QUEUE, jobToProcess);
        }
        else if (n >= 31 && n <= 60)
        {
            cout << "jobGenerator: job placed in power user queue " << n << endl;
            jobQueueAppend(n, POW_USER_QUEUE, jobToProcess);
        }
        else if (n >= 61 && n <= 100)
        {
            cout << "executeJob: job placed in user queue " << n << endl;
            jobQueueAppend(n, USER_QUEUE, jobToProcess);
        }
        */
        for (queue<string> &queue : queueList){
        if (n >= 1 && n <= 30)
        {
            cout << "jobGenerator: job placed in server queue " << n << endl;
            jobQueueAppend(n, SERVER_QUEUE, jobToProcess);
        }
        else if (n >= 31 && n <= 60)
        {
            cout << "jobGenerator: job placed in power user queue " << n << endl;
            jobQueueAppend(n, POW_USER_QUEUE, jobToProcess);
        }
        else if (n >= 61 && n <= 100)
        {
            cout << "executeJob: job placed in user queue " << n << endl;
            jobQueueAppend(n, USER_QUEUE, jobToProcess);
        }
        }

        usleep(100); //100 can be adjusted to synchronize the job generation and job scheduling processes.
        i++;
    }
}

void jobScheduler()
{
    int i = 0, n = 0, pid = 0;
    while (i < N)
    {                    /* schedule and run maximum N jobs */
        n = selectJob(); /* pick a job from the job priority queues */
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

int selectJob()
{
    queueServer.close();
    queuePUser.close();
    queueRUser.close();
    int n = -1;
    cout << "selectJob: Select a highest priority job from the priority queue: \n";

    // TODO: Rewrite the function
    std::ifstream serverStream(SERVER_QUEUE);
    string serverContents;
    getline(serverStream, serverContents);
    cout << "Contents:" << serverContents << ":" << endl;

    if (!serverContents.empty())
    {
        cout << "selectJob: Server queue isn't empty: \n";
        return stoi(serverContents.substr(0));
    }
    queueServer.close();

    queuePUser.open(POW_USER_QUEUE, ios::in);
    if (isFileEmpty(queuePUser))
    {
        cout << "selectJob: Pow User queue isn't empty: \n";
        n = retrieveJobFromStream(queuePUser);
        queuePUser.close();
        return n;
    }
    queuePUser.close();

    queueRUser.open(USER_QUEUE, ios::in);
    if (isFileEmpty(queueRUser))
    {
        cout << "selectJob: User queue isn't empty: \n";
        n = retrieveJobFromStream(queueRUser);
        queueRUser.close();
        return n;
    }
    queueRUser.close();

    cout << "output n is:" << n << "\n";
    return n;
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