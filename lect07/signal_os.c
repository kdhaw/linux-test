#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#define NPROC 10
#define TIME_QUANTUM 3

typedef enum {
       	READY,
       	RUNNING,
       	SLEEP,
       	END
} state_t;

typedef struct {
	pid_t pid;
	state_t state;
	int tq;
	int sleep;
} PCB;

PCB pcb[NPROC];
int current = 0;
int tick = 0;

int next_ready(int start) {
	for (int i = 1; i <= NPROC; i++) {
		int idx = (start + i) % NPROC;
		if (pcb[idx].state == READY) return idx;
		}
	return -1;
}

int all_tq_zero() {
	for (int i = 0; i < NPROC; i++) {
		if (pcb[i].state != END && pcb[i].tq > 0)
			return 0;
	}
	return 1;
}

void reset_all_tq() {
	printf("  >>> ALL TIME QUANTUM RESET <<<\n");
	for (int i = 0; i < NPROC; i++) {
		if (pcb[i].state != END)
			pcb[i].tq = TIME_QUANTUM;
	}
}

void run(int idx) {
	pcb[idx].state = RUNNING;
	kill(pcb[idx].pid, SIGUSR1);
	printf("  RUN PID=%d tq=%d\n", pcb[idx].pid, pcb[idx].tq);
}

void context_switch() {
	int n = next_ready(current);
	if (n != -1) {
		current = n;
		run(current);
	}
}

void child_handler(int sig) {
	static int burst = -1;

	if (burst < 0) {
		srand(getpid());
		burst = rand() % 10 + 1;
	}

	burst--;
	printf("    [CHILD %d] burst=%d\n", getpid(), burst);

	if (burst <= 0) 
		exit(0);

	if (rand() % 4 == 0) {
		kill(getppid(), SIGUSR2);
		pause();
	}
}

void io_request(int sig) {
	pcb[current].state = SLEEP;
	pcb[current].sleep = rand() % 5 + 1;
	
	printf("  PID=%d I/O REQUEST  → sleep=%d\n",
			pcb[current].pid, pcb[current].sleep);
	context_switch();
}

void child_exit(int sig) {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        for (int i = 0; i < NPROC; i++) {
            if (pcb[i].pid == pid) {
                pcb[i].state = END;
                pcb[i].tq = 0;
                printf("  [CHILD %d TERMINATED]\n", pid);
            }
        }
    }
}

void timer_handler(int sig) {
	tick++;
	printf("\n[TICK %d]\n", tick);

	for (int i = 0; i < NPROC; i++) {
		if (pcb[i].state == SLEEP) {
			pcb[i].sleep--;
			if (pcb[i].sleep == 0) {
				pcb[i].state = READY;
				printf("  PID=%d WAKEUP → READY\n", pcb[i].pid);
			}
		}
	}

	if (pcb[current].state == RUNNING) {
		pcb[current].tq--;
		printf("  PID=%d tq=%d\n",
				pcb[current].pid, pcb[current].tq);

		if (pcb[current].tq == 0) {
			pcb[current].state = READY;
			printf("  PID=%d tq=0 → CONTEXT SWITCH\n",
				       	pcb[current].pid);
			context_switch();
		}
	}

	if (all_tq_zero())
		reset_all_tq();
}

int main() {
	srand(time(NULL));

	signal(SIGALRM, timer_handler);
	signal(SIGUSR2, io_request);
	signal(SIGCHLD, child_exit);

	for (int i = 0; i < NPROC; i++) {
		pid_t pid = fork();
		if (pid == 0) {
			signal(SIGUSR1, child_handler);
			while (1) pause();
		}
		pcb[i].pid = pid;
		pcb[i].state = READY;
		pcb[i].tq = TIME_QUANTUM;
	}
	run(0); 

	while (1) {
		alarm(1);
		pause();
	}
}
