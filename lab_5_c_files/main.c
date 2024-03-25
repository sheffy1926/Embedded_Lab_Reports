#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

struct p_task {
    char task_name[3];
    int ex_time;
    int period;
    int deadline;
    int priority;
    int blocked; 
    int task_count;
    int slack;
    int preemptions;
    int misses;
};
struct a_task {
    char task_name[3];
    int ex_time;
    int release_time;
    int deadline;
    int priority;
    int blocked;
    int task_count;
    int slack;
    int preemptions;
    int misses;
};

//Prototypes
void temp_clear(char *temp);

int releases_deadlines(struct p_task *per_tasks, struct a_task *aper_tasks, int periodic, int aperiodic, int cur_time, FILE *fptr_write);
int preemptions(struct p_task *per_tasks, struct a_task *aper_tasks, int p_task_processed, int a_task_processed, int cur_task, int cur_time, int prev, int periodic, FILE *fptr_write);

void find_priority_RMA(struct p_task *per_tasks, struct a_task *aper_tasks, int periodic, int aperiodic);
int cur_task_running_RMA(struct p_task *per_tasks, struct a_task *aper_tasks, int periodic, int aperiodic);
void schedule_RMA(struct p_task *per_tasks, struct a_task *aper_tasks, int sim_time, int periodic, int aperiodic, FILE * fptr_write);

void schedule_EDF(struct p_task *per_tasks, struct a_task *aper_tasks, int sim_time, int periodic, int aperiodic, FILE *fptr_write);
int cur_task_running_EDF(struct p_task *per_tasks, struct a_task *aper_tasks, int periodic, int aperiodic);

void schedule_LST(struct p_task *per_tasks, struct a_task *aper_tasks, int sim_time, int periodic, int aperiodic, FILE *fptr_write);
int cur_task_running_LST(struct p_task *per_tasks, struct a_task *aper_tasks, int periodic, int aperiodic, int cur_time);

int main(int argc, char **argv){
    char line[30] = {0};
    char temp[30] = {0};
    int j;
    int k;
    int periodic;
    int aperiodic;  
    int sim_time;
    struct p_task *per_tasks;
    struct a_task *aper_tasks;

    if (argc != 3){
        printf("Command line arguments are invalid\n");
        exit(0);
    }

    //Open the file
    FILE* fptr_read;
    fptr_read = fopen(argv[1], "r");
    if (fptr_read == NULL){
        printf("File could not be opened\n");
        exit(0);
    }

    //Read in the number of periodic tasks
    fgets(line, 30, fptr_read);
    for(int i = 0; line[i] >= '0' && line[i] <= '9'; i++){
        temp[i] = line[i];
        periodic = atoi(temp);
    }

    //Read in the total simulation time
    fgets(line, 30, fptr_read);
    for(int i = 0; line[i] >= '0' && line[i] <= '9'; i++){
        temp[i] = line[i];
        sim_time = atoi(temp);
    }
    for(int i = 0; i < 30; i++){
        temp[i] = 0;
    }
    //printf("Sim_time: %d\n",sim_time);

    //Read in the periodic tasks: name, ex_time, period
    per_tasks = malloc(sizeof(struct p_task) * (periodic));
    for(int i = 0; i < periodic; i++){
        //Read in task name
        fgets(line, 30, fptr_read);
        for(j = 0; (line[j] >= 'A') && (line[j] <= 'Z'); j++){
            per_tasks[i].task_name[j] = line[j];
        }
        //Add Null character
        per_tasks[i].task_name[j+1] = 0;
        j += 2;

        //Read in Ex_time
        for(k = 0; (line[j] >= '0') && (line[j] <= '9'); k++, j++){
            temp[k] = line[j];
        }
        temp[k+1] = 0;
        per_tasks[i].ex_time = atoi(temp);
        j += 2;
        temp_clear(temp);

        //Read in Period
        for(k = 0; (line[j] >= '0') && (line[j] <= '9'); k++,j++){
            temp[k] = line[j]; 
        }
        temp[k+1] = 0;
        per_tasks[i].period = atoi(temp);
        temp_clear(temp);
    }

    //Read in the number of aperiodic tasks
    fgets(line, 30, fptr_read);
    for(int i = 0; line[i] >= '0' && line[i] <= '9'; i++){
        temp[i] = line[i];
        aperiodic = atoi(temp);
    }

    //Read in the aperiodic tasks: name, ex_time, release_time
    aper_tasks = malloc(sizeof(struct a_task) * (aperiodic));
    for(int i = 0; i < aperiodic; i++){
        //Read in task name
        fgets(line, 30, fptr_read);
        for(j = 0; (line[j] >= 'A') && (line[j] <= 'Z'); j++){
            aper_tasks[i].task_name[j] = line[j];
        }
        //Add Null character
        aper_tasks[i].task_name[j+1] = 0;
        j += 2;

        //Read in Ex_time
        for(k = 0; (line[j] >= '0') && (line[j] <= '9'); k++, j++){
            temp[k] = line[j];
        }
        temp[k+1] = 0;
        aper_tasks[i].ex_time = atoi(temp);
        j += 2;
        temp_clear(temp);

        //Read in Release_time
        for(k = 0; (line[j] >= '0') && (line[j] <= '9'); k++,j++){
            temp[k] = line[j]; 
        }
        temp[k+1] = 0;
        aper_tasks[i].release_time = atoi(temp);
        temp_clear(temp);
    }
    
    //Close input file from reading
    fclose(fptr_read);

    FILE *fptr_write;
    fptr_write = fopen(argv[2], "w");
    if (fptr_write == NULL){
        printf("File could not be opened");
        exit(0);
    }

    //Print Periodic Tasks
    /*printf("Periodic: %d\n",periodic);
    for(k = 0; k < periodic; k++){
        printf("Task Name: %s,\t",per_tasks[k].task_name);
        printf("Ex_Time: %d,\t",per_tasks[k].ex_time);
        printf("Period: %d,\n",per_tasks[k].period);
    }

    //Print APeriodic Tasks
    printf("Aperiodic: %d\n",aperiodic);
    for(k = 0; k < aperiodic; k++){
        printf("Task Name: %s,\t",aper_tasks[k].task_name);
        printf("Ex_Time: %d,\t",aper_tasks[k].ex_time);
        printf("Release_Time: %d,\n",aper_tasks[k].release_time);
    }*/

    //Schedule Tasks using RMA Algorithm 
    schedule_RMA(per_tasks, aper_tasks, sim_time, periodic, aperiodic, fptr_write);

    //Schedule Tasks using EDF Algorithm
    schedule_EDF(per_tasks, aper_tasks, sim_time, periodic, aperiodic, fptr_write);

    //Schedule Tasks using LST Algorithm
    schedule_LST(per_tasks, aper_tasks, sim_time, periodic, aperiodic, fptr_write);

    fclose(fptr_write);

    return 0;
}

