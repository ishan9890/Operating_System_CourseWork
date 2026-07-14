#include <stdio.h>

#define NUM_FRAMES 3
#define REF_LENGTH 13

#define PAGE_SIZE_KB 4
#define PROCESS_SIZE_KB 50

int calculate_pages_needed(int process_size_kb, int page_size_kb){
    return (process_size_kb + page_size_kb - 1) / page_size_kb;
}

void print_memory_config(){
    int pages_needed = calculate_pages_needed(PROCESS_SIZE_KB, PAGE_SIZE_KB);
    int physical_memory_size_kb = NUM_FRAMES * PAGE_SIZE_KB;
    printf("Memory Configuration:\n");
    printf("Page Size: %d KB\n", PAGE_SIZE_KB);
    printf("Process Size: %d KB\n", PROCESS_SIZE_KB);
    printf("Pages Needed by Processes: %d (ceil(%d / %d))\n", pages_needed, PROCESS_SIZE_KB, PAGE_SIZE_KB);
    printf("Physical Memory Available: %d KB (%d frames x %d KB)\n", physical_memory_size_kb, NUM_FRAMES, PAGE_SIZE_KB);

    if (pages_needed > NUM_FRAMES) {
        printf("Note: Process requires more pages (%d) than available frames (%d).\n",
                pages_needed, NUM_FRAMES);
        printf("This means not all pages can be resident simultaneously,\n");
        printf("making page replacement necessary — this is exactly what\n");
        printf("the FIFO/LRU simulations below demonstrate.\n");
    }
    printf("\n");
}


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

int lru_frames[NUM_FRAMES];
int last_used[NUM_FRAMES];

int is_page_in_lru_memory(int page){
    for (int i = 0; i < NUM_FRAMES; i++){
        if (lru_frames[i] == page){
            return i;
        }
    }
    return -1; 
}

int find_lru_victim(){
    int victim = 0;
    for(int i = 1; i<NUM_FRAMES; i++){
        if(last_used[i] < last_used[victim]){
            victim = i;
        }
    }
    return victim;
}

void print_lru_frames(){
    printf("[ ");
    for (int i = 0; i< NUM_FRAMES; i++){
        if (lru_frames[i] == -1){
            printf("_ ");
        } else {
            printf("%d ", lru_frames[i]);
        }
    }
    printf("]");
}

void run_lru(int ref_string[], int n){
    int hits = 0, faults = 0;
    int clock = 0;

    for (int i = 0; i<NUM_FRAMES; i++){
        lru_frames[i] = -1;
        last_used[i] = -1;
    }

    printf("LRU page replacement \n");
    for (int i = 0; i<n; i++){
        int page = ref_string[i];
        printf("Request page %d:", page);

        int found_index = is_page_in_lru_memory(page);
        if (found_index != -1){
            hits++;
            last_used[found_index] = clock;
            printf("Hit! ");
        } else {
            faults++;
            int victim = -1;
            for (int j = 0; j<NUM_FRAMES; j++){
                if (lru_frames[j] == -1){
                    victim = j;
                    break;
                }
            }
            if (victim == -1){
                victim = find_lru_victim();
            }
            lru_frames[victim] = page;
            last_used[victim] = clock;
            printf("Fault! ");
        } 
        
        print_lru_frames();
        printf("\n");
        clock++;
    }
    printf("\nTotal Hits: %d\n", hits);
    printf("Total Faults: %d\n", faults);
    printf("Hit Ratio: %.2f%%\n", (float)hits / n * 100);
    printf("Miss Ratio: %.2f%%\n", (float)faults / n * 100);
}
int main(){
    int ref_string[REF_LENGTH] = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2};

    print_memory_config();
    run_fifo(ref_string, REF_LENGTH);
    run_lru(ref_string, REF_LENGTH);
    return 0;
}