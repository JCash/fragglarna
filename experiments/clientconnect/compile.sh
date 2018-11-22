#!/usr/bin/env bash

BUILDDIR=build

mkdir -p $BUILDDIR

set -e

CFLAGS="-g -O0 -g -fsanitize=address -fno-omit-frame-pointer -std=c99"

clang -o $BUILDDIR/client $CFLAGS socket.c message.c common.c client.c
clang -o $BUILDDIR/server $CFLAGS socket.c message.c common.c server.c
