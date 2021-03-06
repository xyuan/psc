
#include "psc_push_particles_private.h"

#include "psc_fields_single.h"

#include "psc_particles_as_single.h"
#include "psc_fields_as_single.h"

#define F3_CURR F3_S
#define F3_CACHE F3_S
#define F3_CACHE_TYPE "single"

#include "1vb_yz_2.c"

// ======================================================================
// psc_push_particles: subclass "1vb2_single"

struct psc_push_particles_ops psc_push_particles_1vb2_single_ops = {
  .name                  = "1vb2_single2",
  .push_a_yz             = psc_push_particles_push_a_yz,
};

