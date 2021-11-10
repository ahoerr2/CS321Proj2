
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

#define N 30 //N is the number of the worker processes. You may increase N to 100 when your program runs correctly
#define M 30 //M is the number of jobs. You may increase M to 50 when your program runs correctly
#define debug 1
using namespace std;

// TODO: QUEUE job size

//C++ functions
void jobQueueAppend(int n, string queueString);
void popLineFromFile(fstream &stream, const string streamFileName);
string genJobProcess(int n);
void blockExecutionByPriority(const string &queuePriority);
void runTaskCode(const int &jobID, const string &queuePriority);
void setJobQueues();
void jobGenerator();
void jobScheduler();
string selectJob();
void executeJob(int n);
void down(int *semid, char *semname);
void up(int semid, char *semname);
int getFileLinesNum(fstream &queue);

const string SERVER_QUEUE = "queueServerFile";
const string POW_USER_QUEUE = "queuePUserFile";
const string USER_QUEUE = "queueRUserFile";
const int MAX_SERVER_QUEUE = 15;
const int MAX_POW_USER_QUEUE = 15;
const int MAX_USER_QUEUE = 15;

int serverFileSize = 0;
int rUserFileSize = 0;
int pUserFileSize = 0;

// For processing signals
void wake_up(int s);
int intr = 0;
bool signalCleared = true;

// For handling semnaphores
int semid;
char semname[10];

