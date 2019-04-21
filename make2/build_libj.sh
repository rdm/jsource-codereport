#!/bin/bash

cd "$(dirname "$(readlink -f "$0" || realpath "$0")")"

jplatform="${jplatform:=linux}"
j64="${j64:=j64}"

# gcc 5 vs 4 - killing off linux asm routines (overflow detection)
# new fast code uses builtins not available in gcc 4
# use -DC_NOMULTINTRINSIC to continue to use more standard c in version 4
# too early to move main linux release package to gcc 5

macmin="-mmacosx-version-min=10.6"

if [ "x$CC" = x'' ] ; then
if [ -f "/usr/bin/cc" ]; then
CC=cc
else
if [ -f "/usr/bin/clang" ]; then
CC=clang
else
CC=gcc
fi
fi
export CC
fi
# compiler=`$CC --version | head -n 1`
compiler=`readlink -f $(command -v $CC)`
echo "CC=$CC"
echo "compiler=$compiler"

USE_OPENMP="${USE_OPENMP:=0}"
if [ $USE_OPENMP -eq 1 ] ; then
OPENMP=" -fopenmp "
LDOPENMP=" -fopenmp "
if [ -z "${compiler##*gcc*}" ]; then
LDOPENMP32=" -l:libgomp.so.1 "    # gcc
else
LDOPENMP32=" -l:libomp.so.5 "     # clang
fi
fi

if [ -z "${compiler##*gcc*}" ]; then
# gcc
common="$OPENMP -fPIC -O1 -fwrapv -fno-strict-aliasing -Wextra -Wno-maybe-uninitialized -Wno-unused-parameter -Wno-sign-compare -Wno-clobbered -Wno-empty-body -Wno-unused-value -Wno-pointer-sign -Wno-parentheses"
OVER_GCC_VER6=$(echo `$CC -dumpversion | cut -f1 -d.` \>= 6 | bc)
if [ $OVER_GCC_VER6 -eq 1 ] ; then
common="$common -Wno-shift-negative-value"
else
common="$common -Wno-type-limits"
fi
# alternatively, add comment /* fall through */
OVER_GCC_VER7=$(echo `$CC -dumpversion | cut -f1 -d.` \>= 7 | bc)
if [ $OVER_GCC_VER7 -eq 1 ] ; then
common="$common -Wno-implicit-fallthrough"
fi
OVER_GCC_VER8=$(echo `$CC -dumpversion | cut -f1 -d.` \>= 8 | bc)
if [ $OVER_GCC_VER8 -eq 1 ] ; then
common="$common -Wno-cast-function-type"
fi
else
# clang 3.5 .. 5.0
common="$OPENMP -Werror -fPIC -O1 -fwrapv -fno-strict-aliasing -Wextra -Wno-consumed -Wno-uninitialized -Wno-unused-parameter -Wno-sign-compare -Wno-empty-body -Wno-unused-value -Wno-pointer-sign -Wno-parentheses -Wno-unsequenced -Wno-string-plus-int"
fi
darwin="$OPENMP -fPIC -O1 -fwrapv -fno-strict-aliasing -Wno-string-plus-int -Wno-empty-body -Wno-unsequenced -Wno-unused-value -Wno-pointer-sign -Wno-parentheses -Wno-return-type -Wno-constant-logical-operand -Wno-comment -Wno-unsequenced"

case $jplatform\_$j64 in

linux_j32) # linux x86
TARGET=libj.so
# faster, but sse2 not available for 32-bit amd cpu
# sse does not support mfpmath=sse in 32-bit gcc
CFLAGS="$common -m32 -msse2 -mfpmath=sse -DC_NOMULTINTRINSIC "
# slower, use 387 fpu and truncate extra precision
# CFLAGS="$common -m32 -ffloat-store "
LDFLAGS=" -shared -Wl,-soname,libj.so -m32 -lm -ldl $LDOPENMP32"
;;

linux_j64nonavx) # linux intel 64bit nonavx
TARGET=libj.so
CFLAGS="$common "
LDFLAGS=" -shared -Wl,-soname,libj.so -lm -ldl $LDOPENMP"
;;

linux_j64) # linux intel 64bit avx
TARGET=libj.so
CFLAGS="$common -mavx -DC_AVX=1 -mfma "
LDFLAGS=" -shared -Wl,-soname,libj.so -lm -ldl $LDOPENMP"
OBJS_FMA=" gemm_int-fma.o "
;;

raspberry_j32) # linux raspbian arm
TARGET=libj.so
CFLAGS="$common -marm -march=armv6 -mfloat-abi=hard -mfpu=vfp -DRASPI -DC_NOMULTINTRINSIC "
LDFLAGS=" -shared -Wl,-soname,libj.so -lm -ldl $LDOPENMP"
;;

raspberry_j64) # linux arm64
TARGET=libj.so
CFLAGS="$common -march=armv8-a+crc -DRASPI -DC_CRC32C=1 "
LDFLAGS=" -shared -Wl,-soname,libj.so -lm -ldl $LDOPENMP"
;;

darwin_j32) # darwin x86
TARGET=libj.dylib
CFLAGS="$darwin -m32 $macmin"
LDFLAGS=" -dynamiclib -lm -ldl $LDOPENMP -m32 $macmin"
;;

darwin_j64nonavx) # darwin intel 64bit nonavx
TARGET=libj.dylib
CFLAGS="$darwin $macmin"
LDFLAGS=" -dynamiclib -lm -ldl $LDOPENMP $macmin"
;;

darwin_j64) # darwin intel 64bit
TARGET=libj.dylib
CFLAGS="$darwin -mavx $macmin -DC_AVX=1 -mfma "
LDFLAGS=" -dynamiclib -lm -ldl $LDOPENMP $macmin"
OBJS_FMA=" gemm_int-fma.o "
;;

*)
echo no case for those parameters
exit
esac

echo "CFLAGS=$CFLAGS"

mkdir -p ../bin/$jplatform/$j64
mkdir -p obj/$jplatform/$j64/
cp makefile-libj obj/$jplatform/$j64/.
export CFLAGS LDFLAGS TARGET OBJS_FMA jplatform j64
cd obj/$jplatform/$j64/
make -f makefile-libj
cd -
