#include "header.h"

int status;
pid_t pid[PLAYERS];
char *team;

mex injMess;
key_t key;
int msgid;

void handle_signal(int signal) {
    switch (signal) {
        case SIGINT: //finisce la partita
            for (int i = 0; i < PLAYERS; i++) { //cancello i giocatori
                //printf("Team: Removing player id: %d\n", pid[i]);
                kill(pid[i], SIGINT);
                waitpid(pid[i], &status, 0);
            }
            printf("Team: Removing self id: %d\n", getpid());
            exit(EXIT_SUCCESS);
        case SIGUSR1: 
            if (strcmp(team, "A")) {
                kill(getppid(), SIGUSR2);
            } else if (strcmp(team, "B")) {
                kill(getppid(), SIGUSR1);
            }
            break;
    }
}

int main(int argc, char *argv[]) {
    int i;
    team = argv[1]; //get team name (A or B)
    printf("Team %s -pid: %d\n", team, getpid());

    struct sigaction sa, sa_old;
    sigset_t new_m, old_m;
    sa.sa_handler = &handle_signal;
    sa.sa_flags = SA_NODEFER;
    sigemptyset(&new_m);
    sigaddset(&new_m, SIGTERM);
    sigaddset(&new_m, SIGUSR1);
    sa.sa_mask = new_m;

    if (sigaction(SIGINT, &sa, NULL) == -1)
        perror("Team: Error Handler SIGINT");

    if (sigaction(SIGUSR1, &sa, NULL) == -1)
        perror("Team: Error Handler SIGUSR1");

    key = ftok("squadre.c", 'i'); //Coda di messaggi per notificare gli infortuni
    msgid = msgget(key, IPC_CREAT | MSG_PERM);
    if (msgid < 0)
        perror("Team: Error creating msg\n");

    printf("Team: Msg id: %d\n", msgid);

    for (i = 0; i < PLAYERS; i++) { //creo tutti i giocatori secondo la macro nel config
        switch (pid[i] = fork()) { 
            case -1:
                perror("Team: Error during fork");
                break;
            case 0:;
                char player_n[2];
                snprintf(player_n, 2, "%d", i);
                char *args[] = {"player", player_n, team, NULL};
                char *env_vars[] = {NULL};
                execve("./giocatori", args, env_vars);
                exit(EXIT_FAILURE);
                break;
        }
    }
    char player_n[2];
    while (1) {
        
        int err = msgrcv(msgid, &injMess, sizeof (injMess), 0, IPC_NOWAIT);      // ricevo i messaggi per l'infortunio   
/*
        if (err < 0) {
            perror("");
            printf("Team: Error receiving injmsg err:%d\n", err);
        }
*/
        if (err >= 0) { 
            if (injMess.action == TO_TEAM) { 
                kill(pid[injMess.n_player], SIGINT); //elimino il giocatore
                waitpid(pid[injMess.n_player], &status, 0); //aspetto che sia eliminato
                switch (pid[injMess.n_player] = fork()) { //creo un nuovo giocatore con stesso numero di quello eliminato
                    case -1:
                        perror("Team: Error during fork");
                        break;
                    case 0:
                        snprintf(player_n, 2, "%d", injMess.n_player);
                        char *args[] = {"player", player_n, team, NULL};
                        char *env_vars[] = {NULL};
                        execve("./giocatori", args, env_vars);
                        break;
                }
                injMess.action = 0;
            }
        }
        sigprocmask(SIG_BLOCK, &new_m, &old_m); //mi accerto che il semaforo rimanga sveglio
        sigprocmask(SIG_UNBLOCK, &new_m, &old_m);
    }
}
