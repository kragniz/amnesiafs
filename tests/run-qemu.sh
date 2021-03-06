#!/usr/bin/env bash

set -euxo pipefail

here="$(dirname $0)"
tests_dir="$(realpath "$here")"
src_dir="$(realpath "$here"/..)"

test_drive="${tests_dir}/test.img"
dd if=/dev/zero of="${test_drive}" bs=4096 count=128KiB

virtme-run \
    --installed-kernel \
    --cwd $src_dir \
    --disk "test=${test_drive}" \
    --script-sh "$tests_dir/run-tests.sh" \
    --qemu-opts -m 2048 -smp 2
