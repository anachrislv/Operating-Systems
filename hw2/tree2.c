#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <assert.h>

#include <sys/wait.h>

#include "proc-common.h"
#include "tree.h"
#define SLEEP_SEC 10

void traverse(struct tree_node* node, int current_child){
    pid_t pid;
    int status, j;

    printf("Process %c created with ID = %d\n",
    node->name[0], getpid());

    if(node->nr_children == 0){
        sleep(SLEEP_SEC);
        printf("Process %c exiting\n", 
        node->name[0]); 
        exit(0);
    }
    struct tree_node* cp;
    cp=node->children + current_child - 1;
    pid_t PID[node->nr_children];
    if(node->nr_children != 0){
        for (j=0; j<node->nr_children; j++){
            PID[j] = fork();
            if (PID[j] < 0) {
                perror("fork");
                exit(1);
            }
            else if (PID[j] == 0) {
                traverse(cp,1);
            }
            else{
                //pid=wait(&status);
            }
            cp++;
            }
            for(j=0; j<node->nr_children; j++){
                PID[j]=wait(&status);
                explain_wait_status(PID[j],
                status);             }
        }
    exit(0);
    return;
}

int main(int argc, char* argv[]){
    pid_t pid;
    int status;
    if (argc!=2)
        printf("Wrong usage of command.\n");

    struct tree_node *root;
    root = get_tree_from_file(argv[1]);

    pid=fork();
    if (pid < 0) {
        perror("main: fork");
        exit(1);
    }
    else if (pid == 0) {
        //printf("hello, root  here!\n");
        traverse(root, 1);
        exit(16);
    }
    else {
        show_pstree(pid);
        pid=wait(&status);
        explain_wait_status(pid,status);
    }
    return 0;
}
