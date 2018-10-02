#include "header.h"

pid_t pid[TEAMS]; 
int status;
key_t key;
int semid; //id del semaforo
struct sembuf sem; //buffer del semaforo
int gol_a = 0;
int gol_b = 0;

void handle_signal(int signal) {
    switch (signal) {
        case SIGINT:
            printf("Referee: removing self, childs & sem int 1=%d int 2=%d int 3=%d\n", pid[0], pid[1], pid[2]);
            for (int i = 0; i < TEAMS; i++) { //rimuovo le squadre
                kill(pid[i], SIGINT);
                waitpid(pid[i], &status, 0);
            }
            semctl(semid, 0, IPC_RMID); //rimuovo il semaforo
            printf("Referee: removing semaphore\n");
            exit(EXIT_SUCCESS);
        case SIGUSR1: //gol della squadra A
            gol_a++;
            printf("GOL A: %d vs. GOL B: %d\n", gol_a, gol_b);
            break;
        case SIGUSR2: //Gol della squadra B
            gol_b++;
            printf("GOL A: %d vs. GOL B: %d\n", gol_a, gol_b);
            break;
    }
}

int main() {
    system("gcc fato.c -o fato");
    system("gcc squadre.c -o squadre");
    system("gcc giocatori.c -o giocatori");

    printf("Referee pid= %d\n", getpid());

    struct sigaction sa, sa_old;
    sigset_t new_m, old_m;
    sa.sa_handler = &handle_signal;

    sigfillset(&sa.sa_mask);
    sa.sa_flags = SA_NODEFER;
	//creazione dei semafori
    if (sigaction(SIGINT, &sa, NULL) == -1)
        perror("Error Handler SIGINT");

    if (sigaction(SIGUSR1, &sa, NULL) == -1)
        perror("Error Handler SIGUSR1");

    if (sigaction(SIGUSR2, &sa, NULL) == -1)
        perror("Error Handler SIGUSR2");


    key = ftok("arbitro.c", 'a'); 
    semid = semget(key, 1, IPC_CREAT | MSG_PERM); //coda verso l'albitro
    semctl(semid, 0, SETVAL, 1); //valore del semaforo di partita
    sem.sem_num = 0;
    sem.sem_flg = 0;
    printf("Referee: Sem id= %d\n", semid);

    char *env_vars[] = {NULL};

    for (int i = 0; i < TEAMS; i++) { //creo i team
        if (pid[i] = fork() == 0) {//sono il figlio 
            if (i == 0) {
                char *args[] = {"fato", NULL};
                execve("./fato", args, env_vars); // creo prima fato
            } else if (i == 1) {
                char *args[] = {"squadra", "A", NULL};
                execve("./squadre", args, env_vars); //poi creo la squadra A  
                // exit(0);
            } else if (i == 2) { //creo la squadra B                
                char *args[] = {"squadra", "B", NULL}; //e infine la squadra B
                execve("./squadre", args, env_vars);
            }
        } else if (pid[i] == -1) {
            perror("Error during fork");
            exit(EXIT_FAILURE);
        }
    }

    while (1) {
        sigprocmask(SIG_BLOCK, &new_m, &old_m); //tengo vivo il semaforo
        sigprocmask(SIG_UNBLOCK, &new_m, &old_m);
    }


} //end main
