
AM_CFLAGS = -I$(top_srcdir)/src/include -I$(top_srcdir)/src/libmrc/include

if USE_CUDA

noinst_LTLIBRARIES = libsubdir.la

libsubdir_la_SOURCES = \
	psc_fields_cuda.c \
	psc_particles_cuda.c \
	psc_push_particles_cuda.c \
	psc_push_fields_cuda.c \
	psc_bnd_cuda.c \
	psc_bnd_particles_cuda.c \
	psc_bnd_fields_cuda.c \
	b40c/kernel_utils.h \
	b40c/radixsort_kernel_common.h \
	b40c/radixsort_key_conversion.h \
	b40c/radixsort_reduction_kernel.h \
	b40c/radixsort_scanscatter_kernel.h \
	b40c/radixsort_scanscatter_kernel3.h \
	b40c/radixsort_scanscatter_kernel4.h \
	b40c/radixsort_spine_kernel.h \
	b40c/vector_types.h \
	cuda_common.h \
	cuda_sort2.h \
	cuda_sort2_spine.h \
	particles_cuda.h \
	psc_bnd_cuda.h \
	psc_bnd_cuda_fields.h \
	psc_cuda.h

libsubdir_la_LIBADD = \
	cuda_transfer.lo \
	cuda_find_indices.lo \
	cuda_spine.lo \
	cuda_sort.lo \
	cuda_sort2.lo \
	cuda_sort3.lo \
	cuda_exchange_particles.lo \
	cuda_bnd.lo \
	cuda_exclusive_scan_2.lo \
	cuda_push_fields_yz.lo \
	cuda_push_part_yz_1vb.lo
#	cuda_push_part_yz_a.lo \
	cuda_push_part_yz_b.lo \
	cuda_push_part_yz.lo \
	cuda_push_part_yz2.lo \
	cuda_push_part_yz3.lo \
	cuda_push_part_yz4.lo \
	cuda_push_part_yz6.lo \
	cuda_push_part_yz_1st.lo \
	cuda_push_part_z.lo \
	cuda_push_part_z2.lo \
	cuda_push_part_z3.lo \
	cuda_push_part_yz5.lo

CUDACC_FLAGS = -g --ptxas-options=-v --maxrregcount=32 --use_fast_math $(filter-out -fopenmp -msse2 -msse4.1 -Wall -ffast-math,$(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(CPPFLAGS) $(CFLAGS) $(AM_CFLAGS)) $(CUDAMPIFLAGS) $(CUDACFLAGS)

CUDA_DEPS = \
        psc_cuda.h \
        constants.c \
        common.c \
        common_push.c \
        common_fld_cache.c \
        common_curr.c \
        push_part_p1.c \
        push_part_p2.c \
        push_part_kernelcall.c
#        psc_particles_cuda.h FIXME

cuda_push_part_z3.lo: push_part_p2_noshift.c
cuda_push_part_yz3.lo: push_part_p2_noshift.c
cuda_push_part_yz4.lo: push_part_p2_noshift_2.c
cuda_push_part_yz5.lo: push_part_p2_noshift_3.c push_part_kernelcall_3.c
cuda_push_part_yz6.lo: push_part_p2_noshift_6.c push_part_kernelcall_6.c
cuda_push_part_yz_1st.lo: push_part_p1_1st.c push_part_p2_1st.c push_part_kernelcall_6.c
cuda_push_part_yz_1vb.lo: cuda_common.h particles_cuda.h ../../include/psc_particles_cuda.h

cuda_spine.lo: b40c/radixsort_reduction_kernel.h b40c/radixsort_scanscatter_kernel.h cuda_sort2_spine.h
#cuda_sort3.lo: b40c/radixsort_reduction_kernel.h b40c/radixsort_scanscatter_kernel3.h
cuda_sort2.lo cuda_exchange_particles.lo cuda_sort2_spine.lo: psc_cuda.h cuda_sort2.h
cuda_sort3.lo: psc_cuda.h cuda_sort2.h
cuda_exchange_particles.lo: psc_bnd_cuda.h particles_cuda.h ../../include/psc_particles_cuda.h
cuda_exclusive_scan_2.lo: particles_cuda.h ../../include/psc_particles_cuda.h
cuda_transfer.lo: particles_cuda.h ../../include/psc_particles_cuda.h
cuda_spine.lo: particles_cuda.h ../../include/psc_particles_cuda.h b40c/radixsort_reduction_kernel.h b40c/radixsort_scanscatter_kernel3.h

#FIXME, this is just really too hacky...

%.lo: %.cu $(CUDA_DEPS)
	@echo "Tricking libtool"
	echo "void lotrickdummy() {}" > dummy.c
	$(LTCOMPILE) -MT $@ -MD -MP -MF "$(DEPDIR)/$*.Tpo" -c -o $@ dummy.c
	rm ${@:.lo=.o}
	rm dummy.c
	$(CUDACC) $(CUDACC_FLAGS) -c $< -o ${@:.lo=.o} || rm -f $@

CLEANFILES = *.fatbin.c *.cudafe1.c *.cudafe2.c *.stub.c *.cubin *.i *.ii *.ptx *.hash *.fatbin *.cudafe1.cpp *.cudafe1.gpu *.cudafe2.gpu *.cu.cpp

endif USE_CUDA

