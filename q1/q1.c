#define _POSIX_C_SOURCE 199309L //required for clock
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <limits.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <inttypes.h>
#include <math.h>

void swap(int *a, int *b)
{
    int t = *a;
    *a = *b;
    *b = t;
}

int *shareMem(size_t size)
{
    key_t mem_key = IPC_PRIVATE;
    int shm_id = shmget(mem_key, size, IPC_CREAT | 0666);
    return (int *)shmat(shm_id, NULL, 0);
}

void selectionSort(int arr[], int low, int high)
{
    int i, j, min_idx;

    for (i = low; i < high + 1; i++)
    {
        min_idx = i;
        for (j = i + 1; j < high + 1; j++)
            if (arr[j] < arr[min_idx])
                min_idx = j;

        swap(&arr[min_idx], &arr[i]);
    }
}

void merge(int *arr, int low, int mid, int high)
{
    // int mid = (low + high) / 2;
    int n1 = mid - low + 1;
    int n2 = high - mid;

    int L[n1], R[n2];
    for (int i = 0; i < n1; i++)
        L[i] = arr[low + i];
    for (int i = 0; i < n2; i++)
        R[i] = arr[mid + 1 + i];
    int i = 0, j = 0, k = low;
    while (i < n1 && j < n2)
    {
        if (L[i] <= R[j])
        {
            arr[k] = L[i];
            i++;
        }
        else
        {
            arr[k] = R[j];
            j++;
        }
        k++;
    }
    while (i < n1)
    {
        arr[k] = L[i];
        k++;
        i++;
    }
    while (j < n2)
    {
        arr[k] = R[j];
        k++;
        j++;
    }
}

void normal_mergesort(int *arr, int low, int high)
{
    if (low < high && high - low + 1 < 5)
    {
        selectionSort(arr, low, high);
        return;
    }
    if (low < high)
    {
        /* pi is partitioning index, arr[p] is now
          at right place */
        int mid = (low + high) / 2;

        // Separately sort elements before
        // partition and after partition
        normal_mergesort(arr, low, mid);
        normal_mergesort(arr, mid + 1, high);
        merge(arr, low, mid, high);
    }
}

void mergesort(int *arr, int low, int high)
{
    if (low < high && high - low + 1 < 5)
    {
        selectionSort(arr, low, high);
        return;
    }
    if (low < high)
    {
        /* pi is partitioning index, arr[p] is now
          at right place */
        int mid = (low + high) / 2;
        int pid1 = fork();
        int pid2;
        if (pid1 == 0)
        {
            mergesort(arr, low, mid);
            _exit(1);
        }
        else
        {
            pid2 = fork();
            if (pid2 == 0)
            {
                mergesort(arr, mid + 1, high);
                _exit(1);
            }
            else
            {
                int status;
                waitpid(pid1, &status, 0);
                waitpid(pid2, &status, 0);
                merge(arr, low, mid, high);
            }
        }
        return;
        // Separately sort elements before
        // partition and after partition
    }
}

struct arg
{
    int l;
    int r;
    int *arr;
};

void *threaded_mergesort(void *a)
{
    //note that we are passing a struct to the threads for simplicity.
    struct arg *args = (struct arg *)a;

    int l = args->l;
    int r = args->r;
    int *arr = args->arr;
    if (r - l + 1 < 5)
    {
        selectionSort(arr, l, r);
        return NULL;
    }

    int ind = (l + r) / 2;
    //sort left half array
    struct arg a1;
    a1.l = l;
    a1.r = ind;
    a1.arr = arr;
    pthread_t tid1;
    if (pthread_create(&tid1, NULL, threaded_mergesort, &a1) != 0)
    {
        printf("Error");
        exit(0);
    }

    //sort right half array
    struct arg a2;
    a2.l = ind + 1;
    a2.r = r;
    a2.arr = arr;
    pthread_t tid2;
    if (pthread_create(&tid2, NULL, threaded_mergesort, &a2) != 0)
    {
        printf("Error");
        exit(0);
    }
    //wait for the two halves to get sorted
    if (pthread_join(tid1, NULL) != 0)
    {
        printf("Error");
        exit(0);
    }
    if (pthread_join(tid2, NULL) != 0)
    {
        printf("Error");
        exit(0);
    }
    merge(arr, l, ind, r);
    return NULL;
}

void runSorts(long long int n)
{

    struct timespec ts;

    //getting shared memory
    int *arr = shareMem(sizeof(int) * (n + 1));
    for (int i = 0; i < n; i++)
        scanf("%d", arr + i);

    int brr[n + 1], crr[n + 1];
    for (int i = 0; i < n; i++)
    {
        brr[i] = arr[i];
        crr[i] = arr[i];
    }
    printf("Running concurrent_mergesort for n = %lld\n", n);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    long double st = ts.tv_nsec / (1e9) + ts.tv_sec;

    // multiprocess mergesort
    mergesort(arr, 0, n - 1);
    for (int i = 0; i < n; i++)
    {
        printf("%d ", arr[i]);
    }
    printf("\n");
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    long double en = ts.tv_nsec / (1e9) + ts.tv_sec;
    printf("time = %Lf\n", en - st);
    long double t1 = en - st;
    printf("\n");

    printf("Running normal_mergesort for n = %lld\n", n);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    st = ts.tv_nsec / (1e9) + ts.tv_sec;

    // normal mergesort
    normal_mergesort(crr, 0, n - 1);
    for (int i = 0; i < n; i++)
    {
        printf("%d ", crr[i]);
    }
    printf("\n");
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    en = ts.tv_nsec / (1e9) + ts.tv_sec;
    printf("time = %Lf\n", en - st);
    long double t3 = en - st;
    printf("\n");

    pthread_t tid;
    struct arg a;
    a.l = 0;
    a.r = n - 1;
    a.arr = brr;

    printf("Running threaded_concurrent_mergesort for n = %lld\n", n);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    st = ts.tv_nsec / (1e9) + ts.tv_sec;
    //multithreaded mergesort
    pthread_create(&tid, NULL, threaded_mergesort, &a);
    pthread_join(tid, NULL);
    for (int i = 0; i < n; i++)
    {
        printf("%d ", brr[i]);
    }
    printf("\n");
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    en = ts.tv_nsec / (1e9) + ts.tv_sec;
    printf("time = %Lf\n", en - st);
    long double t2 = en - st;

    printf("normal_mergesort ran:\n\t[ %Lf ] times faster than concurrent_mergesort\n\t[ %Lf ] times faster than threaded_concurrent_mergesort\n\n\n", t1 / t3, t2 / t3);
    shmdt(arr);
    return;
}

int main()
{
    long long int n;
    scanf("%lld", &n);
    runSorts(n);
    return 0;
}
