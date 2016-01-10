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
};

struct job_d {
    int pid;
    int input;
    int output;
    int errorput;
    char* pro_info[MAXPARA];
};

struct file_d fd_list[MAXFD]; /* The file descriptor list */
struct job_d job_list[MAXJOB];

int addfd(int fd);


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
            {"creat", no_argument, 0, 'e'},
            {"trunc", no_argument, 0, 't'},
            {"append",no_argument, 0, 'a'},
            {"wait", no_argument, 0, 'p'},
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
            case 'w': //rdonly & wronly
                if(verbose_flag) {
                    printf("wronly %s\n", optarg);
                }
                mode |= O_WRONLY;
                addfd(open(optarg, mode, 0644));
                mode = O_RDONLY;
                break;
            case 'r':
                if(verbose_flag) {
                    printf("rdonly %s\n", optarg);
                }
                addfd(open(optarg, mode));
                mode = O_RDONLY;
                break;
            case 'c': //command
                if(verbose_flag) {
                    printf("command");
                }
                optind--;
                struct job_d new_job_info;
                int k=0;
                for( ; optind<argc && *argv[optind] != '-'; optind++) {
                    if(verbose_flag) printf(" %s", argv[optind]);
                    if(k==0) new_job_info.input = atoi(argv[optind]);
                    if(k==1) new_job_info.output = atoi(argv[optind]);
                    if(k==2) new_job_info.errorput = atoi(argv[optind]);
                    if(k>=3) new_job_info.pro_info[k-3] = argv[optind];
                    k++;
                }
                if(verbose_flag) printf("\n");
                job_list[nextjob++] = new_job_info;
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
            case 'p':
                if(verbose_flag) printf("wait\n");
                for(scanfile=0; scanfile<nextjob; scanfile++) {
                    dup2(fd_list[job_list[scanfile].input].fd, 0);
                    dup2(fd_list[job_list[scanfile].output].fd, 1);
                    dup2(fd_list[job_list[scanfile].errorput].fd, 2);
                    curpid = fork();
                    if(curpid == 0) { //child process
                        execvp(job_list[scanfile].pro_info[0],job_list[scanfile].pro_info);
                        break;
                    } else { //parent process
                        job_list[scanfile].pid = curpid;
                        waitpid(curpid, &status, 0);
                        close(fd_list[job_list[scanfile].input].fd);
                        close(fd_list[job_list[scanfile].output].fd);
                        close(fd_list[job_list[scanfile].errorput].fd);
                    }
                }
                if(scanfile == nextjob)  {//top parent process
                
                }
                break;
            case '?':
                if (optopt == 'c' || optopt == 'r' || optopt == 'w')
                    fprintf (stderr, "Option -%c requires an argument.\n", optopt);
//                else
//                    fprintf (stderr,
//                             "Unknown option --%s.\n",
//                             argv[optind-1]);
//                return 1;
                break;
            default:
                abort ();
        }
    }
    
    
    exit (0);
}

int addfd(int fd) {
    if(fd<0) return 0;
    if(fd>MAXFD) {
        printf("Tried to open too many files.");
        return 0;
    }
    struct file_d new_file_descriptor = {fd};
    fd_list[nextfd] = new_file_descriptor;
    nextfd++;
    return 1;
}
