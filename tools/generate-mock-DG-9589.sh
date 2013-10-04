#!/bin/bash

# generate mock files
rm -r /mnt/files/tangle099.prodx/sites/g/files

# f directory and files symlink
mkdir -p /mnt/files/tangle099.prodx/sites/g/files/g2000003316/f
touch /mnt/files/tangle099.prodx/sites/g/files/g2000003316/f/README-f
ln -s /mnt/files/tangle099.prodx/sites/g/files/g2000003316/f /mnt/files/tangle099.prodx/sites/g/files/g2000003316/files

mkdir -p /mnt/files/tangle099.prodx/sites/g/files/g2000003317/f
touch /mnt/files/tangle099.prodx/sites/g/files/g2000003317/f/README-f
ln -s /mnt/files/tangle099.prodx/sites/g/files/g2000003317/f /mnt/files/tangle099.prodx/sites/g/files/g2000003317/files

# two directories
mkdir -p /mnt/files/tangle099.prodx/sites/g/files/g2000003361/f
touch /mnt/files/tangle099.prodx/sites/g/files/g2000003361/f/README-f
mkdir -p /mnt/files/tangle099.prodx/sites/g/files/g2000003361/files
touch /mnt/files/tangle099.prodx/sites/g/files/g2000003361/files/README-files

mkdir -p /mnt/files/tangle099.prodx/sites/g/files/g2000003362/f
touch /mnt/files/tangle099.prodx/sites/g/files/g2000003362/f/README-f
mkdir -p /mnt/files/tangle099.prodx/sites/g/files/g2000003362/files
touch /mnt/files/tangle099.prodx/sites/g/files/g2000003362/files/README-files
