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
 * 생산자와 소비자가 진행할 수 없을 때 대기하기 위한 조건변수와 조합으로 사용되는 뮤텍스락
 */
pthread_cond_t pro_q = PTHREAD_COND_INITIALIZER;
pthread_cond_t con_q = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
/*
 * alive 값이 false가 될 때까지 스레드 내의 루프가 무한히 반복된다.
 */
bool alive = true;

/*
 * 생산자 스레드로 실행할 함수이다. 임의의 수를 생성하여 버퍼에 넣는다.
 */
void *producer(void *arg)
{
    int i = *(int *)arg;
    int item;
    
    while (alive) {
        /*
         * 뮤텍스락을 걸고 버퍼에 빈 공간이 있는지 검사한다.
         * 버퍼에 빈 공간이 없으면 조건변수 pro_q에서 대기한다.
         * 빈 공간이 있으면 루프를 빠져 나와 생산하기 위해 임계구역으로 들어간다.
         */
        pthread_mutex_lock(&lock);
        while (counter == BUFSIZE) {
            /*
             * alive 값을 검사하여 스레드가 비활성이면 즉시 종료시킨다.
             * 그 이유는 counter 값이 BUFSIZE인 상태에서 모든 소비자가 종료되면
             * 생산자는 이 while 루프를 빠저나오지 못해서 교착상태에 빠진다.
             */
            if (!alive) {
                pthread_mutex_unlock(&lock);
                pthread_exit(NULL);
            }
            /*
             * 버퍼에 빈 공간이 없으면 조건변수 pro_q에서 대기한다.
             */
            pthread_cond_wait(&pro_q, &lock);
        }
        /*
         * 새로운 아이템(난수)을 생산하여 버퍼에 넣고 관련 변수를 갱신한다.
         */
        item = rand();
        buffer[in] = item;
        in = (in + 1) % BUFSIZE;
        counter++;
        produced++;
        /*
         * 새 아이템을 생산하였으므로 대기하고 있을지 모르는 소비자를 깨우고 뮤택스락을 푼다.
         */
        pthread_cond_signal(&con_q);
        pthread_mutex_unlock(&lock);
        /*
         * 생산한 아이템을 출력한다.
         */
        printf("<P%d,%d>\n", i, item);
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
        /*
         * 뮤텍스락을 걸고 버퍼에 아이템이 있는지 검사한다.
         * 버퍼에 아이템이 없으면 조건변수 con_q에서 대기한다.
         * 아이템이 있으면 루프를 빠져 나와 소비하기 위해 임계구역으로 들어간다.
         */
        pthread_mutex_lock(&lock);
        while (counter == 0) {
            /*
             * alive 값을 검사하여 스레드가 비활성이면 즉시 종료시킨다.
             * 그 이유는 counter 값이 0인 상태에서 모든 생산자가 종료되면
             * 소비자는 이 while 루프를 빠저나오지 못해서 교착상태에 빠진다.
             */
            if (!alive) {
                pthread_mutex_unlock(&lock);
                pthread_exit(NULL);
            }
            /*
             * 버퍼에 아이템이 없으면 조건변수 con_q에서 대기한다.
             */
            pthread_cond_wait(&con_q, &lock);
        }
        /*
         * 버퍼에서 아이템(난수)을 꺼내고 관련 변수를 갱신한다.
         */
        item = buffer[out];
        out = (out + 1) % BUFSIZE;
        counter--;
        consumed++;
        /*
         * 아이템을 소비하였으므로 대기하고 있을지 모르는 생산자를 깨우고 뮤택스락을 푼다.
         */
        pthread_cond_signal(&pro_q);
        pthread_mutex_unlock(&lock);
        /*
         * 소비할 아이템을 빨간색으로 출력한다.
         */
        printf(RED"<C%d,%d>"RESET"\n", i, item);
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
     * 조건변수에서 대기하고 있을지 모를 생산자와 소비자를 모두 깨워서 종료하게 한다.
     */
    pthread_cond_broadcast(&con_q);
    pthread_cond_broadcast(&pro_q);
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
    /*
     * 뮤텍스락과 조간변수를 지우고 메인함수를 종료한다.
     */
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&pro_q);
    pthread_cond_destroy(&con_q);

    return 0;
}
