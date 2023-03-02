#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

/*
./proj2 P H S R W C
• P je poˇcet osob generovaných v každé kategorii; bude vytvoˇreno P hackers a P serfs.
P >= 2 && (P % 2) == 0
• H je maximální hodnota doby (v milisekundách), po které je generován nový proces hackers.
H >= 0 && H <= 2000.
• S je maximální hodnota doby (v milisekundách), po které je generován nový proces serfs.
S >= 0 && S <= 2000.
• R je maximální hodnota doby (v milisekundách) plavby.
R >= 0 && R <= 2000.
• W je maximální hodnota doby (v milisekundách), po které se osoba vrací zpˇet na molo (pokud
bylo pˇred tím molo plné).
W >= 20 && W <= 2000.
• C je kapacita mola. C >= 5.
• Všechny parametry jsou celá ˇcísla.
*/
/*
 * • A je poˇradové ˇcíslo provádˇené akce,
 * • NAME je zkratka kategorie pˇríslušného procesu, tj. HACK nebo SERF,
 * • I je interní identifikátor procesu v rámci pˇríslušné kategorie,
 * • NH je aktuální poˇcet hackers na molu a
 * • NS je aktuální poˇcet serfs na molu.
 * • Pˇri vyhodnocování výstupu budou ignorovány mezery a tabelátory
 */

#define MMAP(pointer) {(pointer) = mmap(NULL, sizeof(*(pointer)), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);}
#define UNMAP(pointer) {munmap((pointer), sizeof((pointer)));}


int *A = 0;
int *I = 0;
int *molo = 0;
int *NH = 0;
int *NS = 0;
int *captain = 0;
sem_t *semafor_main = NULL;
sem_t *semafor_func = NULL;
sem_t *semafor_gen = NULL;
sem_t *semafor_serfs = NULL;
sem_t *semafor_hackers = NULL;
sem_t *semafor_boat = NULL;
sem_t *semafor_freeboat = NULL;
sem_t *semafor_passenger = NULL;
sem_t *semafor_molo_hack = NULL;
sem_t *semafor_molo_serf = NULL;
FILE *file = NULL;

