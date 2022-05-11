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
 * alive 값이 false가 될 때까지 스레드 내의 루프가 무한히 반복된다.
 */
bool alive = true;

/* 임계구역을 위한 Mutex Lock 선언 및 초기화 */
pthread_mutex_t mutex;

/* 상호배제를 위한 Condition 선언 및 초기화 */
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
        /* 공유 변수에 접근하기 전에 Mutex lock을 건다. */
        pthread_mutex_lock(&mutex);

        /* 버퍼가 꽉 찼다면 소비자가 버퍼를 하나 소비할 때까지 기다린다. */
        while (counter == BUFSIZE)
            pthread_cond_wait(&buffer_has_space, &mutex);

        item = rand();
        buffer[in] = item;
        printf("<P%d,%d>\n", i, item);
        in = (in + 1) % BUFSIZE;
        counter++;

        /* 버퍼가 비어있지 않다는 의미의 signal을 보낸다. */
        pthread_cond_signal(&buffer_has_data);
        /* Mutex Lock을 해지한다. */
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
        /* 공유 변수에 접근하기 전에 Mutex lock을 건다. */
        pthread_mutex_lock(&mutex);

        /* 버퍼가 비어있다면 생산자가 버퍼를 하나 생산할 때까지 기다린다. */
        while (counter == 0)
            pthread_cond_wait(&buffer_has_data, &mutex);

        item = buffer[out];
        printf(RED"<C%d,%d>"RESET"\n", i, item);
        out = (out + 1) % BUFSIZE;
        counter--;

        /* 버퍼가 꽉 차지 않았다는 의미의 signal을 보낸다. */
        pthread_cond_signal(&buffer_has_space);
        /* Mutex Lock을 해지한다. */
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}

int main(void)
{
    pthread_t tid[N];
    int i, id[N];

    /* mutex 변수 초기화 */
    if (pthread_mutex_init(&mutex, NULL) != 0) {
        printf("\n mutex init failed. \n");
        return 1;
    }

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
     * Mutual Exclusion에 사용한 condition을 소멸시킨다.
     */
    pthread_cond_destroy(&buffer_has_data);
    pthread_cond_destroy(&buffer_has_space);

    /*
     * 메인함수를 종료한다.
     */
    return 0;
}
