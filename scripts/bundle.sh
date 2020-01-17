#!/bin/bash

set -o nounset -o pipefail -o errexit

TMP=$(mktemp -d)
trap 'rm -rf $TMP' EXIT

while getopts "f:r:b:-" OPT; do
    case $OPT in
        f) OUT=$OPTARG ;;
        r) ROOT=$OPTARG ;;
        b) BUILD=$OPTARG ;;
        -) break ;;
        ?) exit 2 ;;
    esac
done
shift $((OPTIND-1))

PREFIX=${PREFIX-/usr/local/bin}

install -D -t "$TMP/$PREFIX" "$BUILD"/bin/*
tar -cf- -C "$ROOT" . | tar -xf- -C "$TMP"

tar -cf "$OUT" -C "$TMP" .
