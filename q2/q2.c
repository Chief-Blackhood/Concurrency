#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>
#include <math.h>

#define NRM "\x1B[1;0m"
#define RED "\x1B[1;31m"
#define GRN "\x1B[1;32m"
#define YEL "\x1B[1;33m"
#define BLU "\x1B[1;34m"
#define MAG "\x1B[1;35m"
#define CYN "\x1B[1;36m"
#define WHT "\x1B[1;37m"

typedef struct Pharmaceutical
{
    int idx;
    int batches;
    int capacity[5];
    int status;
    float probability;
    int total_vaccines;
    pthread_mutex_t pharmaceutical_mutex;
    pthread_t pharmaceutical_thread_id;
} Pharmaceutical;

typedef struct Zones
{
    int idx;
    int capacity;
    int initial_capacity;
    int slots;
    int occupied;
    int buying_pharmacy_id;
    float probability;
    pthread_mutex_t zones_mutex;
    pthread_t zones_thread_id;
} Zones;

typedef struct Students
{
    int idx;
    int round;
    int vacinated;
    int zone_id;
    pthread_t student_thread_id;
    pthread_mutex_t student_mutex;
} Students;

Pharmaceutical pharmaceutical[300];
Zones zones[300];
Students students[300];

int n_pharmaceutical, m_zones, o_students, waiting_students;
int minimum_batches = 1;
int maximum_batches = 5;
int minimum_preparation_time = 2;
int maximum_perparation_time = 5;

void *pharmaceutical_thread(void *args);
void *zone_thread(void *args);
void *student_thread(void *args);

int min(int a, int b)
{
    return (a > b) ? b : a;
}

int main()
{
    srand(time(0));
    printf("%sEnter number of Pharmaceutical Companies, zones, and students:\n", BLU);
    scanf("%d %d %d", &n_pharmaceutical, &m_zones, &o_students);
    waiting_students = o_students;
    for (int i = 0; i < n_pharmaceutical; i++)
        scanf("%f", &pharmaceutical[i].probability);

    if (n_pharmaceutical == 0 || m_zones == 0)
    {
        printf("%sThe students can't get vaccinated because either there are no vaccination zones or no pharmaceutical companies\n%s", BLU, NRM);
        return 0;
    }

    if (n_pharmaceutical > 300 || m_zones > 300 || o_students > 300)
    {
        printf("%sPharmaceutical companies, vaccination zones and students all can be atmost 300 each\n%s", BLU, NRM);
        return 0;
    }

    for (int i = 0; i < n_pharmaceutical; i++)
    {
        pharmaceutical[i].idx = i + 1;
        pthread_mutex_init(&(pharmaceutical[i].pharmaceutical_mutex), NULL);
    }

    for (int i = 0; i < m_zones; i++)
    {
        zones[i].idx = i + 1;
        pthread_mutex_init(&(zones[i].zones_mutex), NULL);
    }

    for (int i = 0; i < o_students; i++)
    {
        students[i].idx = i + 1;
        pthread_mutex_init(&(students[i].student_mutex), NULL);
    }

    printf("%sBeginning Simulation\n\n", GRN);

    for (int i = 0; i < n_pharmaceutical; i++)
        pthread_create(&(pharmaceutical[i].pharmaceutical_thread_id), NULL, pharmaceutical_thread, &pharmaceutical[i]);
    for (int i = 0; i < m_zones; i++)
        pthread_create(&(zones[i].zones_thread_id), NULL, zone_thread, &zones[i]);
    for (int i = 0; i < o_students; i++)
        pthread_create(&(students[i].student_thread_id), NULL, student_thread, &students[i]);
    for (int i = 0; i < o_students; i++)
        pthread_join(students[i].student_thread_id, 0);
    printf("\n%sSimulation over!\n", GRN);

    for (int i = 0; i < n_pharmaceutical; i++)
        pthread_mutex_destroy(&(pharmaceutical[i].pharmaceutical_mutex));

    for (int i = 0; i < m_zones; i++)
        pthread_mutex_destroy(&(zones[i].zones_mutex));

    return 0;
}

void *pharmaceutical_thread(void *args)
{
    Pharmaceutical *pharmaceutical = (Pharmaceutical *)args;
    while (1)
    {
        int r = rand() % (maximum_batches) + minimum_batches;
        printf("%sPharmaceutical Company %d is preparing %d batches of vaccines which have success probability %.3f\n", MAG, pharmaceutical->idx, r, pharmaceutical->probability);
        int w = rand() % (maximum_perparation_time - minimum_preparation_time + 1) + minimum_preparation_time;
        sleep(w);
        pthread_mutex_lock(&(pharmaceutical->pharmaceutical_mutex));
        pharmaceutical->batches = r;
        for (int i = 0; i < 5; i++)
        {
            if (i < r)
            {
                pharmaceutical->capacity[i] = rand() % 11 + 10;
                pharmaceutical->total_vaccines += pharmaceutical->capacity[i];
            }
            else
                pharmaceutical->capacity[i] = 0;
        }
        printf("%sPharmaceutical Companay %d has prepared %d batches of vaccines which have a success probability %.3f. Waiting for all vaccines to be used to resume production\n", BLU, pharmaceutical->idx, pharmaceutical->batches, pharmaceutical->probability);
        pthread_mutex_unlock(&(pharmaceutical->pharmaceutical_mutex));
        while (1)
        {
            if (pharmaceutical->total_vaccines == 0)
                break;

            // else
            //     pthread_cond_wait(&(Pharmaceutical->cv_zones), &(Pharmaceutical->Pharmaceutical_mutex));
        }
        printf("%sAll the vaccines prepared by Pharmaceutical Company %d are emptied. Resuming production now.\n", RED, pharmaceutical->idx);
    }

    return NULL;
}

