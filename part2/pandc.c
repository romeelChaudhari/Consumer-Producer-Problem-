#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>


int N, P, C, X; // int N, P, C, X are amount of buffers, producer and consumer threads
//amount of consumers and extra consumers
int Consumer_N = 0;
int ExtraConsumer = 0;

// shared value for producing and consuming in the buffer
int sValue = 0;
int buffer = 0;
int *buffer1;
int Producer_Val = 0;
int *prod;
int Consumer_Val = 0;
int *cons;
int out = 0;
int in = 0;
struct timespec Ptime, time2, Ctime, time4;// sleep time for producer and consumers
pthread_mutex_t mutex;
sem_t *full;
sem_t *empty;
int counter=26;
int consume=1;
int c_amount=28;
int enqueue(int id)
{
    sem_wait(empty);
    pthread_mutex_lock(&mutex);
    int check_Val;
    if(buffer < N)
    {
        sValue++;
        buffer1[in] = sValue;
        nanosleep(&Ptime, &time2);
        buffer++;
        in = (in +1) % N;
        prod[Producer_Val] = sValue;
        Producer_Val++;
        printf("   %d was produced by producer->    %d \n", sValue, id);
        check_Val = 1;
    }
    else
    {
        check_Val = -1;
    }
    sem_post(full);
    pthread_mutex_unlock(&mutex);
    return check_Val;
}
int dequeue(int id)
{
    sem_wait(full);
    pthread_mutex_lock(&mutex);
    int check_Val;
    if(buffer > 0)
    {
        int val;
        val = buffer1[out];
        nanosleep(&Ctime, &time4);
        out = (out + 1) % N;
        buffer--;
        cons[Consumer_Val] = val;
        Consumer_Val++;
        printf("   %d was consumed by consumer->    %d \n", val, id);
        counter++;
        check_Val= 1;
    }
    else
    {
        check_Val = -1;
    }
    sem_post(empty);
    pthread_mutex_unlock(&mutex);
    return check_Val;
}
void *producer(void *args)
{
    int ProducerID = (int) ((long) args);
    int ProducerCount = 0;
    do
    {
        int check_Val = enqueue(ProducerID);
        if(check_Val == 1)
        {
            ProducerCount++;
        }
    }while(ProducerCount < X);
    pthread_exit(NULL);
}
void *consumer(void *args)
{
    int ConsumerID = (int) ((long) args);
    int c_count=0;
    int check = Consumer_N;
    if(ExtraConsumer >0)
    {
        check = Consumer_N + 1;
        ExtraConsumer--;
    }
    do
    {
        if(dequeue(ConsumerID) == 1)
        {
            c_count++;
        }
    }while(c_count < check);
    pthread_exit(NULL);
}
int main(int argc, char *argv[])
{
    time2.tv_nsec = 0;
    time4.tv_nsec = 0;
    time_t start, end;
    time(&start);
    N=atoi(argv[1]);    // command line arguments
    P=atoi(argv[2]);    // command line arguments
    C=atoi(argv[3]);    // command line arguments
    X=atoi(argv[4]);    // command line arguments
    Ptime.tv_sec=atoi(argv[5]);    // command line arguments
    Ctime.tv_sec=atoi(argv[6]);    // command line arguments
    Consumer_N = (P*X) / C;
    ExtraConsumer = (P*X) % C;
    int bArray[N];
    int proArray[(P*X)];
    int cArray[(P*X)];
    buffer1 = bArray;
    prod = proArray;
    cons = cArray;
    pthread_t proThread[P];
    pthread_t conThread[C];
    pthread_mutex_init(&mutex, NULL);
    sem_open("/full", O_CREAT, 0644, 0);
    sem_open("/empty", O_CREAT, 0644, N);
    time_t clock = time(NULL);
    printf("Current time: %s \n", ctime(&clock));
    printf(" Number of Buffers : %29d \n Number of Producers : %27d \n Number of Consumers : %27d \n Number of items Produced by each Producer : %5d  \n Number of items consumed by each consumer : %5d \n Over consume on? : %30d \n Over consume amount : %27d \n Time each Producer Sleeps (seconds) : %11ld \n Time each Consumer Sleeps (seconds) : %11ld \n", N, P, C, X,counter,consume, c_amount, Ptime.tv_sec, Ctime.tv_sec);
    for(long i =0; i < P; i++)
    {
        pthread_create(&proThread[i], NULL, producer, (void *) i+1);
    }
    for(long i =0; i < C; i++)
    {
        pthread_create(&conThread[i], NULL, consumer, (void *) i+1);
    }
    for(int i = 0; i < P; i++)
    {
        pthread_join(proThread[i], NULL);
        printf("Producer Thread Join:  %d \n", i+1);
    }
    for(int i = 0; i < C; i++)
    {
        pthread_join(conThread[i], NULL);
        printf("Consumer Thread Join:  %d \n", i+1);
    }
    clock = time(NULL);
    printf("Current time: %s \n", ctime(&clock));
    int check = 0;
    printf("Producer Array | Consumer Array \n");
    for(int i = 0; i< (P*X); i++)
    {
        printf("%d              | %d \n", prod[i], cons[i]);
    }
    if(Producer_Val == Consumer_Val)
    {
        printf("Consumer and Producer Arrays Match!\n");
    }
    pthread_mutex_destroy(&mutex);
    sem_close(full);
    sem_unlink("/full");
    sem_close(empty);
    sem_unlink("/empty");
    time(&end);
    int total_time = (end-start);
    printf("Total Time: %d secs \n", total_time);
    pthread_exit(NULL);
    return 0;
}