//Clears temp variable after each use
void temp_clear(char *temp){
    for(int i = 0; i < 30; i++){
        temp[i] = 0;
    }
}

//Function calulates release times, blocking/unblocking, and whether deadlines are missed or not
int releases_deadlines(struct p_task *per_tasks, struct a_task *aper_tasks, int periodic, int aperiodic, int cur_time, FILE *fptr_write){
    int total_misses = 0;
    //Check if any periodic tasks release on this current clock cycle and update to unblocked if they are released.
    for (int i = 0; i < periodic; i++){
        //Calculate if a deadline was missed for a periodic task.
        if ((cur_time == (per_tasks[i].deadline)) && (per_tasks[i].blocked == 0) && (per_tasks[i].task_count > 0)){
            printf("Task %s's deadline missed at time %d\n",per_tasks[i].task_name, cur_time);
            fprintf(fptr_write,"Task %s's deadline missed at %d time\n",per_tasks[i].task_name, cur_time);
            per_tasks[i].misses++;
            total_misses++;
        }

        //Calculate release times and if a task is blocked or not
        if ((cur_time % per_tasks[i].period == 0)){
            per_tasks[i].blocked = 0; 
            per_tasks[i].task_count = per_tasks[i].ex_time - 1;
            //Calculate deadline for task based on current time and period of task.
            if (cur_time == 0){
                per_tasks[i].deadline = per_tasks[i].period;
            }
            else {
                per_tasks[i].deadline = (((cur_time / per_tasks[i].period) + 1) * per_tasks[i].period);
            }

            //Recalculate the slack for all tasks when a task finishes or releases
            for (int i = 0; i < periodic; i++){
                //Calculate each task's current slack available based on S = (D - R) - C
                if(per_tasks[i].blocked == 0){
                    per_tasks[i].slack = per_tasks[i].deadline - cur_time - per_tasks[i].task_count;
                }
            }
            for (int i = 0; i < aperiodic; i++){
                //Calculate each task's current slack available based on S = (D - R) - C
                if(aper_tasks[i].blocked == 0){
                    aper_tasks[i].slack = aper_tasks[i].deadline - cur_time - aper_tasks[i].task_count;
                }
            }

            printf("Task %s released at time %d\n",per_tasks[i].task_name,cur_time);
            fprintf(fptr_write,"Task %s released at time %d\n",per_tasks[i].task_name,cur_time);
        }
    }

    //Check if any aperiodic tasks release on this current clock cycle and update to unblocked if they are released ********************************************************
    for (int i = 0; i < aperiodic; i++){
        //Calculate if a deadline was missed for an aperiodic task.
        if ((cur_time == (aper_tasks[i].deadline)) && (aper_tasks[i].blocked == 0) && (aper_tasks[i].task_count > 0)){
            printf("\tTask %s's deadline missed at time %d\n",aper_tasks[i].task_name, cur_time);
            fprintf(fptr_write,"\tTask %s's deadline missed at time %d\n",aper_tasks[i].task_name, cur_time);
            aper_tasks[i].blocked = 1;
            aper_tasks[i].misses++;
            total_misses++;
        }

        //Calculate release times and if a task is blocked or not
        if ((cur_time == aper_tasks[i].release_time) && (cur_time != 0)){
            aper_tasks[i].blocked = 0;
            aper_tasks[i].task_count = aper_tasks[i].ex_time - 1;
            //Calculate deadline for task based on current time and period of task or set it as the end of the sim_time 
            aper_tasks[i].deadline = aper_tasks[i].release_time + 500;

            //Recalculate the slack for all tasks when a task finishes or releases
            for (int i = 0; i < periodic; i++){
                //Calculate each task's current slack available based on S = (D - R) - C
                if(per_tasks[i].blocked == 0){
                    per_tasks[i].slack = per_tasks[i].deadline - cur_time - per_tasks[i].task_count;
                }
            }
            for (int i = 0; i < aperiodic; i++){
                //Calculate each task's current slack available based on S = (D - R) - C
                if(aper_tasks[i].blocked == 0){
                    aper_tasks[i].slack = aper_tasks[i].deadline - cur_time - aper_tasks[i].task_count;
                }
            }

            printf("\tTask %s released at time %d\n",aper_tasks[i].task_name,cur_time);
            fprintf(fptr_write,"\tTask %s released at time %d\n",aper_tasks[i].task_name,cur_time);
        }
    } //********************************************************************************************************************************************************************
    return total_misses;
} 

//Function calulates if a task was preempted or not
int preemptions(struct p_task *per_tasks, struct a_task *aper_tasks, int p_task_processed, int a_task_processed, int cur_task, int cur_time, int prev, int periodic, FILE *fptr_write){
    int total_preemptions = 0;
    //Mark if a task is preempted and by what task
    //If a higher periodic task is preempting another periodic task
    if (p_task_processed == 1 && (cur_task < periodic)){
        if ((prev != cur_task) && (per_tasks[prev].blocked == 0)){
            printf("Task %s preempted by Task %s at time %d\n",per_tasks[prev].task_name, per_tasks[cur_task].task_name, cur_time);
            fprintf(fptr_write,"Task %s preempted by Task %s at time %d\n",per_tasks[prev].task_name, per_tasks[cur_task].task_name, cur_time);
            per_tasks[prev].preemptions++;
            total_preemptions++;
        }
    }
    //If an aperiodic task is preempting a periodic task
    else if (p_task_processed == 1 && (cur_task >= periodic)){
        if ((prev != (cur_task)) && (per_tasks[prev].blocked == 0)){ 
            printf("\tTask %s preempted by Task %s at time %d\n",per_tasks[prev].task_name, per_tasks[cur_task - periodic].task_name, cur_time);
            fprintf(fptr_write,"\tTask %s preempted by Task %s at time %d\n",per_tasks[prev].task_name, per_tasks[cur_task - periodic].task_name, cur_time);
            aper_tasks[prev].preemptions++;
            total_preemptions++;
        }
    }
    //If a periodic task is preempting an aperiodic task (aperiodic cannot preempt another aperiodic task)
    else if (a_task_processed == 1){
        if ((prev != (cur_task - periodic)) && (aper_tasks[prev].blocked == 0)){ 
            printf("Task %s preempted by Task %s at time %d\n",aper_tasks[prev].task_name, per_tasks[cur_task].task_name, cur_time);
            fprintf(fptr_write,"Task %s preempted by Task %s at time %d\n",aper_tasks[prev].task_name, per_tasks[cur_task].task_name, cur_time);
            aper_tasks[prev].preemptions++;
            total_preemptions++;
        }
    }
    return total_preemptions;
}

