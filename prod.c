#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include <pthread.h>
#include <math.h>
typedef int buffer_item;
double lamda;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

struct shared_use_st
{
    int Index[20];  //为0表示对应的缓冲区未被生产者使用，可分配但不可消费；为1表示对应的缓冲区已被生产者使用，不可分配但可消费
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

void *producer(void* param)
{
    struct shared_use_st *shared = param;
    buffer_item item;
    int i;
    
    while(1)
    {
        if(sem_wait(&(shared->sem)) == -1)
        {
            exit(EXIT_FAILURE);
        }
        
        pthread_mutex_lock(&mutex);
        
        srand(time(NULL)-pthread_self()*100);
        item = rand() % 1000;
        for(i = 0; i < 20 && shared->Index[i] == 1; i++);
        if(i == 20)  ////Index为1表示缓冲池被消费者占用
        {
            pthread_mutex_unlock(&mutex);
            sem_post(&shared->sem);  //sem_post为V操作，用来增加信号量的值
            printf("waiting\n");
            usleep(expntl(1000000 * (1.0/lamda)));
        }
        else
        {
            pthread_mutex_unlock(&mutex);
            sem_post(&shared->sem);  //V 操作增加信号量
            shared->Buffer[i%20] = item;
            printf("producer %lu create %d\n", pthread_self(), item);
            shared->Index[i%20] = 1;  //表示该缓冲区被生产者使用了
            usleep(expntl(1000000 * (1.0/lamda)));
        }
    }
}

int main(int argc, char const *argv[])
{
    int i = 0;
    void *shm = NULL;  //共享存储段连接的实际地址
    struct shared_use_st *shared = NULL;
    int shmid;  //共享内存标识符
    shmid = shmget((key_t)1121, sizeof(struct shared_use_st), 0666|IPC_CREAT);
    if(shmid == -1)
    {
        exit(EXIT_FAILURE);
    }
    shm = shmat(shmid, (void*)0, 0);  //返回共享存储段连接的实际地址
    if(shm == (void*)-1)
    {
        exit(EXIT_FAILURE);
    }
    printf("Memory attached at %ld\n", (intptr_t)shm);
    shared = (struct shared_use_st*)shm;  //缓冲池为共享存储段连接地址
    for( ; i < 20; i++ )
    {
        shared->Index[i] = 0;  //对缓冲池初始化，Index为0表示可以生产
    }
    sem_init(&(shared->sem),1,1);  //信号量化初始化，且信号量初始值为第二个1
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
    pthread_t tid1, tid2, tid3;
    pthread_attr_t attr1, attr2, attr3;
    pthread_attr_init(&attr1);
    pthread_attr_init(&attr2);
    pthread_attr_init(&attr3);
    pthread_create(&tid1, &attr1, producer, shared);
    pthread_create(&tid2, &attr2, producer, shared);
    pthread_create(&tid3, &attr3, producer, shared);
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    pthread_join(tid3, NULL);
    
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

