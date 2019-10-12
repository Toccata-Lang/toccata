#! /bin/sh

TOCCATA_DIR="$(cd -P -- $(dirname -- $0)/../ && pwd)"

PATH=$PATH:$TOCCATA_DIR

usage() {
    echo "Usage: ./bin/toccata.sh input.toc > output.c"
    exit 1
}

if [ -z "$1" ]; then
    usage
fi

toccata $1
