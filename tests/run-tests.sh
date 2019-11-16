#!/usr/bin/env bash

set -euxo pipefail

lsmod

disk="/dev/disk/by-id/scsi-0virtme_disk_test"
#mkfs.xfs "${disk}"

dmesg -C
dmesg -w &

insmod amnesiafs.ko

xxd -l 48 "${disk}"
./mkfs.amnesiafs "${disk}"
sync
xxd -l 48 "${disk}"

mkdir /tmp/mount
mount -t amnesiafs -o "key_id=12345" "${disk}" "/tmp/mount"

mount
df -h
