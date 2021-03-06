
#include "ggcm_mhd_bnd_private.h"

#include "ggcm_mhd.h"

#include <mrc_io.h>

#include <assert.h>

// ======================================================================
// ggcm_mhd_bnd class

// ----------------------------------------------------------------------
// ggcm_mhd_bnd_fill_ghosts

void
ggcm_mhd_bnd_fill_ghosts(struct ggcm_mhd_bnd *bnd, struct mrc_fld *fld,
			 int m, float bntim)
{
  struct ggcm_mhd_bnd_ops *ops = ggcm_mhd_bnd_ops(bnd);
  assert(ops && ops->fill_ghosts);
  ops->fill_ghosts(bnd, fld, m, bntim);
}

// ----------------------------------------------------------------------
// ggcm_mhd_bnd_init

static void
ggcm_mhd_bnd_init()
{
  mrc_class_register_subclass(&mrc_class_ggcm_mhd_bnd, &ggcm_mhd_bnd_ops_none);
}

// ----------------------------------------------------------------------
// ggcm_mhd_bnd description

#define VAR(x) (void *)offsetof(struct ggcm_mhd_bnd, x)
static struct param ggcm_mhd_bnd_descr[] = {
  { "mhd"             , VAR(mhd)             , PARAM_OBJ(ggcm_mhd)      },
  {},
};
#undef VAR

// ----------------------------------------------------------------------
// ggcm_mhd_bnd class description

struct mrc_class_ggcm_mhd_bnd mrc_class_ggcm_mhd_bnd = {
  .name             = "ggcm_mhd_bnd",
  .size             = sizeof(struct ggcm_mhd_bnd),
  .param_descr      = ggcm_mhd_bnd_descr,
  .init             = ggcm_mhd_bnd_init,
};

