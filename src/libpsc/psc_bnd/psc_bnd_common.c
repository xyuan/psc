
#include <mrc_domain.h>
#include <mrc_profile.h>
#include <string.h>

struct psc_bnd_sub {
  struct ddc_particles *ddcp;
  struct ddc_particles *ddcp_photons;
};

#define to_psc_bnd_sub(bnd) ((struct psc_bnd_sub *)((bnd)->obj.subctx))

static void
ddcp_particles_realloc(void *_particles, int p, int new_n_particles)
{
  mparticles_t *particles = _particles;
  struct psc_particles *prts = psc_mparticles_get_patch(particles, p);
  particles_realloc(prts, new_n_particles);
}

static void *
ddcp_particles_get_addr(void *_particles, int p, int n)
{
  mparticles_t *particles = _particles;
  struct psc_particles *prts = psc_mparticles_get_patch(particles, p);
  return particles_get_one(prts, n);
}

static void
ddcp_photons_realloc(void *_particles, int p, int new_n_particles)
{
  mphotons_t *particles = _particles;
  photons_t *pp = &particles->p[p];
  photons_realloc(pp, new_n_particles);
}

static void *
ddcp_photons_get_addr(void *_particles, int p, int n)
{
  mphotons_t *mphotons = _particles;
  photons_t *photons = &mphotons->p[p];
  return &photons->photons[n];
}

// ----------------------------------------------------------------------
// psc_bnd_sub_setup

static void
psc_bnd_sub_setup(struct psc_bnd *bnd)
{
  struct psc_bnd_sub *bnd_sub = to_psc_bnd_sub(bnd);
  struct psc *psc = bnd->psc;

  bnd->ddc = psc_bnd_lib_create_ddc(psc);

  bnd_sub->ddcp = ddc_particles_create(bnd->ddc, sizeof(particle_t),
				       sizeof(particle_real_t),
				       MPI_PARTICLES_REAL,
				       ddcp_particles_realloc,
				       ddcp_particles_get_addr);

  bnd_sub->ddcp_photons = ddc_particles_create(bnd->ddc, sizeof(photon_t),
					       sizeof(photon_real_t),
					       MPI_PHOTONS_REAL,
					       ddcp_photons_realloc,
					       ddcp_photons_get_addr);
}

// ----------------------------------------------------------------------
// psc_bnd_sub_unsetup

static void
psc_bnd_sub_unsetup(struct psc_bnd *bnd)
{
  struct psc_bnd_sub *bnd_sub = to_psc_bnd_sub(bnd);

  mrc_ddc_destroy(bnd->ddc);
  ddc_particles_destroy(bnd_sub->ddcp);
  ddc_particles_destroy(bnd_sub->ddcp_photons);
}

// ----------------------------------------------------------------------
// psc_bnd_sub_destroy

static void
psc_bnd_sub_destroy(struct psc_bnd *bnd)
{
  psc_bnd_sub_unsetup(bnd);
}

// ----------------------------------------------------------------------
// psc_bnd_sub_add_ghosts

static void
psc_bnd_sub_add_ghosts(struct psc_bnd *bnd, mfields_base_t *flds_base, int mb, int me)
{
  psc_bnd_lib_add_ghosts(bnd->ddc, flds_base, mb, me);
}

// ----------------------------------------------------------------------
// psc_bnd_sub_fill_ghosts

static void
psc_bnd_sub_fill_ghosts(struct psc_bnd *bnd, mfields_base_t *flds_base, int mb, int me)
{
  psc_bnd_lib_fill_ghosts(bnd->ddc, flds_base, mb, me);
}

// ----------------------------------------------------------------------
// calc_domain_bounds
//
// calculate bounds of local patch, and global domain

static void
calc_domain_bounds(struct psc *psc, int p, double xb[3], double xe[3],
		   double xgb[3], double xge[3], double xgl[3])
{
  struct psc_patch *psc_patch = &psc->patch[p];

  for (int d = 0; d < 3; d++) {
    xb[d] = psc_patch->off[d] * psc->dx[d];
    if (psc->domain.bnd_fld_lo[d] == BND_FLD_UPML) {
      xgb[d] = psc->pml.size * psc->dx[d];
    } else {
      xgb[d] = 0.;
    }
    
    xe[d] = (psc_patch->off[d] + psc_patch->ldims[d]) * psc->dx[d];
    if (psc->domain.bnd_fld_lo[d] == BND_FLD_UPML) {
      xge[d] = (psc->domain.gdims[d] - psc->pml.size) * psc->dx[d];
    } else {
      xge[d] = psc->domain.gdims[d] * psc->dx[d];
    }
    
    xgl[d] = xge[d] - xgb[d];
  }
  for (int d = 0; d < 3; d++) {
    xb[d]  += ppsc->domain.corner[d] / ppsc->coeff.ld;
    xe[d]  += ppsc->domain.corner[d] / ppsc->coeff.ld;
    xgb[d] += ppsc->domain.corner[d] / ppsc->coeff.ld;
    xge[d] += ppsc->domain.corner[d] / ppsc->coeff.ld;
  }
}

