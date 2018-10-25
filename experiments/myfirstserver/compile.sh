#!/usr/bin/env bash

BUILDDIR=build

mkdir -p $BUILDDIR

set -e

CFLAGS="-O0"

clang -o $BUILDDIR/client $CFLAGS client.c
clang -o $BUILDDIR/server $CFLAGS server.c