//Finds the priority of periodic and aperiodic tasks for RMA
void find_priority_RMA(struct p_task *per_tasks, struct a_task *aper_tasks, int periodic, int aperiodic){
    int lowest = 0;
    //set priority to initial value of 0 for periodic tasks
    for (int i = 0; i < periodic; i++){
        per_tasks[i].priority = 0;
    }
    //set priority of aperiodic tasks to initial value of 0
    for (int i = 0; i < aperiodic; i++){
        aper_tasks[i].priority = 0;
    }

    //Assign priorities to periodic tasks based on period length, shorter period = higher priority (1 is the highest)
    for (int i = 0; i < periodic; i++){
        for (int j = 0; j < periodic; j++){
            if((per_tasks[j].period < per_tasks[lowest].period) && (per_tasks[j].priority == 0)){
                lowest = j; 
            }
            else if ((per_tasks[j].period == per_tasks[lowest].period) && (per_tasks[j].priority == 0)){
                if(per_tasks[j].ex_time < per_tasks[lowest].ex_time){
                    lowest = j;
                }
            }
        }
        per_tasks[lowest].priority = i+1;

        //Update Lowest priority that has not been assigned 
        for (int k = 0; k < periodic; k++){
            if(per_tasks[k].priority == 0){
                lowest = k;
            }
        }
    }

    lowest = 0;
    //Assign priorities to aperiodic tasks based on release time, shorter period = higher priority (1 is the highest)
    for (int i = 0; i < aperiodic; i++){
        for (int j = 0; j < aperiodic; j++){
            if((aper_tasks[j].release_time < aper_tasks[lowest].release_time) && (aper_tasks[j].priority == 0)){
                lowest = j; 
            }
            else if ((aper_tasks[j].release_time == aper_tasks[lowest].release_time) && (aper_tasks[j].priority == 0)){
                if(aper_tasks[j].ex_time < aper_tasks[lowest].ex_time){
                    lowest = j;
                }
            }
        }
        aper_tasks[lowest].priority = i+periodic+1;
        
        //Update Lowest priority that has not been assigned 
        for (int k = 0; k < aperiodic; k++){
            if(aper_tasks[k].priority == 0){
                lowest = k;
            }
        }
    }
}

