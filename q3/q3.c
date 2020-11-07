#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>
#include <math.h>
#include <string.h>

#define NRM "\x1B[1;0m"
#define RED "\x1B[1;31m"
#define GRN "\x1B[1;32m"
#define YEL "\x1B[1;33m"
#define BLU "\x1B[1;34m"
#define MAG "\x1B[1;35m"
#define CYN "\x1B[1;36m"
#define WHT "\x1B[1;37m"

int k, a, e, c, t1, t2, t;
sem_t sem_stage_a;
sem_t sem_stage_e;
sem_t sem_stage_singer;
sem_t thread_join_wait;
sem_t sem_coordinators;

typedef struct Performer
{
    int idx;
    int time_of_arrival;
    char name[50];
    char type;
    int waiting_time;
    int has_arrived;
    int got_stage;
    int wait_print;
    pthread_t performer_thread_id;
    pthread_t performer_thread_id2;
    pthread_cond_t cv_singer;
    pthread_mutex_t performer_mutex;
} Performer;

typedef struct Stage
{
    int idx;
    int has_musician;
    int singer_id;
    char name_musician[50];
    int has_singer;
    char type;
    pthread_mutex_t stage_mutex;
} Stage;

Performer performers[100];
Stage stage[100];

void *electrical_thread(void *args);
void *acoustic_thread(void *args);
void *singer_thread(void *args);
void get_shirts(char *name);

int main()
{
    srand(time(0));
    printf("%sEnter the number of performers, acoustic stages,​ electric stages, coordinators, minimum performance time, maximum performance time, waiting time\n", BLU);
    scanf("%d%d%d%d%d%d%d", &k, &a, &e, &c, &t1, &t2, &t);
    if (k < 0 || a < 0 || e < 0 || t1 < 0 || t2 < 0 || t < 0 || c < 0)
    {
        printf("%sNone of the values can be negative\n", RED);
        return 0;
    }
    else if (c <= 0)
    {
        printf("%sThere must be one Co-ordinator to distribute T-shirts\n", RED);
        return 0;
    }
    else if (t1 > t2)
    {
        printf("%sMinimum performance time can't be greater than maximum performance time\n", RED);
        return 0;
    }
    sem_init(&(sem_stage_a), 0, a);
    sem_init(&(sem_stage_e), 0, e);
    sem_init(&(sem_stage_singer), 0, a + e);
    sem_init(&(sem_coordinators), 0, c);
    sem_init(&(thread_join_wait), 0, 0);

    printf("%sEnter the name, instrument type, arrival time\n", WHT);
    for (int i = 0; i < k; i++)
    {
        scanf("%s %c %d", performers[i].name, &performers[i].type, &performers[i].time_of_arrival);
        performers[i].idx = i + 1;
        performers[i].waiting_time = t;
        performers[i].has_arrived = 0;
        performers[i].got_stage = 0;
        performers[i].wait_print = 0;
        pthread_mutex_init(&(performers[i].performer_mutex), NULL);
    }
    for (int i = 0; i < a + e; i++)
    {
        stage[i].idx = i + 1;
        stage[i].has_singer = 0;
        stage[i].has_musician = 0;
        pthread_mutex_init(&(stage[i].stage_mutex), NULL);
        if (i < a)
            stage[i].type = 'a';
        else
            stage[i].type = 'e';
    }
    printf("%sBeginning Simulation\n\n", GRN);
    for (int i = 0; i < k; i++)
    {
        if (performers[i].type == 's')
        {
            pthread_create(&(performers[i].performer_thread_id), NULL, singer_thread, &performers[i]);
        }
        else if (performers[i].type == 'p' || performers[i].type == 'g')
        {
            pthread_create(&(performers[i].performer_thread_id), NULL, electrical_thread, &performers[i]);
            pthread_create(&(performers[i].performer_thread_id2), NULL, acoustic_thread, &performers[i]);
        }
        else if (performers[i].type == 'v')
        {
            pthread_create(&(performers[i].performer_thread_id), NULL, acoustic_thread, &performers[i]);
        }
        else if (performers[i].type == 'b')
        {
            pthread_create(&(performers[i].performer_thread_id), NULL, electrical_thread, &performers[i]);
        }
    }
    for (int i = 0; i < k; i++)
    {
        sem_wait(&(thread_join_wait));
    }
    printf("\n%sSimulation over!\n%s", GRN, NRM);
    return 0;
}

