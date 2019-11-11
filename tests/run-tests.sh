#!/usr/bin/env bash

set -euxo pipefail

lsmod

disk="/dev/disk/by-id/scsi-0virtme_disk_test"
mkfs.xfs "${disk}"

mkdir /tmp/mount
mount -t xfs "${disk}" "/tmp/mount"

ls /tmp/mount
df -h
