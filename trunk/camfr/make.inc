# compiler environment

cc     = gcc
CC     = g++
F77    = g77
LINKER = g++
MAKE   = make
ARCH   = ar
AFLAGS = cr
RANLIB = ranlib

# location of the Blitz library

BLITZ  = /home/pbienst/blitz-20001213

# location of the BLAS/LAPACK libraries

#LAPACK = /home/pbienst/ATLAS/lib/Linux_PIIISSE1/liblapack.a \
#	 /home/pbienst/ATLAS/lib/Linux_PIIISSE1/libcblas.a \
#	 /home/pbienst/ATLAS/lib/Linux_PIIISSE1/libf77blas.a \
#	 /home/pbienst/ATLAS/lib/Linux_PIIISSE1/libatlas.a

LAPACK = /usr/local/intel/mkl/LIB/libmkl32_lapack.a \
	 /usr/local/intel/mkl/LIB/libmkl32_p3.a \

# location of the Python header files

PYTHON = /home/pbienst/local/include/python2.1

# location of the Boost library include files

BOOST_INC = /home/pbienst/boost_1_25_0

# location of the Boost library

BOOST_LIB = /home/pbienst/boost_1_25/libs/python/src

# location of the math libraries (usually -lm, but can be replaced by
# other routines to improve performance)

MATH   = -lm

# compiler flags

#FLAGS  = -fPIC -O3 --param max-inline-insns=600 -march=pentiumpro \
#	-ftemplate-depth-30  -ffast-math \
#      	-funroll-loops -fstrict-aliasing -I$(BLITZ)\
#	-I$(PYTHON) -I$(BOOST_INC)
FLAGS  =  -g -fPIC -I$(BLITZ) -I$(PYTHON) -I$(BOOST_INC)
#FLAGS  = -fPIC -I$(BLITZ) -I$(PYTHON) -ftemplate-depth-30 -g \
#	-march=pentiumpro -I$(BOOST_INC)
FFLAGS = $(FLAGS) # Fortran flags

# linker flags

LFLAGS = $(BLITZ)/lib/libblitz.a $(BOOST_LIB)/libboost_python.a $(LAPACK) \
	$(MATH) -lg2c

# Format of Fortran symbol names. Possible values are 
#
#   -DFORTRAN_SYMBOLS_WITHOUT_TRAILING_UNDERSCORES
#   -DFORTRAN_SYMBOLS_WITH_SINGLE_TRAILING_UNDERSCORE
#   -DFORTRAN_SYMBOLS_WITH_DOUBLE_TRAILING_UNDERSCORES

FSYMB  = -DFORTRAN_SYMBOLS_WITH_SINGLE_TRAILING_UNDERSCORE