void *electrical_thread(void *args)
{
    Performer *musician = (Performer *)args;
    sleep(musician->time_of_arrival);
    pthread_mutex_lock(&(musician->performer_mutex));
    if (musician->has_arrived == 0)
    {
        printf("%s%s who plays %c has arrived\n", CYN, musician->name, musician->type);
        musician->has_arrived = 1;
    }
    pthread_mutex_unlock(&(musician->performer_mutex));
    while (musician->waiting_time)
    {
        clock_t time_req;
        struct timespec ts;
        if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
            return NULL;
        ts.tv_sec += musician->waiting_time;
        time_req = clock();
        if (sem_timedwait(&sem_stage_e, &ts) != 0)
        {
            pthread_mutex_lock(&(musician->performer_mutex));
            if (musician->type == 'p' || musician->type == 'g')
            {
                if (musician->got_stage == 0 && musician->wait_print == 0)
                {
                    printf("%s%s playing instrument %c has left due to impatience\n", RED, musician->name, musician->type);
                    musician->wait_print = 1;
                    pthread_mutex_unlock(&(musician->performer_mutex));
                    sem_post(&(thread_join_wait));
                    return NULL;
                }
                else
                {
                    pthread_mutex_unlock(&(musician->performer_mutex));
                    return NULL;
                }
            }
            else
            {
                pthread_mutex_unlock(&(musician->performer_mutex));
                printf("%s%s playing instrument %c has left due to impatience\n", RED, musician->name, musician->type);
                sem_post(&(thread_join_wait));
                return NULL;
            }
        }
        pthread_mutex_lock(&(musician->performer_mutex));
        if (musician->got_stage == 0 && musician->wait_print == 0)
        {
            musician->got_stage = 1;
        }
        else
        {
            sem_post(&(sem_stage_e));
            pthread_mutex_unlock(&(musician->performer_mutex));
            return NULL;
        }
        pthread_mutex_unlock(&(musician->performer_mutex));
        for (int i = 0; i < a + e; i++)
        {
            pthread_mutex_lock(&(stage[i].stage_mutex));
            if (stage[i].has_musician == 0 && stage[i].has_singer == 0 && stage[i].type == 'e')
            {
                stage[i].has_musician = 1;
                strcpy(stage[i].name_musician, musician->name);
                pthread_mutex_unlock(&(stage[i].stage_mutex));
                int mod = t2 - t1 + 1;
                int duration = rand() % mod + t1;
                printf("%s%s has started playing on stage with type %c having id %d on instrument %c for %d seconds\n", YEL, musician->name, stage[i].type, stage[i].idx, musician->type, duration);
                sleep(duration);
                pthread_mutex_lock(&(stage[i].stage_mutex));
                if (stage[i].has_singer == 1)
                {
                    pthread_mutex_unlock(&(stage[i].stage_mutex));
                    sleep(2);
                    pthread_cond_signal(&(performers[stage[i].singer_id - 1].cv_singer));
                }
                else
                {
                    pthread_mutex_unlock(&(stage[i].stage_mutex));
                }

                pthread_mutex_unlock(&(stage[i].stage_mutex));
                printf("%s%s has ended his performance on stage having type %c and id %d\n", GRN, musician->name, stage[i].type, stage[i].idx);
                pthread_mutex_lock(&(stage[i].stage_mutex));
                stage[i].has_musician = 0;
                strcpy(stage[i].name_musician, "\0");
                stage[i].has_singer = 0;
                pthread_mutex_unlock(&(stage[i].stage_mutex));
                sem_post(&(sem_stage_e));
                get_shirts(musician->name);
                sem_post(&(thread_join_wait));
                return NULL;
            }
            pthread_mutex_unlock(&(stage[i].stage_mutex));
        }
        time_req = clock() - time_req;
        musician->waiting_time -= time_req / CLOCKS_PER_SEC;
        sem_post(&(sem_stage_e));
    }
    pthread_mutex_lock(&(musician->performer_mutex));
    if (musician->type == 'p' || musician->type == 'g')
    {
        if (musician->wait_print == 0)
        {
            printf("%s%s playing instrument %c has left due to impatience\n", RED, musician->name, musician->type);
            musician->wait_print = 1;
            pthread_mutex_unlock(&(musician->performer_mutex));
            sem_post(&(thread_join_wait));
            return NULL;
        }
        else
        {
            pthread_mutex_unlock(&(musician->performer_mutex));
            return NULL;
        }
    }
    else
    {
        pthread_mutex_unlock(&(musician->performer_mutex));
        printf("%s%s playing instrument %c has left due to impatience\n", RED, musician->name, musician->type);
        sem_post(&(thread_join_wait));
        return NULL;
    }
    pthread_mutex_unlock(&(musician->performer_mutex));
}

