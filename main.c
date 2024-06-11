#include "msp.h"
#include "Clock.h"

#define TIME 400
#define SPEED 3300
#define RNODE 0b0110
#define LNODE 0b1100
#define NODE 0b0100

int edge[10][10], angle[10][10], ans[50];
int top = -1;
int count;

uint16_t first_left, first_right;
uint16_t period_left, period_right;

void timer_A3_capture_init() {
    P10->SEL0 |= 0x30;
    P10->SEL1 &= ~0x30;
    P10->DIR &= ~0x30;

    TIMER_A3->CTL &= ~0x0030;
    TIMER_A3->CTL = 0x0200;

    TIMER_A3->CCTL[0] = 0x4910;
    TIMER_A3->CCTL[1] = 0x4910;
    TIMER_A3->EX0 &= ~0x0007;

    NVIC->IP[3] = (NVIC->IP[3]&0x0000FFFF) | 0x404000000;
    NVIC->ISER[0] = 0x0000C000;
    TIMER_A3->CTL |= 0x0024;
}

void TA3_0_IRQHandler(void) {
    TIMER_A3->CCTL[0] &= ~0x0001;
    period_right = TIMER_A3->CCR[0] - first_right;
    first_right = TIMER_A3->CCR[0];
}

void TA3_N_IRQHandler(void)
{
    TIMER_A3->CCTL[1] &= ~0x0001;
    count++;
}

void PWM_Duty3(uint16_t duty3){
    TIMER_A0->CCR[3] = duty3;
}

void PWM_Duty4(uint16_t duty4){
    TIMER_A0->CCR[4] = duty4;
}


void loadSensor() {
   P7->DIR = 0xFF;
   P7->OUT = 0xFF;
   Clock_Delay1us(1000);
   P7->DIR = 0x00;
   Clock_Delay1us(1000);
}

void Left_Forward() {
    P5->OUT &= ~0x10;
}

void Left_Backward() {
    P5->OUT |= 0x10;
}

void Right_Forward() {
    P5->OUT &= ~0x20;
}

void Right_Backward() {
    P5->OUT |= 0x20;
}

void PWM_Init34(uint16_t period, uint16_t duty3, uint16_t duty4){
    P2->DIR |= 0xC0;
    P2->SEL0 |= 0xC0;
    P2->SEL1 &= ~0xC0;

    TIMER_A0->CCTL[0] = 0x800;
    TIMER_A0->CCR[0] = period;

    TIMER_A0->EX0 = 0x0000;

    TIMER_A0->CCTL[3] = 0x0040;
    TIMER_A0->CCR[3] = duty3;
    TIMER_A0->CCTL[4] = 0x0040;
    TIMER_A0->CCR[4] = duty4;

    TIMER_A0->CTL=0x02F0;
}

void Motor_Init(){
    P3->SEL0 &= ~0xC0;
    P3->SEL1 &= ~0xC0;
    P3->DIR |= 0xC0;
    P3->OUT &= ~0xC0;

    P5->SEL0 &= ~0x30;
    P5->SEL1 &= ~0x30;
    P5->DIR |=0x30;
    P5->OUT &= ~0x30;

    P2->SEL0 &= ~0xC0;
    P2->SEL1 &= ~0xC0;
    P2->DIR |= 0xC0;
    P2->OUT &= ~0xC0;

    PWM_Init34(15000, 0, 0);
}



void Move(uint16_t leftDuty, uint16_t rightDuty){
    P3->OUT |= 0xC0;
    PWM_Duty3(rightDuty);
    PWM_Duty4(leftDuty);
}

void stop(int delay) {
    Left_Forward();
    Right_Forward();
    Move(0, 0);
    Clock_Delay1ms(delay);
}

void moveForward(int leftDuty, int rightDuty, int delay) {
    Left_Forward();
    Right_Forward();
    Move(leftDuty, rightDuty);
    Clock_Delay1ms(delay);
}

void moveBackward(int leftDuty, int rightDuty, int delay) {
    Left_Backward();
    Right_Backward();
    Move(leftDuty, rightDuty);
    Clock_Delay1ms(delay);
}

void Rotate_Clockwise(int degree){
    degree = degree * 2;
    count = 0;
    while(1){
        Left_Forward();
        Right_Backward();
        Move(SPEED, SPEED);
        if(count>degree){
            Move(0,0);
            stop(1);
            break;
        }
    }
}

void Rotate_Counter_Clockwise(int degree){
    degree = degree * 2;
    count = 0;
    while(1){
        Left_Backward();
        Right_Forward();
        Move(SPEED, SPEED);
        if(count>degree){
            Move(0,0);
            stop(1);
            break;
        }
    }
}

void tmp1(int degree){
    degree = degree * 2;
    count = 0;
    while(1){
        Left_Forward();
        Right_Backward();
        Move(SPEED, SPEED);
        if(count>degree){
            Move(0,0);
            stop(1);
            break;
        }
    }
}

void tmp2(){
    int scan;

    count = 0;
    while(1){
        Left_Backward();
        Right_Forward();
        Move(SPEED, SPEED);
        loadSensor();
        if(P7->IN & 0b00011000){
            Move(0,0);
            stop(1000);
            break;
        }
    }
}

void tmp3(){
    int scan;

    count = 0;
    while(1){
        Left_Backward();
        Right_Forward();
        Move(SPEED, SPEED);
        loadSensor();
        if(!(P7->IN & 0b00011000)){
            Move(0,0);
//            stop(1);
            break;
        }
    }
}

void tmp4(){
    int scan;

    count = 0;
    while(1){
        moveForward(SPEED,SPEED,1);
        loadSensor();
        if(!(P7->IN & 0b00011000)){
            moveForward(SPEED,SPEED,50);
//            Move(0,0);
//            stop(1);
            break;
        }
    }
}

