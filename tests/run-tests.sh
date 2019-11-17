#!/usr/bin/env bash

set -euxo pipefail

# start logging kernel messages
dmesg -C
dmesg -w &

insmod amnesiafs.ko

disk="/dev/disk/by-id/scsi-0virtme_disk_test"
./mkfs.amnesiafs "${disk}"
od -x "${disk}"

key_name="amnesiafs:$(hexdump -n 4 -e '4/4 "%08x" 1 "\n"' /dev/random | xargs)"
echo -n "my passphrase" | keyctl padd logon "${key_name}" @u

mkdir /tmp/mount

if mount -t amnesiafs "${disk}" "/tmp/mount"; then
    echo "mounting without key_name option should have failed"
    exit 1
fi

if mount -t amnesiafs -o "key_name=${key_name},toot=42" "${disk}" "/tmp/mount"; then
    echo "mounting with bad options should have failed"
    exit 1
fi

mount -t amnesiafs -o "key_name=${key_name}" "${disk}" "/tmp/mount"
