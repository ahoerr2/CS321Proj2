
/* This is a simple process scheduler, it will generate a set number of jobs and then select the jobs from multiple processes simultaneously and execute them*/
/* This is processed by using three files that act as queues, in addition each queue operation has critical regions to protect from face condition*/
/* By pressing control C instead of terminating the program it will process all the jobs of the power user queue */

/* include c header files */
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h> // for function fork()
#include <stdio.h>
#include <time.h> //for generate random seed

// include c++ header files
#include <cstring>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define N 100 //N is the number of the worker processes. You may increase N to 100 when your program runs correctly
#define M 50 //M is the number of jobs. You may increase M to 50 when your program runs correctly
#define debug 1
using namespace std;

// TODO: QUEUE job size

//C++ functions
void jobQueueAppend(int n, string queueString, string jobToProcess);
void popLineFromFile(fstream &stream, const string streamFileName);
string genJobProcess(int n);
void blockExecutionByPriority(const string &queuePriority);
void runTaskCode(const int &jobID, const string &queuePriority);
void setJobQueues();
void jobGenerator();
void jobScheduler();
string selectJob();
void executeJob(int n, const string &token);
void down(int *semid, char *semname);
void up(int semid, char *semname);
const string SERVER_QUEUE = "queueServerFile";
const string POW_USER_QUEUE = "queuePUserFile";
const string USER_QUEUE = "queueRUserFile";

// For processing signals
void wake_up(int s);
int intr = 0;

// For handling semnaphores
int semid;
char semname[10];