void *acoustic_thread(void *args)
{
    Performer *musician = (Performer *)args;
    sleep(musician->time_of_arrival);
    pthread_mutex_lock(&(musician->performer_mutex));
    if (musician->has_arrived == 0)
    {
        printf("%s%s who plays %c has arrived\n", CYN, musician->name, musician->type);
        musician->has_arrived = 1;
    }
    pthread_mutex_unlock(&(musician->performer_mutex));
    while (musician->waiting_time > 0)
    {
        clock_t time_req;
        struct timespec ts;
        if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
            return NULL;
        ts.tv_sec += musician->waiting_time;
        time_req = clock();
        if (sem_timedwait(&sem_stage_a, &ts) != 0)
        {
            pthread_mutex_lock(&(musician->performer_mutex));
            if (musician->type == 'p' || musician->type == 'g')
            {
                if (musician->got_stage == 0 && musician->wait_print == 0)
                {
                    printf("%s%s playing instrument %c has left due to impatience\n", RED, musician->name, musician->type);
                    musician->wait_print = 1;
                    pthread_mutex_unlock(&(musician->performer_mutex));
                    sem_post(&(thread_join_wait));
                    return NULL;
                }
                else
                {
                    pthread_mutex_unlock(&(musician->performer_mutex));
                    return NULL;
                }
            }
            else
            {
                pthread_mutex_unlock(&(musician->performer_mutex));
                printf("%s%s playing instrument %c has left due to impatience\n", RED, musician->name, musician->type);
                sem_post(&(thread_join_wait));
                return NULL;
            }
        }
        pthread_mutex_lock(&(musician->performer_mutex));
        if (musician->got_stage == 0 && musician->wait_print == 0)
        {
            musician->got_stage = 1;
        }
        else
        {
            sem_post(&(sem_stage_a));
            pthread_mutex_unlock(&(musician->performer_mutex));
            return NULL;
        }
        pthread_mutex_unlock(&(musician->performer_mutex));
        for (int i = 0; i < a + e; i++)
        {
            pthread_mutex_lock(&(stage[i].stage_mutex));
            if (stage[i].has_musician == 0 && stage[i].has_singer == 0 && stage[i].type == 'a')
            {
                stage[i].has_musician = 1;
                strcpy(stage[i].name_musician, musician->name);
                pthread_mutex_unlock(&(stage[i].stage_mutex));
                int mod = t2 - t1 + 1;
                int duration = rand() % mod + t1;
                printf("%s%s has started playing on stage with type %c having id %d on instrument %c for %d seconds\n", YEL, musician->name, stage[i].type, stage[i].idx, musician->type, duration);
                sleep(duration);
                pthread_mutex_lock(&(stage[i].stage_mutex));
                if (stage[i].has_singer == 1)
                {
                    pthread_mutex_unlock(&(stage[i].stage_mutex));
                    sleep(2);
                    pthread_cond_signal(&(performers[stage[i].singer_id - 1].cv_singer));
                }
                else
                {
                    pthread_mutex_unlock(&(stage[i].stage_mutex));
                }

                printf("%s%s has ended his performance on stage having type %c and id %d\n", GRN, musician->name, stage[i].type, stage[i].idx);
                pthread_mutex_lock(&(stage[i].stage_mutex));
                stage[i].has_musician = 0;
                strcpy(stage[i].name_musician, "\0");
                stage[i].has_singer = 0;
                pthread_mutex_unlock(&(stage[i].stage_mutex));
                sem_post(&(sem_stage_a));
                get_shirts(musician->name);
                sem_post(&(thread_join_wait));
                return NULL;
            }
            pthread_mutex_unlock(&(stage[i].stage_mutex));
        }
        time_req = clock() - time_req;
        musician->waiting_time -= time_req / CLOCKS_PER_SEC;
        sem_post(&(sem_stage_e));
    }
    pthread_mutex_lock(&(musician->performer_mutex));
    if (musician->type == 'p' || musician->type == 'g')
    {
        if (musician->wait_print == 0)
        {
            printf("%s%s playing instrument %c has left due to impatience\n", RED, musician->name, musician->type);
            musician->wait_print = 1;
            pthread_mutex_unlock(&(musician->performer_mutex));
            sem_post(&(thread_join_wait));
            return NULL;
        }
        else
        {
            pthread_mutex_unlock(&(musician->performer_mutex));
            return NULL;
        }
    }
    else
    {
        pthread_mutex_unlock(&(musician->performer_mutex));
        printf("%s%s playing instrument %c has left due to impatience\n", RED, musician->name, musician->type);
        sem_post(&(thread_join_wait));
        return NULL;
    }
    pthread_mutex_unlock(&(musician->performer_mutex));
}

