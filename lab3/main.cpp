#include <iostream>
#include <cstdio>
#include <fstream>
#include <string>
#include <deque>
#include <unistd.h>
#include "Process.h"
#include "Pager.h"
using namespace std;

typedef enum {T_UNMAP, T_MAP, T_IN, T_OUT, T_FIN, T_FOUT, T_ZERO, T_SEGV, T_SEGPROT} pop_type;
const int MAX_VPAGES = 64;
int MAX_FRAMES = 128, vpage, num_rands;
unsigned long inst_count = 0, ctx_switches = 0, process_exits = 0;
unsigned long long cost = 0;
char operation, algo;
char *options;
bool print_O = false, print_P = false, print_F = false, print_S = false;
bool print_x = false, print_y = false, print_f = false, print_a = false;

string filename = "../lab3_assign/in11", rfilename = "../lab3_assign/rfile", line;
ifstream file;
vector<Process*> process_list;
vector<frame_t> frame_table;
vector<int> randvals;
deque<frame_t*> free_frame_list;
Process* current_process;
Pager* THE_PAGER;

void read_randfile(const string& rfile);
void initialize();
void simulation();
void print_istr(Process* proc);
void print_frame_table();
void print_current_page_table(Process* proc);
void print_all_page_table();
void print_summary();
void read_process();
bool get_next_instruction(char &op, int &vp);
void populate(Process* proc, pop_type type, int para1, int para2);
void update_pte(Process* proc, pte_t* pte, int vpage);
void unmap_pte(Process* proc, pte_t* pte, frame_t* frame);
void map_pte(Process* proc, pte_t* pte, frame_t* frame, int vpage);
frame_t* allocate_frame_from_free_list();
frame_t* get_frame();


int main(int argc, char *argv[]) {
    int c;
    while ((c = getopt (argc, argv, "f:a:o:")) != -1) {
        switch (c) {
            case 'f':
                sscanf(optarg, "%d", &MAX_FRAMES); break;
            case 'a':
                sscanf(optarg, "%c", &algo); break;
            case 'o':
                options = optarg; break;
            default:
                break;
        }
    }
    if (MAX_FRAMES < 1) {
        fprintf (stderr, "Really funny .. you need at least one frame\n");
        return 1;
    }
    else if (MAX_FRAMES > 128) {
        fprintf (stderr, "Sorry max frames supported = 128\n");
        return 1;
    }
    if (algo != 'f' && algo != 'r' && algo != 'c' && algo != 'e' && algo != 'a' && algo != 'w') {
        fprintf (stderr, "Unknown Replacement Algorithm: <%c>\n", algo);
        return 1;
    }
    for (int i=0; options[i] != '\0'; i++) {
        if (options[i] == 'O') print_O = true;
        if (options[i] == 'P') print_P = true;
        if (options[i] == 'F') print_F = true;
        if (options[i] == 'S') print_S = true;
        if (options[i] == 'x') print_x = true;
        if (options[i] == 'y') print_y = true;
        if (options[i] == 'f') print_f = true;
        if (options[i] == 'a') print_a = true;
    }
    if (optind == argc) {
        fprintf(stderr, "Missing filename and randfile.\n");
        return 1;
    }
    else if (optind == argc-1) {
        fprintf(stderr, "Missing randfile.\n");
        return 1;
    }
    else {
        filename = argv[optind];
        rfilename = argv[optind + 1];
    }

    read_randfile(rfilename);
    file.open(filename.c_str());
    initialize();
    simulation();
    if (print_P) print_all_page_table();
    if (print_F) print_frame_table();
    if (print_S) print_summary();
    return 0;
}


void initialize() {
    read_process();
    frame_table = vector<frame_t>(MAX_FRAMES);
    for (int i=0; i<MAX_FRAMES; i++) {
        frame_table[i].id = i;
        free_frame_list.push_back(&frame_table[i]);
    }
    switch (algo) {
        case 'f':
            THE_PAGER = new FIFO(&frame_table, print_a); break;
        case 'r':
            THE_PAGER = new Random(&frame_table, print_a, &randvals); break;
        case 'c':
            THE_PAGER = new Clock(&frame_table, print_a); break;
        case 'e':
            THE_PAGER = new NRU(&frame_table, print_a); break;
        case 'a':
            THE_PAGER = new Aging(&frame_table, print_a); break;
        case 'w':
            THE_PAGER = new WorkingSet(&frame_table, print_a); break;
    }
    current_process = 0;
}


