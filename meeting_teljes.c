#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define STR_SIZE 256

typedef struct {
	long mtype;
	char mtext[STR_SIZE];
} msg_t;

int pipefd1[2];
int pipefd2[2];
char *s;
int semid;


void handler(int signumber)
{
  puts("(oktato) Indit");
}

void handler2(int signumber)
{
  puts("(hallgato) Bejelentkezve");
}

void closePipe()
{
    close(pipefd1[0]);
    close(pipefd1[1]);
    close(pipefd2[0]);
    close(pipefd2[1]);
}

void startPipe()
{

    if (pipe(pipefd1) == -1)
    {
        perror("Pipe hivas sikertelen!");
        exit(EXIT_FAILURE);
    }

    if (pipe(pipefd2) == -1)
    {
        perror("Pipe hivas sikertelen!");
        exit(EXIT_FAILURE);
    }
}

int szemafor_letrehozas(const char *pathname, int szemafor_ertek)
{
    int semid;
    key_t key;

    key = ftok(pathname, 1);
    if ((semid = semget(key, 1, IPC_CREAT | S_IRUSR | S_IWUSR)) < 0)
        perror("semget");
    if (semctl(semid, 0, SETVAL, szemafor_ertek) < 0)
        perror("semctl");

    return semid;
}

void szemafor_muvelet(int semid, int op)
{
    struct sembuf muvelet;

    muvelet.sem_num = 0;
    muvelet.sem_op = op;
    muvelet.sem_flg = 0;

    if (semop(semid, &muvelet, 1) < 0)
        perror("semop");
}

void szemafor_torles(int semid)
{
    semctl(semid, 0, IPC_RMID);
}

//--------------------------------------------------

void tovabbit()
{
    sleep(1);
	
	char a[STR_SIZE];

    read(pipefd1[0], &a,STR_SIZE);

    write(pipefd2[1], &a,STR_SIZE);
}

void kerdezz()
{
    write(pipefd1[1],"Melyik a default ütemező a Linux rendszerben?", STR_SIZE);
}

void fogad()
{

    sleep(2);
	
	char a[STR_SIZE];

    read(pipefd2[0], &a, STR_SIZE);

	printf("(hallgato) kapott kerdes:%s\n",a);
}

void valasz()
{
    strcpy(s,"CFS");
	szemafor_muvelet(semid, 1);
    shmdt(s);
}

void erkezett_valasz()
{
    sleep(1);
	szemafor_muvelet(semid, -1);
    printf("(oktato) kapott valasz: %s\n", s);
	szemafor_muvelet(semid, 1);
    shmdt(s);
}

//----------------------------------------------

int main(int argc, char const *argv[])
{
	signal(SIGUSR1, handler);
	signal(SIGUSR2, handler2);
	
    startPipe();

    pid_t child = fork();
	
	key_t key = ftok(argv[0], 1);
	int mq = msgget(key, 0600 | IPC_CREAT);
	
	int oszt_mem_id;
	oszt_mem_id=shmget(key,500,IPC_CREAT|S_IRUSR|S_IWUSR);
	s = shmat(oszt_mem_id,NULL,0);
	semid = szemafor_letrehozas(argv[0], 0);

    if (child < 0)
    {
        printf("Fork hivas sikertelen!\n");
        exit(EXIT_FAILURE);
    }
    if (child > 0)
    {

        pid_t child2 = fork();

        if (child2 < 0)
        {
            printf("Fork hivas sikertelen!\n");
            exit(EXIT_FAILURE);
        }
        if (child2 > 0)
        { //parent teams
			puts("(teams) fut");
			pause();
			pause();
			
			tovabbit();

            closePipe();
            wait(NULL);
            wait(NULL);
			
			puts("(teams) vege");
        }
        else
        { // child2 oktato 
			kill(getppid(),SIGUSR1); 
			
			msg_t msg;
			msgrcv(mq, &msg, STR_SIZE, 1, 0);
			printf("(oktato) kapott uzenet: %s\n",msg.mtext);
			
			kerdezz();

			sleep(3);
			
			erkezett_valasz();
			shmctl(oszt_mem_id, IPC_RMID, NULL);
			closePipe();
			puts("(oktato) vege");
            exit(0);
        }
    }
    else
    { // child1 hallgato
		sleep(1);
		kill(getppid(),SIGUSR2); 
		
		msg_t msg;
		msg.mtype = 1;
		strcpy(msg.mtext,"Jo napot");
		msgsnd(mq, &msg, STR_SIZE, 0);
		
		fogad();
		valasz();
		
        closePipe();
		sleep(4);
		puts("(hallgato) vege");
        exit(0);
    }

    return 0;
}