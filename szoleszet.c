#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h> 
#include <time.h> 
#include <string.h> 

#define STR_SIZE 256

struct jelentkezo{
	char nev[31]; // 6 + a lezaro nulla
    char cim[101];
    char napok[7][10];
};

struct jelentkezo adat[100];
int db = 0;

int hmax=3;
int kmax=2;
int szemax=1;
int cmax=3;
int pmax=4;
int szomax=5;
int vmax=3;

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

char teruletek[6][15]={"Jeno telek","Lovas Dulo","Hosszu","Selyem telek","Malom telek","Szula"};
char munkak[4][25]={"metszes","rugyfakaszto permetezes","tavaszi nyitas","horolas"};

char maiterulet[15];
char maimunka[25];  
char regisztraltak[STR_SIZE];

int letszam;
int regszam;
int value;
//////////////////////////////////////////////////////////////////
void handler(int sig, siginfo_t *si, void *ucontext)
{
	value=si->si_int;
}

void mailetszam(){
	letszam=0;
	srand(time(NULL));
	int szuksletszam = (rand()%6)+5;
	int azonnaliak;
	szuksletszam-=regszam;
	if(szuksletszam<=0){
		letszam=regszam;
	}else{
		azonnaliak = (rand()%szuksletszam+1);
		letszam=azonnaliak+regszam;
	}
}
void mainap(){
	srand(time(NULL));
	int r = rand()%6;
	strcpy(maiterulet,teruletek[r]);
	r = rand()%4;
	strcpy(maimunka,munkak[r]);
}

void resztvevok(char* akt,char* nap){
	trim(akt);
    if(!strcmp(akt,nap)){
		regszam=0;
		strcpy(regisztraltak,"");
        for(int i=0; i<db; i++){
            for(int j=0;j<7;j++){
                if(!strcmp(adat[i].napok[j],nap)){
					strcpy(regisztraltak,strcat(regisztraltak,adat[i].nev));
					strcpy(regisztraltak,strcat(regisztraltak,","));
					++regszam;
                }
            }
        }
		regisztraltak[strlen(regisztraltak)-1]=0;
    }
}

void egynap(char* nevsor){
	struct sigaction sigact;
	sigact.sa_sigaction=handler;
	sigemptyset(&sigact.sa_mask); 
	sigact.sa_flags=SA_SIGINFO;
	sigaction(SIGTERM,&sigact,NULL); 
	
	int pipefd[2];
	int pipefd2[2];
	pipe(pipefd);
	pipe(pipefd2);
  


	pid_t child=fork();
	pid_t child2;
	if (child>0){
		child2=fork();
		if(child2>0){
			printf("Parent start\n"); //iroda
			close(pipefd2[0]);
			close(pipefd[1]);
			char buf[STR_SIZE];
			wait(NULL);
			read(pipefd[0], &buf, STR_SIZE);
			printf("(Iroda) mai munka: %s\n", buf);
			
			strcpy(buf,strcat(buf," es "));
			if(!strcmp(nevsor,"")){
				strcpy(buf,strcat(buf,"nincs regisztralt"));
			}else{
				strcpy(buf,strcat(buf,nevsor));
			}
			write(pipefd2[1],&buf, STR_SIZE);
			close(pipefd2[1]);
			close(pipefd[0]);
			
			
			wait(NULL);
			printf("Parent process end:%d\n",value);
		}else{
			printf("Child2 process start\n"); //tiszt
			mainap();
			close(pipefd2[0]);
			close(pipefd2[1]);
			close(pipefd[0]);
			char mes[STR_SIZE];
			strcpy(mes,strcat(maiterulet,","));
			strcpy(mes,strcat(mes,maimunka));
			printf("(Gazdatiszt) melok kuldese az irodanak...\n");
			write(pipefd[1], &mes, STR_SIZE);
			fflush(NULL);
			close(pipefd[1]);
			printf("Child2 process end\n");
		}
	}else{
		printf("Child process start\n"); //vezeto
		close(pipefd2[1]);
		close(pipefd[0]);
		close(pipefd[1]);
		
		char buf[STR_SIZE];
		read(pipefd2[0], &buf, STR_SIZE);
		printf("(Vezeto) mai munka,nevsor: %s\n", buf);
		mailetszam(11);
		close(pipefd2[0]);
		

		close(pipefd2[0]);
		union sigval s_value_int={letszam};
		sigqueue(getppid(),SIGTERM,s_value_int); 
		
		printf("Child process ended\n");  
	}
}


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void trim(char *str){
	char *end;
	while(isspace((char)*str)){
		str++;
	}
	end = str + strlen(str) - 1;
	while(end > str && isspace((char)*end)){
		end--;
	}
	end[1] = '\0';
}

void napkeszito(char* akt,char* nap,int *aktdb,int *napmax){
	trim(akt);
    if(!strcmp(akt,nap)){
        if((*napmax)>0){
            strcpy(adat[db].napok[(*aktdb)],akt);
            (*napmax)--;
            (*aktdb)++;
        }else{
			printf("A kovetkezo nap betelt: %s\n",nap);
		}
    }
} 

