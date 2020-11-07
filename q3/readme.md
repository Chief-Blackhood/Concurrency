### int main()

1. This is where the simulation begins and ends.
2. It first takes in the input of number of performers, acoustic stages,​ electric stages, coordinators, minimum performance time, maximum performance time and waiting time. It then check if all the values are vaild. Then it takes the the information of each performer.
3. Then it creates threads and calls the functions according to which kind of preformer it is.
4. Assignment of functions to threads:

   - If the performer is only acoustic(electric) musician then only acoustic(electric) function is called.
   - If the performer can play both acoustic/electric then two threads are created each calling there respective functions.
   - If the performer is a singer, singer function is called only.

5. Then the main function waits on the semaphore `thread_join_wait` until all the performers have either performerd or left due to impatience.

### electrical_thread()

1. All the musicians that can perform on electric stage are sent here.
2. First the musician calls `sleep()` function to simulate there corresponding arrival time.
3. It then prints that it has arrived. There is a need for mutex lock for musicians that can perform on both electrical and acoustic stages so that the print statement is executed in only on of the functions for this particular performer.
4. Then the musician does a timed wait on the semaphore `sem_stage_e` which represents the number of available electric stages. If the wait is unsuccessful then it prints that it left due to impatience. This also requires a mutex lock for musicians that can perform on both electrical and acoustic stages so that the print statement is executed in only on of the function for this particular performer.
5. If the timed wait is successful:

   - then it checks if the variable `got_stage` is equal zero and `wait_print` is equal zero
     - If so then set `got_stage` equal to 1.
   - otherwise it returns the stage and leaves the function.This is done so that the performer that can play on electrical and acoustic doesn't acquire two stages simultaneously.
   - For the performers that can only play on one stage this part has not effect.

6. Then it loops over all the stages and checks for a compatible one. It then sleeps for a variable time between (t1, t2) to simulate performance time.
7. It then checks if a singer has also joined him in the meanwhile,
   - If so then sleep for another 2 seconds and then signal the singer too to leave the stage.
   - otherwise just leave the stage.
8. It then calls get_shirts to wait for the shirt if it has performed.

### acoustic_thread()

This function is very similar to the electric_thread one with the difference of only waiting on the semaphore which represents number of free acoustic stages.

### singer_thread()

1. All the singers are sent here.
2. First the singer calls `sleep()` function to simulate there corresponding arrival time.
3. Then the singer does a timed wait on the semaphore `sem_stage_singer` and checks if any stage is available for a singer to enter. If the wait is unsuccessful then the singer leaves due to impatience.
4. It then try waits on the semaphore `sem_stage_a` just to check if it can get a solo stage. If the wait is successful then a loop is done over all the stages
   - if there a solo stage then the singer acquires that stage and sleeps to simulate performance time.
   - else if there is a stage with only musician then it posts the semaphore `sem_stage_a` and joins the musician for a duet. Here the singer conditionally waits so that the musician can finish his performance.(Note here the performance of the muscian is extended by 2 seconds by setting the variable `has_singer` equal to 1 in that particular stage using mutex locks)
5. If the wait is unsuccessful then a similar try wait is done on semaphore `sem_stage_e`.
6. If this wait fails as well then we know that there are not solo stages present for the singer and he must perform in a duet. Thus now the singer loops over all the stages which are occupied by a musician but not a singer and join him/her in a duet by conditionally waiting for the musician to complete his performance. (Note here the performance of the muscian is extended by 2 seconds by setting the variable `has_singer` equal to 1 in that particular stage using mutex locks)
7. After his/her solo/duet performance the singer calls the get_shirts function to get a shirt and then leaves.
   (Note there is a small possibily that both the above try wait fails i.e. singer doesn't find a solo stage but while looping over the stages the musician who had occuiped those stages leave and the singer can't join any musican for a duet. Here, we check if the waiting time of singer is still greater than zero then the above procedure is repeated until the siger gets a stage or the waiting time of the singer elapses)
