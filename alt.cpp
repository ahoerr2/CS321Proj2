// lab04.cpp
/* Simple process scheduler simulator program */
/* include c header files */
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <cmath>
#include <unistd.h> // for function fork()
#include <stdio.h>
#include <time.h> //for generate random seed
// include c++ header files
#include <cstring>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#define N 10 //N is the number of the worker processes. You may increase N to 100 when your program runs correctly
#define M 10 //M is the number of jobs. You may increase M to 50 when your program runs correctly
#define debug 1
using namespace std;

void jobQueueAppend(int n, string queueString, string jobToProcess);
void popLineFromFile(fstream &stream, const string streamFileName);
string genJobProcess(int n);
void runTaskCode(const string &taskCode, const string &queuePriority);
void blockExecutionByPriority(const string &queuePriority);
void setJobQueues();
void jobGenerator();
void jobScheduler();
string selectJob();
void executeJob(int n, const string &token);
const string SERVER_QUEUE = "queueServerFile";
const string POW_USER_QUEUE = "queuePUserFile";
const string USER_QUEUE = "queueRUserFile";

vector<string> queueVector;

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
    cout << "Alt: Set up the job priority queue: \n";
    fstream queueServer;
    fstream queuePUser;
    fstream queueRUser;
    queueServer.open(SERVER_QUEUE, ios::out | ios::trunc);
    queueServer.close();
    queuePUser.open(POW_USER_QUEUE, ios::out | ios::trunc);
    queuePUser.close();
    queueRUser.open(USER_QUEUE, ios::out | ios::trunc);
    queueRUser.close();
}

void jobQueueAppend(int n, fstream &queue, string jobToProcess)
{
    queue << n << '|' << jobToProcess << "\n";
}

string genJobProcess(int n)
{
    string ret = "ERROR";
    switch (n % 5)
    {
    //Sleep(random amount of time)
    case 0:
        ret = "slp";
        break;
        // Prints hello world
    case 1:
        ret = "phw";
        break;
        // Print a paradox
    case 2:
        ret = "pap";
        break;
        // Prints out a power of two between one and ten
    case 3:
        ret = "pot";
        break;
        // Prints out the parent pid of the process
    case 4:
        ret = "gid";
        break;
    }
    return ret;
}

void jobGenerator()
{
    int i = 0, n = 0;
    cout << "jobGenerator: Use a loop to generate M jobs in the priority queue: \n";
    // initialize random seed
    srand(time(0));
    string serverAppend = "";
    string pUserAppend = "";
    string rUserAppend = "";
    // TODO: Open semaphone
    fstream queueServer;
    fstream queuePUser;
    fstream queueRUser;
    while (i < M)
    {
        // generate a random number between 1-100
        n = rand() % 100 + 1;
        string jobToProcess = genJobProcess(n);
        cout << "jobGenerator: Job number is : " << n << endl;

        if (n >= 1 && n <= 30)
        {
            queueServer.open(SERVER_QUEUE, ios::out | ios::app);
            cout << "jobGenerator: job placed in server queue " << n << endl;
            jobQueueAppend(n, queueServer, jobToProcess);
            queueServer.close();
        }
        else if (n >= 31 && n <= 60)
        {
            queuePUser.open(POW_USER_QUEUE, ios::out | ios::app);
            cout << "jobGenerator: job placed in power user queue " << n << endl;
            jobQueueAppend(n, queuePUser, jobToProcess);
            queuePUser.close();
        }
        else if (n >= 61 && n <= 100)
        {
            queueRUser.open(USER_QUEUE, ios::out | ios::app);
            cout << "jobGenerator: job placed in user queue " << n << endl;
            jobQueueAppend(n, queueRUser, jobToProcess);
            queueRUser.close();
        }

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
        //cout << "jobString: " << jobString << endl;
        n = stoi(jobString.substr(0, jobString.find('|')));
        jobToken = jobString.substr(jobString.find('|') + 1, jobString.find(';') - jobString.find('|') - 1);
        cout << "N INDEX: " << n << endl;
        cout << "TOKEN: " << jobToken << endl;
        
        if (n > 0)
        { /* valid job id */
            if (pid = fork() == 0)
            {                  /* child worker process */
                executeJob(n,jobToken); /* execute the job */
                exit(0);
            }
        }
        i++;
    }
}

