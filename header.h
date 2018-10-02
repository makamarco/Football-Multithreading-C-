#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>

#define MSG_EXCEPT 020000 //permette di selezionare il primo mesaggio diverso da quello ricevuto
#define MSG_PERM 0600 //messaggio di permesso della coda

#define PLAYERS 5
#define TEAMS 3
#define TO_FATO 1
#define INJURY 2
#define DRIBBLING 3
#define GOAL 4
#define TO_TEAM 5

typedef struct message {
    long action;
    int result;
    int n_player;
    char team;
}mex;