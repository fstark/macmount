# macmount
A linux utility to find mac partitions in SCSI2SD images

Note: also see my for of minivmac that supports .hda files.

# Building

```
fred@Fred-Linux:~/Development/macmount$ make macmount
g++     macmount.cpp   -o macmount
```

# Usage

For now usage is tailored to my workflow. The tool outputs the commands to mount the partitions under linux, and to link them into a interestingly named directory "~/AAA-Mac/temp" (from which I can easily drag/drop them to minivmac: https://github.com/fstark/minivmac)

There are two outputs, one to mount a disk image (for standard minivmac), the other one to mount it as a '.hda' file (for ZuluSCSI).

```
fred@Fred-Linux:~/Development/macmount$ sudo ./macmount /dev/sdh
[sudo] password for fred: 
* Device is 15634268160 bytes
* Scanning for apple partitions...
** Found Apple Partition Map at offset 0 (0)
**  Blocksize: 200 (512)
**  Blockcount: 3fffff (4194303)

# To mount as IMG:
DEVICE=`sudo losetup --find --show --offset 49152 --sizelimit 2147433984 /dev/sdh`
rm  ~/AAA-Mac/temp
ln -s $DEVICE ~/AAA-Mac/temp
sudo chmod o+rw $DEVICE

# To mount as hda:
DEVICE=`sudo losetup --find --show --offset 0 --sizelimit 2147483136 /dev/sdh`
rm  ~/AAA-Mac/temp
ln -s $DEVICE ~/AAA-Mac/temp
sudo chmod o+rw $DEVICE

# To unmount:
sudo losetup -d $DEVICE

** Found Apple Partition Map at offset 7ffffe00 (2147483136)
**  Blocksize: 200 (512)
**  Blockcount: 3fffff (4194303)

# To mount as IMG:
DEVICE=`sudo losetup --find --show --offset 2147532288 --sizelimit 2147433984 /dev/sdh`
rm  ~/AAA-Mac/temp
ln -s $DEVICE ~/AAA-Mac/temp
sudo chmod o+rw $DEVICE

# To mount as hda:
DEVICE=`sudo losetup --find --show --offset 2147483136 --sizelimit 2147483136 /dev/sdh`
rm  ~/AAA-Mac/temp
ln -s $DEVICE ~/AAA-Mac/temp
sudo chmod o+rw $DEVICE

# To unmount:
sudo losetup -d $DEVICE

```