// 오일러 회로 찾기
void dfs(int cur)
{
    int nxt;
    for (nxt = 1; nxt <= 8; nxt++)
    {
        if (edge[cur][nxt])
        {
            edge[cur][nxt]--;
            edge[nxt][cur]--;
            dfs(nxt);
        }
    }
    ans[++top] = cur;
}


int direction() {
    loadSensor();

    int res = 0;

    if (P7->IN == 0b11111111)
        res += 0b1000;
    else if (P7->IN == 0b00111100)
        res += 0b0100;
    else if((P7->IN & 0b00011000) == 0b00011000)
        res += 0b0010;
    // white
    else if (P7->IN == 0)
        res += 0b0001;
    return res;
}


int get_nxtnode(int cur, int cnt)
{
    int ret;
    if (cnt == 0)
       ret = cur + 1;
    else if (cnt == 1)
       ret = cur + 3;
    else if (cnt == 2)
       ret = cur + 5;
    else if (cnt == 3)
       ret = cur + 7;

    if (ret > 8)
       ret %= 8;

    return ret;
}



void find_node(int node)
{
    int scan;

    // 앞으로 먼저 살짝 이동
    moveForward(SPEED,SPEED, 200);

    while(!((scan=direction()) & node))
    {
        moveForward(SPEED,SPEED,1);
        loadSensor();
        if (!(P7->IN & 0b00001000))
        {
            while((P7->IN & 0b00011000) != 0b00011000)
            {
                loadSensor();
                Left_Backward();
                Right_Forward();
                Move(0, SPEED);
            }
        }
        else if (!(P7->IN & 0b00010000))
        {
            while((P7->IN & 0b00011000) != 0b00011000)
            {
                loadSensor();
                Left_Forward();
                Right_Backward();
                Move(SPEED, 0);
            }
        }
    }

    stop(100);
    moveForward(SPEED,SPEED,300);
    return 1;
}

int main(void) {
    int scan, i, j, cur, cnt, nxt, ret;
    Clock_Init48MHz();
    Motor_Init();
    timer_A3_capture_init();

    // Phase 1: 경로 찾기
    // 1. 각도 초기값 설정
    for (i=1; i<=8; i++)
    {
        for (j=1; j<=8; j++)
            angle[i][j] = -1;
    }

    scan = 0;
    while(scan != 0b1000)
    {
        scan = direction();
    }
    find_node(NODE);
//    moveBackward(SPEED,SPEED,200);
//    stop(200);
    stop(100);
    Rotate_Clockwise(90);
    stop(100);

    for(cur=1; cur<=8; cur++)
    {
        for (cnt=0; cnt<4; cnt++)
        {
            // 1. 앞으로 조금 이동해서 경로 탐색

            // moveForward(SPEED,SPEED,200);

            // 2. 해당 위치에 경로가 있는 경우, 각도를 기록하기
            // 45도 마다 경로가 존재.
            if (1)
            {
                nxt = get_nxtnode(cur, cnt);
                angle[cur][nxt] = cnt;
//                edge[cur][nxt]++;
            }

            // 3. 45만큼 회전해서 다음 경로 확인
            if (cnt != 3)
            {
                Rotate_Counter_Clockwise(40);
                stop(300);
            }
        }


        // 4. 현재 node의 경로 탐색이 끝난 후, 다음 node로 이동하기 위해 회전
        Rotate_Clockwise(140);


        // 5. 다음 node를 찾을 때까지 직진
        find_node(NODE);

        // 6. 노드를 찾은 후, 시작 line으로 회전
        Rotate_Counter_Clockwise(40);
        stop(300);
    }


    for (i=1; i<=8; i++)
    {
        edge[i][j] = 0;
        for (j=1; j<=8; j++)
        {
            if (angle[i][j] >= 0)
                edge[i][j] = 1;
        }
    }

    dfs(1);

    int tmp = 0;
    // Phase 2
    cur = ans[top--];

    while (top >= 0)
    {
        nxt = ans[top--];
        cnt = angle[cur][nxt] + tmp;

        // 1. 기록된 값만큼 회전하기
        if(cnt <=4){
            while(cnt--)
            {
                Rotate_Counter_Clockwise(45);
            }
        }
        else{
            cnt = 8 - cnt;
            while(cnt--){
                Rotate_Clockwise(45);
            }
        }

        // 2.다음 노드로 이동하기
        find_node(NODE);

        tmp = angle[cur][nxt] + 1;
        cur = nxt;
    }
    stop(100);
    // Finish
}

//while (top >= 0)
//{
//    nxt = ans[top--];
//    cnt = angle[cur][nxt];
//
//    // 1. 기록된 값만큼 회전하기
//    int tmp = 0;
//    while(tmp++ != cnt)
//        Rotate_Counter_Clockwise(45);
//
//    // 2.다음 노드로 이동하기
//    find_node(NODE);
//
//    // 3. 시작 위치 맞추기
//    tmp = 0;
//    while(tmp++ != cnt)
//        Rotate_Counter_Clockwise(45);
//    Rotate_Counter_Clockwise(25);
//
//    cur = nxt;
//}


//for (i=1; i<=8; i++)
//{
//    for (j=1; j<=8; j++)
//        angle[i][j] = -1;
//}
//for(cur=1; cur<=8; cur++) {
//    for (cnt=0; cnt<4; cnt++)
//    {
//        nxt = get_nxtnode(cur, cnt);
//        angle[cur][nxt] = cnt;
//    }
//}
//
//for (i=1; i<=8; i++)
//{
//    for (j=1; j<=8; j++)
//    {
//        edge[i][j] = 0;
//        if(angle[i][j] >= 0)
//            edge[i][j] = 1;
//    }
//}