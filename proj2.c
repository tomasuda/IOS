#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <time.h> // Include for srand() and rand()
#include <sys/time.h> // Include for gettimeofday()
#include <sys/mman.h> 
#include <string.h>
#include <fcntl.h>

#define L 20000
#define Z 10
#define K 100
#define TL 10000
#define TB 1000
FILE *file;

int *number_of_lines;
int *number_of_pasangers;
int *remaining_pasangers;
int *current_bussstop;
int *busstop_array;
int *morning_bird;
int *capacity;

sem_t   *printing,
        *waiting_for_bus,
      //  *going_to_buss,//netreba
      //  *waiting_for_boarding,//netreba
        *buss_in_motion,//netreba
        *going_to_ski,
        *pasangers_are_boarding,
        *buss_stop_semafor,
        *leaving_buss;

typedef struct {
    int skier;
    int walk;
    int busstop;
    int buss_capacity;
    int ride;
} str_lyzovacka;

void destroier() {
        sem_destroy(printing);                     
        munmap(printing, sizeof(sem_t));           
        sem_destroy(waiting_for_bus);                
        munmap(waiting_for_bus, sizeof(sem_t));      
        sem_destroy(buss_in_motion);              
        munmap(buss_in_motion, sizeof(sem_t));    
        sem_destroy(buss_stop_semafor);                     
        munmap(buss_stop_semafor, 10*sizeof(sem_t));           
        sem_destroy(leaving_buss);              
        munmap(leaving_buss, sizeof(sem_t));    
        sem_destroy(going_to_ski);             
        munmap(going_to_ski, sizeof(sem_t));   
        sem_destroy(pasangers_are_boarding);           
        munmap(pasangers_are_boarding, sizeof(sem_t));
        // sem_destroy(in_buss);           
        // munmap(in_buss, sizeof(sem_t));         
   
    // Unmap shared memory regions
    munmap(number_of_lines, sizeof(int));
    munmap(number_of_pasangers, sizeof(bool));
    munmap(remaining_pasangers, sizeof(int));
    munmap(current_bussstop, sizeof(int));
    munmap(busstop_array, 10*sizeof(int));
    munmap(morning_bird, sizeof(int));
    fclose(file);
}

int moj_exit(int chyba);

int Nahodne_cislo(int min, int max) {
    int dlzka = max - min + 1;
    int random_cislo = rand() % dlzka + min;
    return random_cislo;
}

void lyziar_proces( int i,str_lyzovacka *data) {
    sem_wait(printing);
    struct timeval tv;
    gettimeofday(&tv, NULL);
    srand(tv.tv_usec * getpid());
    int running_for_buss = Nahodne_cislo(0, data->walk);
    int random_busstop = Nahodne_cislo(0, (data->busstop)-1)+1;
    sem_post(printing);
   
    sem_wait(printing);
    fprintf(file,"%d: L %i: started\n",(*number_of_lines)++,i);
    fflush(file);
    usleep(running_for_buss);
    sem_post(printing);

    sem_wait(printing);
    fprintf(file,"%d: L %i: arrived to %d\n",(*number_of_lines)++,i,random_busstop);
    fflush(file);
    (*morning_bird)++;
    busstop_array[random_busstop-1]+=1;
        sem_post(printing);
        sem_post(buss_in_motion);// hovori autobusu ze sa moze zacat 
        sem_wait(&buss_stop_semafor[(random_busstop-1)]);
        (*capacity)--;
        (*number_of_pasangers)++;
        fprintf(file,"%d: L %i: boarding\n",(*number_of_lines)++,i);
        fflush(file);
         busstop_array[*current_bussstop-1]-=1;
        sem_post(pasangers_are_boarding);
        sem_wait(going_to_ski);
        fprintf(file,"%d: L %i: going to ski\n",(*number_of_lines)++,i);
        fflush(file);
        sem_post(leaving_buss);
        exit(0);
    }

