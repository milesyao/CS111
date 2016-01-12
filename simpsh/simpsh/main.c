//
//  main.c
//  simpsh
//
//  Created by cnyao on 1/9/16.
//  Copyright © 2016 cnyao. All rights reserved.
//

//
//  simpsh.c
//  simpsh
//
//  Created by cnyao on 1/9/16.
//
//
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>

#define MAXFD 1<<16 //subject to change
#define MAXJOB 1<<16 //subject to change
#define MAXPARA 1<<7 //max parameters for a job


static int verbose_flag;
int nextfd = 0; //next file descriptor to allocate
int nextjob = 0;
struct file_d {              /* The internal file descriptor */
    int fd;                 /* system level file descriptor */
    int peer; //if file descriptor is one end of pipe, peer will store the file descriptor of another end. It will be sent to -1 by default
};

//typedef int file_d;

struct job_d {
    int pid;
    int input;
    int output;
    int errorput;
    char* pro_info[MAXPARA];
};

struct file_d fd_list[MAXFD]; /* The file descriptor list */
struct job_d job_list[MAXJOB];

int addfd(int fd, int peer);
int addjob(struct job_d newjob);

int main (int argc, char **argv)
{
    int c;
    mode_t mode = O_RDONLY;
    int curpid;
    int scanfile;
    int status;

    while (1)
    {
        static struct option long_options[] =
        {
            /* These options set a flag. */
            {"verbose", no_argument, &verbose_flag, 1},
            {"rdonly",  required_argument, 0, 'r'},
            {"wronly",  required_argument, 0, 'w'},
            {"command", required_argument, 0, 'c'},
            {"pipe", no_argument, 0, 'p'},
            {"creat", no_argument, 0, 'e'},
            {"trunc", no_argument, 0, 't'},
            {"append",no_argument, 0, 'a'},
            {"wait", no_argument, 0, 'q'},
            {0, 0, 0, 0}
        };
        /* getopt_long stores the option index here. */
        int option_index = 0;
        
        c = getopt_long (argc, argv, "r:w:c:etap",
                         long_options, &option_index);
        
        /* Detect the end of the options. */
        if (c == -1)
            break;
        
        switch (c)
        {
            case 0:
                /* If this option set a flag, do nothing else now. */
//                if (long_options[option_index].flag != 0)
//                    break;
//                printf ("option %s", long_options[option_index].name);
//                if (optarg)
//                    printf (" with arg %s", optarg);
//                printf ("\n");
                break;
            case 'p': //pipe
                if(verbose_flag) printf("pipe\n");
                int fd[2];
                pipe(fd);
                addfd(fd[0], fd[1]);
                addfd(fd[1], fd[0]);
                break;
            case 'w': //rdonly & wronly
                if(verbose_flag) {
                    printf("wronly %s\n", optarg);
                }
                mode |= O_WRONLY;
                addfd(open(optarg, mode, 0644), -1);
                mode = O_RDONLY;
                break;
            case 'r':
                if(verbose_flag) {
                    printf("rdonly %s\n", optarg);
                }
                addfd(open(optarg, mode), -1);
                mode = O_RDONLY;
                break;
            case 'c': //command
                if(verbose_flag) {
                    printf("command");
                }
                optind--;
                struct job_d new_job_info;
                int k=0;
                for( ; optind<argc && !(*argv[optind] == '-' && *(argv[optind]+1) == '-'); optind++) {
                    if(verbose_flag) printf(" %s", argv[optind]);
                    if(k==0) new_job_info.input = atoi(argv[optind]);
                    if(k==1) new_job_info.output = atoi(argv[optind]);
                    if(k==2) new_job_info.errorput = atoi(argv[optind]);
                    if(k>=3) new_job_info.pro_info[k-3] = argv[optind];
                    k++;
                }
                if(verbose_flag) printf("\n");
                addjob(new_job_info);
                break;
            case 'e': //creat
                if(verbose_flag) printf("creat\n");
                mode |= O_CREAT;
                break;
            case 't': //trunc
                if(verbose_flag) printf("trunc\n");
                mode |= O_TRUNC;
                break;
            case 'a': //append
                if(verbose_flag) printf("append\n");
                mode |= O_APPEND;
                break;
            case 'q':
                if(verbose_flag) printf("wait\n");
                for(scanfile=0; scanfile<nextjob; scanfile++) {
                    curpid = fork();
                    if(curpid == 0) { //child process
                        if(fd_list[job_list[scanfile].input].peer > 0) {
                            close(fd_list[job_list[scanfile].input].peer);
                        }
                        dup2(fd_list[job_list[scanfile].input].fd, 0);
                        if(fd_list[job_list[scanfile].output].peer > 0) {
                            close(fd_list[job_list[scanfile].output].peer);
                        }
                        dup2(fd_list[job_list[scanfile].output].fd, 1);
                        dup2(fd_list[job_list[scanfile].errorput].fd, 2);
                        execvp(job_list[scanfile].pro_info[0],job_list[scanfile].pro_info);
                        break;
                    } else { //parent process
                        job_list[scanfile].pid = curpid;
                    }
                }
                if(scanfile == nextjob)  {//top parent process
                    for(int i=0; i<nextfd; i++) {
                        close(fd_list[i].fd);
                    }
                    int pid_count=0;
                    while(pid_count < nextjob) {
                        waitpid(-1, &status, 0);
                        pid_count++;
                    }
                }
                break;
            case '?':
                if (optopt == 'c' || optopt == 'r' || optopt == 'w')
                    fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                break;
            default:
                abort ();
        }
    }
    
    
    exit (0);
}

int addfd(int fd, int peer) {
    if(fd<0) return 0;
    if(fd>MAXFD) {
        printf("Tried to open too many files.");
        return 0;
    }
    struct file_d new_file_descriptor = {fd, peer};
    fd_list[nextfd++] = new_file_descriptor;
    return 1;
}

int addjob(struct job_d newjob) {
    if(nextjob>MAXJOB) {
        printf("Tried to run too mand commands.");
        return 0;
    }
    job_list[nextjob++] = newjob;
    return 1;
}