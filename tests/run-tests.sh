#!/usr/bin/env bash

set -euxo pipefail

function start_test {
    {
        sleep 0.5
        echo
        echo -e "\tTesting ${1}..."
        echo
    } 2> /dev/null
}

export PATH=$(realpath .):${PATH}

# start logging kernel messages
dmesg -C
dmesg -w &

start_test "loading kmodule"
insmod amnesiafs.ko

start_test "mkfs.amnesiafs"
disk="/dev/disk/by-id/scsi-0virtme_disk_test"
mkfs.amnesiafs "${disk}"
od -x "${disk}"

start_test "amnesiafs-store-passphrase"
key_name="$(hexdump -n 4 -e '4/4 "%08x" 1 "\n"' /dev/random | xargs)"
echo "my passphrase" | amnesiafs-store-passphrase "${key_name}" "${disk}"

keyctl show

start_test "no key_name option"
mkdir /tmp/mount
if mount -t amnesiafs "${disk}" "/tmp/mount"; then
    echo "mounting without key_name option should fail"
    exit 1
fi

start_test "undefined option"
if mount -t amnesiafs -o "key_name=${key_name},toot=42" "${disk}" "/tmp/mount"; then
    echo "mounting with bad options should fail"
    exit 1
fi

start_test "well formed key_name"
mount -t amnesiafs -o "key_name=${key_name}" "${disk}" "/tmp/mount"

start_test "listing"
ls -la "/tmp/"
ls -la "/tmp/mount"

start_test "umount"
umount "/tmp/mount"

start_test "key reuse"
if mount -t amnesiafs -o "key_name=${key_name}" "${disk}" "/tmp/mount"; then
    echo "key should have been revoked"
    exit 1
fi

start_test "unloading kmodule"
rmmod amnesiafs

{ printf "\n\t success (ॢ˘⌣˘ ॢ⑅)\n\n"; } 2> /dev/null
