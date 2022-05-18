/*
 * Copyright 2022. Heekuck Oh, all rights reserved.
 * 이 프로그램은 한양대학교 ERICA 소프트웨어학부 재학생을 위해 작성하였다.
 */
#include <stdio.h>

int main(void)
{
    int x;
    
    x = 0;
    #pragma omp parallel
    {
        #pragma omp critical
        {
            x = x + 1;
        }
    }
    printf("x = %d\n", x);
    
    return 0;
}