void napmodosito(char* akt,char* nap,int *aktdb,int *napmax,int i){
	trim(akt);
    if(!strcmp(akt,nap)){
        if((*napmax)>0){
			(*napmax)--;
            strcpy(adat[i].napok[(*aktdb)],akt);
            (*aktdb)++;
        }else{
            printf("bizonyos napok beteltek mar\n");
        }
    }
}

void naplistazo(char* akt,char* nap){
	trim(akt);
    if(!strcmp(akt,nap)){
        for(int i=0; i<db; i++){
            for(int j=0;j<7;j++){
                if(!strcmp(adat[i].napok[j],nap)){
                    printf("%s\n",adat[i].nev);
                }
            }
        }
    }
}

void napvisszall(int i){
	for(int j=0;j<7;++j){
		if(!strcmp(adat[i].napok[j],"Hetfo")){
			++hmax;
			strcpy(adat[i].napok[j],"\0");
		}
		else if(!strcmp(adat[i].napok[j],"Kedd")){
			++kmax;
			strcpy(adat[i].napok[j],"\0");
		}
		else if(!strcmp(adat[i].napok[j],"Szerda")){
			++szemax;
			strcpy(adat[i].napok[j],"\0");
		}
		else if(!strcmp(adat[i].napok[j],"Csutortok")){
			++cmax;
			strcpy(adat[i].napok[j],"\0");
		}
		else if(!strcmp(adat[i].napok[j],"Pentek")){
			++pmax;
			strcpy(adat[i].napok[j],"\0");
		}
		else if(!strcmp(adat[i].napok[j],"Szombat")){
			++szomax;
			strcpy(adat[i].napok[j],"\0");
		}
		else if(!strcmp(adat[i].napok[j],"Vasarnap")){
			++vmax;
			strcpy(adat[i].napok[j],"\0");
		}
		
	}
}

void napmaxbealito(char* aktnap){
	if(!strcmp(aktnap,"Hetfo")){
			--hmax;
	}
	else if(!strcmp(aktnap,"Kedd")){
		--kmax;
	}
	else if(!strcmp(aktnap,"Szerda")){
		--szemax;
	}
	else if(!strcmp(aktnap,"Csutortok")){
		--cmax;
	}
	else if(!strcmp(aktnap,"Pentek")){
		--pmax;
	}
	else if(!strcmp(aktnap,"Szombat")){
		--szomax;
	}
	else if(!strcmp(aktnap,"Vasarnap")){
		--vmax;
	}
}


//////////////////////////////////////////////////////////////////