//RMA Scheduler function
void schedule_RMA(struct p_task *per_tasks, struct a_task *aper_tasks, int sim_time, int periodic, int aperiodic, FILE *fptr_write) {
    int cur_time;
    int total_misses = 0;
    int total_preemptions = 0;
    int cur_task = 0; 
    int prev = -1;
    int p_task_processed = 0;
    int a_task_processed = 0;
    int response_time = 0;
    int a_tasks_finished = 0;
    int idle_time = 0;

    //Set fixed priorities for periodic and aperiodic tasks under RMA
    find_priority_RMA(per_tasks,aper_tasks, periodic, aperiodic);

    printf("\nRMA Algorithm Task Schedule: *******************************************\n\n");
    fprintf(fptr_write,"RMA Algorithm Task Schedule: *****************************************\n\n");

    //Initialize periodic tasks
    for (int i = 0; i < periodic; i++){
        per_tasks[i].misses = 0;
        per_tasks[i].preemptions = 0;
        per_tasks[i].slack = 0;
    }
    //Initialize aperiodic tasks
    for (int i = 0; i < aperiodic; i++){
        aper_tasks[i].misses = 0;
        aper_tasks[i].preemptions = 0;
        aper_tasks[i].blocked = 1;
        aper_tasks[i].slack = 0;
    }

    //Simulate RMA scheduler for Simulation time
    for (cur_time = 0; cur_time < sim_time; cur_time++){
                //Function calulates release times, blocking/unblocking, and whether deadlines are missed or not
        total_misses += releases_deadlines(per_tasks,aper_tasks,periodic,aperiodic,cur_time,fptr_write);

        //Find the current task that should be running that is unblocked and highest priority
        cur_task = cur_task_running_RMA(per_tasks,aper_tasks,periodic,aperiodic);

        //Finds if a task is preempted by another task and increments the preemptions calculator
        total_preemptions += preemptions(per_tasks,aper_tasks,p_task_processed,a_task_processed,cur_task,cur_time,prev,periodic,fptr_write);

        p_task_processed = 0;
        a_task_processed = 0;
        
        //If a periodic task is available to run
        if (cur_task < periodic){
            //current task finished executing and is now in the blocked state so another task can run
            if ((per_tasks[cur_task].task_count == 0) && (per_tasks[cur_task].blocked == 0)){
                printf("Task %s finished at time %d\n",per_tasks[cur_task].task_name,cur_time);
                fprintf(fptr_write,"Task %s finished at time %d\n",per_tasks[cur_task].task_name,cur_time);
                per_tasks[cur_task].blocked = 1;
                prev = cur_task; 
                p_task_processed = 1;
            }
            //Check if the current task still needs to finish executing
            else if (per_tasks[cur_task].task_count > 0){
                //printf("Task %s Task Count: %d,",per_tasks[cur_task].task_name,per_tasks[cur_task].task_count);
                //printf("Task %s, Current Time: %d\n",per_tasks[cur_task].task_name, cur_time);
                //fprintf(fptr_write,"Task %s, Current Time: %d\n",per_tasks[cur_task].task_name, cur_time);
                per_tasks[cur_task].task_count--;
                prev = cur_task;
                p_task_processed = 1;
            }
        }
        //If all periodic tasks are blocked check if any aperiodic tasks can run based on current highest aperiodic priority task available **********************************
        else if ((cur_task >= periodic) && (cur_task < aperiodic+periodic)){
            //Loop through all aperiodic tasks to find which is the highest priority released task available
            //current task finished executing and is now in the blocked state so another task can run
            if ((aper_tasks[cur_task - periodic].task_count == 0) && (aper_tasks[cur_task - periodic].blocked == 0)){
                printf("\tTask %s finished at %d time\n",aper_tasks[cur_task - periodic].task_name,cur_time);
                fprintf(fptr_write,"\tTask %s finished at %d time\n",aper_tasks[cur_task - periodic].task_name,cur_time);
                response_time += (cur_time - aper_tasks[cur_task - periodic].release_time);
                a_tasks_finished++;
                aper_tasks[cur_task - periodic].blocked = 1;
                prev = cur_task - periodic;
                a_task_processed = 1;
            }
            //Check if the current task still needs to finish executing
            else if (aper_tasks[cur_task - periodic].task_count > 0){
                //printf("Task %s Task Count: %d\n",aper_tasks[cur_task].task_name,aper_tasks[cur_task].task_count);
                //printf("\tTask %s, Current Time: %d\n",aper_tasks[cur_task - periodic].task_name, cur_time);
                //fprintf(fptr_write,"\tTask %s, Current Time: %d\n",aper_tasks[cur_task - periodic].task_name, cur_time);
                aper_tasks[cur_task - periodic].task_count--;
                prev = cur_task - periodic;
                a_task_processed = 1;
            }
        } //******************************************************************************************************************************************************************
        
        //If no periodic or aperiodic tasks are unblocked then run Idle Time
        if ((p_task_processed == 0) && (a_task_processed == 0)){
            printf("Idle at Time %d\n",cur_time);
            fprintf(fptr_write,"Idle at Time %d\n",cur_time);
            prev = -1;
            idle_time++;
        }
    }

    //Check if any aperiodic tasks did not finish simulating in simulation time
    for (int i = 0; i < aperiodic; i++){
        if (aper_tasks[i].blocked == 0){
            printf("\tTask %s not completed within simulation time. Deadline %d is outside of simulation time. Requires %d more clock cycles to complete.\n",aper_tasks[i].task_name, aper_tasks[i].deadline, aper_tasks[i].task_count + 1);
            fprintf(fptr_write,"\tTask %s not completed within simulation time. Deadline %d is outside of simulation time. Requires %d more clock cycles to complete.\n",aper_tasks[i].task_name, aper_tasks[i].deadline, aper_tasks[i].task_count +1);
            //total_misses++;
        }
    }

    printf("\nRMA Scheduler Summary:\n");
    fprintf(fptr_write,"\nRMA Scheduler Summary:\n");

    for (int i = 0; i < periodic; i++){
        printf("Task %s misses: %d, preemptions: %d\n",per_tasks[i].task_name, per_tasks[i].misses, per_tasks[i].preemptions);
        fprintf(fptr_write,"Task %s misses: %d, preemptions: %d\n",per_tasks[i].task_name, per_tasks[i].misses, per_tasks[i].preemptions);
    }
    for (int i = 0; i < aperiodic; i++){
        printf("Task %s misses: %d, preemptions: %d\n",aper_tasks[i].task_name, aper_tasks[i].misses, aper_tasks[i].preemptions);
        fprintf(fptr_write,"Task %s misses: %d, preemptions: %d\n",per_tasks[i].task_name, per_tasks[i].misses, per_tasks[i].preemptions);
    }
    printf("\nRMA Total Misses: %d\n", total_misses);
    fprintf(fptr_write,"RMA Total Misses: %d\n",total_misses);
    printf("RMA Total Preemptions: %d\n", total_preemptions);
    fprintf(fptr_write,"RMA Total Preemptions: %d\n",total_preemptions);

    printf("Aperiodic Tasks Finished: %d\n", a_tasks_finished);
    //Print Average Response Time for Aperiodic Tasks that finished 
    if (a_tasks_finished != 0){
        printf("Aperiodic Average Response Time: %d\n", response_time / a_tasks_finished);
        fprintf(fptr_write,"Aperiodic Average Response Time: %d\n",response_time / a_tasks_finished);
    }
    else {
        printf("No Aperiodic Tasks Finished\n");
        fprintf(fptr_write,"No Aperiodic Tasks Finished\n");
    }
    printf("Idle Time Spent: %d\n",idle_time);
}

//Finds what task should currently be running that is highest_priority and unblocked, Works for all 3 scheduling algorithms 
int cur_task_running_RMA(struct p_task *per_tasks, struct a_task *aper_tasks, int periodic, int aperiodic){
    int highest_priority = 0;
    int cur_task = 0;
    //Find the task that should currently be running
    for (int i = 0; i < periodic+aperiodic; i++){
        if (i < periodic){
            //If the highest priority is unset, set it to first unblocked task
            if (highest_priority == 0 && per_tasks[i].blocked == 0){
                highest_priority = per_tasks[i].priority;
            }
            //If another task has a higher priority and is unblocked then make it the current highest priority for execution
            else if ((per_tasks[i].priority < highest_priority) && (per_tasks[i].blocked == 0)){
                highest_priority = per_tasks[i].priority;
            } 
        }
        else {
            //If the highest priority is unset, set it to first unblocked task
            if (highest_priority == 0 && aper_tasks[i - periodic].blocked == 0){
                highest_priority = aper_tasks[i - periodic].priority;
            }
            //If another task has a higher priority and is unblocked then make it the current highest priority for execution
            else if ((aper_tasks[i - periodic].priority < highest_priority) && (aper_tasks[i - periodic].blocked == 0)){
                highest_priority = aper_tasks[i - periodic].priority;
            } 
        }
    }

    //Find cur_task iterator that marks what the current highest priority task is that is unblocked
    for(cur_task = 0; cur_task < periodic; cur_task++){
        if (highest_priority == per_tasks[cur_task].priority){
            return cur_task;
        }
    }

    //Find cur_task iterator that marks what the current highest priority task is that is unblocked
    for(; cur_task < aperiodic+periodic; cur_task++){
        if (highest_priority == aper_tasks[cur_task - periodic].priority){
            return cur_task;
        }
    }

}