// ======================================================================
//
// ----------------------------------------------------------------------
// find_block_index

static inline void
find_block_index(int b_pos[3], particle_real_t xi[3], particle_real_t b_dxi[3])
{
  for (int d = 0; d < 3; d++) {
    b_pos[d] = particle_real_fint(xi[d] * b_dxi[d]);
  }
}

// ----------------------------------------------------------------------
// psc_bnd_sub_exchange_particles

static void
psc_bnd_sub_exchange_particles(struct psc_bnd *bnd, mparticles_base_t *particles_base)
{
  struct psc_bnd_sub *bnd_sub = to_psc_bnd_sub(bnd);
  struct psc *psc = bnd->psc;

  mparticles_t *particles = psc_mparticles_get_cf(particles_base, 0);

  static int pr_A, pr_B;
  if (!pr_A) {
    pr_A = prof_register("xchg_prep_" PARTICLE_TYPE, 1., 0, 0);
    pr_B = prof_register("xchg_comm_" PARTICLE_TYPE, 1., 0, 0);
  }
  prof_start(pr_A);

  struct ddc_particles *ddcp = bnd_sub->ddcp;

  // FIXME we should make sure (assert) we don't quietly drop particle which left
  // in the invariant direction

  // New-style boundary requirements.
  // These will need revisiting when it comes to non-periodic domains.
  // FIXME, calculate once => But then please recalculate whenever the dynamic window changes

  particle_real_t b_dxi[3] = { 1.f / psc->dx[0], 1.f / psc->dx[1], 1.f / psc->dx[2] };
  psc_foreach_patch(psc, p) {
    struct psc_patch *ppatch = &psc->patch[p];
    particle_real_t xm[3];
    for (int d = 0; d < 3; d++) {
      xm[d] = ppatch->ldims[d] * psc->dx[d];
    }
    int *b_mx = ppatch->ldims;

    struct psc_particles *prts = psc_mparticles_get_patch(particles, p);
    struct ddcp_patch *patch = &ddcp->patches[p];
    patch->head = 0;
    for (int dir1 = 0; dir1 < N_DIR; dir1++) {
      patch->nei[dir1].n_send = 0;
    }
    for (int i = 0; i < prts->n_part; i++) {
      particle_t *part = particles_get_one(prts, i);
      particle_real_t *xi = &part->xi; // slightly hacky relies on xi, yi, zi to be contiguous in the struct. FIXME

      int b_pos[3];
      find_block_index(b_pos, xi, b_dxi);
      particle_real_t *pxi = &part->pxi;
      if (b_pos[0] >= 0 && b_pos[0] < b_mx[0] &&
	  b_pos[1] >= 0 && b_pos[1] < b_mx[1] &&
	  b_pos[2] >= 0 && b_pos[2] < b_mx[2]) {
	// fast path
	// inside domain: move into right position
	*particles_get_one(prts, patch->head++) = *part;
      } else {
	// slow path
	bool drop = false;
	int dir[3];
	for (int d = 0; d < 3; d++) {
	  if (b_pos[d] < 0) {
	    if (ppatch->off[d] > 0 ||
		psc->domain.bnd_part_lo[d] == BND_PART_PERIODIC) {
	      xi[d] += xm[d];
	      dir[d] = -1;
	      int bi = particle_real_fint(xi[d] * b_dxi[d]);
	      if (bi >= b_mx[d]) {
		xi[d] = 0.;
		dir[d] = 0;
	      }
	    } else {
	      switch (psc->domain.bnd_part_lo[d]) {
	      case BND_PART_REFLECTING:
		xi[d] =  -xi[d];
		pxi[d] = -pxi[d];
		dir[d] = 0;
		break;
	      case BND_PART_ABSORBING:
		drop = true;
		break;
	      default:
		assert(0);
	      }
	    }
	  } else if (b_pos[d] >= b_mx[d]) {
	    if (ppatch->off[d] + ppatch->ldims[d] < psc->domain.gdims[d] ||
		psc->domain.bnd_part_hi[d] == BND_PART_PERIODIC) {
	      xi[d] -= xm[d];
	      dir[d] = +1;
	      int bi = particle_real_fint(xi[d] * b_dxi[d]);
	      if (bi < 0) {
		xi[d] = 0.;
	      }
	    } else {
	      switch (psc->domain.bnd_part_hi[d]) {
	      case BND_PART_REFLECTING:
		xi[d] = 2.f * xm[d] - xi[d];
		pxi[d] = -pxi[d];
		dir[d] = 0;
		int bi = particle_real_fint(xi[d] * b_dxi[d]);
		if (bi >= b_mx[d]) {
		  xi[d] *= (1. - 1e-6);
		}
		break;
	      case BND_PART_ABSORBING:
		drop = true;
		break;
	      default:
		assert(0);
	      }
	    }
	  } else {
	    // computational bnd
	    dir[d] = 0;
	  }
	  assert(xi[d] >= 0.f);
	  assert(xi[d] <= xm[d]);
	}
	if (!drop) {
	  if (dir[0] == 0 && dir[1] == 0 && dir[2] == 0) {
	    *particles_get_one(prts, patch->head++) = *part;
	  } else {
	    ddc_particles_queue(ddcp, patch, dir, part);
	  }
	}
      }
    }
  }
  prof_stop(pr_A);

  prof_start(pr_B);
  ddc_particles_comm(ddcp, particles);

  psc_foreach_patch(psc, p) {
    struct psc_particles *prts = psc_mparticles_get_patch(particles, p);
    struct ddcp_patch *patch = &ddcp->patches[p];
    prts->n_part = patch->head;
  }
  prof_stop(pr_B);

  psc_mparticles_put_cf(particles, particles_base, 0);
}

