
#include "psc.h"
#include "psc_particles_as_c.h"
#include "psc_fields_as_c.h"

#include <stdlib.h>
#include <string.h>

// debugging stuff...

static void
ascii_dump_field_yz(fields_t *pf, int m, FILE *file)
{
  for (int iz = pf->ib[2]; iz < pf->ib[2] + pf->im[2]; iz++) {
    for (int iy = pf->ib[1]; iy < pf->ib[1] + pf->im[1]; iy++) {
      int ix = 0; {
	fprintf(file, "%d %d %d %g\n", ix, iy, iz, F3(pf, m, ix,iy,iz));
      }
    }
    fprintf(file, "\n");
  }
  fprintf(file, "\n");
}

static void
ascii_dump_field(mfields_base_t *flds_base, int m, const char *fname)
{
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  for (int p = 0; p < flds_base->nr_patches; p++) {
    char *filename = malloc(strlen(fname) + 20);
    sprintf(filename, "%s-p%d-p%d.asc", fname, rank, p);
    mpi_printf(MPI_COMM_WORLD, "ascii_dump_field: '%s'\n", filename);

    fields_t *pf_base = psc_mfields_get_patch(flds_base, p);
    fields_t *pf = psc_fields_get_as(pf_base, "c", m, m+1);
    FILE *file = fopen(filename, "w");
    free(filename);
    if (pf->im[0] + 2*pf->ib[0] == 1) {
      ascii_dump_field_yz(pf, m, file);
    } else {
      for (int iz = pf->ib[2]; iz < pf->ib[2] + pf->im[2]; iz++) {
	for (int iy = pf->ib[1]; iy < pf->ib[1] + pf->im[1]; iy++) {
	  for (int ix = pf->ib[0]; ix < pf->ib[0] + pf->im[0]; ix++) {
	    fprintf(file, "%d %d %d %g\n", ix, iy, iz, F3(pf, m, ix,iy,iz));
	  }
	  fprintf(file, "\n");
	}
	fprintf(file, "\n");
      }
    }
    fclose(file);
    psc_fields_put_as(pf, pf_base, 0, 0);
  }
}

static void
ascii_dump_particles(mparticles_base_t *particles_base, const char *fname)
{
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  psc_foreach_patch(ppsc, p) {
    struct psc_particles *prts_base = psc_mparticles_get_patch(particles_base, p);
    struct psc_particles *prts = psc_particles_get_as(prts_base, "c", 0);
    char *filename = malloc(strlen(fname) + 20);
    sprintf(filename, "%s-p%d-p%d.asc", fname, rank, p);
    mpi_printf(MPI_COMM_WORLD, "ascii_dump_particles: '%s'\n", filename);
    
    FILE *file = fopen(filename, "w");
    fprintf(file, "i\txi\tyi\tzi\tpxi\tpyi\tpzi\tqni\tmni\twni\n");
    for (int i = 0; i < prts->n_part; i++) {
      particle_t *p = particles_get_one(prts, i);
      fprintf(file, "%d\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\n",
	      i, p->xi, p->yi, p->zi,
	      p->pxi, p->pyi, p->pzi, p->qni, p->mni, p->wni);
    }
    fclose(file);
    free(filename);
    psc_particles_put_as(prts, prts_base, MP_DONT_COPY);
  }
}

void
psc_dump_field(mfields_base_t *flds, int m, const char *fname)
{
  ascii_dump_field(flds, m, fname);
}

void
psc_dump_particles(mparticles_base_t *particles, const char *fname)
{
  ascii_dump_particles(particles, fname);
}

