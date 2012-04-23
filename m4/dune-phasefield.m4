dnl -*- autoconf -*-
# Macros needed to find dune-phasefield and dependent libraries.  They are called by
# the macros in ${top_src_dir}/dependencies.m4, which is generated by
# "dunecontrol autogen"

# Additional checks needed to build dune-phasefield
# This macro should be invoked by every module which depends on dune-phasefield, as
# well as by dune-phasefield itself
AC_DEFUN([DUNE_PHASEFIELD_CHECKS])

# Additional checks needed to find dune-phasefield
# This macro should be invoked by every module which depends on dune-phasefield, but
# not by dune-phasefield itself
AC_DEFUN([DUNE_PHASEFIELD_CHECK_MODULE],
[
  DUNE_CHECK_MODULES([dune-phasefield],[phasefield/phasefield.hh])
])
