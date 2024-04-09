//
// Created by Yujia Zhu on 4/5/24.
//

#ifndef LAB3_PROCESS_H
#define LAB3_PROCESS_H

#include <vector>
using namespace std;

typedef struct {
    // Required bits
    unsigned int present: 1;
    unsigned int referenced: 1;
    unsigned int modified: 1;
    unsigned int write_protect: 1;
    unsigned int paged_out: 1;
    unsigned int frame: 7;
    // Self defined bits
    unsigned int accessed: 1;
    unsigned int file_mapped: 1;
    unsigned int updated: 1;
    unsigned int temp: 17;
} pte_t;

typedef struct {
    int start_vpage, end_vpage, write_protected, file_mapped;
} vma_t;

typedef struct {
    unsigned long UNMAP, MAP, IN, OUT, FIN, FOUT, ZERO, SEGV, SEGPROT;
} stats;

class Process {
    static int id;
public:
    int pid;
    vector<pte_t> page_table;
    vector<vma_t> vma;
    stats pstats;

    Process();
    Process(int num_vpage);
    void updateVMA(int start_vpage, int end_vpage, int write_protected, int file_mapped);
};


#endif //LAB3_PROCESS_H