int main (int argc,char* argv[]) {
	/*----------------------------------*/

        FILE *f;
        char fajl[201];
        f=fopen("fajl.txt","a+");
		char delim[] = " ";
		int ind=0;
		while(!feof(f)){
			fgets(fajl,201,f);
			trim(fajl);
			if(strcmp(fajl,"-")){
				strcpy(adat[db].nev,fajl);
				fgets(fajl,201,f);
				trim(fajl);
				strcpy(adat[db].cim,fajl);
				fgets(fajl,201,f);
				trim(fajl);
				char *ptr = strtok(fajl, delim);
				while(ptr != NULL){
					napmaxbealito(ptr);
					strcpy(adat[db].napok[ind],ptr);
					ptr = strtok(NULL, delim);
					++ind;
				}
			}else{
				ind=0;
				db++;
			}
		}
    
	
    /*-----------------------------*/
    int kilep=0;
    while(kilep!=1){
        printf("1.)Jelentkezes\n2.)Adat modositas\n3.)Adat torles\n4.)Napi lista\n5.)Teljes lista\n6.)Masodik beadando\n7.)Kilepes\n");
        printf("A kivalasztani kivant menupont szamat adja meg:\n");
        int opcio;
        scanf("%d",&opcio);
        char aktnapok[101];
        char aktnap[10];
        char aktnev[31];
        char aktadat[101];
        int aktszam;
        char* akt;


        switch(opcio){
            /*-----------------------------*/
            case 1:

            if (db>=100){
                printf("tobbet nem lehet beirni\n");
                break; 
			}
            printf("adja meg a nevet: ");
            scanf("%s",&adat[db].nev);
            printf("adja meg a cimet: ");
            scanf("%s",&adat[db].cim);

            while ((getchar()) != '\n');
            printf("adja meg a napokat amikor tudna dolgozni: ");
            fgets(aktnapok,sizeof(aktnapok),stdin);


            akt=strtok(aktnapok," ");
            int aktdb=0;
            while(akt!=NULL){
                napkeszito(akt,"Hetfo",&aktdb,&hmax);
                napkeszito(akt,"Kedd",&aktdb,&kmax);
                napkeszito(akt,"Szerda",&aktdb,&szemax);
                napkeszito(akt,"Csutortok",&aktdb,&cmax);
                napkeszito(akt,"Pentek",&aktdb,&pmax);
                napkeszito(akt,"Szombat",&aktdb,&szomax);
                napkeszito(akt,"Vasarnap",&aktdb,&vmax);
                akt=strtok(NULL," ");
            }

            
            fprintf(f,"\n-------------------------------\n");
            

            db++;
            if(strlen(aktnapok)==1 ){
                printf("kerem toltson ki minden adatot\n");
                db--;
            }

            
            
            printf("\nkesz\n\n");
            break;
            /*-----------------------------*/
            case 2:
                printf("mi a neve annak akinek az adatain modositana?\n");
				scanf("%s",&aktnev);
                printf("melyik adatot modositana?\n1.nev 2.cim 3.napok\n");
				scanf("%d",&aktszam);
                for(int i=0;i<db;++i){
                    if(!strcmp(adat[i].nev,aktnev)){
                        printf("mi legyen az uj adat?\n");
                        switch(aktszam){
                            case 1:
				            scanf("%s",&aktadat);
                            strcpy(adat[i].nev,aktadat);
                            break;
                            case 2:
                            scanf("%s",&aktadat);
                            strcpy(adat[i].cim,aktadat);
                            break;
                            case 3:
							napvisszall(i);
                            while ((getchar()) != '\n');
                            fgets(aktadat,sizeof(aktadat),stdin);
                            akt=strtok(aktadat," ");
                            int aktdb=0;
                            while(akt!=NULL){
                                napmodosito(akt,"Hetfo",&aktdb,&hmax,i);
                                napmodosito(akt,"Kedd",&aktdb,&kmax,i);
                                napmodosito(akt,"Szerda",&aktdb,&szemax,i);
                                napmodosito(akt,"Csutortok",&aktdb,&cmax,i);
                                napmodosito(akt,"Pentek",&aktdb,&pmax,i);
                                napmodosito(akt,"Szombat",&aktdb,&szomax,i);
                                napmodosito(akt,"Vasarnap",&aktdb,&vmax,i);
                                akt=strtok(NULL," ");
                            }
                            break;
                        }
                    }
                }
            printf("\nkesz\n\n");
            break;
            /*-----------------------------*/
            case 3:
            printf("Neve annak akit torolni kivan:\n");
				scanf("%s",&aktnev);
                for(int i=0;i<db;++i){
                    if(!strcmp(adat[i].nev,aktnev)){
                        adat[i]=adat[i+1];
                        db--;
                        break;
                    }
                    
                }	
            printf("\nkesz\n\n");
            break;
            /*-----------------------------*/
            case 4:
            printf("A kivalasztani kivant nap nevét adja meg:\n");
            scanf("%s",&aktnap);
            naplistazo(aktnap,"Hetfo");
            naplistazo(aktnap,"Kedd");
            naplistazo(aktnap,"Szerda");
            naplistazo(aktnap,"Csutortok");
            naplistazo(aktnap,"Pentek");
            naplistazo(aktnap,"Szombat");
            naplistazo(aktnap,"Vasarnap");
            printf("\nkesz\n\n");
            break;
            /*-----------------------------*/
            case 5:
            for(int i=0; i<db; i++){
                printf("%d:\n%s\n%s\n",i+1,adat[i].nev,adat[i].cim);
				int ind=0;
				while(ind!=6){
					printf("%s ",adat[i].napok[ind]);
					++ind;
				}
				printf("\n");
            }
			printf("H:%d , K:%d , Sze:%d , Cs:%d , P:%d , Szo:%d , V:%d\n",hmax,kmax,szemax,cmax,pmax,szomax,vmax);
            printf("\nkesz\n\n");
            break;
			/*-----------------------------*/
			case 6:
			printf("Kivant nap: ");
			scanf("%s",&aktnap);
			
			resztvevok(aktnap,"Hetfo");
			resztvevok(aktnap,"Kedd");
			resztvevok(aktnap,"Szerda");
			resztvevok(aktnap,"Csutortok");
			resztvevok(aktnap,"Pentek");
			resztvevok(aktnap,"Szombat");
			resztvevok(aktnap,"Vasarnap");
			
			mailetszam();
			egynap(regisztraltak);
			/*-----------------------------*/
            case 7:
            kilep=1;
			fclose(f);
			f=fopen("fajl.txt","w");
			for(int i=0; i<db; i++){
				fprintf(f,adat[i].nev);
				fprintf(f,"\n");
				fprintf(f,adat[i].cim);
				fprintf(f,"\n");
				int ind=0;
				while(ind!=6){
					fprintf(f,adat[i].napok[ind]);
					fprintf(f," ");
					++ind;
				}
				if(i!=(db-1)){
					fprintf(f,"\n-\n");
				}else{
					fprintf(f,"\n-");
				}
			}
			
            fclose(f);
            break;
        }
    }
    return 0;
}