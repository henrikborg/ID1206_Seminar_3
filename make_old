#!/bin/bash

#Initial arguments
ARGS=""

I=$[$1+1]
i=0
#echo "I=$I 0=${0} 1=${1} 2=${2} 3=${3}"

for var in "$@"
do
#  echo "i=${i+1} I=${I+1} ARGS=$ARGS"
  ARGS="$ARGS"" ""$var"
done

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
