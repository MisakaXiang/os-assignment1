#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define LEFT(i) ((i) + 5 - 1) % 5 //define the left philosopher
#define RIGHT(i) ((i) + 1) % 5 //define the right philosopher

/* the state of philosophers */
enum { THINKING, HUNGRY, EATING } state[5] = {};
pthread_mutex_t mutex;
pthread_cond_t waiting[5];
int identity[5] = {0, 1, 2, 3, 4};

void can_eat(int i)
{
    pthread_mutex_lock(&mutex);
    if ((state[LEFT(i)] != EATING) && (state[i] == HUNGRY) &&
        (state[RIGHT(i)] != EATING))
    {
        state[i] = EATING;
        pthread_cond_signal(&waiting[i]);
    }
    pthread_mutex_unlock(&mutex);
}

void pickup_forks(int philosopher_number)
{
    pthread_mutex_lock(&mutex);
    state[philosopher_number] = HUNGRY;
    pthread_mutex_unlock(&mutex);

    can_eat(philosopher_number);

    pthread_mutex_lock(&mutex);
    if (state[philosopher_number] != EATING)
        pthread_cond_wait(&waiting[philosopher_number], &mutex);
    pthread_mutex_unlock(&mutex);
}

void return_forks(int philosopher_number)
{
    pthread_mutex_lock(&mutex);
    /* stop eating, start thinking */
    state[philosopher_number] = THINKING;
    pthread_mutex_unlock(&mutex);

    // test if neighbours can take forks
    can_eat(LEFT(philosopher_number));
    can_eat(RIGHT(philosopher_number));
}

void *philosopher(void *param)
{
    int id = *(int *)param;
    
    while (1)
    {
        int think_time = rand() % 3 + 1;
        sleep(think_time);
        pickup_forks(id);
        printf("Philospher %d is eating\n", id);
        int eat_time = rand() % 3 + 1;
        sleep(eat_time);
        return_forks(id);
        printf("Philospher %d is thinking\n", id);
    }
}

int main()
{
    pthread_t philosophers[5];
    
    // initialize the condition variable and its associated mutex lock
    pthread_mutex_init(&mutex, NULL);
    for (int i = 0; i < 5; i++)
        pthread_cond_init(&waiting[i], NULL);
    
    // create five philosophers
    for (int i = 0; i < 5; i++)
    {
        pthread_create(&philosophers[i], NULL, philosopher, &identity[i]);
        state[i] = THINKING;
    }
    
    for (int i = 0; i < 5; i++)
        pthread_join(philosophers[i], NULL);
    
}

