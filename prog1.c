#include <math.h>
#include <unistd.h>
#include <mraa/aio.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const int B=4275;
const int R0 = 100000;
const int pinTempSensor = 0;

int main(int argc, char* argv[]){
    mraa_init();
    mraa_aio_context aio;
    aio = mraa_aio_init(pinTempSensor);
    int a;
    float R;
    float temperature;
    
    FILE *f = fopen("lab4_1.log", "w");
    
    time_t rawtime;
    struct tm *info;
    char buffer[20];
    setenv("TZ", "PST8PDT", 1);
    
    while (1){
        
        f = fopen("lab4_1.log", "a");
        a = mraa_aio_read(aio);
        R = 1023.0/((float)a)-1.0;
        R = 100000.0*R;
        temperature = 1.0/(log(R/100000.0)/B+1/298.15)-273.15;
        temperature = temperature * 9/5 + 32;
        
        time(&rawtime);
        info = localtime(&rawtime);
        strftime(buffer, 20, "%T", info);
        
        fprintf(f, "%s %0.1f\n", buffer, temperature);
        fprintf(stdout, "%s %0.1f\n", buffer, temperature);
        fclose(f);
        sleep(1);

    }
    
}
