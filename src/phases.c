#include "allocate.h"
#include "internal.h"
#include "logging.h"
#include "print.h"
#include <string.h>
#include <stdio.h>    // FILE, fscanf

#define realloc_phases(NAME) \
  do { \
    solver->phases.NAME = \
        kissat_realloc (solver, solver->phases.NAME, old_size, new_size); \
  } while (0)

#define increase_phases(NAME) \
  do { \
    assert (old_size < new_size); \
    realloc_phases (NAME); \
    memset (solver->phases.NAME + old_size, 0, new_size - old_size); \
  } while (0)

void kissat_increase_phases (kissat *solver, unsigned new_size) {
  const unsigned old_size = solver->size;
  assert (old_size < new_size);
  LOG ("increasing phases from %u to %u", old_size, new_size);
  increase_phases (best);
  increase_phases (saved);
  increase_phases (target);
}

void kissat_decrease_phases (kissat *solver, unsigned new_size) {
  const unsigned old_size = solver->size;
  assert (old_size > new_size);
  LOG ("decreasing phases from %u to %u", old_size, new_size);
  realloc_phases (best);
  realloc_phases (saved);
  realloc_phases (target);
}

#define release_phases(NAME, SIZE) \
  kissat_free (solver, solver->phases.NAME, SIZE)

void kissat_release_phases (kissat *solver) {
  const unsigned size = solver->size;
  release_phases (best, size);
  release_phases (saved, size);
  release_phases (target, size);
}

static void save_phases (kissat *solver, value *phases) {
  const value *const values = solver->values;
  const value *const end = phases + VARS;
  value const *v = values;
  for (value *p = phases, tmp; p != end; p++, v += 2)
    if ((tmp = *v))
      *p = tmp;
  assert (v == values + LITS);
}

void kissat_save_best_phases (kissat *solver) {
  assert (sizeof (value) == 1);
  LOG ("saving %u best values", VARS);
  save_phases (solver, solver->phases.best);
}

void kissat_save_target_phases (kissat *solver) {
  assert (sizeof (value) == 1);
  LOG ("saving %u target values", VARS);
  save_phases (solver, solver->phases.target);
}
static int token_to_phase (const char *s, int len) {
  if ((len == 1 && s[0] == '1') || !strcasecmp(s, "true") || !strcasecmp(s, "t") || !strcmp(s, "+1"))
    return +1;
  if ((len == 1 && s[0] == '0') || !strcasecmp(s, "false")|| !strcasecmp(s, "f") || !strcmp(s, "-1"))
    return -1;
  return 0;
}

bool read_init_phase_file_binary (kissat *solver,
                                  const char *path,
                                  value *out, int n) {
  FILE *f = fopen(path, "r");
  if (!f) {
    kissat_message(solver, "could not open init phase file '%s'", path);
    return false;
  }

  int i = 0, n_pos = 0, n_neg = 0, n_skip = 0;
  char buf[64];
  while (i < n && fscanf(f, "%63s", buf) == 1) {
    int v = token_to_phase(buf, (int)strlen(buf));
    if (!v) {
      n_skip++;
      kissat_message(solver, "invalid token '%s' at index %d in '%s' (use 0/1,+1/-1,true/false)",
                     buf, i, path);
      fclose(f);
      return false;
    }
    out[i++] = (value) v;
    if (v > 0) n_pos++; else n_neg++;
  }
  fclose(f);

  if (i != n) {
    kissat_message(solver, "init phase file '%s' has %d entries but %d variables", path, i, n);
    return false;
  }

  kissat_message(solver, "init-phase: parsed '%s': +1=%d, -1=%d, skip=%d (total %d of %d)",
                 path, n_pos, n_neg, n_skip, i, n);
  return true;
}
void kissat_load_initial_phases_binary (kissat *solver, const value *phases) {
  const int vars = solver->vars;
  value *values = solver->values;      // length = 2*vars
  for (int i = 0; i < vars; i++) {
    const value p = phases[i];         // prefer+1 (true) or -1 (false)
    values[2*i]   = p;                 // positive literal
    values[2*i^1] = -p;                // negative literal
  }
}