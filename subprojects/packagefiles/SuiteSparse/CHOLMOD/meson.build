source = [
   configure_file(input: 'Config/cholmod.h.in', output: 'cholmod.h', configuration: config),
   'Check/cholmod_check.c',            # 'Check/cholmod_l_check.c',
   'Check/cholmod_read.c',             # 'Check/cholmod_l_read.c',
   'Check/cholmod_write.c',            # 'Check/cholmod_l_write.c',
   'Cholesky/cholmod_amd.c',           # 'Cholesky/cholmod_l_amd.c',
   'Cholesky/cholmod_analyze.c',       # 'Cholesky/cholmod_l_analyze.c',
   'Cholesky/cholmod_colamd.c',        # 'Cholesky/cholmod_l_colamd.c',
   'Cholesky/cholmod_etree.c',         # 'Cholesky/cholmod_l_etree.c',
   'Cholesky/cholmod_factorize.c',     # 'Cholesky/cholmod_l_factorize.c',
   'Cholesky/cholmod_postorder.c',     # 'Cholesky/cholmod_l_postorder.c',
   'Cholesky/cholmod_rcond.c',         # 'Cholesky/cholmod_l_rcond.c',
   'Cholesky/cholmod_resymbol.c',      # 'Cholesky/cholmod_l_resymbol.c',
   'Cholesky/cholmod_rowcolcounts.c',  # 'Cholesky/cholmod_l_rowcolcounts.c',
   'Cholesky/cholmod_rowfac.c',        # 'Cholesky/cholmod_l_rowfac.c',
   'Cholesky/cholmod_solve.c',         # 'Cholesky/cholmod_l_solve.c',
   'Cholesky/cholmod_spsolve.c',       # 'Cholesky/cholmod_l_spsolve.c',
   'Core/cholmod_aat.c',               # 'Core/cholmod_l_aat.c',
   'Core/cholmod_add.c',               # 'Core/cholmod_l_add.c',
   'Core/cholmod_band.c',              # 'Core/cholmod_l_band.c',
   'Core/cholmod_change_factor.c',     # 'Core/cholmod_l_change_factor.c',
   'Core/cholmod_common.c',            # 'Core/cholmod_l_common.c',
   'Core/cholmod_complex.c',           # 'Core/cholmod_l_complex.c',
   'Core/cholmod_copy.c',              # 'Core/cholmod_l_copy.c',
   'Core/cholmod_dense.c',             # 'Core/cholmod_l_dense.c',
   'Core/cholmod_error.c',             # 'Core/cholmod_l_error.c',
   'Core/cholmod_factor.c',            # 'Core/cholmod_l_factor.c',
   'Core/cholmod_memory.c',            # 'Core/cholmod_l_memory.c',
   'Core/cholmod_sparse.c',            # 'Core/cholmod_l_sparse.c',
   'Core/cholmod_transpose.c',         # 'Core/cholmod_l_transpose.c',
   'Core/cholmod_triplet.c',           # 'Core/cholmod_l_triplet.c',
   'Core/cholmod_version.c',           # 'Core/cholmod_l_version.c',
   'MatrixOps/cholmod_drop.c',         # 'MatrixOps/cholmod_l_drop.c',
   'MatrixOps/cholmod_horzcat.c',      # 'MatrixOps/cholmod_l_horzcat.c',
   'MatrixOps/cholmod_norm.c',         # 'MatrixOps/cholmod_l_norm.c',
   'MatrixOps/cholmod_scale.c',        # 'MatrixOps/cholmod_l_scale.c',
   'MatrixOps/cholmod_sdmult.c',       # 'MatrixOps/cholmod_l_sdmult.c',
   'MatrixOps/cholmod_ssmult.c',       # 'MatrixOps/cholmod_l_ssmult.c',
   'MatrixOps/cholmod_submatrix.c',    # 'MatrixOps/cholmod_l_submatrix.c',
   'MatrixOps/cholmod_symmetry.c',     # 'MatrixOps/cholmod_l_symmetry.c',
   'MatrixOps/cholmod_vertcat.c',      # 'MatrixOps/cholmod_l_vertcat.c',
   'Modify/cholmod_rowadd.c',          # 'Modify/cholmod_l_rowadd.c',
   'Modify/cholmod_rowdel.c',          # 'Modify/cholmod_l_rowdel.c',
   'Modify/cholmod_updown.c',          # 'Modify/cholmod_l_updown.c',
   # Naev does not need Partition/* or Supernodal/*
]

include = include_directories([
   'Include',
   '../AMD/Include',
   '../COLAMD/Include',
   '../SuiteSparse_config',
])

cc = meson.get_compiler('c')
system_deps = [
   cc.find_library('m', required : false)
]

libcholmod = static_library(
   'cholmod',
   source,
   c_args: ['-DNCAMD', '-DNPARTITION', '-DNSUPERNODAL'],
   dependencies: system_deps,
   include_directories: include
)

CHOLMOD_dep = declare_dependency(
   link_with: libcholmod,
   include_directories: include,
   compile_args: ['-DNCAMD', '-DNPARTITION', '-DNSUPERNODAL'],
   link_args: ['-DNCAMD', '-DNPARTITION', '-DNSUPERNODAL'],
   dependencies: system_deps
)
