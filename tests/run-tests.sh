#!/usr/bin/env bash

set -euxo pipefail

lsmod

disk="/dev/disk/by-id/scsi-0virtme_disk_test"
#mkfs.xfs "${disk}"

dmesg -C
dmesg -w &

insmod amnesiafs.ko

mkdir /tmp/mount
mount -t amnesiafs "${disk}" "/tmp/mount"

mount
df -h
