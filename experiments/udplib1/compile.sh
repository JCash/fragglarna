#!/usr/bin/env bash

BUILDDIR=build

mkdir -p $BUILDDIR

set -e

CFLAGS="-O0 -std=c99"

clang -o $BUILDDIR/client $CFLAGS socket.c common.c client.c
clang -o $BUILDDIR/server $CFLAGS socket.c common.c server.c