//EDF Scheduler Function
void schedule_EDF(struct p_task *per_tasks, struct a_task *aper_tasks, int sim_time, int periodic, int aperiodic, FILE *fptr_write){
    int cur_time;
    int total_misses = 0;
    int total_preemptions = 0;
    int cur_task = 0; 
    int prev = -1;
    int p_task_processed = 0;
    int a_task_processed = 0;
    int response_time = 0;
    int a_tasks_finished = 0;
    int idle_time = 0;

    printf("\nEDF Algorithm Task Schedule: ***************************************\n\n");
    fprintf(fptr_write,"\nEDF Algorithm Task Schedule: ****************************************\n\n");

    //Initialize periodic tasks
    for (int i = 0; i < periodic; i++){
        per_tasks[i].priority = 0;
        per_tasks[i].misses = 0;
        per_tasks[i].preemptions = 0;
        per_tasks[i].slack = 0;
    }
    //Initialize aperiodic tasks
    for (int i = 0; i < aperiodic; i++){
        aper_tasks[i].priority = 0;
        aper_tasks[i].misses = 0;
        aper_tasks[i].preemptions = 0;
        aper_tasks[i].blocked = 1;
        aper_tasks[i].slack = 0;
    }

    //Simulate EDF scheduler for Simulation time
    for (cur_time = 0; cur_time < sim_time; cur_time++){
        //Function calulates release times, blocking/unblocking, and whether deadlines are missed or not
        total_misses += releases_deadlines(per_tasks,aper_tasks,periodic,aperiodic,cur_time,fptr_write);

        //Find the current task that should be running that is unblocked and has the earliest deadline 
        cur_task = cur_task_running_EDF(per_tasks,aper_tasks,periodic,aperiodic);

        //Finds if a task is preempted by another task and increments the preemptions calculator
        total_preemptions += preemptions(per_tasks,aper_tasks,p_task_processed,a_task_processed,cur_task,cur_time,prev,periodic,fptr_write);

        p_task_processed = 0;
        a_task_processed = 0;
        
        //If a periodic task is available to run
        if (cur_task < periodic){
            //current task finished executing and is now in the blocked state so another task can run
            if ((per_tasks[cur_task].task_count == 0) && (per_tasks[cur_task].blocked == 0)){
                printf("Task %s finished at time %d\n",per_tasks[cur_task].task_name,cur_time);
                fprintf(fptr_write,"Task %s finished at time %d\n",per_tasks[cur_task].task_name,cur_time);
                per_tasks[cur_task].blocked = 1;
                prev = cur_task; 
                p_task_processed = 1;
            }
            //Check if the current task still needs to finish executing
            else if (per_tasks[cur_task].task_count > 0){
                //printf("Task %s, Current Time: %d, Deadline: %d\n",per_tasks[cur_task].task_name, cur_time, per_tasks[cur_task].deadline);
                //fprintf(fptr_write,"Task %s, Current Time: %d, Deadline: %d\n",per_tasks[cur_task].task_name, cur_time, per_tasks[cur_task].deadline);
                per_tasks[cur_task].task_count--;
                prev = cur_task;
                p_task_processed = 1;
            }
        }
        //If all periodic tasks are blocked check if any aperiodic tasks can run based on current highest aperiodic priority task available **********************************
        else if ((cur_task >= periodic) && (cur_task < aperiodic+periodic)){
            //Loop through all aperiodic tasks to find which is the highest priority released task available
            //current task finished executing and is now in the blocked state so another task can run
            if ((aper_tasks[cur_task - periodic].task_count == 0) && (aper_tasks[cur_task - periodic].blocked == 0)){
                printf("\tTask %s finished at %d time\n",aper_tasks[cur_task - periodic].task_name,cur_time);
                fprintf(fptr_write,"\tTask %s finished at %d time\n",aper_tasks[cur_task - periodic].task_name,cur_time);
                response_time += (cur_time - aper_tasks[cur_task - periodic].release_time);
                a_tasks_finished++;
                aper_tasks[cur_task - periodic].blocked = 1;
                prev = cur_task - periodic;
                a_task_processed = 1;
            }
            //Check if the current task still needs to finish executing
            else if (aper_tasks[cur_task - periodic].task_count > 0){
                //printf("\tTask %s, Current Time: %d, Deadline: %d\n",aper_tasks[cur_task - periodic].task_name, cur_time, aper_tasks[cur_task - periodic].deadline);
                //fprintf(fptr_write,"\tTask %s, Current Time: %d Deadline: %d\n",aper_tasks[cur_task - periodic].task_name, cur_time, aper_tasks[cur_task - periodic].deadline);
                aper_tasks[cur_task - periodic].task_count--;
                prev = cur_task - periodic;
                a_task_processed = 1;
            }
        } //******************************************************************************************************************************************************************
        
        //If no periodic or aperiodic tasks are unblocked then run Idle Time
        if ((p_task_processed == 0) && (a_task_processed == 0)){
            printf("Idle at Time %d\n",cur_time);
            fprintf(fptr_write,"Idle at Time %d\n",cur_time);
            prev = -1;
            idle_time++;
        }
    }

    //Check if any aperiodic tasks did not finish simulating in simulation time
    for (int i = 0; i < aperiodic; i++){
        if (aper_tasks[i].blocked == 0){
            printf("\tTask %s not completed within simulation time. Deadline %d is outside of simulation time. Requires %d more clock cycles to complete.\n",aper_tasks[i].task_name, aper_tasks[i].deadline, aper_tasks[i].task_count + 1);
            fprintf(fptr_write,"\tTask %s not completed within simulation time. Deadline %d is outside of simulation time. Requires %d more clock cycles to complete.\n",aper_tasks[i].task_name, aper_tasks[i].deadline, aper_tasks[i].task_count +1);
        }
    }

    printf("\nEDF Scheduler Summary:\n");
    fprintf(fptr_write,"\nEDF Scheduler Summary:\n");

    for (int i = 0; i < periodic; i++){
        printf("Task %s misses: %d, preemptions: %d\n",per_tasks[i].task_name, per_tasks[i].misses, per_tasks[i].preemptions);
        fprintf(fptr_write,"Task %s misses: %d, preemptions: %d\n",per_tasks[i].task_name, per_tasks[i].misses, per_tasks[i].preemptions);
    }
    for (int i = 0; i < aperiodic; i++){
        printf("Task %s misses: %d, preemptions: %d\n",aper_tasks[i].task_name, aper_tasks[i].misses, aper_tasks[i].preemptions);
        fprintf(fptr_write,"Task %s misses: %d, preemptions: %d\n",per_tasks[i].task_name, per_tasks[i].misses, per_tasks[i].preemptions);
    }
    printf("\nEDF Total Misses: %d\n", total_misses);
    fprintf(fptr_write,"EDF Total Misses: %d\n",total_misses);
    printf("EDF Total Preemptions: %d\n", total_preemptions);
    fprintf(fptr_write,"EDF Total Preemptions: %d\n",total_preemptions);

    //printf("Aperiodic Tasks Finished: %d\n", a_tasks_finished);
    //Print Average Response Time for Aperiodic Tasks that finished 
    if (a_tasks_finished != 0){
        printf("Aperiodic Average Response Time: %d\n", response_time / a_tasks_finished);
        fprintf(fptr_write,"Aperiodic Average Response Time: %d\n",response_time / a_tasks_finished);
    }
    else {
        printf("No Aperiodic Tasks Finished\n");
        fprintf(fptr_write,"No Aperiodic Tasks Finished\n");
    }
    printf("Idle Time Spent: %d\n",idle_time);
}

