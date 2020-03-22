#include "main_memory.h"

int check_eof(FILE* process[NUM_PROCESSES]){
    int flag = 0;
    for(int j=0; j<NUM_PROCESSES; j++){
        if(!feof(process[j]))
            flag = 1;        
    }

    return (flag == 0);
}

int main(int argc, char* argv[]){
    //Initialize main memory;
    main_memory_struct *main_memory = mm_initialize_mm();
    mm_initialize_kernel(&main_memory);

    FILE *process[NUM_PROCESSES];
    int pid[NUM_PROCESSES];
    for(int i=0; i<NUM_PROCESSES; i++){
        process[i] = fopen(argv[i+1], "r");
        pid[i] = (i+1)*100;
        if(process[i] == NULL){
            printf("Error: Opening file %d\n", i);
            return 0;
        }
    }

    for(int i=0; i<NUM_PROCESSES; i++){
        mm_initialize_page_table(pid[i], &main_memory);
    }

    int page_hit[NUM_PROCESSES] = {0};
    int page_fault[NUM_PROCESSES] = {0};
    int total_pages[NUM_PROCESSES] = {0};
    int terminate_process[NUM_PROCESSES] = {0};
    int prefetch[NUM_PROCESSES] = {0};


    int i=0;
    int count = 1;
    while(1){
        if(check_eof(process))
            break;

        if(terminate_process[i] == 1){
            i = (i+1)%5;
            continue;
        }
        
        if(prefetch[i] == 0){
            prefetch[i] = 1;
            mm_prefetch_pages(pid[i], process[i], &main_memory);
        }

        printf("Count = %d\n", count);

        if(feof(process[i])){
            terminate_process[i] = 1;
            mm_terminate_process(pid[i], &main_memory);
            printf("Process %d terminated\n", i);
            sleep(5);
            i = (i+1)%5;
            continue;
        }

        int n;
        fscanf(process[i], "%x", &n);
        total_pages[i]++;
        printf("Process %d, address = %x\n", i, n);
        logical_address la = mm_convert(n);
        int frame_no;
        if((frame_no = mm_search_page_table(la, pid[i], &main_memory)) == -1){
            page_fault[i]++;
            i = (i+1)%NUM_PROCESSES;
        }
        else{
            page_hit[i]++;
        }
        count++;
    }

    double page_fault_rate[NUM_PROCESSES];
    for(int i=0; i<NUM_PROCESSES; i++){
        page_fault_rate[i] = ((double)page_fault[i]/(double)total_pages[i])*100;
    }

    printf("\nPage faults rate of each process:\n");
    for(int i=0; i<NUM_PROCESSES; i++){
        printf("Process %d : page fault rate = %f\n", i+1, page_fault_rate[i]);
    }
    printf("\n");


    // FILE* process1, *process2;
    // process1 = fopen("APSI.txt", "r");
    // process2 = fopen("CC1.txt", "r");

    // int pid1 = 400;
    // mm_initialize_page_table(pid1, &main_memory);

    // int pid2 = 500;
    // mm_initialize_page_table(pid2, &main_memory);

    // printf("\n\n\n-----------------------Process 1----------------------\n\n\n");
    // for(int i=0; i<20000; i++){
    //     int n;
    //     fscanf(process1, "%x", &n); 
    //     printf("%d address = %x\n", i, n);
    //     logical_address la = mm_convert(n);
    //     int frame_no;
    //     while((frame_no = mm_search_page_table(la, pid1, &main_memory)) == -1);
    //     printf("Frame no = %d\n\n", frame_no);

    // }

// //    mm_print_frame_table(main_memory->frame_table);

//     printf("\n\n\n-----------------------Process 2----------------------\n\n\n");
//     for (int i=0; i<20000; i++)
//     {
//         int n;
//         fscanf(process2, "%x", &n);
//         printf("%x\n", n);
//         logical_address la = mm_convert(n);
//         int frame_no;
//         while((frame_no = mm_search_page_table(la, pid2, &kernel, &main_memory)) == -1);
//         printf("Frame no = %d\n\n", frame_no);
//     }

    // printf("\n\n\n-----------------------Process 3----------------------\n\n\n");
    // for(int i=0; i<20000; i++){
    //     int n;
    //     fscanf(process3, "%x", &n); 
    //     printf("%d address = %x\n", i, n);
    //     logical_address la = mm_convert(n);
    //     int frame_no;
    //     while((frame_no = mm_search_page_table(la, pid3, &kernel, &main_memory)) == -1);
    //     printf("Frame no = %d\n\n", frame_no);

    // }

    // printf("\n\n\n-----------------------Process 1----------------------\n\n\n");
    // for(int i=0; i<20000; i++){
    //     int n;
    //     fscanf(process1, "%x", &n); 
    //     printf("%d address = %x\n", i, n);
    //     logical_address la = mm_convert(n);
    //     int frame_no;
    //     while((frame_no = mm_search_page_table(la, pid1, &kernel, &main_memory)) == -1);
    //     printf("Frame no = %d\n\n", frame_no);

    // }

    // frame_table_struct frame_table = main_memory->frame_table;
    // mm_print_frame_table(frame_table);


    

}