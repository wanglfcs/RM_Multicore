#include<stdio.h>
#include<stdlib.h>
#include "task_scheduler.h"
#include "my_math.h"
#include <stddef.h>
#include <string.h>

Task* new_tasks(char *names[], int size){
	Task *t = (Task *)malloc(sizeof(Task)*size);
	for (int i = 0; i < size; i++){
		t[i].c = 0;
		t[i].d = 0;
		t[i].p = 0;
		t[i].t = 0;
		t[i].index = i + 1;
		t[i].func = NULL;
		char *name = names[i];
		int namelen = strlen(name);
		printf("name:%s len:%d", name, namelen);
		t[i].name = (char *)malloc(sizeof(char) *namelen);
		strcpy(t[i].name, name);
		//t[i].name = names[i];
		t[i].ready = 0;
		t[i].execute_time = 0;
	}
	return t;
}

Processor* new_processors(char* names[], int size){
	Processor *p = (Processor *)malloc(sizeof(Processor)*size);
	Processor *t;
	for (int i = 0; i < size; i++){
		//t = p+i;
		//t->usage = 0;
		//t->tasksList = NULL;
		//t->task_cnt = 0;
		//t->cur_task = NULL;
		//t->name = name[i];
		p[i].usage = 0;
		p[i].tasksList = NULL;
		p[i].task_cnt = 0;
		p[i].cur_task = NULL;
		char *name = names[i];
		int namelen = strlen(name);
		printf("name:%s len:%d", name, namelen);
		p[i].name = (char *)malloc(sizeof(char) *namelen);
		strcpy(p[i].name, name);
		//p[i].name = name[i];
	}
	return p;
}

void init_tasks_priority(Task *tasks, int task_cnt){
	insert_sort(tasks, task_cnt);
	for (int i = 0; i < task_cnt; i++){
		tasks[i].p = task_cnt - i;
	};
}

void insert_sort(Task *x, int n)
{
	int i, j;
	Task t;
	for (i = 1; i < n; i++)
	{
		t = *(x + i);
		for (j = i - 1; j >= 0 && t.t < (*(x + j)).t; j--)
		{
			*(x + j + 1) = *(x + j);
		}
		*(x + j + 1) = t;
	}
}

int rm_schedulable_inner(double totle_usage, int task_count)
{
	double max_usage = task_count*(pow(2, ((double)1 / (double)task_count)) - 1);
	if (totle_usage >= 0 && totle_usage<max_usage && totle_usage <= 1){
		return 1;
	}
	else{
		return 0;
	}
}

int rm_schedulable(Task *tasks, int n)
{
	double usage = 0, ui = 0;
	Task t;
	for (int i = 0; i < n; i++){
		t = *(tasks + i);
		ui = (double)t.c / (double)t.t;
		usage += ui;
	}
	return rm_schedulable_inner(usage, n);
}

Task* rm_schedule(int time, Processor *processor){
	TaskNode *cur_node = processor->tasksList;
	Task *task_to_run = NULL;
	Task *cur_task = NULL;
	int first = 1;
	while (cur_node!=NULL){
		cur_task = cur_node->task;
		if (time%cur_task->t == 0){
			cur_task->ready++;
		}
		if (first > 0 && cur_task->ready > 0){
			first = 0;
			processor->cur_task = cur_task;
			task_to_run = cur_task;
			task_to_run->execute_time++;
		}
		cur_node = cur_node->next;
	}
	return task_to_run;
}

void task_done(Processor *p){
	Task *done_task = p->cur_task;
	p->cur_task = NULL;
	done_task->ready--;
}

void set_tasks(Processor *p, Task *t, int task_cnt){
	for (int i = 0; i < task_cnt; i++){
		push_task(p,t+i);
	}
	p->task_cnt = task_cnt;
}

void push_task(Processor *p, Task *t){
	 TaskNode *new_node = malloc(sizeof(TaskNode));
	 new_node->task = t;
	 new_node->next = NULL;

	 if (p->tasksList == NULL){
		 p->tasksList = new_node;
	 }
	 else{
		 TaskNode *cur_node = p->tasksList;
		 TaskNode *next_node;
		 while (cur_node != NULL){
			 next_node = cur_node->next;
			 if (next_node == NULL || next_node->task->t > t->t){
				 cur_node->next = new_node;
				 new_node->next = next_node;
				 break;
			 }
			 cur_node = cur_node->next;
		 }
	 }
	 p->task_cnt++;
	 p->usage += (double)t->c / (double)t->t;
}

void ff_distribute_task(Task *tasks, int task_cnt, Processor *processors, int processor_cnt){
	Task maxPeriodTask = *(tasks + task_cnt - 1);
	Task curTask;

	// devide tasks for processors using FF algorithm
	for (int i = 0; i < task_cnt; i++){
		curTask = *(tasks + i);
		//Find first non-empty processor and try to put curTask in it.
		for (int j = 0; j < processor_cnt; j++){
			Processor p = *(processors + j);
			double new_usage = p.usage + (double)curTask.c / (double)curTask.t;
			int count = p.task_cnt;
			if (rm_schedulable_inner(new_usage, count + 1)){
				push_task(processors + j, tasks + i);
				break;
			}
		}
	}
}

void dump_processor(Processor *p){
	printf("{\n");
	printf("  Processor name=%s: usage=%0.3f\n", p->name, p->usage);
	printf("  Tasks %d:", p->task_cnt);
	TaskNode *node = p->tasksList;
	while (node!=NULL){
		printf("%s ", node->task->name);
		node = node->next;
	}
	printf("\n}\n");
}

void dump_task(Task *t){
	printf("{\n");
	printf("  Task %s index=%d\n", t->name, t->index);
	printf("  C:%d T:%d D:%d P:%d ExeTime:%d Ready:%d\n", t->c, t->t, t->d, t->p, t->execute_time, t->ready);
	printf("}\n");
}
