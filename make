#!/bin/bash

if gcc -g -c green.c; then
  if gcc -g -o test green.o green.h test.c; then
    echo "SUCCESS"
  else
    echo "ERROR"
  fi;
else
  echo "ERROR"
fi;

exit 0;
