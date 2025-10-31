#ifndef _phases_h_INCLUDED
#define _phases_h_INCLUDED

#include "value.h"

typedef struct phases phases;

struct phases {
  value *best;
  value *saved;
  value *target;
};

#define BEST(IDX) \
  (solver->phases.best[assert (VALID_INTERNAL_INDEX (IDX)), (IDX)])

#define SAVED(IDX) \
  (solver->phases.saved[assert (VALID_INTERNAL_INDEX (IDX)), (IDX)])

#define TARGET(IDX) \
  (solver->phases.target[assert (VALID_INTERNAL_INDEX (IDX)), (IDX)])

struct kissat;

void kissat_increase_phases (struct kissat *, unsigned);
void kissat_decrease_phases (struct kissat *, unsigned);
void kissat_release_phases (struct kissat *);

void kissat_save_best_phases (struct kissat *);
void kissat_save_target_phases (struct kissat *);
bool read_init_phase_file_binary (kissat *solver, const char *path, value *out, int n);
void kissat_load_initial_phases_binary (kissat *solver, const value *phases); 
#endif
