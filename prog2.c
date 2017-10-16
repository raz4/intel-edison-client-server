#include <math.h>
#include <unistd.h>
#include <mraa/aio.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <netdb.h>
#include <netinet/in.h>

#include <pthread.h>

#define h_addr h_addr_list[0]

const int B=4275;
const int R0 = 100000;
const int pinTempSensor = 0;
int end = 0;

struct thread_arg {
    int thread_num;
    int socketfd;
};

int pipefd[2];

int command_count = 0;

int period = 3;

int scale = 0; // 0 = F, 1 = C

int stop = 0;

int invalid_flag = 0;

// time
time_t rawtime;
struct tm *info;
char buffer[20];


void *read_socket(void *arg){
    
    struct thread_arg* data = arg;
    
    char read_buf[256];
    
    while (!end){
        memset(read_buf, 0, 256);
        read(data->socketfd, read_buf, sizeof(read_buf));
        command_count++;
        write(pipefd[1], read_buf, 256);
    }
    
}

void *write_socket(void *arg){
    
    struct thread_arg* data = arg;
    
    // setup sensor
    mraa_init();
    mraa_aio_context aio;
    aio = mraa_aio_init(pinTempSensor);
    int a;
    float R;
    float temperature;
    FILE *f = fopen("lab4_2.log", "w");
    //time_t rawtime;
    struct tm *info;
    char buffer[20];
    //setenv("TZ", "PST8PDT", 1);
    
    char write_buf[256];
    
    while (!end){
        
        f = fopen("lab4_2.log", "a");
        
        if (!stop){
            
            a = mraa_aio_read(aio);
            R = 1023.0/((float)a)-1.0;
            R = 100000.0*R;
            temperature = 1.0/(log(R/100000.0)/B+1/298.15)-273.15;
            
            if (!scale){
                temperature = temperature * 9/5 + 32;
            }
            
            //time
            time(&rawtime);
            info = localtime(&rawtime);
            strftime(buffer, 20, "%T", info);
            
            fprintf(f, "%s %0.1f\n", buffer, temperature);
            fprintf(stdout, "704666892 TEMP=%0.1f\n", temperature);
            
            memset(write_buf, 0, 256);
            sprintf(write_buf, "704666892 TEMP=%0.1f", temperature);
            if (!stop){
                
            }
            if (write(data->socketfd, write_buf, sizeof(write_buf)) < 0){
                perror("Error writing to socket!");
                exit(1);
            }

            
        }
        
        sleep(period);
        
        // read commands
        memset(write_buf, 0, 256);
        invalid_flag = 0;
        if (command_count > 0){
            read(pipefd[0], write_buf, 256);
            command_count = command_count - 1;
            invalid_flag = 1;
        }
        
        // apply commands
        
        if (!strcmp(write_buf, "STOP")){
            stop = 1;
            invalid_flag = 0;
        }
        
        char* ret = NULL;
        if (ret = strstr(write_buf, "PERIOD=")){
            int prev_period = period;
            int i = 0;
            period = atoi(ret+7);
            if (period > 0 && period < 3601){
                invalid_flag = 0;
            }
            else{
                period = prev_period;
            }
        }
        
        if (ret = strstr(write_buf, "DISP Y")){
            invalid_flag = 0;
        }
        
        if (ret = strstr(write_buf, "DISP N")){
            invalid_flag = 0;
        }
        
        if (!strcmp(write_buf, "SCALE=C")){
            scale = 1;
            invalid_flag = 0;
        }
        
        if (!strcmp(write_buf, "SCALE=F")){
            scale = 0;
            invalid_flag = 0;
        }
        
        if (!strcmp(write_buf, "START")){
            invalid_flag = 0;
            stop = 0;
        }
        
        if (!strcmp(write_buf, "OFF")){
            invalid_flag = 0;
            end = 1;
        }
        
        if (invalid_flag){
            fprintf(f, "%sI\n", write_buf);
            fprintf(stdout, "%sI\n", write_buf);
        }
        else if (write_buf[0] != 0){
            fprintf(f, "%s\n", write_buf);
            fprintf(stdout, "%s\n", write_buf);
        }
        
        fclose(f);
        
    }
    
}

int main(int argc, char* argv[]){
    
    pipe(pipefd);
    
    // time
    setenv("TZ", "PST8PDT", 1);
    
    // server stuff
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd < 0){
        perror("Error opening socket");
        exit(1);
    }
    
    int port = 16000;
    struct sockaddr_in serv_addr;
    
    char string[] = "704666892 TEMP = 95.0";
    
    char buffer2[256];
    memset(buffer2, 0, 256);
    
    strcpy(buffer2, string);
    
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    
    struct hostent *server = NULL;
    server = gethostbyname("r01.cs.ucla.edu");
    if (server == NULL){
        perror("ERROR! Host not found.");
        exit(1);
    }
    
    memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr_list[0], server->h_length);
    serv_addr.sin_port = htons(port);
    
    if (connect(socketfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
        perror("Error connecting to server!");
        exit(1);
    }
    
    pthread_t threads[2];
    struct thread_arg* args = calloc(2, sizeof(struct thread_arg));
    
    for (int i = 0; i < 2; i++){
        args[i].thread_num = i;
        args[i].socketfd = socketfd;
    }
    
    int b;
    b = pthread_create(&threads[0], NULL, write_socket, &args[0]);
    if (b){
        perror("Error creating thread!");
        exit(1);
    }
    
    b = pthread_create(&threads[1], NULL, read_socket, &args[1]);
    if (b){
        perror("Error creating thread!");
        exit(1);
    }

    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);
    
    close(socketfd);
    
    return 0;

}