//Finds and updates the priority of the EDF scheduler each clock cycle
int cur_task_running_EDF(struct p_task *per_tasks, struct a_task *aper_tasks, int periodic, int aperiodic){
    int p_earliest = 0;
    int a_earliest = 0;
    int cur_task = 0;
    static int prev_task = 0;

    //Find first unblocked periodic task
    while((per_tasks[p_earliest].blocked == 1) && (p_earliest < periodic)){
        p_earliest++;
    }

    //Assign current task running for periodic tasks based on deadline, sooner deadline = higher priority (1 is the highest) and shorter execution time
    for (int i = p_earliest; i < periodic; i++){
        //If a task has a deadline that is earlier it becomes the highest priority task
        if ((per_tasks[i].deadline < per_tasks[p_earliest].deadline) && (per_tasks[i].blocked == 0)){
            p_earliest = i;
        }
        //If a task has the same deadline it becomes the highest priority task if it was previously executing or has the shortest ex time
        else if (per_tasks[i].deadline == per_tasks[p_earliest].deadline && per_tasks[i].blocked == 0){
            //Keep executing the same task if the shortest deadline is still the same as last clock cycle
            if (prev_task == i){
                p_earliest = i;
            }
            else if (prev_task == p_earliest){
                p_earliest = p_earliest;
            }
            else if (per_tasks[i].ex_time <= per_tasks[p_earliest].ex_time){
                p_earliest = i;
            }
        }
    }

    //Find first unblocked aperiodic task
    while((aper_tasks[a_earliest].blocked == 1) && (a_earliest < aperiodic)){
        a_earliest++;
    }
    
    //Assign current task running for aperiodic tasks based on release time, sooner deadline = higher priority (1 is the highest)
    for (int i = a_earliest; i < aperiodic; i++){
        //If a task has a deadline that is earlier it becomes the highest priority task
        if ((aper_tasks[i].deadline < aper_tasks[a_earliest].deadline) && (aper_tasks[i].blocked == 0)){
            a_earliest = i;
        }
        //If a task has the deadline it becomes the highest priority task if it was previously executing or has a shortest ex time
        else if (aper_tasks[i].deadline == aper_tasks[a_earliest].deadline && aper_tasks[i].blocked == 0){
            //Keep executing the same task if the shortest deadline is still the same as last clock cycle
            if (prev_task == i+periodic){
                a_earliest = i;
            }
            else if (prev_task == a_earliest+periodic){
                a_earliest = a_earliest;
            }
            else if (aper_tasks[i].ex_time <= aper_tasks[a_earliest].ex_time){
                a_earliest = i;
            }
        }
    }

    //If at least one periodic task and one aperiodic is unblocked
    //Test to see which lowest one of the two types should be executing
    if (p_earliest < periodic && a_earliest < aperiodic){
        if (per_tasks[p_earliest].deadline < aper_tasks[a_earliest].deadline){
            cur_task = p_earliest;
        }
        else if (per_tasks[p_earliest].deadline == aper_tasks[a_earliest].deadline){
            if (prev_task == a_earliest+periodic){
                cur_task = a_earliest+periodic;
            }
            else if (prev_task == p_earliest){
                cur_task = p_earliest;
            }
            else if (per_tasks[p_earliest].ex_time <= aper_tasks[a_earliest].ex_time){
                cur_task = p_earliest;
            }
        }
        else {
            cur_task = a_earliest+periodic;
        }
    }
    //If only periodic tasks are unblocked run the periodic task with the earliest deadline
    else if (p_earliest < periodic && a_earliest >= aperiodic){
        cur_task = p_earliest;
    }
    //If only aperiodic tasks are unblocked run the aperiodic task with the earliest deadline
    else if (p_earliest >= periodic && a_earliest < aperiodic){
        cur_task = a_earliest+periodic;
    }
    //If no tasks are unblocked then IDLE
    else{
        cur_task = periodic + aperiodic;
    }
    prev_task = cur_task;

    return cur_task;
}