// Defines the semnaphore as a mutex, forks a process to generate jobs and to run the scheduler, waits for all the children to complete and then turns off.
int main()
{
    strcpy(semname, "mutex");
    int pid = 0;
    int status;
    signal(SIGINT, wake_up);
    setJobQueues(); /* Set up the priority job queues with chosen file and/or data structure */
    if (pid = fork() > 0)
    {                   /* jobGenerator process */
        jobGenerator(); /* generate random jobs and put them into the priority queues. The priority queues must be protected in a critical region */
        cout << "Exiting Program..." << endl;
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

//Adds the line denoting a job to process to the end of the queue
void jobQueueAppend(int n, fstream &queue)
{
    queue << n << '|' << "VALID"
          << "\n";
}

int getFileLinesNum(fstream &queue)
{
    int lineNum = 0;
    string line;
    while (std::getline(queue, line))
        ++lineNum;
    return lineNum;
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
    int lines = 0;
    while (i < M)
    {
        // generate a random number between 1-100
        n = rand() % 100 + 1;
        cout << "jobGenerator: Job number is : " << n << endl;

        // Place random number in one of three queues and open a critical region to protect from race conditions

        //Place job number in the server queue by appending it to the end of the file
        if (n >= 1 && n <= 30)
        {
            cout << "made it to down " << n << endl;
            down(&semid, semname);
            queueServer.open(SERVER_QUEUE, ios::in);
            lines = getFileLinesNum(queueServer);
            queueServer.close();
            if (lines <= MAX_SERVER_QUEUE)
            {
                queueServer.open(SERVER_QUEUE, ios::out | ios::app);
                cout << "jobGenerator: job placed in server queue " << n << endl;
                cout << "server File Size: " << lines << endl;
                jobQueueAppend(n, queueServer);
                queueServer.close();
            }
            else
            {
                cout << "jobGenerator: server queue is full " << n << endl;
                cout << "server File Size: " << lines << endl;
            }
            up(semid, semname);
        }

        // Place job in power user queue by appending it to the end of the file
        else if (n >= 31 && n <= 60)
        {
            down(&semid, semname);
            queuePUser.open(POW_USER_QUEUE, ios::in);
            lines = getFileLinesNum(queuePUser);
            queuePUser.close();
            if (lines <= MAX_POW_USER_QUEUE)
            {
                queuePUser.open(POW_USER_QUEUE, ios::out | ios::app);
                cout << "jobGenerator: job placed in power user queue " << n << endl;
                cout << "pow user File Size: " << lines << endl;
                jobQueueAppend(n, queuePUser);
                queuePUser.close();
            }
            else
            {
                cout << "jobGenerator: pow user queue is full " << n << endl;
                cout << "pow user File Size: " << lines << endl;
            }
            queuePUser.close();
            up(semid, semname);
        }

        // Place job in user queue by appending job to the end of the file
        else if (n >= 61 && n <= 100)
        {
            down(&semid, semname);
            queueRUser.open(USER_QUEUE, ios::in);
            lines = getFileLinesNum(queueRUser);
            queueRUser.close();
            if (lines <= MAX_USER_QUEUE)
            {
                queueRUser.open(USER_QUEUE, ios::out | ios::app);
                cout << "jobGenerator: job placed in user queue " << n << endl;
                cout << "user File Size: " << lines << endl;
                jobQueueAppend(n, queueRUser);
                queueRUser.close();
            }
            else
            {
                cout << "jobGenerator: user queue is full " << n << endl;
                cout << "user File Size: " << lines << endl;
            }
            queueRUser.close();
            up(semid, semname);
        }

        usleep(10000); //100 can be adjusted to synchronize the job generation and job scheduling processes.
        i++;
    }
}

// The Scheduler schedules a number of jobs by first selecting a job from the highest priority queues, then it splits the string to find the job number and the debug token
// The number and token are then fed into a function that will detrimine the job priority level and execute job
void jobScheduler()
{
    string jobString;
    string jobToken;
    int i = 0, n = 0, pid = 0;
    int status = 0;
    /* schedule and run maximum N jobs */
    while (i < N)
    { /* pick a job from the job priority queues */
        jobString = selectJob();
        //cout << "jobString: " << jobString << endl;
        n = stoi(jobString.substr(0, jobString.find('|')));
        //jobToken = jobString.substr(jobString.find('|') + 1, jobString.find(';') - jobString.find('|') - 1);

        //Debug info to check how jobs are processed
        //cout << "N INDEX: " << n << endl;
        //cout << "TOKEN: " << jobToken << endl;

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

// Selects a job from the highest priority queue (ORDER: SERVER, POW USER, USER), then returns the job string if a job was found or zero to skip the job if nothing was in queues
string selectJob()
{
    // Filestream variables and variable to denote the extracted job
    fstream queueServer;
    fstream queuePUser;
    fstream queueRUser;
    string job;

    cout << "selectJob: Select a highest priority job from the priority queue: \n";

    // Puts down the semaphore and checks if there is any data in server queue, if so it will extract the top line of the queue, delete the line and return it
    down(&semid, semname);
    queueServer.open(SERVER_QUEUE, ios::in);
    if (getline(queueServer, job))
    {
        queueServer.close();
        up(semid, semname);

        //The top line of the file is deleted, and the queue size is decremented
        down(&semid, semname);
        queueServer.open(SERVER_QUEUE, ios::in);
        popLineFromFile(queueServer, SERVER_QUEUE);
        serverFileSize--;
        queueServer.close();
        up(semid, semname);

        return job;
    }
    queueServer.close();
    up(semid, semname);

    // Puts down the semaphore and checks if there is any data in pow user queue, if so it will extract the top line of the queue, delete the line and return it
    down(&semid, semname);
    queuePUser.open(POW_USER_QUEUE, ios::in);
    if (getline(queuePUser, job))
    {
        queuePUser.close();
        up(semid, semname);

        //The top line of the file is deleted, and the queue size is decremented
        down(&semid, semname);
        queuePUser.open(POW_USER_QUEUE, ios::in);
        popLineFromFile(queuePUser, POW_USER_QUEUE);
        pUserFileSize--;
        up(semid, semname);

        return job;
    }
    queuePUser.close();
    up(semid, semname);

    // Puts down the semaphore and checks if there is any data in pow user queue, if so it will extract the top line of the queue, delete the line and return it
    down(&semid, semname);
    queueRUser.open(USER_QUEUE, ios::in);
    if (getline(queueRUser, job))
    {
        queueRUser.close();
        up(semid, semname);

        //The top line of the file is deleted, and the queue size is decremented
        down(&semid, semname);
        queueRUser.open(USER_QUEUE, ios::in);
        popLineFromFile(queueRUser, USER_QUEUE);
        rUserFileSize--;
        up(semid, semname);
        return job;
    }
    up(semid, semname);

    // If all the queues are empty zero is returned to skip executing the job
    return "0|NULL";
}

// Removes the top line from the file by copying every line from the queue file besides the top line to a temp queue
// Then the old queue file is deleted and the temp file is renamed to the queues file name, effectively decrementing the queue
void popLineFromFile(fstream &stream, const string streamFileName)
{
    // Creating the temp fileand variables to copy
    fstream tempFile;
    tempFile.open("temp", ios::out);
    int index = 0;
    string streamLine = "";
    string ret = "";

    // Copy every line to the temp file but the first line
    while (getline(stream, streamLine))
    {
        if (index == 0)
        {
            ++index;
            continue;
        }
        tempFile << streamLine << endl;
        index++;
    }
    tempFile.close();

    // The old queue file is deleted and the copied queue file becomes the new queue file.
    remove(streamFileName.c_str());
    rename("temp", streamFileName.c_str());
}

// Determines what priority the job is by its job number then executes a specific job routiene for each job priority
void executeJob(int n)
{
    // Runs server queue job, which prints out its pid and exits
    if (n >= 1 && n <= 30)
    {
        cout << "executeJob: execute server job " << n << endl;
        cout << "server job pid is: " << getpid() << endl;
        runTaskCode(n, SERVER_QUEUE);
    }
    // Runs power user queue job, which waits for control C signal for user and then prints that it has woken up and exits
    else if (n >= 31 && n <= 60)
    {
        cout << "executeJob: execute power user job " << n << endl;
        runTaskCode(n, POW_USER_QUEUE);
    }
    // Runs user queue job, which prints job id, sleeps for two seconds, and then prints its woken up and exits
    else if (n >= 61 && n <= 100)
    {
        cout << "executeJob: execute user job " << n << endl;
        runTaskCode(n, USER_QUEUE);
    }
}

// Responsible for the executing each specific routiens of each job priority
void runTaskCode(const int &jobID, const string &queuePriority)
{
    if (queuePriority == SERVER_QUEUE)
    {
        cout << "Pid: " << getpid() << endl;
    }
    else if (queuePriority == POW_USER_QUEUE)
    {
        int pid = getpid();
        cout << "Process " << pid << " will go to sleep and wait for the ^C signal to wake up\n";
        signal(SIGINT, wake_up);
        while (!intr)
            ; //wait for the ^C to wake up
        cout << "Power User Process has woken up" << endl;
    }
    else if (queuePriority == USER_QUEUE)
    {
        cout << "job id: " << jobID << endl;
        sleep(2);
        cout << "I am wake up" << endl;
        return;
    }
}

// Part of the semaphore system, sets the mutex into a down lock position that doesnt let other programs enter the critical region
void down(int *semid, char *semname)
{
    while (*semid = creat(semname, 0) == -1) /* && error == EACCES)*/
    {
        cout << "down " << semname << ": I am blocked.\n";
        sleep(1);
    }
}

// Part of the semaphore system, sets the mutex into a up unlock position that let other programs enter a critical region
void up(int semid, char *semname)
{
    close(semid);
    unlink(semname);
    cout << "up " << semname << ": I am waked up.\n";
}

// When the signal is inputted an integer is set to one which allows all programs waiting on the signal to exit
void wake_up(int s)
{
    cout << "\nI will wake up now.\n";
    intr = 1;
}