int init()
{
	
	MMAP(captain);
	MMAP(A);
	MMAP(I);
	MMAP(molo);	
	MMAP(NH);
	MMAP(NS);
	*captain -= 1;
	*A += 1;
	if ((semafor_main = sem_open("/xstafl01.proj2.main.semafor", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED) return -1;
        if ((semafor_func = sem_open("/xstafl01.proj2.func.semafor", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED) return -1;	
        if ((semafor_gen = sem_open("/xstafl01.proj2.gen.semafor", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED) return -1;	
        if ((semafor_hackers = sem_open("/xstafl01.proj2.hackers.semafor", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED) return -1;
        if ((semafor_serfs = sem_open("/xstafl01.proj2.serfs.semafor", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED) return -1;
        if ((semafor_boat = sem_open("/xstafl01.proj2.boat.semafor", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED) return -1;
        if ((semafor_freeboat = sem_open("/xstafl01.proj2.freeboat.semafor", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED) return -1;
        if ((semafor_passenger = sem_open("/xstafl01.proj2.passenger.semafor", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED) return -1;
        if ((semafor_molo_hack = sem_open("/xstafl01.proj2.molohack.semafor", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED) return -1;
        if ((semafor_molo_serf = sem_open("/xstafl01.proj2.moloserf.semafor", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED) return -1;

	return 0;
}

void clear()
{
	UNMAP(A);
	UNMAP(I);
	UNMAP(molo);
	UNMAP(NH);
	UNMAP(NS);
	UNMAP(captain);
	
	sem_close(semafor_boat);
	sem_close(semafor_freeboat);
	sem_close(semafor_main);
	sem_close(semafor_func);
	sem_close(semafor_gen);
	sem_close(semafor_serfs);
	sem_close(semafor_hackers);
	sem_close(semafor_passenger);
	sem_close(semafor_molo_hack);
	sem_close(semafor_molo_serf);


	sem_unlink("/xstafl01.proj2.boat.semafor");
	sem_unlink("/xstafl01.proj2.freeboat.semafor");
	sem_unlink("/xstafl01.proj2.main.semafor");
        sem_unlink("/xstafl01.proj2.hackers.semafor");
        sem_unlink("/xstafl01.proj2.func.semafor");
        sem_unlink("/xstafl01.proj2.gen.semafor");
        sem_unlink("/xstafl01.proj2.serfs.semafor");
        sem_unlink("/xstafl01.proj2.passenger.semafor");
        sem_unlink("/xstafl01.proj2.molohack.semafor");
        sem_unlink("/xstafl01.proj2.moloserf.semafor");

	if (file != NULL) fclose(file);
}

void serve_passenger(int id, int C, int type, int R, int W)
{
	id=id;
	//WAITING FOR PREVIOUS PASSENGER TO ENTER BEFORE ENTERING
	sem_wait(semafor_func);
	if (*NH+*NS < C)
	{
		//HACKER OR SERF
		if (type == 0)
		{
			*NH += 1;
		}
		else if (type == 1)
		{
			*NS += 1;
		}
		//LETTING ANOTHER PASSENGER ENTER MOLO
		sem_post(semafor_func);	
		
		//CAPTAINS
		//SHIP FULL OF HACKERS
		if(*NH >= 4 && type==0 && *captain <= 0)
		{
			*captain += 1;
			fprintf(file,"%d	:HACK %d		: waits:		%d: %d\n", *A, id, *NH, *NS);
			*A += 1;
			
			sem_wait(semafor_freeboat);

			*NH -= 4;
			fprintf(file,"%d	:HACK %d		: boards:		%d: %d\n", *A, id, *NH, *NS);
			*A += 1;
			//boat goes off
			if (R != 0){
				usleep((rand() % R)*1000);
			}
			sem_post(semafor_molo_hack);
			sem_wait(semafor_passenger);
			sem_post(semafor_molo_hack);
			sem_wait(semafor_passenger);
			sem_post(semafor_molo_hack);
			sem_wait(semafor_passenger);
			//wait for all members to exit first
			sem_wait(semafor_boat);
			sem_wait(semafor_boat);
			sem_wait(semafor_boat);
			fprintf(file,"%d	:HACK %d		: captain exits:	%d: %d\n", *A, id, *NH, *NS);
			*A += 1;
			*captain -= 1;
			sem_post(semafor_freeboat);
		}
		//SHIP FULL OF SERFS
		else if(*NS >= 4 && type==1 && *captain <= 0)
		{	
			*captain += 1;
			fprintf(file,"%d	:SERF %d		: waits:		%d: %d\n", *A, id, *NH, *NS);
			*A += 1;
			
			sem_wait(semafor_freeboat);
			
			*NS -= 4;
			fprintf(file,"%d	:SERF %d		: boards:		%d: %d\n", *A, id, *NH, *NS);
			*A += 1;
			//boat's going
			if (R != 0){
				usleep((rand() % R)*1000);
			}
			sem_post(semafor_molo_serf);
			sem_wait(semafor_passenger);
			sem_post(semafor_molo_serf);
			sem_wait(semafor_passenger);
			sem_post(semafor_molo_serf);
			sem_wait(semafor_passenger);
			//wait for all members to exit first
			sem_wait(semafor_boat);
			sem_wait(semafor_boat);
			sem_wait(semafor_boat);
			fprintf(file,"%d	:SERF %d		: captain exits:	%d: %d\n", *A, id, *NH, *NS);
			*A += 1;
			*captain -= 1;
			sem_post(semafor_freeboat);
		}
		//COMBINED SHIP
		else if(*NH >= 2 && *NS >= 2 && *captain <= 0)
		{
			*captain += 1;
			//IF CAPTAIN PROCESS == HACKER
			if (type == 0)
			{
				fprintf(file,"%d	:HACK %d		: waits:		%d: %d\n", *A, id, *NH, *NS);
				*A += 1;
				sem_wait(semafor_freeboat);
				
				*NS -= 2;
				*NH -= 2;
				fprintf(file,"%d	:HACK %d		: boards:		%d: %d\n", *A, id, *NH, *NS);
				*A += 1;
				//boat goes on a trip
				if (R != 0){
					usleep((rand() % R)*1000);
				}
				sem_post(semafor_molo_hack);
				sem_wait(semafor_passenger);
				sem_post(semafor_molo_serf);
				sem_wait(semafor_passenger);
				sem_post(semafor_molo_serf);
				sem_wait(semafor_passenger);
				//wait for all members to exit first
				sem_wait(semafor_boat);
				sem_wait(semafor_boat);
				sem_wait(semafor_boat);
				fprintf(file,"%d	:HACK %d		: captain exits:	%d: %d\n", *A, id, *NH, *NS);
				*A += 1;
				*captain -= 1;
				sem_post(semafor_freeboat);
			}
			//IF CAPTAIN PROCESS == SERF
			else if (type == 1)
			{
				fprintf(file,"%d	:SERF %d		: waits:		%d: %d\n", *A, id, *NH, *NS);
				*A += 1;
				sem_wait(semafor_freeboat);
		
				*NS -= 2;
				*NH -= 2;
				fprintf(file,"%d	:SERF %d		: boards:		%d: %d\n", *A, id, *NH, *NS);
				*A += 1;
				//boat is sailing
				if (R != 0){
					usleep((rand() % R)*1000);
				}
				sem_post(semafor_molo_hack);
				sem_wait(semafor_passenger);
				sem_post(semafor_molo_serf);
				sem_wait(semafor_passenger);
				sem_post(semafor_molo_hack);	
				sem_wait(semafor_passenger);
				//wait for all members to exit first
				sem_wait(semafor_boat);
				sem_wait(semafor_boat);
				sem_wait(semafor_boat);
				fprintf(file,"%d	:SERF %d		: captain exits:	%d: %d\n", *A, id, *NH, *NS);
				*A += 1;
				*captain -= 1;
				sem_post(semafor_freeboat);
			}
		}
		//Passengers waiting on molo
		else
		{
			if (type == 0)
			{
				fprintf(file,"%d	:HACK %d		: waits:		%d: %d\n", *A, id, *NH, *NS);
				*A += 1;
				sem_post(semafor_func);
				//waiting on molo
				sem_wait(semafor_molo_hack);
				fprintf(file,"%d	:HACK %d		: member exits:		%d: %d\n", *A, id, *NH, *NS);
				*A += 1;
				sem_post(semafor_passenger);
				sem_post(semafor_boat);
				
			}
			else if (type == 1)
			{
				fprintf(file,"%d	:SERF %d		: waits:		%d: %d\n", *A, id, *NH, *NS);
				*A += 1;
				sem_post(semafor_func);
				//waiting on molo
				sem_wait(semafor_molo_serf);
				fprintf(file,"%d	:SERF %d		: member exits:		%d: %d\n", *A, id, *NH, *NS);
				*A += 1;
				sem_post(semafor_passenger);
				sem_post(semafor_boat);
			}
		}
	}
	//Molo is full, passenger leaves queue and comes back later
	else
	{
		if (type == 0)
		{	
			fprintf(file,"%d	:HACK %d		: leaves queue:		%d: %d\n", *A, id, *NH, *NS);
			*A += 1;
			sem_post(semafor_func);
			if (W != 0)
			{	
				usleep((rand() % (W)+20)*1000);
			}
			else	
				usleep(20*1000);
			fprintf(file,"%d	:HACK %d		: is back\n", *A, id);
			*A += 1;
			serve_passenger(id, C, type, R, W);
			
		}
		else if(type == 1)
		{
			fprintf(file,"%d	:SERF %d		: leaves queue:		%d: %d\n", *A, id, *NH, *NS);
			*A += 1;
			sem_post(semafor_func);
			if (W != 0)
			{	
				usleep((rand() % (W)+20)*1000);
			}
			else	
				usleep(20*1000);
			fprintf(file,"%d	:SERF %d		: is back\n", *A, id);
			*A += 1;
			serve_passenger(id, C, type, R, W);
		}
	}
	return;
}
//Generates 'count' amount of passengers(processes)
void generate_passengers(int count, int delay, int type, int C, int R, int W)
{
	delay=delay;
	char *name;
	srand(time(NULL));
	if (type == 0)
	{
		name = "HACK";
	}
	else
	{
		name = "SERF";
	}
	int wait_time = 0;
	if (delay != 0)
	{
		wait_time = rand() % delay;
	}
	for (int j = 0; j<count; j++)
	{
		usleep(wait_time*1000);
		sem_wait(semafor_gen);
		pid_t new_passenger = fork();
		if (new_passenger == 0)
		{
			fprintf(file,"%d	:%s %d		: starts\n", *A, name, j+1);
			*A += 1;
			sem_post(semafor_gen);
			serve_passenger(j+1, C, type, R, W);
			sem_post(semafor_main);
			exit(0);
		}
		else if (new_passenger < 0)
		{
			fprintf(stderr, "Could not fork a process.\n");
			exit(1);
		}
		new_passenger = 1;
	}
}
//Checks if argument is a number and converts it to int
int isint(char *arg)
{
	char *ptr = NULL;
	int number = strtoul(arg, &ptr, 10);	
        if (*ptr != '\0')
        {
                return -1;
        }
	return number;
}

int main(int argc, char *argv[])
{
	//./proj2 P H S R W C
	if ((file = fopen("proj2.out", "w+"))==NULL)
	{
		fprintf(stderr,"FILE ERROR\n");
		return -1;
	}
	setbuf(file,NULL);
	if (argc == 1)
	{
		clear();
		return 0;
	}
	if (argc != 7)
	{
		fprintf(stderr,"USAGE STRING\n");
		return -1;
	}
	int P, H, S, C, R, W;
	if ((P = isint(argv[1]))== -1 || (H = isint(argv[2]))==-1 || (S = isint(argv[3]))==-1 || (C = isint(argv[6]))==-1 || (R = isint(argv[4]))==-1 || (W = isint(argv[5]))==-1)
	{
		printf("USAGE STRING\n");
		return -1;
	}
	//printf("P: %d\nH: %d\nS: %d\nR: %d\nW: %d\nC: %d\n",P,H,S,R,W,C);
	//Passenger Generator Processes	
	init();
	//Hackers
	pid_t hackgen = fork();
	if (hackgen == 0)
	{
		generate_passengers(P, H, 0, C, R, W);
		exit(0);
	}
	else if (hackgen < 0)
	{
		fprintf(stderr, "Could not fork process.\n");
		clear();
		return -1;
	}
	//Serfs
	pid_t serfgen = fork();
	if (serfgen == 0)
	{
		generate_passengers(P, S, 1, C, R, W);
		exit(0);
	}
	else if (serfgen < 0)
	{
		fprintf(stderr, "Could not fork process.\n");
		clear();
		return -1;
	}
	//Waiting for all processes to end before ending the main process
	for (int i = 0; i<P*2; i++)
	{
		sem_wait(semafor_main);	
	}
	clear();
	return 0;
}