//LST Scheduler Function 
void schedule_LST(struct p_task *per_tasks, struct a_task *aper_tasks, int sim_time, int periodic, int aperiodic, FILE *fptr_write){
    int cur_time;
    int total_misses = 0;
    int total_preemptions = 0;
    int cur_task = 0; 
    int prev = 0;
    int p_task_processed = 0;
    int a_task_processed = 0;
    int response_time = 0;
    int a_tasks_finished = 0;
    int idle_time = 0;

    printf("\nLST Algorithm Task Schedule: **************************************\n\n");
    fprintf(fptr_write,"\nLST Algorithm Task Schedule: **************************************\n\n");

    //Initialize periodic tasks
    for (int i = 0; i < periodic; i++){
        per_tasks[i].priority = 0;
        per_tasks[i].misses = 0;
        per_tasks[i].preemptions = 0;
        per_tasks[i].slack = 0;
    }
    //Initialize aperiodic tasks
    for (int i = 0; i < aperiodic; i++){
        aper_tasks[i].priority = 0;
        aper_tasks[i].misses = 0;
        aper_tasks[i].preemptions = 0;
        aper_tasks[i].blocked = 1;
        aper_tasks[i].slack = 0;
    }

    //Simulate LST scheduler for Simulation time
    for (cur_time = 0; cur_time < sim_time; cur_time++){
        //Function calulates release times, blocking/unblocking, and whether deadlines are missed or not
        total_misses += releases_deadlines(per_tasks,aper_tasks,periodic,aperiodic,cur_time,fptr_write);

        //Find the current task that should be running that is unblocked and has the earliest deadline 
        cur_task = cur_task_running_LST(per_tasks,aper_tasks,periodic,aperiodic,cur_time);

        //Finds if a task is preempted by another task and increments the preemptions calculator
        total_preemptions += preemptions(per_tasks,aper_tasks,p_task_processed,a_task_processed,cur_task,cur_time,prev,periodic,fptr_write);

        p_task_processed = 0;
        a_task_processed = 0;

        //If a periodic task is available to run
        if (cur_task < periodic){
            //current task finished executing and is now in the blocked state so another task can run
            if ((per_tasks[cur_task].task_count == 0) && (per_tasks[cur_task].blocked == 0)){
                printf("Task %s finished at time %d\n",per_tasks[cur_task].task_name,cur_time);
                fprintf(fptr_write,"Task %s finished at time %d\n",per_tasks[cur_task].task_name,cur_time);
                per_tasks[cur_task].blocked = 1;
                per_tasks[cur_task].slack = 0;
                prev = cur_task; 
                p_task_processed = 1;
                //Recalculate the slack for all tasks when a task finishes or releases
                for (int i = 0; i < periodic; i++){
                    //Calculate each task's current slack available based on S = (D - R) - C
                    if(per_tasks[i].blocked == 0){
                        per_tasks[i].slack = per_tasks[i].deadline - cur_time - per_tasks[i].task_count;
                    }
                }
                for (int i = 0; i < aperiodic; i++){
                    //Calculate each task's current slack available based on S = (D - R) - C
                    if(aper_tasks[i].blocked == 0){
                        aper_tasks[i].slack = aper_tasks[i].deadline - cur_time - aper_tasks[i].task_count;
                    }
                }
            }
            //Check if the current task still needs to finish executing
            else if (per_tasks[cur_task].task_count > 0){
                //printf("Task %s, Current Time: %d, Task Count: %d, Slack: %d\n",per_tasks[cur_task].task_name, cur_time, per_tasks[cur_task].task_count, per_tasks[cur_task].slack);
                //fprintf(fptr_write,"Task %s, Current Time: %d, Slack: %d\n",per_tasks[cur_task].task_name, cur_time, per_tasks[cur_task].slack);
                per_tasks[cur_task].task_count--;
                prev = cur_task;
                p_task_processed = 1;
            }
        }
        //If all periodic tasks are blocked check if any aperiodic tasks can run based on current highest aperiodic priority task available **********************************
        else if ((cur_task >= periodic) && (cur_task < aperiodic+periodic)){
            //Loop through all aperiodic tasks to find which is the highest priority released task available
            //current task finished executing and is now in the blocked state so another task can run
            if ((aper_tasks[cur_task - periodic].task_count == 0) && (aper_tasks[cur_task - periodic].blocked == 0)){
                printf("\tTask %s finished at %d time\n",aper_tasks[cur_task - periodic].task_name,cur_time);
                fprintf(fptr_write,"\tTask %s finished at %d time\n",aper_tasks[cur_task - periodic].task_name,cur_time);
                response_time += (cur_time - aper_tasks[cur_task - periodic].release_time);
                a_tasks_finished++;
                aper_tasks[cur_task - periodic].blocked = 1;
                aper_tasks[cur_task - periodic].slack = 0;
                prev = cur_task - periodic;
                a_task_processed = 1;
                //Recalculate the slack for all tasks when a task finishes or releases
                for (int i = 0; i < periodic; i++){
                    //Calculate each task's current slack available based on S = (D - R) - C
                    if(per_tasks[i].blocked == 0){
                        per_tasks[i].slack = per_tasks[i].deadline - cur_time - per_tasks[i].task_count;
                    }
                }
                for (int i = 0; i < aperiodic; i++){
                    //Calculate each task's current slack available based on S = (D - R) - C
                    if(aper_tasks[i].blocked == 0){
                        aper_tasks[i].slack = aper_tasks[i].deadline - cur_time - aper_tasks[i].task_count;
                    }
                }
            }
            //Check if the current task still needs to finish executing
            else if (aper_tasks[cur_task - periodic].task_count > 0){
                //printf("\tTask %s, Current Time: %d, Task Count: %d, Slack: %d\n",aper_tasks[cur_task - periodic].task_name, cur_time, aper_tasks[cur_task - periodic].task_count, aper_tasks[cur_task - periodic].slack);
                //fprintf(fptr_write,"\tTask %s, Current Time: %d Slack: %d\n",aper_tasks[cur_task - periodic].task_name, cur_time, aper_tasks[cur_task - periodic].slack);
                aper_tasks[cur_task - periodic].task_count--;
                prev = cur_task - periodic;
                a_task_processed = 1;
            }
        } //******************************************************************************************************************************************************************
        
        //If no periodic or aperiodic tasks are unblocked then run Idle Time
        if ((p_task_processed == 0) && (a_task_processed == 0)){
            printf("Idle at Time %d\n",cur_time);
            fprintf(fptr_write,"Idle at Time %d\n",cur_time);
            idle_time++;
        }
    }

    //Check if any aperiodic tasks did not finish simulating in simulation time
    for (int i = 0; i < aperiodic; i++){
        if (aper_tasks[i].blocked == 0){
            printf("\tTask %s not completed within simulation time. Deadline %d is outside of simulation time. Requires %d more clock cycles to complete.\n",aper_tasks[i].task_name, aper_tasks[i].deadline, aper_tasks[i].task_count + 1);
            fprintf(fptr_write,"\tTask %s not completed within simulation time. Deadline %d is outside of simulation time. Requires %d more clock cycles to complete.\n",aper_tasks[i].task_name, aper_tasks[i].deadline, aper_tasks[i].task_count +1);
        }
    }

    printf("\nLST Scheduler Summary:\n");
    fprintf(fptr_write,"\nLST Scheduler Summary:\n");

    for (int i = 0; i < periodic; i++){
        printf("Task %s misses: %d, preemptions: %d\n",per_tasks[i].task_name, per_tasks[i].misses, per_tasks[i].preemptions);
        fprintf(fptr_write,"Task %s misses: %d, preemptions: %d\n",per_tasks[i].task_name, per_tasks[i].misses, per_tasks[i].preemptions);
    }
    for (int i = 0; i < aperiodic; i++){
        printf("Task %s misses: %d, preemptions: %d\n",aper_tasks[i].task_name, aper_tasks[i].misses, aper_tasks[i].preemptions);
        fprintf(fptr_write,"Task %s misses: %d, preemptions: %d\n",per_tasks[i].task_name, per_tasks[i].misses, per_tasks[i].preemptions);
    }
    printf("\nLST Total Misses: %d\n", total_misses);
    fprintf(fptr_write,"LST Total Misses: %d\n",total_misses);
    printf("LST Total Preemptions: %d\n", total_preemptions);
    fprintf(fptr_write,"LST Total Preemptions: %d\n",total_preemptions);

    //printf("Aperiodic Tasks Finished: %d\n", a_tasks_finished);
    //Print Average Response Time for Aperiodic Tasks that finished 
    if (a_tasks_finished != 0){
        printf("Aperiodic Average Response Time: %d\n", response_time / a_tasks_finished);
        fprintf(fptr_write,"Aperiodic Average Response Time: %d\n",response_time / a_tasks_finished);
    }
    else {
        printf("No Aperiodic Tasks Finished\n");
        fprintf(fptr_write,"No Aperiodic Tasks Finished\n");
    }
    printf("Idle Time Spent: %d\n",idle_time);
}

