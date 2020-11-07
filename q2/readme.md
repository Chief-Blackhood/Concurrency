### int main():

1. is the main function that askes for the input
2. checks if the input is valid
3. creates the mutex and threads for pharmaceutical, zones, students.
4. Begins the simulation by calling the corresponding functions.
5. Waits for the student threads to join.
6. Destorys the mutex for all the threads and exits the code.

### pharmaceutical_thread()

1. This function is called for the pharmaceutical threads.
2. First it create a random amount of batches(1-5) and sleeps to simulate the preparation time.
3. It then assigns capacity of each batch produced randomly between 10 and 20.
4. Then it start a while loop (busy waiting) for all its vaccines to be consumed by students before resuming to create the batches of vaccines again. The production of vaccines is halted if all the students have been processed.

### zone_thread()

1. This function is called for the zone threads.
2. It first checks if it has vaccines available, if not, it loops over all pharmaceutical companies to check if any one of them has batches ready. It continuously loops over the companies until it finds a suitable one having ready batches. This is done using mutex locks for pharmacy companies so that two zones don't aquire the same batch or make any concurrency issues.
3. After acquiring a batch it sets its variable `vaccine_available = 1;` and starts another loop which runs until all the vaccine present in the zone is over.
4. In this while loop it also creates slots which vary between `k belongs (1, min(8, min(waiting_students, zone->capacity)))` then it prints `Vaccination Zone <id> is ready to vaccinate with <number> slots`
5. Inside this loop,it enters another loop which keeps checking if the slots are full or the total number of waiting students for this round is zero. If any of the case satisfy then it check if there is any student which occupied any slot of the zone. If not then it recalibrates its slots otherwise it proceeds with the vaccination of the students who are waiting.
6. After it has exhausted all the vaccines present in the zone it goes back to the first loop.

### student_thread()

1. This function is called for the student threads.
2. First the student threads sleep for a variable amount of time to simulate different arrival times.
3. Student then loops over all zones and checks if any one of them has a slot availble. It continuously loop (busy waiting) until it finds a valid slot in a zone. It then changes the number of slots of the zone using a mutex lock.
4. After getting a slot it loop continuously to check if it has been vaccinated. After being vaccinated, a random variable `chance = rand() % 100 + 1;` is declared which serves as a simulation to check whether antibodies are created inside the student. If not, student's round number is incremented and it again goes to the start of the function to get another slot.(note if round number > 3 it just returns from the function).
5. If he gets a positive result of antibodies then he/she immediately returns from the function.
