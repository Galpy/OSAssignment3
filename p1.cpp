       #include <errno.h>
       #include <stdio.h>
       #include <stdlib.h>
       #include <string.h>
       #include <sys/socket.h>
       #include <sys/un.h>
       #include <unistd.h>
       #include <sys/mman.h>
       #include <sys/stat.h>
       #include <fcntl.h>
       #include <semaphore.h>
       #include <fstream>
       #include <vector>
       #include <iostream>
       #include <pthread.h>

       #define BUFFER_SIZE 300


        #define SEM_NAME1 "/semaphore_example1"
        #define SEM_NAME2 "/semaphore_example2"
        #define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
        #define INITIAL_VALUE 0
       #define SHMPATH "/mysm007"
       using namespace std;
        //https://pubs.opengroup.org/onlinepubs/009695399/functions/stat.html
       size_t getFileSize(const char* fileName) {
           struct stat st;
           if (stat(fileName, &st) != 0) {
               return 0;
           }
           return st.st_size;
       }

       typedef struct{
           vector<string> v_tmp;
           const char * target;
           int begin;
           int end;
           vector<string> res_vec;
       } MY_ARGS;

       int
       main(int argc, char *argv[])
       {
           string file, stringLine, inputLines;
           const char* filePath = argv[1];
           FILE *fp = fopen(filePath,"r");
           // const char * target = argv[2]; //could be set as the target word input through commandline args, might be in p2

           sem_unlink(SEM_NAME1);
           sem_unlink(SEM_NAME2);


           size_t fSize = getFileSize(filePath);

           int fd = shm_open(SHMPATH, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
           if (fd == -1){
               perror("shm_open error!!\n");
               return 1;
           }

           //size_t shm_size = sizeof(size_t) + fSize + 2; //2 for "\n\0"
           ftruncate(fd, fSize+5);

           void* sm = mmap(NULL, fSize+5, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
           if (sm == MAP_FAILED){
               perror("ERROR!! MAP FAILED\n");
               return 1;
           }
           size_t nread = fread(sm, 1, fSize, fp);
           if (nread != fSize){
               perror("Error on fread");
               return 1;
           }

           //send size of file, then send file, then append END\N at the end of file so that we know we have reached the end of the file. Use 
           //same method as below to implement this.

           
           char * p = (char*) sm;
           memset((char*)(p+fSize/sizeof(char)), '\n', 1);
           memset((char*)(p+fSize/sizeof(char)+1), 'E', 1);
           memset((char*)(p+fSize/sizeof(char)+2), 'N', 1);
           memset((char*)(p+fSize/sizeof(char)+3), 'D', 1);
           memset((char*)(p+fSize/sizeof(char)+4), '\n', 1);

           sem_t *s1 = sem_open(SEM_NAME1, O_CREAT | O_EXCL, SEM_PERMS, INITIAL_VALUE);
           sem_t *s2 = sem_open(SEM_NAME2, O_CREAT | O_EXCL, SEM_PERMS, INITIAL_VALUE);

           if (s1 == SEM_FAILED || s2 == SEM_FAILED) {
               perror("sem_open error!\n");
               return 1;
           }

           sem_post(s1);
           cout << "SEM WAIT S2" << endl;
           sem_wait(s2);
           cout << "SEM WAIT S2 DONE" << endl;
           
           sem_unlink(SEM_NAME1);
           sem_unlink(SEM_NAME2);
           shm_unlink(SHMPATH);
           
           cout << "Next step..." << endl;

           s1 = sem_open(SEM_NAME1, 0);
           s2 = sem_open(SEM_NAME2, 0);

           //sem_wait(s2);

           ftruncate(fd, fSize+5);

           sm = mmap(NULL, fSize+5, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
           if (sm == MAP_FAILED){
               perror("ERROR!! MAP FAILED\n");
               return 1;
           }

           printf("Strings containing target word: \n %s \n", (char*) sm);

           if(munmap(sm, fSize +4) == -1){
               perror("UNMAP ERROR!!\n");
               return 1;
           }
           
           sem_unlink(SEM_NAME1);
           sem_unlink(SEM_NAME2);
           fclose(fp);
       }