void bus_proces(str_lyzovacka *data) {
    sem_wait(buss_in_motion);
    struct timeval tv;
    gettimeofday(&tv, NULL);
    srand(tv.tv_usec * getpid());

    sem_wait(printing);
    fprintf(file,"%d: BUS: started\n",(*number_of_lines)++);
    fflush(file);
    sem_post(printing);

    while(*remaining_pasangers>0){ 
     usleep(Nahodne_cislo(0, data->ride));

        sem_wait(printing);
        (*current_bussstop) = ((*current_bussstop)%data->busstop)+1;
        fprintf(file,"%d: BUS: arrived to %d\n",(*number_of_lines)++,*current_bussstop);
        fflush(file);
        sem_post(printing);

            sem_wait(printing);
                      if ((busstop_array[*current_bussstop-1]>0)&& ((*capacity)>0)){
                do{
                    sem_post(&buss_stop_semafor[*current_bussstop-1]);
                    sem_wait(pasangers_are_boarding);
                }while((busstop_array[*current_bussstop-1]>0) && ((*capacity)>0));
            }
        sem_post(printing);

        sem_wait(printing);
        fprintf(file,"%d: BUS: leaving %d\n",(*number_of_lines)++,*current_bussstop);
        fflush(file);
        sem_post(printing);

        sem_wait(printing);
                if ((*current_bussstop) == data->busstop){                   

                       fprintf(file,"%d: BUS: arrived to final\n",(*number_of_lines)++);
                       fflush(file);
                        while(*number_of_pasangers>0){
                            sem_post(going_to_ski);
                            sem_wait(leaving_buss);
                            (*capacity)=data->buss_capacity+1;
                            (*number_of_pasangers)--;
                            (*remaining_pasangers)--;
                        }
                   fprintf(file,"%d: BUS: leaving final\n",(*number_of_lines)++); 
                    fflush(file);    
                 }
                sem_post(printing);
    }
        sem_wait(printing);
        for(int i=0;i<10;i++) {

        }      
        // printf("\n");
        // printf("%d akkskaks\n",*current_bussstop);
        // fprintf(file,"%d: BUS: finish\n",(*number_of_lines));
        // fflush(file);
        // sem_post(printing);
    exit(0);
    }

int moj_exit(int chyba) {
    switch (chyba) {
        case 1:
            fprintf(stderr, "zly skier parametrov\n");
            exit(1);
            break;
        case 2:
            fprintf(stderr, "jeden z argumentov nie je cislo\n");
            exit(1);
            break;
        case 3:
            fprintf(stderr, "chyba tvorby procesu\n");
            exit(1);
            break;
        default:
            break;
    }
    return 0;
}

bool is_number(char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (!isdigit(str[i])) {
            return false;
        }
    }
    return true;
}

int main(int argc, char *argv[]) {
    setbuf(stdout,NULL);
    
    if (argc != 6) moj_exit(1);
    if (!is_number(argv[1]) || atoi(argv[1]) < 0 || atoi(argv[1]) >= L) moj_exit(2);
    if (!is_number(argv[2]) || atoi(argv[2]) <= 0 || atoi(argv[2]) > Z) moj_exit(2);
    if (!is_number(argv[3]) || atoi(argv[3]) < 10 || atoi(argv[3]) > K) moj_exit(2);
    if (!is_number(argv[4]) || atoi(argv[4]) < 0 || atoi(argv[4]) > TL) moj_exit(2);
    if (!is_number(argv[5]) || atoi(argv[5]) < 0 || atoi(argv[5]) > TB) moj_exit(2);

    printf("L: %s\n", argv[1]);// skier lyziarov
    printf("Z: %s\n", argv[2]);// skier zastavok
    printf("K: %s\n", argv[3]);// kapacita autobusu
    printf("TL: %s\n", argv[4]);// max doba prichodu na zastavku
    printf("TB: %s\n", argv[5]);// max doba medzi zastavkami

    str_lyzovacka data = {atoi(argv[1]), atoi(argv[4]), atoi(argv[2]),atoi(argv[3]), atoi(argv[5])};
    file = fopen("proj2.out", "w");

    printing = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    waiting_for_bus = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    buss_in_motion = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    going_to_ski = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    pasangers_are_boarding = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    buss_stop_semafor =   mmap(NULL, data.busstop*sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    leaving_buss= mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    
    // Initializace semaforov
    sem_init(printing, 1, 1); // mutex semaphore
    sem_init(waiting_for_bus, 1, 0);
    sem_init(buss_in_motion, 1, 0);//netreba
    sem_init(going_to_ski, 1, 0);
    sem_init(pasangers_are_boarding, 1, 0);
    sem_init(leaving_buss, 1, 0);

    for (int i = 0; i < data.busstop; i++) {
          sem_init(&buss_stop_semafor[i], 1, 0);  
    }

    number_of_lines = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    number_of_pasangers = mmap(NULL, sizeof(bool), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    remaining_pasangers = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    current_bussstop = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    busstop_array = mmap(NULL, (10*sizeof(int)), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    morning_bird= mmap(NULL, (sizeof(int)), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    capacity= mmap(NULL, (sizeof(int)), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    *remaining_pasangers=data.skier;
    *number_of_lines=1;
    *current_bussstop=0;
    *morning_bird=0;
    *capacity = data.buss_capacity+1;

    int bus_pid = fork();
    if (bus_pid == -1) {
        moj_exit(3);
    } else if (bus_pid == 0) {
        bus_proces(&data);
    }
  
    for (int i = 0; i < data.skier; i++) {
        int skier_pid = fork();
        if (skier_pid == -1) {
            moj_exit(3);
        } else if (skier_pid == 0 && bus_pid!=0) {
            lyziar_proces((i+1),&data);
        }
    }

    while (wait(NULL) > 0);
    destroier();
    return 0;
}
