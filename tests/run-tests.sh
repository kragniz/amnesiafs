#!/usr/bin/env bash

set -euxo pipefail

# start logging kernel messages
dmesg -C
dmesg -w &

insmod amnesiafs.ko

disk="/dev/disk/by-id/scsi-0virtme_disk_test"
./mkfs.amnesiafs "${disk}"
xxd -l 48 "${disk}"

key_id=$(echo -n "my passphrase" | keyctl padd user amnesiafs:key @u)

mkdir /tmp/mount

if mount -t amnesiafs "${disk}" "/tmp/mount"; then
    echo "mounting without key_id option should have failed"
    exit 1
fi

if mount -t amnesiafs -o "key_id=${key_id},toot=42" "${disk}" "/tmp/mount"; then
    echo "mounting with bad options should have failed"
    exit 1
fi

mount -t amnesiafs -o "key_id=${key_id}" "${disk}" "/tmp/mount"
