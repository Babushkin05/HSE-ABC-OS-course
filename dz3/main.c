#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    int arg = atoi(argv[1]);
    if(arg < 0){
        printf("program argument must be positive\n");
        exit(1);
    }
    pid_t son_id = fork();
    if(son_id == 0){
        uint64_t fact = 1;
        for( int i = 2; i <= arg; ++i){
            uint64_t prev = fact;
            fact *= i;
            if(fact/i != prev){
                printf("overflow happend in factorial computing\n");
                exit(1);
            }
        }
        printf("factorial: %llu\n", fact);
        exit(0);
    }
    else{
        uint64_t a = 0;
        uint64_t b = 1;
        if(arg == 0)
            printf("fibonacci: 0\n");
        else if (arg == 1)
        {
            printf("fibonacci: 1\n");
        }
        else{
            uint64_t c = 0;
            for(int i = 1; i < arg; ++i)
            {
                uint64_t prev = c;
                c = a + b;
                a = b;
                b = c;
                if(prev > c){
                    printf("overflow happend in fibbonacci computing\n");
                    exit(1);
                }
            }
            printf("fibonacci: %llu\n", c);
        }
        exit(0);
    }
}