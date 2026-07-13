#include <stdio.h>

#define NUM_FRAMES 3
#define REF_LENGTH 13

int frames[NUM_FRAMES];

int is_page_in_memory(int page){
    for (int i = 0; i < NUM_FRAMES; i++){
        if (frames[i] == page){
            return 1; // Page is in memory
        }
    }
    return 0; // Page is not in memory
}

void print_frames(){
    printf("[ ");
    for (int i = 0; i < NUM_FRAMES; i++){
        if (frames[i] == -1){
            printf("_ ");
        } else {
            printf("%d ", frames[i]);
        }
    }
    printf("]");
}

void run_fifo(int ref_string[], int n){
    int hits = 0, faults = 0;
    int oldest_index = 0;

    for (int i = 0; i<NUM_FRAMES; i++){
        frames[i] = -1;
    }

    printf("FIFO page replacement \n");

    for (int i = 0; i <n; i++){
        int page = ref_string[i];
        printf("Request page %d: ", page);

        if (is_page_in_memory(page)){
            hits++;
            printf("Hit! ");
        } else {
            faults++;
            frames[oldest_index] = page;
            oldest_index = (oldest_index + 1) % NUM_FRAMES;
            printf("Fault! ");
        }
        print_frames();
        printf("\n");
    }

    printf("\nTotal Hits: %d\n", hits);
    printf("Total Faults: %d\n", faults);
    printf("Hit Ratio: %.2f%%\n", (float)hits / n * 100);
    printf("Miss Ratio: %.2f%%\n", (float)faults / n * 100);
}

int main(){
    int ref_string[REF_LENGTH] = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2};
    run_fifo(ref_string, REF_LENGTH);
    return 0;
}