
#include "psc_output_particles_private.h"
#include "psc_particles_c.h"

#include <mrc_params.h>
#include <string.h>

#include <string.h>

#define to_psc_output_particles_ascii(out) \
  mrc_to_subobj(out, struct psc_output_particles_ascii)

struct psc_output_particles_ascii {
  const char *data_dir;
  const char *basename;
  int every_step;
};

#define VAR(x) (void *)offsetof(struct psc_output_particles_ascii, x)
static struct param psc_output_particles_ascii_descr[] = {
  { "data_dir"           , VAR(data_dir)             , PARAM_STRING(".")       },
  { "basename"           , VAR(basename)             , PARAM_STRING("prt")     },
  { "every_step"         , VAR(every_step)           , PARAM_INT(-1)           },
  {},
};
#undef VAR

// ----------------------------------------------------------------------
// psc_output_particles_ascii_run

static void
psc_output_particles_ascii_run(struct psc_output_particles *out,
			       mparticles_base_t *particles_base)
{
  struct psc_output_particles_ascii *asc = to_psc_output_particles_ascii(out);

  if (asc->every_step < 0 ||
      ppsc->timestep % asc->every_step != 0) {
    return;
  }

  int rank;
  MPI_Comm_rank(psc_output_particles_comm(out), &rank);
  char filename[strlen(asc->data_dir) + strlen(asc->basename) + 19];
  sprintf(filename, "%s/%s.%06d_p%06d.asc", asc->data_dir,
	  asc->basename, ppsc->timestep, rank);

  FILE *file = fopen(filename, "w");
  for (int p = 0; p < particles_base->nr_patches; p++) {
    struct psc_particles *prts_base = psc_mparticles_get_patch(particles_base, p);
    struct psc_particles *prts = psc_particles_get_as(prts_base, "c", 0);
    for (int n = 0; n < prts->n_part; n++) {
      particle_c_t *part = particles_c_get_one(prts, n);
      fprintf(file, "%d %g %g %g %g %g %g %g %g %g\n",
	      n, part->xi, part->yi, part->zi,
	      part->pxi, part->pyi, part->pzi,
	      part->qni, part->mni, part->wni);
    }
    psc_particles_put_as(prts, prts_base, MP_DONT_COPY);
  }
  fclose(file);
}

// ======================================================================
// psc_output_particles: subclass "ascii"

struct psc_output_particles_ops psc_output_particles_ascii_ops = {
  .name                  = "ascii",
  .size                  = sizeof(struct psc_output_particles_ascii),
  .param_descr           = psc_output_particles_ascii_descr,
  .run                   = psc_output_particles_ascii_run,
};
