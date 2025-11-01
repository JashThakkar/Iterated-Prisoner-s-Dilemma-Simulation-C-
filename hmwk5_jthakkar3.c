/*
hmwk5.c

Jash Thakkar
CSc 3320 Lab/Homework #5
Account: jthakkar3
Due date: 12/09

Description: The Iterated Prisoner's Dilemma

Input: either the number of rounds you would want to play or -h for help

Output: The Score of each round and the final scores / avg score per player

Usage: simulation of the tit for tat stratagy vs nnconditional defector stratagy
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

// Read and writing defs
#define READ 0
#define WRITE 1

// Set to 1 for debug mode which shows all process as they happend between each round (1 = on 0 = off)
int DEBUG = 0;

// Debug functionality
#define debug_print(process, message) \
    if (DEBUG) { printf("[%s DEBUG]: %s\n", process, message); fflush(stdout); }

// player / parent def
void player1(int read_fd, int write_fd);
void player2(int read_fd, int write_fd);
void parent_process(int rounds, int p1_write, int p1_read, int p2_write, int p2_read);

int main(int argc, char *argv[]) {
    int i;
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0) {
            // If -h is found, print the help message and exit
            printf("Help command called \nUsage: ./programOutFileName [number_of_rounds] [-h anywhere for help]\n");
            return 0;
        }
    }

    int rounds = (argc > 1) ? atoi(argv[1]) : 0;

    if (rounds == 0){
        // If the user puts no amount of rounds to play they will get this error message
        printf("Please put a number after the ./programOutFileName example: ./a 5");
        return 0;
    }

    int p1_pipe[2], p2_pipe[2], p1_reply[2], p2_reply[2];

    pipe(p1_pipe); // Parent to player 1
    pipe(p2_pipe);  // Parent to player 2
    pipe(p1_reply);  // Player 1 to parent
    pipe(p2_reply);  // Player 2 to parent

    pid_t p1 = fork();
    if (p1 == 0) {
        close(p1_pipe[WRITE]);
        close(p1_reply[READ]);
        player1(p1_pipe[READ], p1_reply[WRITE]);
        exit(0);
    }

    pid_t p2 = fork();
    if (p2 == 0) {
        close(p2_pipe[WRITE]);
        close(p2_reply[READ]);
        player2(p2_pipe[READ], p2_reply[WRITE]);
        exit(0);
    }

    close(p1_pipe[READ]);
    close(p1_reply[WRITE]);
    close(p2_pipe[READ]);
    close(p2_reply[WRITE]);

    parent_process(rounds, p1_pipe[WRITE], p1_reply[READ], p2_pipe[WRITE], p2_reply[READ]);

    wait(NULL); // Null wait times p1
    wait(NULL); // Null wait times p2

    return 0;
}

// P1 strat: Unconditional Defector aka always defect
void player1(int read_fd, int write_fd) {
    char command;
    debug_print("Player 1", "Started process");

    while (read(read_fd, &command, 1) > 0) {
        if (command == 'R') {
            debug_print("Player 1", "Received 'R' - Sending 'Y'");
            write(write_fd, "Y", 1);
        } else if (command == 'P') {
            debug_print("Player 1", "Received 'P' - Sending 'D' (Defect)");
            write(write_fd, "D", 1);
        } else if (command == 'Q') {
            debug_print("Player 1", "Received 'Q' - Exiting");
            break;
        } else {
            char score;
            char debug_msg[50];
            snprintf(debug_msg, sizeof(debug_msg), "Received score: %c - Sending 'Y'", score);
            debug_print("Player 1", debug_msg);
            write(write_fd, "Y", 1);
            }
        }
}

// P2 strat: Tit for Tat aka Do what they do
void player2(int read_fd, int write_fd) {
    char command;
    char last_move = 'C';
    debug_print("Player 2", "Started process");
    while (read(read_fd, &command, 1) > 0) {
        if (command == 'R') {
            debug_print("Player 2", "Received 'R' - Sending 'Y'");
            write(write_fd, "Y", 1);
        } else if (command == 'P') {
            debug_print("Player 2", "Received 'P' - Sending move");
            write(write_fd, &last_move, 1);
        } else if (command == 'Q') {
            debug_print("Player 2", "Received 'Q' - Exiting");
            break;
        } else {
            // p2 needs to read scores to make next dec
            char score;
            char debug_msg[50];
            snprintf(debug_msg, sizeof(debug_msg), "Received score: %c - Sending 'Y'", score);
            debug_print("Player 2", debug_msg);
            write(write_fd, "Y", 1);
            if (score != '5' || score != '0') {
                last_move = 'D';
            } else {
                last_move = 'C';
                }
            }
        }
}

// Parenting / running the game through this function
void parent_process(int rounds, int p1_write, int p1_read, int p2_write, int p2_read) {
    int score1 = 0, score2 = 0;
    debug_print("Parent", "Started process");
    for (int i = 0; i < rounds; i++) {
        // Sending ready up commands
        debug_print("Parent", "Sending 'R' to both players");
        write(p1_write, "R", 1);
        write(p2_write, "R", 1);

        char reply1, reply2;
        read(p1_read, &reply1, 1);
        read(p2_read, &reply2, 1);

        if (reply1 == 'Y' && reply2 == 'Y') {
            // Sending the Play commands
            debug_print("Parent", "Both players are ready");
            debug_print("Parent", "Sending 'P' to both players");
            write(p1_write, "P", 1);
            write(p2_write, "P", 1);

            char move1, move2;
            read(p1_read, &move1, 1);
            read(p2_read, &move2, 1);

            char debug_msg[100];
            snprintf(debug_msg, sizeof(debug_msg), "Player 1 move: %c, Player 2 move: %c", move1, move2);
            debug_print("Parent", debug_msg);

            // Scoring logics
            if (move1 == 'C' && move2 == 'C') {
                score1 += 3;
                score2 += 3;
                write(p1_write, "3", 1);
                write(p2_write, "3", 1);
            } else if (move1 == 'C' && move2 == 'D') {
                score1 += 0;
                score2 += 5;
                write(p1_write, "0", 1);
                write(p2_write, "5", 1);
            } else if (move1 == 'D' && move2 == 'C') {
                score1 += 5;
                score2 += 0;
                write(p1_write, "5", 1);
                write(p2_write, "0", 1);
            } else {
                score1 += 1;
                score2 += 1;
                write(p1_write, "1", 1);
                write(p2_write, "1", 1);
            }

            
            char ack1, ack2;
            read(p1_read, &ack1, 1);
            read(p2_read, &ack2, 1);

            printf("Round %d: Player 1 %c, Player 2 %c. Scores: %d, %d\n", i + 1, move1, move2, score1, score2);
        }
    }

    write(p1_write, "Q", 1);
    write(p2_write, "Q", 1);

    // Final stats listings / scores
    printf("Final Scores: Player 1 = %d, Player 2 = %d\n", score1, score2);
    printf("Average Scores: Player 1 = %.2f, Player 2 = %.2f\n", (float)score1 / rounds, (float)score2 / rounds);
}