void simulation() {
    while (get_next_instruction(operation, vpage)) {
        inst_count++;
        THE_PAGER->NRU_cnt++;
        THE_PAGER->instr_cnt++;
        if (print_O) printf("%lu: ==> %c %d\n", inst_count-1, operation, vpage);
        // context switch operation
        if (operation == 'c') {
            cost+=130;
            ctx_switches++;
            current_process = process_list[vpage];
            continue;
        }
        // exit operation
        if (operation == 'e') {
            cost+=1230;
            process_exits++;
            if (print_O) printf("EXIT current process %d\n", current_process->pid);
            // UNMAP pages
            for (int i=0; i<MAX_VPAGES; i++) {
                pte_t* temp_pte = &current_process->page_table[i];
                if (temp_pte->present) {
                    frame_t* temp_frame = &frame_table[temp_pte->frame];
                    unmap_pte(current_process,temp_pte, temp_frame);
                    free_frame_list.push_back(temp_frame);
                    // FOUT modified and file mapped pages
                    if (temp_pte->modified && temp_pte->file_mapped)
                        populate(current_process, T_FOUT, 0, 0);
                }
                temp_pte->paged_out = 0;
            }
            current_process = 0;
            continue;
        }
        // operation for read and write
        pte_t* pte = &current_process->page_table[vpage];
        if (! pte->present) {
            if (! pte->updated) update_pte(current_process, pte, vpage);
            if (! pte->accessed) {
                cost+=1;
                populate(current_process, T_SEGV, 0, 0);
                continue;
            }
            // select a victim frame
            frame_t *newframe = get_frame();
            // UNMAP from current user
            if (newframe->used) {
                Process* used_proc = process_list[newframe->pid];
                pte_t* prev_pte = &used_proc->page_table[newframe->vpage];
                unmap_pte(used_proc, prev_pte, newframe);
                // page OUT or FOUT modified and file mapped pages
                if (prev_pte->modified) {
                    if (prev_pte->file_mapped) populate(used_proc, T_FOUT, 0, 0);
                    else {
                        prev_pte->paged_out = 1;
                        populate(used_proc, T_OUT, 0, 0);
                    }
                }
            }
            // page IN or FIN current PTE
            if (pte->paged_out) populate(current_process, T_IN, 0, 0);
            else {
                if (pte->file_mapped) populate(current_process, T_FIN, 0, 0);
                else populate(current_process, T_ZERO, 0, 0);
            }
            // MAP current PTE to newframe
            pte->present = 0, pte->referenced = 0, pte->modified = 0, pte->frame = 0;
            map_pte(current_process, pte, newframe, vpage);
        }
        // read operation
        if (operation == 'r') {
            cost+=1;
            pte->referenced = 1;
            print_istr(current_process);
        }
        // write operation
        if (operation == 'w') {
            cost+=1;
            pte->referenced = 1;
            if (pte->write_protect) {
                populate(current_process, T_SEGPROT, 0, 0);
                print_istr(current_process);
                continue;
            }
            else pte->modified = 1;
            print_istr(current_process);
        }
    }
    file.close();
}


void read_randfile(const string& rfile) {
    freopen(rfile.c_str(), "r", stdin);
    scanf("%d", &num_rands);
    int rand;
    for (int i=0; i<num_rands; i++) {
        scanf("%d", &rand);
        randvals.push_back(rand);
    }
    fclose(stdin);
}


void update_pte(Process* proc, pte_t* pte, int vpage) {
    for (int i=0; i<proc->vma.size(); i++) {
        vma_t vma = proc->vma[i];
        if (vma.start_vpage <= vpage && vpage <= vma.end_vpage) {
            pte->accessed = 1;
            pte->write_protect = vma.write_protected;
            pte->file_mapped = vma.file_mapped;
            break;
        }
    }
    pte->updated = 1;
}


void unmap_pte(Process* proc, pte_t* pte, frame_t* frame) {
    populate(proc, T_UNMAP, frame->pid, frame->vpage);
    pte->frame = 0, pte->present = 0;
    frame->pid = 0, frame->vpage = 0, frame->used = 0, frame->pte = 0;
}


void map_pte(Process* proc, pte_t* pte, frame_t* frame, int vpage) {
    pte->frame = frame->id, pte->present = 1;
    frame->pid = proc->pid, frame->vpage = vpage, frame->used = 1, frame->pte = pte;
    frame->age = 0, frame->time = THE_PAGER->instr_cnt;
    populate(proc, T_MAP, frame->id, 0);
}


frame_t* allocate_frame_from_free_list() {
    frame_t* frame = 0;
    if (! free_frame_list.empty()) {
        frame = free_frame_list.front();
        free_frame_list.pop_front();
    }
    return frame;
}


