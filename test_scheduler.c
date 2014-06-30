#include <stdio.h>
#include<stdlib.h>
#include "task_scheduler.h"
#include "my_math.h"

void test_rmff(){
	int i = 0;
	Task tasks[] = {
		{ "task0", 0, 25, 6, 20, 0, 0 },
		{ "task1", 1, 12, 2, 5, 0, 0 },
		{ "task2", 2, 15, 3, 7, 0, 0 },
		{ "task3", 3, 30, 2, 25, 0, 0 },
		{ "task4", 4, 200, 10, 100, 0, 0 },
		{ "task5", 5, 25, 6, 20, 0, 0 },
		{ "task6", 6, 12, 2, 5, 0, 0 },
		{ "task7", 7, 15, 3, 7, 0, 0 },
		{ "task8", 8, 100, 7, 50, 0, 0 },
		{ "task9", 9, 18, 4, 10, 0, 0 }
	};

	int length;
	GET_ARRAY_LEN(tasks, length);
	init_tasks_priority(tasks, length);

	int p_cnt;
	char *names[] = {"P0", "P1", "P2"};
	GET_ARRAY_LEN(names, p_cnt);
	Processor *processors = new_processors(names, p_cnt);

	ff_distribute_task(tasks, length, processors, p_cnt);
	for (int i = 0; i < p_cnt; i++){
		Processor *p = &processors[i];
		dump_processor(p);
		for (int j = 0; j< 100; j++){
			Task *t = rm_schedule(j, p);
			if (t != NULL){
				if (t->c <= t->execute_time){
					t->execute_time = 0;
					task_done(p);
				}
				printf("%d ", t->index);
			}
			else{
				printf("* ");
			}
		}
		printf("\n");
	}
}


void test_rm()
{
	int i = 0;
	Task tasks[] = {
		{ "task0", 0, 25, 6, 20, 0, 0 },
		{ "task1", 1, 12, 2, 5, 0, 0 },
		{ "task2", 2, 15, 3, 7, 0, 0 },
		{ "task3", 3, 30, 2, 25, 0, 0 },
	};

	int length;
	GET_ARRAY_LEN(tasks, length);
	init_tasks_priority(tasks, length);

	char *names[] = { "P0"};
	Processor *p = new_processors(names, 1);

	for (int i = 0; i<length; i++){
		push_task(p, &tasks[i]);
	}

	int scheduable = rm_schedulable(tasks, length);
	if (scheduable>0){
		printf("RM scheduable\n");
		for (int i = 0; i < 100; i++){
			Task *t = rm_schedule(i, p);
			if (t != NULL){
				if (t->c <= t->execute_time){
					t->execute_time = 0;
					task_done(p);
				}
				printf("%d ", t->index);
			}
			else{
				printf("* ");
			}

		}
		printf("\n");
		printf("Preemptive schedule\n");
		dump_processor(p);
	}
	else{
		printf("Can't schedule by RM\n");
	}
}

int main()
{
	printf("\nRM test\n");
	test_rm();

	printf("\nRMFF test\n");
	test_rmff();
	printf("\nTest done");
}