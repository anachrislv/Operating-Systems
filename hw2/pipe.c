#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <assert.h>

#include <sys/wait.h>

#include "proc-common.h"
#include "tree.h"
#define SLEEP_SEC 10

void traverse(struct tree_node* node, int *pfdback){
    pid_t pid;
    int status;
    int value;
    //printf("%c\n", node->name[0]);
    //printf("%c - %d\n", node->name[0], getpid());

    if(node->nr_children == 0){
        value=atoi(node->name);
        if (write(pfdback[1], &value,
        sizeof(value)) != sizeof(value)) {
            perror("write to pipe");
            exit(1);
        }
        printf("I am returning value = %d to my parent\n", 
        value);
        close(pfdback[1]);
        }
    if(node->nr_children !=0){
        struct tree_node* cp;
        cp=node->children;
        int num[node->nr_children];
        int i;
        for(i=0;i<node->nr_children;i++){
            int pfd[2];
            num[i]=0;

            if (pipe(pfd) < 0) {
                perror("pipe");
                exit(1);
            }

            pid = fork();
            if (pid < 0) {
                perror("fork");
                exit(1);
            }
            else if (pid == 0) {
                traverse(cp,pfd);
            }
            else{
                pid=wait(&status);
                //explain_wait_status(pid,status);
                if (read(pfd[0], &value, 
                sizeof(value)) != sizeof(value)) {
                    perror("read from pipe");
                    exit(1);
                    }
                num[i]=value;
                close(pfd[0]);
                
                        }
                cp++;
                }
            //printf("%d %d\n",num[0],num[1]);
            if(node->name[0]=='+'){
                value=num[0]+num[1];
            }
            else if(node->name[0]=='*'){
                value=num[0]*num[1];
            }
            printf("Performing %d %c %d\n",
            num[0],node->name[0],num[1]);
            if (write(pfdback[1], &value, 
            sizeof(value)) != sizeof(value)) {
                perror("write to pipe");
                exit(1);
            }
            printf("I am returning value = %d to
            my parent\n", value);
        }
    close(pfdback[1]);
    exit(0);
    return ;
}

int main(int argc, char* argv[]){
    pid_t pid;
    if (argc!=2)
        printf("Wrong usage of command.\n");

    struct tree_node *root;
    root = get_tree_from_file(argv[1]);

    int status;
    int value;
    int pfd[2];

    if (pipe(pfd) < 0) {
        perror("pipe");
        exit(1);
    }

    pid=fork();
    if (pid < 0) {
        perror("main: fork");
        exit(1);
        }
    else if (pid == 0) {
        traverse(root, pfd);
        exit(0);
    }
    else {
        pid=wait(&status);
        if (read(pfd[0], &value, 
        sizeof(value)) != sizeof(value)) {
            perror("read from pipe");
            exit(1);
        }
        printf("child received value: 
        value = %d\n", value);
        close(pfd[0]);
    }
    return 0;
}