string selectJob()
{
    // ADD semanophore here
    fstream queueServer;
    fstream queuePUser;
    fstream queueRUser;
    queueServer.open(SERVER_QUEUE, ios::in);
    queuePUser.open(POW_USER_QUEUE, ios::in);
    queueRUser.open(USER_QUEUE, ios::in);

    int index = 0;
    string job;
    cout << "selectJob: Select a highest priority job from the priority queue: \n";
    if (getline(queueServer, job))
    {
        queueServer.close();
        queueServer.open(SERVER_QUEUE, ios::in);
        //cout << "server isn't empty!" << endl;
        popLineFromFile(queueServer, SERVER_QUEUE);
        return job;
    }
    else if (getline(queuePUser, job))
    {
        queuePUser.close();
        queuePUser.open(POW_USER_QUEUE, ios::in);
        //cout << "pu isn't empty!" << endl;
        popLineFromFile(queuePUser, POW_USER_QUEUE);
        return job;
    }
    else if (getline(queueRUser, job))
    {
        queueRUser.close();
        queueRUser.open(USER_QUEUE, ios::in);
        cout << "u isn't empty!" << endl;
        popLineFromFile(queueRUser, USER_QUEUE);
        return job;
    }

    queueServer.close();
    queuePUser.close();
    queueRUser.close();

    return "0|NULL";
}

void popLineFromFile(fstream &stream, const string streamFileName)
{
    // Creates a temp file of all of the passwords, inserts each line into the new file until it finds the username
    // corresponding to the current user name. WHen it does it inserts the current User name and new password onto the line instead
    fstream tempFile;
    tempFile.open("temp", ios::out);
    int index = 0;
    string streamLine = "";
    string ret = "";
    //cout << "Seletcting job..." << endl;
    while (getline(stream, streamLine))
    {
        //cout << "string: " << streamLine << endl;
        if (index == 0)
        {
            //cout << "skip line: " << streamLine << endl;
            ++index;
            continue;
        }
        //cout << "string add back in: " << streamLine << endl;
        tempFile << streamLine << endl;
        index++;
    }
    tempFile.close();

    // The old password file is deleted and the copies password file becomes the new password file.
    remove(streamFileName.c_str());
    rename("temp", streamFileName.c_str());
}

void executeJob(int n, const string& token)
{
    if (n >= 1 && n <= 30)
    {
        cout << "executeJob: execute server job " << n << endl;
        cout << "server job pid is: " << getpid() << endl;
        runTaskCode(token, SERVER_QUEUE);
    }
    else if (n >= 31 && n <= 60)
    {
        cout << "executeJob: execute power user job " << n << endl;
        runTaskCode(token, POW_USER_QUEUE);
    }
    else if (n >= 61 && n <= 100)
    {
        cout << "executeJob: execute user job " << n << endl;
        runTaskCode(token, USER_QUEUE);
    }
}

void runTaskCode(const string& taskCode, const string& queuePriority){
    cout << "Task Code: " << taskCode << endl;
    if (taskCode == "TODO")
    {
        cout << "ERROR: There was no job to process!" << endl;
    }
    else if(taskCode == "slp"){
        usleep(20);
        blockExecutionByPriority(queuePriority);
        usleep(20);
    }
    else if(taskCode == "phw"){
        cout << "Printing Hello World..." << endl;
        blockExecutionByPriority(queuePriority);
        cout << "Hello World!" << endl;
    }
    else if(taskCode == "pap"){
        cout << "Printing a paradox..." << endl;
        blockExecutionByPriority(queuePriority);
        cout << "This statement is false!" << endl;
    }
    else if(taskCode == "pot"){
        cout << "Printing power of two..." << endl;
        int num = rand() % 10 + 1;
        blockExecutionByPriority(queuePriority);
        int ret = pow(2, num);
        cout << "Power of two: " << ret << endl;
    }
    else if(taskCode == "gid"){
        cout << "Printing parent id..." << endl;
        blockExecutionByPriority(queuePriority);
        cout << "Parent id: " << getppid() << endl;
    }
    else{
        cout << "ERROR: task codes do not match!" << endl;
    }
}

void blockExecutionByPriority(const string& queuePriority){
    if (queuePriority == POW_USER_QUEUE)
    {
        cout << "Waiting for signal..." << endl;
        // TODO: ADD SIGNAL CODE
        cout << "Signal Recieved" << endl;
    }
    else if (queuePriority == USER_QUEUE)
    {
        sleep(2);
    }
}

void down(int *semid, char *semname)
{
    while (*semid = creat(semname, 0) == -1) /* && error == EACCES)*/
    {
        cout << "down " << semname << ": I am blocked.\n";
        sleep(1);
    }
}

void up(int semid, char *semname)
{
    close(semid);
    unlink(semname);
    cout << "up " << semname << ": I am waked up.\n";
}