// Defines the semnaphore as a mutex, forks a process to generate jobs and to run the scheduler, waits for all the children to complete and then turns off.
int main()
{
    strcpy(semname, "mutex");
    int pid = 0;
    pid_t wpid;
    int status;
    signal(SIGINT, wake_up);
    setJobQueues(); /* Set up the priority job queues with chosen file and/or data structure */
    if (pid = fork() > 0)
    {                   /* jobGenerator process */
        jobGenerator(); /* generate random jobs and put them into the priority queues. The priority queues must be protected in a critical region */
        exit(0);
    }
    else
    {                   /* job scheduler process */
        jobScheduler(); /* schedule and execute the jobs. */
        while ((wpid = wait(&status)) > 0);
        cout << "Exiting Program..." << endl;
        exit(0);
    }
    //while ((wpid = wait(&status)) > 0);
    //cout << "Exiting Program..." << endl;
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

//Adds the line denoting a job to process to the end of the queue
void jobQueueAppend(int n, fstream &queue, string jobToProcess)
{
    queue << n << '|' << jobToProcess << "\n";
}

//Deprecated job process generator
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

// Generates a randon job number, that job number is then inserted to one of three queues with the size of number determining what queue it goes in
// Each of the queues is opened in a critical region to prevent race conditions
void jobGenerator()
{
    int i = 0, n = 0;
    cout << "jobGenerator: Use a loop to generate M jobs in the priority queue: \n";
    // initialize random seed
    srand(time(0));
    string serverAppend = "";
    string pUserAppend = "";
    string rUserAppend = "";
    fstream queueServer;
    fstream queuePUser;
    fstream queueRUser;
    while (i < M)
    {
        // generate a random number between 1-100
        n = rand() % 100 + 1;
        string jobToProcess = genJobProcess(n);
        cout << "jobGenerator: Job number is : " << n << endl;

        // Place random number in one of three queues and open a critical region to protect from race conditions

        //Place job number in the server queue by appending it to the end of the file
        if (n >= 1 && n <= 30)
        {
            cout << "made it to down " << n << endl;
            down(&semid , semname);
            queueServer.open(SERVER_QUEUE, ios::out | ios::app);
            cout << "jobGenerator: job placed in server queue " << n << endl;
            jobQueueAppend(n, queueServer, jobToProcess);
            queueServer.close();
            up(semid, semname);
        }

        // Place job in power user queue by appending it to the end of the file
        else if (n >= 31 && n <= 60)
        {
            down(&semid, semname);
            queuePUser.open(POW_USER_QUEUE, ios::out | ios::app);
            cout << "jobGenerator: job placed in power user queue " << n << endl;
            jobQueueAppend(n, queuePUser, jobToProcess);
            queuePUser.close();
            up(semid, semname);
        }

        // Place job in user queue by appending job to the end of the file
        else if (n >= 61 && n <= 100)
        {
            down(&semid, semname);
            queueRUser.open(USER_QUEUE, ios::out | ios::app);
            cout << "jobGenerator: job placed in user queue " << n << endl;
            jobQueueAppend(n, queueRUser, jobToProcess);
            queueRUser.close();
            up(semid, semname);
        }

        usleep(100); //100 can be adjusted to synchronize the job generation and job scheduling processes.
        i++;
    }
}

// The Scheduler schedules a number of jobs by first selecting a job from the highest priority queues
void jobScheduler()
{
    string jobString;
    string jobToken;
    int i = 0, n = 0, pid = 0;
    /* schedule and run maximum N jobs */
    while (i < N)
    {                            
        jobString = selectJob(); /* pick a job from the job priority queues */
        //cout << "jobString: " << jobString << endl;
        n = stoi(jobString.substr(0, jobString.find('|')));
        jobToken = jobString.substr(jobString.find('|') + 1, jobString.find(';') - jobString.find('|') - 1);
        //cout << "N INDEX: " << n << endl;
        //cout << "TOKEN: " << jobToken << endl;
        
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
    fstream queueServer;
    fstream queuePUser;
    fstream queueRUser;
    int index = 0;
    string job;
    cout << "selectJob: Select a highest priority job from the priority queue: \n";

    down(&semid, semname);
    queueServer.open(SERVER_QUEUE, ios::in);
    if (getline(queueServer, job))
    {
        queueServer.close();
        up(semid,semname);
        down(&semid, semname);
        queueServer.open(SERVER_QUEUE, ios::in);
        //cout << "server isn't empty!" << endl;
        popLineFromFile(queueServer, SERVER_QUEUE);
        queueServer.close();
        up(semid, semname);
        return job;
    }
    queueServer.close();
    up(semid, semname);

    down(&semid, semname);
    queuePUser.open(POW_USER_QUEUE, ios::in);
    if (getline(queuePUser, job))
    {
        queuePUser.close();
        up(semid, semname);
        down(&semid, semname);
        queuePUser.open(POW_USER_QUEUE, ios::in);
        //cout << "pu isn't empty!" << endl;
        popLineFromFile(queuePUser, POW_USER_QUEUE);
        up(semid, semname);
        return job;
    }
    queuePUser.close();
    up(semid, semname);

    down(&semid, semname);
    queueRUser.open(USER_QUEUE, ios::in);
    if (getline(queueRUser, job))
    {
        queueRUser.close();
        up(semid, semname);
        down(&semid, semname);
        queueRUser.open(USER_QUEUE, ios::in);
        //cout << "u isn't empty!" << endl;
        popLineFromFile(queueRUser, USER_QUEUE);
        up(semid, semname);
        return job;
    }
    up(semid, semname);

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
        runTaskCode(n, SERVER_QUEUE);
    }
    else if (n >= 31 && n <= 60)
    {
        cout << "executeJob: execute power user job " << n << endl;
        runTaskCode(n, POW_USER_QUEUE);
    }
    else if (n >= 61 && n <= 100)
    {
        cout << "executeJob: execute user job " << n << endl;
        runTaskCode(n, USER_QUEUE);
    }
}

void runTaskCode(const int& jobID, const string& queuePriority){
    if(queuePriority == SERVER_QUEUE){
        cout << "Pid: " << getpid() << endl;
    }
    else if (queuePriority == POW_USER_QUEUE)
    {
        int pid = getpid();
        cout << "Process " << pid << " will go to sleep and wait for the ^C signal to wake up\n";
        signal(SIGINT, wake_up);
        while(!intr); //wait for the ^C to wake up
        cout << "Power User Process has woken up" << endl;
        exit(0);
    }
    else if (queuePriority == USER_QUEUE)
    {
        cout << "job id: " << jobID << endl;
        sleep(2);
        cout << "I am wake up" << endl;
        return;
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

void wake_up(int s)
{
    cout << "\nI will wake up now.\n";
    intr = 1;
}
