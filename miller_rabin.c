/*
 * Copyright(c) 2020-2023 All rights reserved by Heekuck Oh.
 * 이 프로그램은 한양대학교 ERICA 컴퓨터학부 학생을 위한 교육용으로 제작되었다.
 * 한양대학교 ERICA 학생이 아닌 자는 이 프로그램을 수정하거나 배포할 수 없다.
 * 프로그램을 수정할 경우 날짜, 학과, 학번, 이름, 수정 내용을 기록한다.
 */
#include "miller_rabin.h"

/*
 * mod_add() - computes a+b mod m
 * a와 b가 m보다 작다는 가정하에서 a+b >= m이면 결과에서 m을 빼줘야 하므로
 * 오버플로가 발생하지 않도록 a-(m-b)를 계산하고, 그렇지 않으면 그냥 a+b를 계산하면 된다.
 * a+b >= m을 검사하는 과정에서 오버플로가 발생할 수 있으므로 a >= m-b를 검사한다.
 */
uint64_t mod_add(uint64_t a, uint64_t b, uint64_t m)
{   
    //a와 b가 m보다 크다면 m보다 작을 때 까지 m을 빼서 숫자를 맞춰준다.
    //문제는, 단순히 m을 계속 빼주기만 한다면 a와 b가 m에 비해 미친듯이 클 때, 이 과정이 느려 터졌다는것이다.
    //그러니까 빼지말고 모듈러 연산을 이용하자. 어차피 목적은 m의 나머지 집합에서 계산하는 것이다.
    a %= m;
    b %= m;

    //주석대로 a+b >= m인지 검사하는 과정. a+b가 오버플로우가 될 수 있으므로 이렇게 돌아서 계산한다.
    if (a >= m - b) return a - (m - b);
    else return a + b;
}

/*
 * mod_sub() - computes a-b mod m
 * 만일 a < b이면 결과가 음수가 되므로 m을 더해서 양수로 만든다.
 */
uint64_t mod_sub(uint64_t a, uint64_t b, uint64_t m)
{
    //a와 b가 m보다 크다면 m보다 작을 때 까지 m을 빼서 숫자를 맞춰준다.
    //문제는, 단순히 m을 계속 빼주기만 한다면 a와 b가 m에 비해 미친듯이 클 때, 이 과정이 느려 터졌다는것이다.
    //그러니까 빼지말고 모듈러 연산을 이용하자. 어차피 목적은 m의 나머지 집합에서 계산하는 것이다.
    a %= m;
    b %= m;

    //unsigned 자료형임을 유의. 음수가 되면 곤란하다.
    if (a < b) return m - b + a;
    else return a - b;
}

/*
 * mod_mul() - computes a*b mod m
 * a*b에서 오버플로가 발생할 수 있기 때문에 덧셈을 사용하여 빠르게 계산할 수 있는
 * "double addition" 알고리즘을 사용한다. 그 알고리즘은 다음과 같다.
 *     r = 0;
 *     while (b > 0) {
 *         if (b & 1)
 *             r = mod_add(r, a, m);
 *         b = b >> 1;
 *         a = mod_add(a, a, m);
 *     }
 */
uint64_t mod_mul(uint64_t a, uint64_t b, uint64_t m)
{
    uint64_t r = 0;

    //a와 b가 m보다 크다면 m보다 작을 때 까지 m을 빼서 숫자를 맞춰준다.
    //문제는, 단순히 m을 계속 빼주기만 한다면 a와 b가 m에 비해 미친듯이 클 때, 이 과정이 느려 터졌다는것이다.
    //그러니까 빼지말고 모듈러 연산을 이용하자. 어차피 목적은 m의 나머지 집합에서 계산하는 것이다.
    a %= m;
    b %= m;

    while (b > 0)
    {
        if (b & 1) r = mod_add(r, a, m);
        b = b >> 1;
        a = mod_add(a, a, m);
    }
    //결과는 a가 아니라 r에 저장된다.
    return r;
}

/*
 * mod_pow() - computes a^b mod m
 * a^b에서 오버플로가 발생할 수 있기 때문에 곱셈을 사용하여 빠르게 계산할 수 있는
 * "square multiplication" 알고리즘을 사용한다. 그 알고리즘은 다음과 같다.
 *     r = 1;
 *     while (b > 0) {
 *         if (b & 1)
 *             r = mod_mul(r, a, m);
 *         b = b >> 1;
 *         a = mod_mul(a, a, m);
 *     }
 */
uint64_t mod_pow(uint64_t a, uint64_t b, uint64_t m)
{
    uint64_t r = 1;
    
    //여기선 b는 modular 연산 하면 안됨!!!!
    //a는 커봐야 좋을 거 없으므로 modular 연산으로 크기 줄여주자
    //양자 컴퓨터를 활용하여 Shor's Algorithm을 사용하면 power 연산에서 주기를 빠르게 발견할 수 있지만
    //아직 양자컴퓨터가 불안정하기도 하고, 여기서 다룰 내용은 아니므로 스킵
    if (a >= m) a %= m;
    while (b > 0)
    {
        if (b & 1) r = mod_mul(r, a, m);
        b = b >> 1;
        a = mod_mul(a, a, m);
    }

    return r;
}

/*
 * Miller-Rabin Primality Testing against small sets of bases
 *
 * if n < 2^64,
 * it is enough to test a = 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, and 37.
 *
 * if n < 3,317,044,064,679,887,385,961,981,
 * it is enough to test a = 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, and 41.
 */
const uint64_t a[BASELEN] = {2,3,5,7,11,13,17,19,23,29,31,37};

/*
 * miller_rabin() - Miller-Rabin Primality Test (deterministic version)
 *
 * n > 3, an odd integer to be tested for primality
 * It returns PRIME if n is prime, COMPOSITE otherwise.
 */
int miller_rabin(uint64_t n)
{
    //입력 자료형이 uint64_t이므로 n이 2^64 미만임을 기대할 수 있다.(0 <= n <= 2^64 - 1)
    //따라서 Deterministic Miller Rabin 알고리즘에 의해 37까지의 소수에 대해서만 돌려보면 된다.
    unsigned int i, j; //반복문용 변수
    uint64_t temp = n - 1, k = 1, q;

    //입력 n은 n > 3의 홀수임이 보장되어 있다.(입력을 그렇게만 준다.)
    //(n-1) = 2^k * q를 만족하는 적당한 k와 q를 찾아야 한다.
    //(n-1)을 비트열로 표현했을 때, 가장 오른쪽에서 부터의 0의 갯수 = k
    //q는 비트열에서 오른쪽에서 k자리 만큼 무시한 값이 된다.
    //예를 들어, 30이면 30 = 0b11110 이므로
    //k = 1, q = 0b1111 = 15가 되는 식
    while(temp >= 1)
    {
        if (temp & 1) break;
        k++;
        temp = temp >> 1;
    }
    q = temp;
    
    //Probabilistic Miller Rabin Algorithm과 다르게 a를 확률적으로 결정하지 않는다.
    //대신, 37 이하의 모든 소수에 대해서 반복문을 돌린다.
    //이 소수들에 대해서는 글로벌 상수로 a가 선언되어 있으므로, 이걸 이용하자.
    for (i = 0; i < BASELEN; i++)
    {
        //본격적인 알고리즘
        if (mod_pow(a[i], q, n) == 1) return PRIME;
        for (j = 0; j < k - 1; j++)
        {
            if (mod_pow(mod_pow(a[i], q, n), mod_pow(2, j, n), n) == n - 1) return PRIME;
        }
    }
    return COMPOSITE;
}
