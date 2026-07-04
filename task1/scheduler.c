#include <stdio.h>

#define MAX_PROCESSES 5
#define QUANTUM 2

typedef struct {
    int pid;
    int remaining_time;
    int burst_time;
    int waiting_time;
    int turnaround_time;
    int completed; //if 0 = not done, 1 = done
} Process;

void round_robin(Process proc[], int n){
    int time = 0;
    int completed_count = 0;

    printf("\n Round Robin Scheduler Simulation (Quantum = %d) \n", QUANTUM);
    while (completed_count < n){
        int all_idle = 1; // to check if every process is finishe this pass

        for(int i = 0; i<n; i++){
            if(proc[i].completed)
            continue; 
            all_idle = 0; 
            if (proc[i].remaining_time > QUANTUM){
                printf("Time %d: Process %d runs for %d units (remaining before: %d)\n",
                time, proc[i].pid, QUANTUM, proc[i].remaining_time);
                time += QUANTUM;
                proc[i].remaining_time -= QUANTUM;
            } else {
                printf("Time %d: Process %d runs for %d units and COMPLETES\n",
                time, proc[i].pid, proc[i].remaining_time);
                time += proc[i].remaining_time;
                proc[i].waiting_time = time - proc[i].burst_time;
                proc[i].turnaround_time = time;
                proc[i].remaining_time = 0;
                proc[i].completed = 1;
                completed_count++;
            }
        }
        if (all_idle){
            break;
        }
    }
}
    void print_results(Process proc[], int n) {
    float total_wait = 0, total_turnaround = 0;

    printf("\n--- Results ---\n");
    printf("PID\tBurst\tWaiting\tTurnaround\n");
    for (int i = 0; i < n; i++) {
        printf("%d\t%d\t%d\t%d\n",
               proc[i].pid, proc[i].burst_time,
               proc[i].waiting_time, proc[i].turnaround_time);
        total_wait += proc[i].waiting_time;
        total_turnaround += proc[i].turnaround_time;
    }

    printf("\nAverage Waiting Time: %.2f\n", total_wait / n);
    printf("Average Turnaround Time: %.2f\n", total_turnaround / n);
}
int main() {
    Process proc[MAX_PROCESSES] = {
        {1, 5, 5, 0, 0, 0},
        {2, 8, 8, 0, 0, 0},
        {3, 3, 3, 0, 0, 0},
        {4, 6, 6, 0, 0, 0}
    };
    int n = 4;  // number of processes we're actually using

    round_robin(proc, n);
    print_results(proc, n);

    return 0;
}

