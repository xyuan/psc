
void
PFX(ps_1vb_yz_pxx_jxyz)(int p, fields_ip_t *pf, particles_single_t *pps, int n_start)
{
  static int pr;
  if (!pr) {
    pr = prof_register("ps_1vb_pxx_jxyz", 1., 0, 0);
  }
  prof_start(pr);

  float dt = ppsc->dt, dth = .5f * ppsc->dt;
  float dxi[3] = { 1.f / ppsc->dx[0], 1.f / ppsc->dx[1], 1.f / ppsc->dx[2] };
  float fnqs = sqr(ppsc->coeff.alpha) * ppsc->coeff.cori / ppsc->coeff.eta;
  float fnqys = ppsc->dx[1] * fnqs / dt;
  float fnqzs = ppsc->dx[2] * fnqs / dt;
  float dq_kind[ppsc->prm.nr_kinds];
  for (int k = 0; k < ppsc->prm.nr_kinds; k++) {
    dq_kind[k] = .5f * ppsc->coeff.eta * dt * ppsc->kinds[k].q / ppsc->kinds[k].m;
  }

  int my = pf->im[1], sz = pf->im[1] * pf->im[2];
  fields_ip_real_t *_p0 = &F3_IP(pf, 0, 0,0,0);
  v4s *_p_jx = (v4s *) &F3_IP(pf, JXI, 0,0,0);
  v4s *_p_jyz = (v4s *) &F3_IP(pf, JYI, 0,0,0);

  int n_simd = pps->n_part & ~(SIMD_SIZE - 1);
  for (int n = n_start; n < n_simd; n += SIMD_SIZE) {
    float * restrict prt = (float *) &pps->particles[n];

    v4s xi, yi, zi, qw;
    PRT_LOAD_X(prt, xi, yi, zi, qw);
    
    // field interpolation
    v4s ym, zm, yp, zp;
    v4s ogy, ogz, ohy, ohz;
    v4si lgy, lgz, lhy, lhz;
    GET_POS_IDX_FRAC(yi, dxi[1],  0.f, ym, lgy, ogy);
    GET_POS_IDX_FRAC(zi, dxi[2],  0.f, zm, lgz, ogz);
    GET_IDX_FRAC(yi, dxi[1], -.5f, lhy, ohy);
    GET_IDX_FRAC(zi, dxi[2], -.5f, lhz, ohz);

    v4s exq = INTERPOLATE_FIELD_1ST_IP(EX, g, g);
    v4s eyq = INTERPOLATE_FIELD_1ST_IP(EY, h, g);
    v4s ezq = INTERPOLATE_FIELD_1ST_IP(EZ, g, h);

    v4s hxq = INTERPOLATE_FIELD_1ST_IP(HX, h, h);
    v4s hyq = INTERPOLATE_FIELD_1ST_IP(HY, g, h);
    v4s hzq = INTERPOLATE_FIELD_1ST_IP(HZ, h, g);

    v4s pxi, pyi, pzi, kind;
    PRT_LOAD_P(prt, pxi, pyi, pzi, kind);
    v4s dq = v4s_gather(dq_kind, v4si_cast(kind));
    _PUSH_PXI(pxi, pyi, pzi, exq, eyq, ezq, hxq, hyq, hzq, dq);

    v4s vxi, vyi, vzi;
    _GET_VXYZ(vxi, vyi, vzi, pxi, pyi, pzi);

    PRT_STORE_P(prt, pxi, pyi, pzi, kind);

    // x^{n+.5} -> x^{n+1}
    yi += vyi * v4s_splat(dth);
    zi += vzi * v4s_splat(dth);

    // OUT OF PLANE CURRENT DENSITY AT (n+1.0)*dt
      
    v4si lfy, lfz;
    v4s ofy, ofz;
    GET_IDX_FRAC(yi, dxi[1],  0.f, lfy, ofy);
    GET_IDX_FRAC(zi, dxi[2],  0.f, lfz, ofz);

    v4s fnqx = vxi * qw * v4s_splat(fnqs);
    CURR_JX(_p_jx, my, lfy, lfz, ofy, ofz, fnqx);

    yi += vyi * v4s_splat(dth);
    zi += vzi * v4s_splat(dth);

    GET_POS_IDX(yi, dxi[1],  0.f, yp, lfy);
    GET_POS_IDX(zi, dxi[2],  0.f, zp, lfz);

    v4s fnq[2] = { qw * v4s_splat(fnqys), qw * v4s_splat(fnqzs) };

    PRT_STORE_X(prt, xi, yi, zi, qw);

    v4si i[2], idiff[2], off0[2];
    v4s dx[2], x[2];
    CURR_JYZ_A(lgy, lgz, lfy, lfz, ogy, ogz, ym, zm, yp, zp, off0);
    CURR_JYZ_(_p_jy, _p_jz, off0);
  }
  for (int n = n_simd; n < pps->n_part; n++) {
  }

  prof_stop(pr);
}

