#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "shared/intercore.h"
#include "shared/counters.h"
#include "shared/timerInterrupt.h"
#include "task_scheduler.h"
struct Task globalTasks[5];
char globalTaskNames[5][80]={"t1","t2","t3","t4","t5"};
char globalProcessorNames[4][80];
struct Processor globalProcessor[4];
struct TaskNode globalTaskNode[8];

//#define CLOCK_TO_MS(clock, ms) { ms = (clock/10000);}
const int CLOCK_PER_MS = 100000;
const int CLOCK_PER_S = 100000000;

static Processor *s_processors;
static Task *s_tasks;
int stacks[5][1000];

static int wait_after_done = 0;

static volatile unsigned int timer = 0;

void task_1();
void task_2(); 
void task_3();

int reverse_integer(int x);
int test_func_execute_time(Func func);
void init();
void idle(){
	while(1);
}

Processor* getProcessorByCore(){
	int core = corenum();
	if(core ==3 || core ==4){
		int offset = core - 3;
		return s_processors + offset;
	}
	return NULL;
}

void print_all(){
	dump_processor(s_processors);
	dump_processor(s_processors+1);
	for(int i=0;i<5;i++){
		dump_task(s_tasks+i);		
	}
}

void on_task_done(){
	Processor *p = getProcessorByCore();
	//icSema_P(2);
	printf("Done %s:%s\n", p->name, p->cur_task->name);
	//icSema_V(2);
	task_done(p);
	idle();
}

void mc_init() {
	
}

void mc_main() {
	int core = corenum();
	
	if(core == 2){
		icSema_P(1);
		wait_after_done = 0;
		init();
		puts("ff distribute task start");
		ff_distribute_task(s_tasks, 5, s_processors, 2);
		puts("ff distribute task done");
		print_all();
		wait_after_done = 1;

		icSema_V(1);	
	}else if(core == 3 
//|| core == 4
){
		icSleep(10000000);
		icSema_P(1);
		printf("core%d waiting for init\n", core);
		// Called concurrently in all cores except core #1, after mc_init returns
		//set timer interrupt handler
		printf("core%d start schedule\n", core);
		set_handlerR((int)isr);
		//set timer interrupt intervals
		set_timerInterval(CLOCK_PER_MS);
		//set interrupt enable
		set_mask(0);
		icSema_V(1);
		task_1();
	}
	idle();
}

int test_func_execute_time(Func func){
	counters_start();
	(func)();
	counters_stop();
	unsigned int count4 = counters_readAndZero(4);//low
	//unsigned int count5 = counters_readAndZero(5);//high
	int time = count4/CLOCK_PER_MS;
	printf("count=%d\n", time);
	//printf("count5=%u\n",count5);
	//return count/CLOCK_TO_MS;
	return time;
}

void init()
{
	puts("init start");
	char *p_names[] = {"P0","P1"};
	s_processors = new_processors(p_names, 2);

	int task_cnt = 5;
	char *names[] = {"T0", "T1", "T2", "T3", "T4"};
	s_tasks = new_tasks(names, task_cnt);

	Func t1 = task_1;
	s_tasks[0].func = t1;
	Func t2 = task_2;
	s_tasks[1].func = t2;
	Func t3 = task_3;
	s_tasks[2].func = t3;
	s_tasks[3].func = t3;
	s_tasks[4].func = t3;

	for(int i=0;i<task_cnt;i++){
		s_tasks[i].c = test_func_execute_time(s_tasks[i].func);
		s_tasks[i].t = s_tasks[i].c*4;
		s_tasks[i].saved_state[30] = (int)isr;
		printf("task %d addr %8x\n",i,(int)s_tasks[i].func);
		printf("s_tasks[%d] addr=%8x\n",i,(int )(s_tasks+i));
	}

	s_tasks[0].saved_state[31] = (int)task_1;
	s_tasks[0].saved_state[27] = (int)stacks[0];
	s_tasks[1].saved_state[31] = (int)task_2;
	s_tasks[1].saved_state[27] = (int)stacks[1];
	s_tasks[2].saved_state[31] = (int)task_3;
	s_tasks[2].saved_state[27] = (int)stacks[2];
	s_tasks[3].saved_state[31] = (int)task_3;
	s_tasks[3].saved_state[27] = (int)stacks[3];
	s_tasks[4].saved_state[31] = (int)task_3;
	s_tasks[4].saved_state[27] = (int)stacks[4];

	init_tasks_priority(s_tasks, task_cnt);
	puts("init done");
}

void schedule(){
	int core = corenum();
	int p_index = core-3;
	Processor *p = s_processors + p_index;
	
	int i;
	int *pData = (int *)(core * 512 + 0x4000);
	Task *pre_task = p->cur_task;
	Task *task_to_run= rm_schedule(timer, p);

	if(task_to_run!=NULL){
		printf("task_to_run addr=%8x\n",(int)task_to_run);
		//icSema_P(2);
		printf("Run %s:%s\n",p->name, task_to_run->name);
		//icSema_V(2);	
		dump_task(task_to_run);
	}else{
		printf("Run task NULL\n");	
	}
	printf("Time:%d\n",timer);
	timer++;

	if (task_to_run != pre_task){
		if (pre_task != NULL){
			//switch task
			//save previous task state
			printf("Save pre task %s\n",pre_task->name);
			for (i = 0; i < 32; i++){
				pre_task->saved_state[i] = pData[i];
			}
		}
		if (task_to_run != NULL){
			//restore current task state
			printf("Restore task %s\n",task_to_run->name);
			for (i = 0; i < 32; i++){
				pData[i] = task_to_run->saved_state[i];
				//pData[i] = pre_task->saved_state[i];
				printf("pData[%d]=%8x\n",i,pData[i]);
			}
		}
		
	}else{
		printf("No Switch\n");	
	}
	dump_task(task_to_run);
	for(i=0;i<5;i++)
		printf("task %d eip=%8x\n",i,s_tasks[i].saved_state[31]);

	printf("eip value=%8x\n",pData[31]);
}

void compute(){
	int dest0 = reverse_integer(123456789);
	int dest1 = reverse_integer(-123456789);
	int dest2 = reverse_integer(0);
	int dest3 = reverse_integer(1000);
	int dest4 = reverse_integer(-1000);
}

/* Task 1 Reverse Integer */
void task_1() {
	puts("Task1 Start");
	for(int i=0;i<5;i++){
		compute();	
	}

	puts("Task1 Done");
	if(wait_after_done==1){
		on_task_done();
	}
}

/* Task 2 Binary Search Tree -- insert & delete */
void task_2(){
	puts("Task2 Start");
	for(int i=0;i<15;i++){
		compute();	
	}
	puts("Task2 Done");
	if(wait_after_done){
		on_task_done();
	}
	
}

/* Task 2 Binary Search Tree -- search */
void task_3(){
	puts("Task3 Start");
	for(int i=0;i<20;i++){
		compute();	
	}
	puts("Task3 Done");
	if(wait_after_done){
		on_task_done();
	}
	
}


int reverse_integer(int x){
	int result = 0;
	int t;
	if (x<0){
		t = -x;
	}
	else{
		t = x;
	}
	int a = 0;
	while (t > 0){
		result *= 10;
		a = t % 10;
		t = t / 10;
		result += a;
	}
	if (x<0){
		result = -result;
	}
	return result;
}


