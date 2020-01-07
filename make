#!/bin/bash

# USAGE
#
# ./make 2 for task 2
# ./make 3 for task 3
# ...
#
# For using your own set of parameters to the linker and the precompiler
# Do not forget to set the -DTASK# for choosing the wanted task
# ./make -DTASK4 -DTIMER -lpthread for task 4

#Initial arguments
ARGS=""

I=$[$1+1]
i=0
#echo "I=$I 0=${0} 1=${1} 2=${2} 3=${3}"

if [[ $1 =~ '^[0-9]+$' ]]; then
  for var in "$@"
  do
#    echo "i=${i+1} I=${I+1} ARGS=$ARGS"
  ARGS="$ARGS"" ""$var"
  done
else
  case $1 in
    2) ARGS="$ARGS"" ""-DTASK2" ;;
    3) ARGS="$ARGS"" ""-DTASK3" ;;
    4) ARGS="$ARGS"" ""-DTASK4 -DTIMER -lpthread" ;;
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
