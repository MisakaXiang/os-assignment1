#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <pthread.h>
#include <math.h>
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
double lamda;

struct shared_use_st
{
    int Index[20];  //为0表示对应的缓冲区未被生产者使用，可分配但不可消费；为1表示对应的缓冲区被生产者使用，不可分配但可消费
    int Buffer[20];
    sem_t sem;  //信号量
};

double expntl(double x)
{
    double z;
    srand(time(NULL));
    do
    {
        z=((double)rand() / RAND_MAX);
    }while((z == 0)||(z == 1));
    return(-x*log(z));
}

void *consumer(void* param)
{
    struct shared_use_st *shared = param;
    int i = 0;
    while(1)
    {
        if(sem_wait(&(shared->sem)) == -1)  //sem_wait为P操作
        {
            exit(EXIT_FAILURE);
        }
        pthread_mutex_lock(&mutex);
        
        for(i = 0; i < 20 && shared->Index[i] == 0; i++);
        
        if(i != 20)
        {
            printf("consumer %lu recieve %d\n", pthread_self(), shared->Buffer[i%20]);
            shared->Index[i % 20] = 0;  //为0时，表示已被消费者使用
            pthread_mutex_unlock(&mutex);
            sem_post(&shared->sem);  //sem_post为V操作
            usleep(expntl(1000000 * (1.0/lamda)));
        }
        else
        {
            pthread_mutex_unlock(&mutex);
            sem_post(&shared->sem);
            printf("waiting\n");
            usleep(expntl(1000000 * (1.0/lamda)));
        }
    }
}

int main(int argc, char const *argv[])
{
    void *shm = NULL;  //共享存储段连接的实际地址
    struct shared_use_st *shared = NULL;
    int shmid;  //声明共享内存标识符
    
    shmid = shmget((key_t)1121, sizeof(struct shared_use_st), 0666|IPC_CREAT);  //获得或创建一个共享内存标识符
    if(shmid == -1)
    {
        exit(EXIT_FAILURE);
    }
    
    shm = shmat(shmid, 0, 0);  //返回共享存储段连接的实际地址
    if(shm == (void*)-1)
    {
        exit(EXIT_FAILURE);
    }
    printf("Memory attached at %ld\n", (intptr_t)shm);
    shared = (struct shared_use_st*)shm;    
    
    if(argc < 2)
    {
        fprintf(stderr, "please input lamda\n");
        return -1;
    }
    
    sscanf( argv[1],"%lf",&lamda);
    if(lamda <= 0 )
    {
        fprintf(stderr, "please input lamda(>0)\n");
        return -1;
    }
    
    pthread_mutex_init(&mutex, NULL);
    
    pthread_t consume[3];
    pthread_attr_t attr[3];
    for (int i = 0;i < 3;i++)
    {
        pthread_attr_init(&attr[i]);
        pthread_create(&consume[i], &attr[i], consumer, shared);
    }
    for (int i = 0;i < 3;i++)
    {
        pthread_join(consume[i], NULL);
    }

    if(shmdt(shm) == -1)
    {
        exit(EXIT_FAILURE);
    }
    
    if(shmctl(shmid, IPC_RMID, 0) == -1)
    {
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}

