 #include <errno.h>
       #include <stdio.h>
       #include <stdlib.h>
       #include <string.h>
       #include <sys/socket.h>
       #include <sys/un.h>
       #include <unistd.h>
       #include <sys/mman.h>
       #include <sys/stat.h>
       #include <sys/shm.h>
       #include <semaphore.h>
       #include <fcntl.h>
       #include <fstream>
       #include <sstream>
       #include <vector>
       #include <iostream>

       #define SEM_NAME1 "/semaphore_example1"
        #define SEM_NAME2 "/semaphore_example2"
        #define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
        #define INITIAL_VALUE 0

       #define SHMPATH "/mysm007"

       using namespace std;

       int containWordIgnoreCase(const char *line, const char* word, size_t n) {
           size_t m = strlen(word);

           char *lower_line = (char*) calloc(n+1, sizeof(char));
           for (size_t i = 0; i < n; ++i) {
               lower_line[i] = tolower((unsigned char)line[i]);
           }
           char *lower_word = (char*) calloc(m+1, sizeof(char));
           for (size_t i = 0; i < m; ++i) {
               lower_word[i] = tolower((unsigned char)word[i]);
           }   

           //printf("%s, %s\n", lower_line, lower_word);
           char * ret = strstr(lower_line, lower_word);
           //printf("%s\n", ret);
           int return_val = 0;
           while (ret != NULL){
               //the matched substring is the end of the sentence
               //or the character after the substring is neither a letter nor a number
               // punctuation marks
               if (strlen(ret)==m || !isalnum(ret[m])) {
                   // the sentence begins with the matched substring
                   if (ret == lower_line) {
                       return_val = 1;
                   } // otherwise the character before the substring is neither a letter nor a number 
                   else {
                       ret--;
                       if (!isalnum(ret[0]))
                       return_val = 1;
                   }
               }

               ret = strstr(ret+m, lower_word);
           }

           free(lower_line);
           free(lower_word);

           return return_val;
       }

       typedef struct{
           vector<string> v_tmp;
           const char * target;
           int begin;
           int end;
           vector<string> res_vec;
       } MY_ARGS;

       void * find_and_output(void* args){
           MY_ARGS* my_args = (MY_ARGS*) args;
           vector<string> v_tmp = my_args -> v_tmp;
           const char* target = my_args -> target;
           int first = my_args -> begin;
           int last = my_args -> end;

           int i = first;
           int counter = 0;
           for (; i < last; i++){
               const char * tmp_line = v_tmp[i].c_str();
               if (containWordIgnoreCase(tmp_line, target, strlen(tmp_line)))
               my_args -> res_vec.push_back(v_tmp[i]);
           }

           return NULL;
       }

       size_t getFileSize(const char* fileName) {
           struct stat st;
           if (stat(fileName, &st) != 0) {
               return 0;
           }
           return st.st_size;
       }

       int main(int argc, char *argv[]) {
           ofstream filePath("output.txt", ofstream::out);
           const char* fileString = "output.txt";
           const char * target = argv[1];
           vector<string> v3; //this needs to be filled by shared memory
           sem_t *s1 = sem_open(SEM_NAME1, 0);
           sem_t *s2 = sem_open(SEM_NAME2, 0);

           

                //shows how many lines there are in the vector

           sem_wait(s1);
           
           int fd = shm_open(SHMPATH, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
           if (fd == -1){
               perror("FD ERROR!!\n");
               return 1;
           }

           size_t shm_size = 100000000; //size of local file, probably not right for the big file
           ftruncate(fd, shm_size);

           void *sm = mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
           if (sm == MAP_FAILED){
               perror("ERROR!! MAP FAILED\n");
               return 1;
           }
           string input = (char*)sm;
           istringstream iss(input);
           string line;
           
              int counter = 0;
              int i = 0;
              char ch = 0;
           int lines = 0;
           do {
               ch = input.at(counter); //puts each char in input = to ch and checks for \n, if found then increase the amount of lines.
               if (ch == '\n'){
                   lines++;
               }
                counter++;
           } while (counter < input.size()-6);
            while (i <= lines){
                getline(iss,line,'\n');
            v3.push_back(line);
            i++;
            }

            string finalString;
            int how_many = v3.size();

            pthread_t th1;
            pthread_t th2;
            pthread_t th3;
            pthread_t th4; // These are created to take the vector and separate them into 4 parts to process

            MY_ARGS args1 = {v3, target, 0, how_many/4};
            MY_ARGS args2 = {v3, target, how_many/4, how_many/2};
            MY_ARGS args3 = {v3, target, how_many/2, (3*how_many)/4};
            MY_ARGS args4 = {v3, target, (3*how_many)/4, how_many};


            pthread_create(&th1, NULL, find_and_output, &args1); //find and output is a function to find and output the target word
            pthread_create(&th2, NULL, find_and_output, &args2);
            pthread_create(&th3, NULL, find_and_output, &args3);
            pthread_create(&th4, NULL, find_and_output, &args4);

            pthread_join(th1, NULL);
            pthread_join(th2, NULL);
            pthread_join(th3, NULL);
            pthread_join(th4, NULL);

            for (vector<string>::iterator t = args1.res_vec.begin(); t != args1.res_vec.end(); t++){
                finalString += *t +'\n';
            }
            for (vector<string>::iterator t = args2.res_vec.begin(); t != args2.res_vec.end(); t++){
                finalString += *t +'\n';
            }
            for (vector<string>::iterator t = args3.res_vec.begin(); t != args3.res_vec.end(); t++){
                finalString += *t +'\n';
            }
            for (vector<string>::iterator t = args4.res_vec.begin(); t != args4.res_vec.end(); t++){
                finalString += *t +'\n';
            }
            filePath << finalString;
            filePath.close();
            FILE *fp = fopen(fileString, "r");

            size_t finalSize = finalString.length();

           //size_t shm_size = sizeof(size_t) + fSize + 2; //2 for "\n\0"
           ftruncate(fd, finalSize);

           sm = mmap(NULL, finalSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
           if (sm == MAP_FAILED){
               perror("ERROR!! MAP FAILED\n");
               return 1;
           }

           size_t nread = fread(sm, 1, finalSize, fp);
           if (nread != finalSize){
               perror("NREAD ERROR!!\n");
               return 1;
           }


           sem_post(s2);


           printf("contents of region: \n %s \n", (char*) sm);

           if(munmap(sm, finalSize) == -1){
               perror("UNMAP ERROR!!\n");
               return 1;
           }

           shm_unlink(SHMPATH);
           sem_unlink(SEM_NAME1);
           sem_unlink(SEM_NAME2);

           return 0;
       }