frame_t* get_frame() {
    frame_t* frame = allocate_frame_from_free_list();
    if (!frame) frame = THE_PAGER->select_victim_frame();
    return frame;
}


void populate(Process* proc, pop_type type, int para1, int para2) {
    switch (type) {
        case T_MAP:
            proc->pstats.MAP++;
            cost+=350;
            if (print_O) printf(" MAP %d\n", para1);
            break;
        case T_UNMAP:
            proc->pstats.UNMAP++;
            cost+=410;
            if (print_O) printf(" UNMAP %d:%d\n", para1, para2);
            break;
        case T_IN:
            proc->pstats.IN++;
            cost+=3200;
            if (print_O) printf(" IN\n");
            break;
        case T_OUT:
            proc->pstats.OUT++;
            cost+=2750;
            if (print_O) printf(" OUT\n");
            break;
        case T_FIN:
            proc->pstats.FIN++;
            cost+=2350;
            if (print_O) printf(" FIN\n");
            break;
        case T_FOUT:
            proc->pstats.FOUT++;
            cost+=2800;
            if (print_O) printf(" FOUT\n");
            break;
        case T_ZERO:
            proc->pstats.ZERO++;
            cost+=150;
            if (print_O) printf(" ZERO\n");
            break;
        case T_SEGV:
            proc->pstats.SEGV++;
            cost+=440;
            if (print_O) printf(" SEGV\n");
            break;
        case T_SEGPROT:
            proc->pstats.SEGPROT++;
            cost+=410;
            if (print_O) printf(" SEGPROT\n");
            break;
    }
}


void print_istr(Process* proc) {
    if (print_y) print_all_page_table();
    else if (print_x) print_current_page_table(proc);
    if (print_f) print_frame_table();
}


void print_frame_table() {
    printf("FT:");
    for (int i=0; i<MAX_FRAMES; i++)
        if (frame_table[i].used == 0) printf(" *");
        else printf(" %d:%d", frame_table[i].pid, frame_table[i].vpage);
    printf("\n");
}


void print_current_page_table(Process* proc) {
    printf("PT[%d]:", proc->pid);
    for (int i=0; i<MAX_VPAGES; i++) {
        pte_t *pte = &proc->page_table[i];
        if (pte->present) {
            printf(" %d:", i);
            if (pte->referenced) printf("R"); else printf("-");
            if (pte->modified) printf("M"); else printf("-");
            if (pte->paged_out) printf("S"); else printf("-");
        }
        else {
            if (pte->paged_out) printf(" #");
            else printf(" *");
        }
    }
    printf("\n");
}


void print_all_page_table() {
    for (int i=0; i<process_list.size(); i++)
        print_current_page_table(process_list[i]);
}


void print_summary() {
    for (int i=0; i<process_list.size(); i++) {
        Process* proc = process_list[i];
        stats pstats = proc->pstats;
        printf("PROC[%d]: U=%lu M=%lu I=%lu O=%lu FI=%lu FO=%lu Z=%lu SV=%lu SP=%lu\n",
               proc->pid,
               pstats.UNMAP, pstats.MAP, pstats.IN, pstats.OUT,
               pstats.FIN, pstats.FOUT, pstats.ZERO,
               pstats.SEGV, pstats.SEGPROT);
    }
    printf("TOTALCOST %lu %lu %lu %llu %lu\n",
           inst_count, ctx_switches, process_exits, cost, sizeof(pte_t));
}


void read_process() {
    int num_process, num_vma, start_vp, end_vp, write_pt, file_mp;
    while (getline(file, line) && line[0] == '#') { }
    sscanf(line.c_str(), "%d", &num_process);
    for (int i=0; i<num_process; i++) {
        while (getline(file, line) && line[0] == '#') { }
        sscanf(line.c_str(), "%d", &num_vma);
        Process* proc = new Process(MAX_VPAGES);
        for (int j=0; j<num_vma; j++) {
            getline(file, line);
            sscanf(line.c_str(), "%d%d%d%d", &start_vp, &end_vp, &write_pt, &file_mp);
            proc->updateVMA(start_vp, end_vp, write_pt, file_mp);
        }
        process_list.push_back(proc);
    }
    while (getline(file, line) && line[0] == '#') { }
}


bool get_next_instruction(char &op, int &vp) {
    if (! line.empty() && line[0] != '#') {
        sscanf(line.c_str(), "%c%d", &op, &vp);
        getline(file, line);
        return true;
    }
    else return false;
}