void *singer_thread(void *args)
{
    Performer *singer = (Performer *)args;
    sleep(singer->time_of_arrival);
    printf("%s%s who sings has arrived\n", CYN, singer->name);
    while (singer->waiting_time > 0)
    {
        clock_t time_req;
        struct timespec ts;
        if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
            return NULL;
        ts.tv_sec += singer->waiting_time;
        time_req = clock();
        if (sem_timedwait(&sem_stage_singer, &ts) != 0)
        {
            printf("%s%s who wanted to sing %c has left due to impatience\n", RED, singer->name, singer->type);
            sem_post(&(thread_join_wait));
            return NULL;
        }
        if (sem_trywait(&(sem_stage_a)) == 0)
        {
            for (int i = 0; i < a + e; i++)
            {
                pthread_mutex_lock(&(stage[i].stage_mutex));
                if (stage[i].has_singer == 0 && stage[i].type == 'a')
                {
                    if (stage[i].has_musician == 0)
                    {
                        stage[i].has_singer = 1;
                        pthread_mutex_unlock(&(stage[i].stage_mutex));
                        int mod = t2 - t1 + 1;
                        int duration = rand() % mod + t1;
                        printf("%s%s has started singing on stage with type %c with id %d for %d seconds\n", YEL, singer->name, stage[i].type, stage[i].idx, duration);
                        sleep(duration);
                        printf("%s%s has finished singing his/her solo performance\n", WHT, singer->name);
                        pthread_mutex_lock(&(stage[i].stage_mutex));
                        stage[i].has_singer = 0;
                        pthread_mutex_unlock(&(stage[i].stage_mutex));
                        sem_post(&(sem_stage_a));
                        sem_post(&(sem_stage_singer));
                        get_shirts(singer->name);
                        sem_post(&(thread_join_wait));
                        return NULL;
                    }
                    else
                    {
                        sem_post(&(sem_stage_a));
                        stage[i].has_singer = 1;
                        stage[i].singer_id = singer->idx;
                        pthread_mutex_unlock(&(stage[i].stage_mutex));
                        printf("%s%s has started singing on stage with type %c along with %s on stage id %d, performance extended by 2sec\n", YEL, singer->name, stage[i].type, stage[i].name_musician, stage[i].idx);
                        pthread_mutex_lock(&(singer->performer_mutex));
                        pthread_cond_wait(&(singer->cv_singer), &(singer->performer_mutex));
                        pthread_mutex_unlock(&(singer->performer_mutex));
                        printf("%s%s has finished singing on stage %d along with another musician\n", GRN, singer->name, stage[i].idx);
                        sem_post(&(sem_stage_singer));
                        get_shirts(singer->name);
                        sem_post(&(thread_join_wait));
                        return NULL;
                    }
                }
                pthread_mutex_unlock(&(stage[i].stage_mutex));
            }
        }
        else
        {
            if (sem_trywait(&(sem_stage_e)) == 0)
            {
                for (int i = 0; i < a + e; i++)
                {
                    pthread_mutex_lock(&(stage[i].stage_mutex));
                    if (stage[i].has_singer == 0 && stage[i].type == 'e')
                    {
                        if (stage[i].has_musician == 0)
                        {
                            stage[i].has_singer = 1;
                            pthread_mutex_unlock(&(stage[i].stage_mutex));
                            int mod = t2 - t1 + 1;
                            int duration = rand() % mod + t1;
                            printf("%s%s has started singing on stage with type %c with id %d for %d seconds\n", YEL, singer->name, stage[i].type, stage[i].idx, duration);
                            sleep(duration);
                            printf("%s%s has finished singing his/her solo performance\n", WHT, singer->name);
                            pthread_mutex_lock(&(stage[i].stage_mutex));
                            stage[i].has_singer = 0;
                            pthread_mutex_unlock(&(stage[i].stage_mutex));
                            sem_post(&(sem_stage_e));
                            sem_post(&(sem_stage_singer));
                            get_shirts(singer->name);
                            sem_post(&(thread_join_wait));
                            return NULL;
                        }
                        else
                        {
                            sem_post(&(sem_stage_e));
                            stage[i].has_singer = 1;
                            stage[i].singer_id = singer->idx;
                            pthread_mutex_unlock(&(stage[i].stage_mutex));
                            printf("%s%s has started singing on stage with type %c along with %s on stage id %d, performance extended by 2sec\n", YEL, singer->name, stage[i].type, stage[i].name_musician, stage[i].idx);
                            pthread_mutex_lock(&(singer->performer_mutex));
                            pthread_cond_wait(&(singer->cv_singer), &(singer->performer_mutex));
                            pthread_mutex_unlock(&(singer->performer_mutex));
                            printf("%s%s has finished singing on stage %d along with another musician\n", GRN, singer->name, stage[i].idx);
                            sem_post(&(sem_stage_singer));
                            get_shirts(singer->name);
                            sem_post(&(thread_join_wait));
                            return NULL;
                        }
                    }
                    pthread_mutex_unlock(&(stage[i].stage_mutex));
                }
            }
            else
            {
                for (int i = 0; i < a + e; i++)
                {
                    pthread_mutex_lock(&(stage[i].stage_mutex));
                    if (stage[i].has_singer == 0 && stage[i].has_musician == 1)
                    {
                        stage[i].has_singer = 1;
                        stage[i].singer_id = singer->idx;
                        pthread_mutex_unlock(&(stage[i].stage_mutex));
                        printf("%s%s has started singing on stage with type %c along with %s on stage id %d, performance extended by 2sec\n", YEL, singer->name, stage[i].type, stage[i].name_musician, stage[i].idx);
                        pthread_mutex_lock(&(singer->performer_mutex));
                        pthread_cond_wait(&(singer->cv_singer), &(singer->performer_mutex));
                        pthread_mutex_unlock(&(singer->performer_mutex));
                        printf("%s%s has finished singing on stage %d along with another musician.\n", GRN, singer->name, stage[i].idx);
                        sem_post(&(sem_stage_singer));
                        get_shirts(singer->name);
                        sem_post(&(thread_join_wait));
                        return NULL;
                    }
                    pthread_mutex_unlock(&(stage[i].stage_mutex));
                }
            }
        }
        time_req = clock() - time_req;
        singer->waiting_time -= (double)time_req / CLOCKS_PER_SEC;
        sem_post(&(sem_stage_singer));
    }
}

void get_shirts(char *name)
{
    sem_wait(&sem_coordinators);
    printf("%s%s is waiting for his T-shirt\n", MAG, name);
    sleep(2);
    printf("%s%s got his T-shirt\n", BLU, name);
    sem_post(&sem_coordinators);
}