# Question 1

---

### NOTE: In all the testcases normal mergesort seems to be working faster than both curcurrent and threaded process

#### At n = 1,

Concurrent processes run faster than threaded processes.  
Running concurrent_mergesort for n = 1  
time = 0.000055

Running normal_mergesort for n = 1  
time = 0.000005

Running threaded_concurrent_mergesort for n = 1  
time = 0.000178

normal_mergesort ran:  
[ 10.948087 ] times faster than concurrent_mergesort  
[ 35.214182 ] times faster than threaded_concurrent_mergesort

#### At n = 10,

Concurrent processes run faster than threaded processes.  
normal_mergesort ran:  
Running concurrent_mergesort for n = 10  
time = 0.001624

Running normal_mergesort for n = 10  
time = 0.000008

Running threaded_concurrent_mergesort for n = 10  
time = 0.003186

normal_mergesort ran:  
[ 207.135213 ] times faster than concurrent_mergesort  
[ 406.483718 ] times faster than threaded_concurrent_mergesort

#### At n = 100,

1. Generally, Concurrent processes run faster than threaded processes.  
   Running concurrent_mergesort for n = 100  
   time = 0.010144  
   Running normal_mergesort for n = 100  
   time = 0.000029  
   Running threaded_concurrent_mergesort for n = 100  
   time = 0.005357  
   normal_mergesort ran:
   [ 344.164017 ] times faster than concurrent_mergesort  
   [ 181.744027 ] times faster than threaded_concurrent_mergesort

2. but sometimes Concurrent processes run slower than threaded processes.  
   normal_mergesort ran:  
   Running concurrent_mergesort for n = 100  
   time = 0.007713  
   Running normal_mergesort for n = 100  
   time = 0.000306  
   Running threaded_concurrent_mergesort for n = 100  
   time = 0.005820  
   normal_mergesort ran:  
   [ 25.202914 ] times faster than concurrent_mergesort  
   [ 19.017801 ] times faster than threaded_concurrent_mergesort

#### At n = 1000,

Concurrent process are always slower than threaded processes.  
Running concurrent_mergesort for n = 1000  
time = 0.046171  
Running normal_mergesort for n = 1000  
time = 0.000365  
Running threaded_concurrent_mergesort for n = 1000  
time = 0.040994  
normal_mergesort ran:  
[ 126.376654 ] times faster than concurrent_mergesort  
[ 112.206503 ] times faster than threaded_concurrent_mergesort

#### At n = 10000,

Concurrent process are always slower than threaded processes.  
Running concurrent_mergesort for n = 10000  
time = 0.796135  
Running normal_mergesort for n = 10000  
time = 0.025153  
Running threaded_concurrent_mergesort for n = 10000  
time = 0.260863  
normal_mergesort ran:  
[ 31.652039 ] times faster than concurrent_mergesort  
[ 10.371163 ] times faster than threaded_concurrent_mergesort

#### At n = 100000,

Concurrent process are always slower than threaded processes.  
Running concurrent_mergesort for n = 100000  
time = 11.344498  
Running normal_mergesort for n = 100000  
time = 0.063479  
Running threaded_concurrent_mergesort for n = 100000  
time = 2.524578  
normal_mergesort ran:  
[ 178.711360 ] times faster than concurrent_mergesort  
[ 39.770007 ] times faster than threaded_concurrent_mergesort

**Conclusion: At all times normal mergesort is the fastest but after n = 1000 numbers, threaded mergesort overtakes concurrent mergesort. This might be because the overhead of context switching in concurrent process becomes higher than the overhead of creation of threads. Also normal mergesort is always better than concurrent processes because where the left child access the left array, the array is loaded into the cache of a processor. Now when the right array is accessed there is a cache miss since the cache is filled with left segment and the right segment is copied to the cache memory. This to-and-fro process continues and it degrades the performance to such a level that it performes poorer than the normal mergesort code.**
