//
//  main.c
//  testsimsh
//
//  Created by cnyao on 1/10/16.
//  Copyright © 2016 cnyao. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>

int main(int argc, const char * argv[]) {
    int fd1 = open("out", O_CREAT | O_TRUNC | O_WRONLY, 0644);
    int fd2 = open("test2", O_RDONLY);
    int test1 = O_CREAT | O_TRUNC | O_WRONLY;
    int test2 = O_RDONLY;
    dup2(fd1, 1);
    dup2(fd2, 0);
    int status;
    int pid = fork();
    if(pid==0) {
        char* a[] = {"cat", "test", "-", NULL, NULL, NULL};
        int status = execvp(a[0], a);
    } else {
        waitpid(pid, &status, 0);
        close(fd1);
//        close(fd2);
    }
    int test = atoi("31dsf2");
    exit(0);
    
}
