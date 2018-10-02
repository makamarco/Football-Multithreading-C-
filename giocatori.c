#include "header.h"

key_t key;
mex mess;
mex injMess;
int msgid;
int injMsgid;
char *team;
int injured;
int semid;
struct sembuf sem;
int player_number;

void genera_azione();

void send_injury() {
    mess.action = INJURY;
    mess.n_player = player_number;
    mess.team = team[0];

    if (msgsnd(msgid, &mess, sizeof (mess), 0) < 0)
        perror("Player: Error sending msg\n");

    if (msgrcv(msgid, &mess, sizeof (mex), TO_FATO, 0) < 0)
        perror("Player: Error receiving msg\n");

    if (mess.result) { //Se il giocatore è infornutato lo dico alla squadra
        printf("Player: Sending Injury msg to Team\n");
        injured = 1;
        injMess.action = TO_TEAM;
        injMess.n_player = player_number;
        injMess.team = team[0];
        if (msgsnd(injMsgid, &injMess, sizeof (injMess), 0) < 0) //notifico la squadra dell'infortunio
            perror("Player: Error sending msg\n");
    }
}

void send_goal() {
    mess.action = GOAL;
    mess.n_player = player_number;
    mess.team = team[0];
    if (msgsnd(msgid, &mess, sizeof (mex), 0) < 0)
        perror("Player: Error sending msg\n");

    if (msgrcv(msgid, &mess, sizeof (mex), TO_FATO, 0) < 0)
        perror("Player: Error receiving msg\n");

    if (mess.result) {
        kill(getppid(), SIGUSR1);
    }
}

void send_dribbling() {
    mess.action = DRIBBLING;
    mess.n_player = player_number;
    mess.team = team[0];

    if (msgsnd(msgid, &mess, sizeof (mex), 0) < 0)
        perror("Player: Error sending msg\n");

    if (msgrcv(msgid, &mess, sizeof (mex), TO_FATO, 0) < 0)
        perror("Player: Error receiving msg\n");

    if (mess.result) {
        genera_azione();
    }
}

void genera_azione() {
    srand((unsigned int) time(0));
    switch (rand() % 3) {
        case 0:
            send_injury();
            break;
        case 1:
            send_goal();
            break;
        case 2:
            send_dribbling();
            break;
    }
}

int main(int argc, char *argv[]) {
    player_number = atoi(argv[1]);
    team = argv[2];

    sleep(1); //aspetto la creazione di tutti i giocatori
    injured = 0;

    printf("Player: Player %d of Team %s with pid: %d\n", player_number, team, getpid()); //Stampo la creazione di ogni giocatore

    key = ftok("fato.c", 'f'); //Creo la chiave per la coda di messaggi con il fato
    msgid = msgget(key, IPC_CREAT | MSG_PERM); //creo la coda
    if (msgid < 0)
        perror("Player: Error creating msg\n");

    key = ftok("squadre.c", 'i'); //Creo la chiave per la coda verso le squadre, serve per gli infortuni
    injMsgid = msgget(key, IPC_CREAT | MSG_PERM); //creo la coda
    if (injMsgid < 0)
        perror("Player: Error creating msg\n");

    key = ftok("arbitro.c", 'a'); //chiave per il semaforo dell'albitro
    semid = semget(key, 1, IPC_CREAT | MSG_PERM);
    sem.sem_num = 0;
    sem.sem_flg = 0;

    while (1) { //il giocatore inizia a giocare
        if (injured <= 0) {
            sem.sem_op = -1;
            semop(semid, &sem, 1);

            genera_azione(); // Se riesco a giocare genero l'azione in modo random

            sem.sem_op = 1;
            semop(semid, &sem, 1);
        } else
            sleep(2);
    }
}