// ----------------------------------------------------------------------
// psc_bnd_sub_exchange_photons

static void
psc_bnd_sub_exchange_photons(struct psc_bnd *bnd, mphotons_t *mphotons)
{
  struct psc_bnd_sub *bnd_sub = to_psc_bnd_sub(bnd);
  struct psc *psc = bnd->psc;

  static int pr;
  if (!pr) {
    pr = prof_register("c_xchg_photon", 1., 0, 0);
  }
  prof_start(pr);

  struct ddc_particles *ddcp = bnd_sub->ddcp_photons;

  double xb[3], xe[3], xgb[3], xge[3], xgl[3];

  // New-style boundary requirements.
  // These will need revisiting when it comes to non-periodic domains.
  // FIXME, calculate once

  psc_foreach_patch(psc, p) {
    calc_domain_bounds(psc, p, xb, xe, xgb, xge, xgl);

    photons_t *photons = &mphotons->p[p];
    struct ddcp_patch *patch = &ddcp->patches[p];
    patch->head = 0;
    for (int dir1 = 0; dir1 < N_DIR; dir1++) {
      patch->nei[dir1].n_send = 0;
    }
    for (int i = 0; i < photons->nr; i++) {
      photon_t *ph = photons_get_one(photons, i);
      photon_real_t *xi = ph->x;
      photon_real_t *pxi = ph->p;
      if (xi[0] >= xb[0] && xi[0] <= xe[0] &&
	  xi[1] >= xb[1] && xi[1] <= xe[1] &&
	  xi[2] >= xb[2] && xi[2] <= xe[2]) {
	// fast path
	// inside domain: move into right position
	photons->photons[patch->head++] = *ph;
      } else {
	// slow path
	int dir[3];
	for (int d = 0; d < 3; d++) {
	  if (xi[d] < xb[d]) {
	    if (xi[d] < xgb[d]) {
	      switch (psc->domain.bnd_part_lo[d]) {
	      case BND_PART_REFLECTING:
		xi[d] = 2.f * xgb[d] - xi[d];
		pxi[d] = -pxi[d];
		dir[d] = 0;
		break;
	      case BND_PART_PERIODIC:
		xi[d] += xgl[d];
		dir[d] = -1;
		break;
	      default:
		assert(0);
	      }
	    } else {
	      // computational bnd
	      dir[d] = -1;
	    }
	  } else if (xi[d] > xe[d]) {
	    if (xi[d] > xge[d]) {
	      switch (psc->domain.bnd_part_hi[d]) {
	      case BND_PART_REFLECTING:
		xi[d] = 2.f * xge[d] - xi[d];
		pxi[d] = -pxi[d];
		dir[d] = 0;
		break;
	      case BND_PART_PERIODIC:
		xi[d] -= xgl[d];
		dir[d] = +1;
		break;
	      default:
		assert(0);
	      }
	    } else {
	      dir[d] = +1;
	    }
	  } else {
	    // computational bnd
	    dir[d] = 0;
	  }
	}
	if (dir[0] == 0 && dir[1] == 0 && dir[2] == 0) {
	  photons->photons[patch->head++] = *ph;
	} else {
	  ddc_particles_queue(ddcp, patch, dir, ph);
	}
      }
    }
  }

  ddc_particles_comm(ddcp, mphotons);
  psc_foreach_patch(psc, p) {
    photons_t *photons = &mphotons->p[p];
    struct ddcp_patch *patch = &ddcp->patches[p];
    photons->nr = patch->head;
  }

  prof_stop(pr);
}