void *zone_thread(void *args)
{
    Zones *zone = (Zones *)args;
    while (1)
    {
        int vaccine_available = 0;
        for (int i = 0; i < n_pharmaceutical; i++)
        {
            pthread_mutex_lock(&(pharmaceutical[i].pharmaceutical_mutex));
            if (pharmaceutical[i].batches > 0)
            {
                vaccine_available = 1;
                zone->capacity = pharmaceutical[i].capacity[pharmaceutical[i].batches - 1];
                zone->initial_capacity = zone->capacity;
                pharmaceutical[i].batches--;
                zone->buying_pharmacy_id = pharmaceutical[i].idx;
                zone->probability = pharmaceutical[i].probability;
                printf("%sPharmaceutical Company %d is delivering a vaccine batch with capacity %d to Vaccination Zone %d which has success probability %.3f\n", YEL, pharmaceutical[i].idx, zone->initial_capacity, zone->idx, pharmaceutical[i].probability);
                sleep(rand() % 3 + 1);
                printf("%sPharmaceutical Company %d has delivered vaccines to Vaccination zone %d, resuming vaccinations now\n", GRN, pharmaceutical[i].idx, zone->idx);
                //pthread_cond_signal(&(pharmaceutical[i].cv_zones));
                pthread_mutex_unlock(&(pharmaceutical[i].pharmaceutical_mutex));
                break;
            }
            //pthread_cond_signal(&(pharmaceutical[i].cv_zones));
            pthread_mutex_unlock(&(pharmaceutical[i].pharmaceutical_mutex));
        }

        while (vaccine_available)
        {
            pthread_mutex_lock(&(zone->zones_mutex));

            if (zone->capacity == 0)
            {
                printf("%sVaccination Zone %d has run out of vaccines\n", RED, zone->idx);
                pthread_mutex_lock(&(pharmaceutical[zone->buying_pharmacy_id - 1].pharmaceutical_mutex));
                pharmaceutical[zone->buying_pharmacy_id - 1].total_vaccines -= zone->initial_capacity;
                pthread_mutex_unlock(&(pharmaceutical[zone->buying_pharmacy_id - 1].pharmaceutical_mutex));
                pthread_mutex_unlock(&(zone->zones_mutex));
                break;
            }
            int slot_upper_limit = min(8, min(waiting_students, zone->capacity));
            while (slot_upper_limit == 0)
            {
                slot_upper_limit = min(8, min(waiting_students, zone->capacity));
            }
            zone->slots = rand() % slot_upper_limit + 1;
            zone->occupied = 0;

            printf("%sVaccination Zone %d is ready to vaccinate with %d slots\n", BLU, zone->idx, zone->slots);
            pthread_mutex_unlock(&(zone->zones_mutex));
            while (1)
            {
                if (zone->slots == zone->occupied || waiting_students == 0)
                {
                    if (zone->occupied == 0)
                    {
                        printf("%sNo student filled any slot in vaccination zone %d. Recalibrating number of slots\n", RED, zone->idx);
                        break;
                    }
                    else
                    {
                        pthread_mutex_lock(&(zone->zones_mutex));
                        printf("%sVaccination Zone %d entering Vaccination Phase\n", YEL, zone->idx);
                        zone->capacity -= zone->occupied;
                        pthread_mutex_unlock(&(zone->zones_mutex));
                        for (int i = 0; i < o_students; i++)
                        {
                            pthread_mutex_lock(&(students[i].student_mutex));
                            if (students[i].zone_id == zone->idx)
                            {
                                printf("%sStudent %d is getting vaccinated\n", BLU, students[i].idx);
                                sleep(1);
                                printf("%sStudent %d on Vaccination Zone %d has been vaccinated which has success probability %.3f\n", CYN, students[i].idx, zone->idx, zone->probability);
                                students[i].vacinated = 1;
                                students[i].zone_id = -1;
                            }
                            pthread_mutex_unlock(&(students[i].student_mutex));
                        }
                        break;
                    }
                }
            }
        }
    }
}

void *student_thread(void *args)
{
    Students *student = (Students *)args;
    int arrival_time = rand() % 5;
    sleep(arrival_time);
    student->round = 1;
    while (student->round <= 3)
    {
        printf("%sStudent %d has arrived for his %d round of Vaccination\n", YEL, student->idx, student->round);
        int got_slot = 0;
        while (!got_slot)
        {
            for (int i = 0; i < m_zones; i++)
            {
                pthread_mutex_lock(&(zones[i].zones_mutex));
                if (zones[i].slots - zones[i].occupied > 0)
                {
                    zones[i].occupied++;
                    got_slot = 1;
                    printf("%sStudent %d is waiting to be allocated a slot on Vaccination Zone %d\n", RED, student->idx, i + 1);
                    //sleep(1);
                    printf("%sStudent %d assigned a slot on the Vaccination Zone %d and waiting to be vaccinated\n", GRN, student->idx, i + 1);
                    waiting_students--;
                    student->zone_id = zones[i].idx;
                    pthread_mutex_unlock(&(zones[i].zones_mutex));
                    break;
                }
                pthread_mutex_unlock(&(zones[i].zones_mutex));
            }
        }

        if (got_slot)
        {
            int threshold = zones[student->zone_id - 1].probability * 100;
            while (student->vacinated == 0)
                ;
            int chance = rand() % 100 + 1;
            if (chance <= threshold)
            {
                printf("%sStudent %d has tested positive for antibodies.\n", BLU, student->idx);
                return NULL;
            }
            else
            {
                printf("%sStudent %d has tested negative for antibodies.\n", MAG, student->idx);
                student->round++;
                waiting_students++;
                student->vacinated = 0;
            }
        }
    }
    waiting_students--;
    return NULL;
}