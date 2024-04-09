//
// Created by Yujia Zhu on 4/5/24.
//

#include <cstring>
#include "Process.h"

int Process::id = 0;

Process::Process() {
    this->pid = 0;
    this->vma = vector<vma_t>(0);
    this->page_table = vector<pte_t>(0);
    memset(&this->pstats, 0, sizeof(this->pstats));
}

Process::Process(int num_vpage) {
    this->pid = id++;
    this->vma = vector<vma_t>(0);
    this->page_table = vector<pte_t>(num_vpage);
    memset(&this->pstats, 0, sizeof(this->pstats));
}

void Process::updateVMA(int start_vpage, int end_vpage, int write_protected, int file_mapped) {
    vma_t VMA;
    VMA.start_vpage = start_vpage;
    VMA.end_vpage = end_vpage;
    VMA.write_protected = write_protected;
    VMA.file_mapped = file_mapped;
    this->vma.push_back(VMA);
}