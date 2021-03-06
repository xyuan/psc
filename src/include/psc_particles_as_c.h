
#ifndef PSC_PARTICLES_AS_C_H
#define PSC_PARTICLES_AS_C_H

#include "psc_particles_c.h"

typedef particle_c_real_t particle_real_t;
typedef particle_c_t particle_t;
typedef mparticles_c_t mparticles_t;

#define psc_mparticles_get_cf       psc_mparticles_get_c
#define psc_mparticles_put_cf       psc_mparticles_put_c
#define particles_get_one           particles_c_get_one
#define particles_realloc           particles_c_realloc
#define particle_qni_div_mni        particle_c_qni_div_mni
#define particle_qni_wni            particle_c_qni_wni
#define particle_qni                particle_c_qni
#define particle_mni                particle_c_mni
#define particle_wni                particle_c_wni
#define particle_kind               particle_c_kind
#define particle_get_relative_pos   particle_c_get_relative_pos
#define particle_real_nint          particle_c_real_nint
#define particle_real_fint          particle_c_real_fint
#define particle_real_sqrt          particle_c_real_sqrt
#define particle_real_abs           particle_c_real_abs

#define MPI_PARTICLES_REAL          MPI_PARTICLES_C_REAL
#define PARTICLE_TYPE               "c"

#endif

