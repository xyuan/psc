
#include "psc_bnd_particles_private.h"
#include "../psc_bnd/ddc_particles.h"
#include "psc_particles_as_single.h"

#include "psc_bnd_particles_common2.c"

// ======================================================================
// psc_bnd_particles: subclass "single2"

struct psc_bnd_particles_ops psc_bnd_particles_single2_ops = {
  .name                    = "single2",
  .setup                   = psc_bnd_particles_sub_setup,
  .unsetup                 = psc_bnd_particles_sub_unsetup,
  .exchange_particles      = psc_bnd_particles_sub_exchange_particles,
  .exchange_particles_prep = psc_bnd_particles_sub_exchange_particles_prep,
  .exchange_particles_post = psc_bnd_particles_sub_exchange_particles_post,
};
