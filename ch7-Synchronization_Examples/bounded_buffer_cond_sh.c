/*
 * Copyright 2021, 2022. Heekuck Oh, all rights reserved
 * 이 프로그램은 한양대학교 ERICA 소프트웨어학부 재학생을 위해 교육용으로 제작되었다.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>

#define N 8
#define BUFSIZE 10
#define RED "\e[0;31m"
#define RESET "\e[0m"
/*
 * 생산자와 소비자가 공유할 버퍼를 만들고 필요한 변수를 초기화한다.
 */
int buffer[BUFSIZE];
int in = 0;
int out = 0;
int counter = 0;
/*
 * 생산된 아이템과 소비된 아이템의 개수를 기록하기 위한 변수
 */
int produced = 0;
int consumed = 0;
/*
 * alive 값이 false가 될 때까지 스레드 내의 루프가 무한히 반복된다.
 */
bool alive = true;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t buffer_has_space = PTHREAD_COND_INITIALIZER;
pthread_cond_t buffer_has_data = PTHREAD_COND_INITIALIZER;

/*
 * 생산자 스레드로 실행할 함수이다. 임의의 수를 생성하여 버퍼에 넣는다.
 */
void *producer(void *arg)
{
    int i = *(int *)arg;
    int item;
    
    while (alive) {

        pthread_mutex_lock(&mutex);

        while (counter == BUFSIZE)
            pthread_cond_wait(&buffer_has_space, &mutex);
        
        item = rand();
        buffer[in] = item;
        in = (in + 1) % BUFSIZE;
        counter++;
        produced++;
        printf("<P%d,%d>\n", i, item);

        pthread_cond_signal(&buffer_has_data);
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}

/*
 * 소비자 스레드로 실행할 함수이다. 버퍼에서 수를 읽고 출력한다.
 */
void *consumer(void *arg)
{
    int i = *(int *)arg;
    int item;
    
    while (alive) {
        
        pthread_mutex_lock(&mutex);

        while (counter == 0)
            pthread_cond_wait(&buffer_has_data, &mutex);

        item = buffer[out];
        out = (out + 1) % BUFSIZE;
        counter--;
        consumed++;
        printf(RED"<C%d,%d>"RESET"\n", i, item);

        pthread_cond_signal(&buffer_has_space);
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}

int main(void)
{
    pthread_t tid[N];
    int i, id[N];

    /*
     * N/2 개의 소비자 스레드를 생성한다.
     */
    for (i = 0; i < N/2; ++i) {
        id[i] = i;
        pthread_create(tid+i, NULL, consumer, id+i);
    }
    /*
     * N/2 개의 생산자 스레드를 생성한다.
     */
    for (i = N/2; i < N; ++i) {
        id[i] = i;
        pthread_create(tid+i, NULL, producer, id+i);
    }
    /*
     * 스레드가 출력하는 동안 1 밀리초 쉰다.
     * 이 시간으로 스레드의 출력량을 조절한다.
     */
    usleep(1000);
    /*
     * 스레드가 자연스럽게 무한 루프를 빠져나올 수 있게 한다.
     */
    alive = false;
    /*
     * 자식 스레드가 종료될 때까지 기다린다.
     */
    for (i = 0; i < N; ++i)
        pthread_join(tid[i], NULL);
    /*
     * 생산된 아이템의 개수와 소비된 아이템의 개수를 출력한다.
     */
    printf("Total %d items were produced.\n", produced);
    printf("Total %d items were consumed.\n", consumed);

    pthread_cond_destroy(&buffer_has_data);
    pthread_cond_destroy(&buffer_has_space);

    /*
     * 메인함수를 종료한다.
     */
    return 0;
}
