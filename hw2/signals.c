#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "tree.h"
#include "proc-common.h"
#define SLEEP_SEC 3


void traverse(struct tree_node* node, 
int current_child){
    pid_t PID[node->nr_children];
    pid_t pid;
    int status;

    printf("%c - %d\n", node->name[0], getpid());

    if(node->nr_children == 0) {
        raise(SIGSTOP);
        printf("PID = %ld, name = %s is awake\n",
        (long)getpid(), node->name);
        exit(0);
        }
    if(node->nr_children != 0){
        struct tree_node* cp;
        cp=node->children + current_child - 1;
        int i;
        for(i=0;i<node->nr_children;i++){
            pid = fork();
            if (pid < 0) {
                perror("fork");
                exit(1);
            }
            else if (pid == 0) {
                traverse(cp,1);
            }
            else{

                PID[i]=pid;
            }
            cp++;
    }
        wait_for_ready_children(node->nr_children);
        raise(SIGSTOP);
        printf("PID = %ld, name = %s is awake\n",
        (long)getpid(), node->name);
        for (i=0;i<node->nr_children;i++){
            //printf("%d\n",i);
            kill(PID[i],SIGCONT);
            PID[i]=wait(&status);
            explain_wait_status(PID[i],status);
    }
        exit(0);
        }

        return;

}


int main(int argc, char *argv[]){
    pid_t pid;
    int status;
    struct tree_node *root;

    if (argc < 2){
        fprintf(stderr, "Usage: %s <tree_file>\n",
        argv[0]);
        exit(1);
    }

    /* Read tree into memory */
    root = get_tree_from_file(argv[1]);

    /* Fork root of process tree */
    pid = fork();
    if (pid < 0) {
        perror("main: fork");
        exit(1);
    }
    if (pid == 0) {
        /* Child */
        traverse(root, 1);
    }
    else{
        wait_for_ready_children(1);
        show_pstree(pid);
        //printf("%d\n",pid);
        kill(pid, SIGCONT);
        wait(&status);
        explain_wait_status(pid, status);
    }
    return 0;
}
