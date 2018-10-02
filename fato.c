#include "header.h"

int c_time=0;
int c_injury=0;
int c_dribbling=0;
int c_goal=0;

mex mess;
FILE *flog;
key_t key;
int msgid;

struct sigaction sa, sa_old;
sigset_t new_m, old_m;

void handle_signal(int signal) {
    sigprocmask(SIG_BLOCK, &new_m, &old_m);
    switch (signal) {
        case SIGALRM: 
            printf("Fato: Time's Up\n");
            kill(getppid(), SIGINT);
            break;
        case SIGINT:
            printf("Fato: Removing self & msg\n");
            msgctl(msgid, IPC_RMID, NULL);
            exit(EXIT_SUCCESS);        
    }
    sigprocmask(SIG_UNBLOCK, &new_m, &old_m); //sblocco il semaforo
}

void close_file() {
    fflush(flog);
    if (fclose(flog) == -1) {
        perror("Fato: Error closing file");
        exit(EXIT_FAILURE);
    }
}

void open_file() {
    flog = fopen("eventi.txt", "a");
    if (flog == NULL) {
        perror("Fato: Error opening file eventi");
        exit(EXIT_FAILURE);
    }
}

void get_config() { //Prendo le macro dal file di config
    FILE *fd;
    char desc[20];
    int valore;
    fd = fopen("config.txt", "r");
    if (fd) {
        while (!feof(fd)) {
            fscanf(fd, "%s\t", desc);
            fscanf(fd, "%d\t", &valore);
            if (strcmp(desc, "GOAL") == 0)
                c_goal = valore;
            if (strcmp(desc, "TIME") == 0)
                c_time = valore;
            if (strcmp(desc, "INJURY") == 0)
                c_injury = valore;
            if (strcmp(desc, "DRIBBLING") == 0)
                c_dribbling = valore;
        }
        fclose(fd);        
    } else printf("Fato: Error opening file");
}

void try_injury() {    
    mess.action = TO_FATO;
    srand((unsigned int) time(0));
    int u = rand() % 101;
    if (u >= c_injury) {
        mess.result = 0;
    } else {
        mess.result = 1;
        printf("Fato: Player %d of Team %c just scored \n", mess.n_player, mess.team);
        open_file();
        fprintf(flog, "Player %d of Team %c has been injured \n", mess.n_player, mess.team);        
        close_file();
    }    
    if (msgsnd(msgid, &mess, sizeof (mess), 0) < 0)//invio il risultato dell'azione
        perror("Fato: Error sending msg\n");
}

void try_goal() {
    mess.action = TO_FATO;
    srand((unsigned int) time(0));
    int u = rand() % 101;
    if (u >= c_goal) {
        mess.result = 0;
    } else {
        mess.result = 1;    
        printf("Fato: Player %d of Team %c just scored \n", mess.n_player, mess.team);
        open_file();
        fprintf(flog, "Player %d of Team %c just scored \n", mess.n_player, mess.team);        
        close_file();
    } 
    if (msgsnd(msgid, &mess, sizeof (mess), 0) < 0) //invio il risultato dell'azione
        perror("Fato: Error sending msg\n");
}

void try_dribbling() {
    mess.action = TO_FATO;
    srand((unsigned int) time(0));
    int u = rand() % 101;
    if (u >= c_dribbling) {
        mess.result = 0;
    } else {
        mess.result = 1;
        printf("Fato: Player %d of Team %c successfully dribbled \n", mess.n_player, mess.team); 
        open_file();
        fprintf(flog, "Player %d of Team %c successfully dribbled \n", mess.n_player, mess.team);
        close_file();
    }
    if (msgsnd(msgid, &mess, sizeof (mess), 0) < 0) //invio il risultato dell'azione
        perror("Fato: Error sending msg\n");
}


int main(int argc, char *argv[]) {
    printf("Fato: created with pid: %d\n", getpid());
    get_config(); //prendo le macro
    printf("RULES: \n Tempo=%d\n Goal=%d\n Injury=%d\n Dribbling=%d\n", c_time, c_goal,
        c_injury, c_dribbling);
    
    sa.sa_handler = &handle_signal;
    sigfillset(&sa.sa_mask);
    sa.sa_flags = SA_NODEFER;

    if (sigaction(SIGINT, &sa, NULL) == -1) //apro il semaforo di interrupt
        perror("Error Handler SIGINT");    

    if (sigaction(SIGALRM, &sa, NULL) == -1) //apro il semaforo di fine partira
        perror("Error Handler SIGALRM");  

    key = ftok("fato.c", 'f'); //chiave della coda di messaggi
    msgid = msgget(key, IPC_CREAT | MSG_PERM); //apro la coda
    if (msgid < 0)
        perror("Fato: Error creating msg\n");
    
    printf("Fato: Msg id: %d\n", msgid); 

    alarm(c_time); //timer per la chiusura della partita

    while (1) {
        if (msgrcv(msgid, &mess, sizeof (mess), TO_FATO, MSG_EXCEPT) < 0)  //ricevo l'azione          
            perror("Fato: Error receving msg\n");
        
        switch (mess.action) { //esegui l'azione
            case INJURY:
                try_injury();
                break;
            case GOAL:
                try_goal();
                break;
            case DRIBBLING:
                try_dribbling();
                break;
        }
        sleep(1);
    }
}