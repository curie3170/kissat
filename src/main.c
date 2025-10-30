#include "application.h"
#include "cover.h"
#include "handle.h"
#include "kissat.h"
#include "print.h"

#include <assert.h>
#include <stdbool.h>
#include <string.h>   // strncmp

static const char *extract_init_phase_path_from_argv (int *argc, char ***argv) {
  // Parser: --init-phase-file=PATH 
  char **av = *argv;
  int ac = *argc;
  const char *path = 0;
  int w = 0;
  for (int r = 0; r < ac; r++) {
    const char *a = av[r];
    if (!strncmp(a, "--init-phase-file=", 18)) {
      path = a + 18;
      continue;            
    }
    av[w++] = av[r];
  }
  av[w] = 0;
  *argc = w;
  return path;
}
static kissat *volatile solver;

// clang-format off

static void
kissat_signal_handler (int sig)
{
  kissat_signal (solver, "caught", sig);
  kissat_print_statistics (solver);
  kissat_signal (solver, "raising", sig);
#ifdef QUIET
  (void) sig;
#endif
  FLUSH_COVERAGE (); } // Keep this '}' in the same line!

// clang-format on

static volatile bool ignore_alarm = false;

static void kissat_alarm_handler (void) {
  if (ignore_alarm)
    return;
  assert (solver);
  kissat_terminate (solver);
}

#ifndef NDEBUG
extern int kissat_dump (kissat *);
#endif

#include "error.h"
#include "random.h"
#include <strings.h>

int main (int argc, char **argv) {
  int res;
  solver = kissat_init ();
  kissat_init_alarm (kissat_alarm_handler);
  kissat_init_signal_handler (kissat_signal_handler);
  // Parsing --init-phase-file=path 
  const char *init_phase_path = extract_init_phase_path_from_argv(&argc, &argv);
  if (init_phase_path) {
    kissat_set_init_phase_file(solver, init_phase_path);  
  }

  res = kissat_application (solver, argc, argv);
  kissat_reset_signal_handler ();
  ignore_alarm = true;
  kissat_reset_alarm ();
  kissat_release (solver);
#ifndef NDEBUG
  if (!res)
    return kissat_dump (0);
#endif
  return res;
}
