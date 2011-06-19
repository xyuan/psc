
#include "psc.h"
#include "psc_push_particles.h"
#include "psc_push_fields.h"
#include "psc_bnd.h"
#include "psc_collision.h"
#include "psc_randomize.h"
#include "psc_sort.h"
#include "psc_output_fields.h"
#include "psc_output_particles.h"
#include "psc_moments.h"
#include "psc_event_generator.h"
#include "psc_balance.h"

#include <mrc_common.h>
#include <mrc_params.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <assert.h>

struct psc *ppsc;

#define VAR(x) (void *)offsetof(struct psc, x)

static struct mrc_param_select bnd_fld_descr[] = {
  { .val = BND_FLD_OPEN        , .str = "open"        },
  { .val = BND_FLD_PERIODIC    , .str = "periodic"    },
  { .val = BND_FLD_UPML        , .str = "upml"        },
  { .val = BND_FLD_TIME        , .str = "time"        },
  {},
};

static struct mrc_param_select bnd_part_descr[] = {
  { .val = BND_PART_REFLECTING , .str = "reflecting"  },
  { .val = BND_PART_PERIODIC   , .str = "periodic"    },
  {},
};

struct param psc_descr[] = {
  { "length_x"      , VAR(domain.length[0])       , PARAM_DOUBLE(1e-6)   },
  { "length_y"      , VAR(domain.length[1])       , PARAM_DOUBLE(1e-6)   },
  { "length_z"      , VAR(domain.length[2])       , PARAM_DOUBLE(20e-6)  },
  { "corner_x"      , VAR(domain.corner[0])       , PARAM_DOUBLE(0.)     },
  { "corner_y"      , VAR(domain.corner[1])       , PARAM_DOUBLE(0.)     },
  { "corner_z"      , VAR(domain.corner[2])       , PARAM_DOUBLE(0.)     },
  { "gdims_x"       , VAR(domain.gdims[0])        , PARAM_INT(1)         },
  { "gdims_y"       , VAR(domain.gdims[1])        , PARAM_INT(1)         },
  { "gdims_z"       , VAR(domain.gdims[2])        , PARAM_INT(400)       },

  { "bnd_field_lo_x", VAR(domain.bnd_fld_lo[0])   , PARAM_SELECT(BND_FLD_PERIODIC,
								 bnd_fld_descr) },
  { "bnd_field_lo_y", VAR(domain.bnd_fld_lo[1])   , PARAM_SELECT(BND_FLD_PERIODIC,
								 bnd_fld_descr) },
  { "bnd_field_lo_z", VAR(domain.bnd_fld_lo[2])   , PARAM_SELECT(BND_FLD_PERIODIC,
								 bnd_fld_descr) },
  { "bnd_field_hi_x", VAR(domain.bnd_fld_hi[0])   , PARAM_SELECT(BND_FLD_PERIODIC,
								 bnd_fld_descr) },
  { "bnd_field_hi_y", VAR(domain.bnd_fld_hi[1])   , PARAM_SELECT(BND_FLD_PERIODIC,
								 bnd_fld_descr) },
  { "bnd_field_hi_z", VAR(domain.bnd_fld_hi[2])   , PARAM_SELECT(BND_FLD_PERIODIC,
								 bnd_fld_descr) },

  { "bnd_particle_x", VAR(domain.bnd_part[0])     , PARAM_SELECT(BND_PART_PERIODIC,
								 bnd_part_descr) },
  { "bnd_particle_y", VAR(domain.bnd_part[1])     , PARAM_SELECT(BND_PART_PERIODIC,
								 bnd_part_descr) },
  { "bnd_particle_z", VAR(domain.bnd_part[2])     , PARAM_SELECT(BND_PART_PERIODIC,
								 bnd_part_descr) },
  { "use_pml",        VAR(domain.use_pml)         , PARAM_BOOL(false)    },
  {},
};

#undef VAR

// ----------------------------------------------------------------------
// psc_create

static void
_psc_create(struct psc *psc)
{
  MPI_Comm comm = psc_comm(psc);

  psc->push_particles = psc_push_particles_create(comm);
  psc_add_child(psc, (struct mrc_obj *) psc->push_particles);
  psc->push_fields = psc_push_fields_create(comm);
  psc_add_child(psc, (struct mrc_obj *) psc->push_fields);
  psc->bnd = psc_bnd_create(comm);
  psc_add_child(psc, (struct mrc_obj *) psc->bnd);
  psc->collision = psc_collision_create(comm);
  psc_add_child(psc, (struct mrc_obj *) psc->collision);
  psc->randomize = psc_randomize_create(comm);
  psc_add_child(psc, (struct mrc_obj *) psc->randomize);
  psc->sort = psc_sort_create(comm);
  psc_add_child(psc, (struct mrc_obj *) psc->sort);
  psc->output_fields = psc_output_fields_create(comm);
  psc_add_child(psc, (struct mrc_obj *) psc->output_fields);
  psc->output_particles = psc_output_particles_create(comm);
  psc_add_child(psc, (struct mrc_obj *) psc->output_particles);
  psc->moments = psc_moments_create(comm);
  psc_add_child(psc, (struct mrc_obj *) psc->moments);
  psc->event_generator = psc_event_generator_create(comm);
  psc_add_child(psc, (struct mrc_obj *) psc->event_generator);
  psc->balance = psc_balance_create(comm);
  psc_add_child(psc, (struct mrc_obj *) psc->balance);

  psc->time_start = MPI_Wtime();

  for (int d = 0; d < 3; d++) {
    psc->ibn[d] = 3;
  }
  psc_set_default_psc(psc);
}

// ----------------------------------------------------------------------
// psc_set_from_options

static void
_psc_set_from_options(struct psc *psc)
{
  psc_set_from_options_psc(psc);
}

// ----------------------------------------------------------------------
// psc_setup

static void
_psc_setup(struct psc *psc)
{
  psc_setup_coeff(psc);
  psc_setup_domain(psc); // needs to be done before setting up psc_bnd
}

// ----------------------------------------------------------------------
// psc_view

static void
_psc_view(struct psc *psc)
{
  psc_view_psc(psc);
}

// ----------------------------------------------------------------------
// psc_destroy

static void
_psc_destroy(struct psc *psc)
{
  mfields_base_destroy(psc->flds);
  mparticles_base_destroy(&psc->particles);
  mphotons_destroy(&psc->mphotons);

  psc_destroy_domain(psc);
}

// ======================================================================
// psc class

struct mrc_class_psc mrc_class_psc = {
  .name             = "psc",
  .size             = sizeof(struct psc),
  .param_descr      = psc_descr,
  .create           = _psc_create,
  .set_from_options = _psc_set_from_options,
  .setup            = _psc_setup,
  .destroy          = _psc_destroy,
  .view             = _psc_view,
};

// ======================================================================

const char *fldname[NR_FIELDS] = {
  [NE]  = "ne",
  [NI]  = "ni",
  [NN]  = "nn",
  [JXI] = "jx",
  [JYI] = "jy",
  [JZI] = "jz",
  [EX]  = "ex",
  [EY]  = "ey",
  [EZ]  = "ez",
  [HX]  = "hx",
  [HY]  = "hy",
  [HZ]  = "hz",
  [DX]  = "dx",
  [DY]  = "dy",
  [DZ]  = "dz",
  [BX]  = "bx",
  [BY]  = "by",
  [BZ]  = "bz",
  [EPS] = "eps",
  [MU]  = "mu",
};
