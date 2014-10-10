#ifndef _PSTAT_H_
#define _PSTAT_H_

#include "param.h"

struct pstat {
    int inuse[NPROC]; 		// whether this slot of the process process table is in use (1 or 0)
    int pid[NPROC];   		// the PID of each process
    //char pname[NPROC][16];	// the name of each process
    //int level[NPROC];			// The level of this process (1 = reserved, 2 = spot)
  	//int percent[NPROC];		// Guaranteed percentage of CPU time for this process
  	//int bid[NPROC];			// Amount of $$ bid for this process, in $n per ms
    int chosen[NPROC]; 		// the number of times the process was chosen to run
    int time[NPROC]; 		// the number of ms the process has run
    int charge[NPROC]; 		// how much money the process has been charged
};


#endif // _PSTAT_H_