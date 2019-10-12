#! /bin/sh

TOCCATA_DIR="$(cd -P -- $(dirname -- $0)/../ && pwd)"

clang -g \
  -v \
  -fno-objc-arc \
  -o toccata \
  -I$TOCCATA_DIR \
  -std=c99 \
  -DCHECK_MEM_LEAK=1 \
  $TOCCATA_DIR/core.c \
  toccata.c \
  -lpthread
