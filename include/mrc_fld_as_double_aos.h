
#ifndef MRC_FLD_AS_DOUBLE_AOS_H
#define MRC_FLD_AS_DOUBLE_AOS_H

typedef double mrc_fld_data_t;
#define mrc_fld_abs fabs
#define mrc_fld_max fmax

#define F3(f, m, i,j,k) MRC_D4(f, m, i,j,k)
#define M3(f, m, i,j,k, p) MRC_D5(f, m, i,j,k, p)
#define FLD_TYPE "double_aos"
#define MPI_MRC_FLD_DATA_T MPI_DOUBLE

#endif
