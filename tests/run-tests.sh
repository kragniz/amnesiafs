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

key_id=$(echo -n "my passphrase" | keyctl padd user amnesiafs:key @u)

mkdir /tmp/mount
mount -t amnesiafs -o "key_id=${key_id}" "${disk}" "/tmp/mount"

mount
df -h
