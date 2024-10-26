#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <assert.h>

#include <sys/wait.h>

#include "proc-common.h"

#define SLEEP_SEC 10


void fork_procsc();

void  fork_proscsd(){
    pid_t pid;
    int status;
    pid=fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    }
    if (pid == 0) {
        /* Child */
        printf("hello, process D  here!\n");
        printf("process D sleeping\n");
        sleep(SLEEP_SEC);
        printf("process D terminated\n");
        exit(13);
    }
    else {
        printf("process B waiting for process D\n");
        //printf("D - %d\n",pid);
        pid=wait(&status);
        explain_wait_status(pid, status);

    }

}


void fork_procsb() {
    pid_t pid;
    int status;

    pid=fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    }
    if (pid == 0) {
        /* Child */
        printf("hello, process B  here!\n");
        fork_proscsd();
        printf("process B terminated\n");
        exit (19);

    }
    else {
        printf("process A waiting for
        process B\n");
        //printf("B - %d\n", pid);
        fork_procsc();
        pid=wait(&status);
        explain_wait_status(pid, status);
        //fork_procsc();

    }



}
void fork_procsc () {
    pid_t pid;
    pid=fork();
    int status;
    if (pid < 0) {
        perror("fork");
        exit(1);
    }
    if (pid == 0) {
        /* Child */
        printf("hello, process C here!\n");
        printf("process C sleeping\n");
        sleep(SLEEP_SEC);
        printf("process C terminated\n");
        exit(17);

    }
    else {
        printf("process A waiting for process C\n");
        //printf("C - %d\n", pid);
        pid=wait(&status);
        explain_wait_status(pid, status);


    }
}


int main(void)
{
    pid_t pid, mpid;
    int status;
    /* Fork root of process tree */
    pid = fork();
    if (pid < 0) {
        perror("main: fork");
        exit(1);
    }
    if (pid == 0) {
        printf("hello, process A here!\n");
        /* Child */
        mpid=getpid();
        fork_procsb();
        printf("process A terminated\n");
        exit(16);

    }
    else {
        mpid=pid;
        show_pstree(mpid);
        pid = wait(&status);
        explain_wait_status(pid, status);
    }
    return 0;
}
