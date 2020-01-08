#!/bin/bash

# USAGE
#
# ./make 2 2 for task 2 with 2 threads
# ./make 3 2 for task 3 with 2 threads
# ...
#
# For using your own set of parameters to the linker and the precompiler
# Do not forget to set the -DTASK# for choosing the wanted task
# ./make -DTASK4 -D2 -DTIMER -lpthread for task 4 with 2 threads

#Initial arguments
ARGS=""

if [[ $1 =~ '^[0-9]+$' ]]; then
  for var in "$@"
  do
#    echo "i=${i+1} I=${I+1} ARGS=$ARGS"
  ARGS="$ARGS"" ""$var"
  done
else
  #THREADS=$2
  case $1 in
    2) ARGS="$ARGS"" ""-DTASK2"" ""-DTHREADS=$2" ;;
    3) ARGS="$ARGS"" ""-DTASK3 -lpthread"" ""-DTHREADS=$2" ;;
    4) ARGS="$ARGS"" ""-DTASK3 -DTIMER -lpthread"" ""-DTHREADS=$2" ;;
    4_1) ARGS="$ARGS"" ""-DTASK4_1 -DTIMER -lpthread"" ""-DTHREADS=$2" ;;
  esac
fi;

if [ ! -z  "$ARGS" ]; then
  echo "Arguments ${ARGS}"
fi;

if gcc -g -c green.c; then
  if gcc $ARGS -g -o test green.o green.h test.c ; then
    echo "SUCCESS"
  else
    echo "ERROR"
  fi;
else
  echo "ERROR"
fi;

exit 0;