//Finds and updates the priority of the LST scheduler each clock cycle
int cur_task_running_LST(struct p_task *per_tasks, struct a_task *aper_tasks, int periodic, int aperiodic, int cur_time){
    int p_slack = 0;
    int a_slack = 0;
    int cur_task = 0;
    static int prev_task = 0;

    //Find first unblocked periodic task
    while((per_tasks[p_slack].blocked == 1) && (p_slack < periodic) && (per_tasks[p_slack].slack >= 0)){
        p_slack++;
    }

    //Assign current task running for periodic tasks based on least slack time, less slack time = higher priority (1 is the highest) and shorter execution time
    for (int i = p_slack; i < periodic; i++){
        //If a task has less slack it becomes the highest priority task
        if ((per_tasks[i].slack < per_tasks[p_slack].slack) && (per_tasks[i].blocked == 0) && (per_tasks[i].slack >= 0)){
            p_slack = i;
        }
        //The least slack time task becomes the highest priority task if it was previously executing or it has the shortest execution time
        else if (per_tasks[i].slack == per_tasks[p_slack].slack && per_tasks[i].blocked == 0){
            //Keep executing the same task if it still has the least slack time as last clock cycle
            if (prev_task == i){
                p_slack = i;
            }
            else if (prev_task == p_slack){
                p_slack = p_slack;
            }
            else if (per_tasks[i].ex_time <= per_tasks[p_slack].ex_time){
                p_slack = i;
            }
        }
    }

    //Find first unblocked aperiodic task
    while((aper_tasks[a_slack].blocked == 1) && (a_slack < aperiodic) && (aper_tasks[a_slack].slack >= 0)){
        a_slack++;
    }
    
    //Assign current task running for aperiodic tasks based on least slack time = higher priority (1 is the highest)
    for (int i = a_slack; i < aperiodic; i++){
        //If a task has the least slack time it becomes the highest priority task
        if ((aper_tasks[i].slack < aper_tasks[a_slack].slack) && (aper_tasks[i].blocked == 0) && (aper_tasks[i].slack >= 0)){
            a_slack = i;
        }
        //If a task has the least slack time it becomes the highest priority task if it was previously executing or has the shortest ex time
        else if (aper_tasks[i].slack == aper_tasks[a_slack].slack && aper_tasks[i].blocked == 0){
            //Keep executing the same task if still has the least slack time
            if (prev_task == i+periodic){
                a_slack = i;
            }
            else if (prev_task == a_slack+periodic){
                a_slack = a_slack;
            }
            else if (aper_tasks[i].ex_time <= aper_tasks[a_slack].ex_time){
                a_slack = i;
            }
        }
    }

    //If at least one periodic task and one aperiodic is unblocked
    //Test to see which lowest one of the two types should be executing
    if (p_slack < periodic && a_slack < aperiodic){
        if (per_tasks[p_slack].slack < aper_tasks[a_slack].slack && (per_tasks[p_slack].slack >= 0)){
            cur_task = p_slack;
        }
        else if (per_tasks[p_slack].slack == aper_tasks[a_slack].slack){
            if (prev_task == a_slack+periodic){
                cur_task = a_slack+periodic;
            }
            else if (prev_task == p_slack){
                cur_task = p_slack;
            }
            else if (per_tasks[p_slack].ex_time <= aper_tasks[a_slack].ex_time){
                cur_task = p_slack;
            }
        }
        else if (aper_tasks[a_slack].slack >= 0){
            cur_task = a_slack+periodic;
        }
        //If no tasks are unblocked or have slack greater than 0 then IDLE 
        else {
            cur_task = periodic + aperiodic;
        }
    }
    //If only periodic tasks are unblocked run the periodic task with the least slack time
    else if (p_slack < periodic && a_slack >= aperiodic && (per_tasks[p_slack].slack >= 0)){
        cur_task = p_slack;
    }
    //If only aperiodic tasks are unblocked run the aperiodic task with the least slack time
    else if (p_slack >= periodic && a_slack < aperiodic && (aper_tasks[a_slack].slack >= 0)){
        cur_task = a_slack+periodic;
    }
    //If no tasks are unblocked or have slack greater than 0 then IDLE 
    else{
        cur_task = periodic + aperiodic;
    }
    prev_task = cur_task;
    return cur_task;
}