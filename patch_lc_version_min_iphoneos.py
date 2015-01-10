#!/usr/bin/env python

import struct
import sys

#hack to patch the LC_VERSION_MIN_IPHONEOS load command
#of armv6 binaries produced with Xcode 4.4.1 (iOS 5.1 SDK)
#this script sets the version info to iOS 10 so that when run on 
#iOS 7/8, the new UITableView style is used, and the
#binary still works on the original iphone running iOS 3
#before (otool -lv)
#   cmd LC_VERSION_MIN_IPHONEOS
#   cmdsize 16
#   version 5.1
#   sdk 5.1
#after
#   version 10.0
#   sdk 10.0

LC_VERSION_MIN_IPHONEOS = 0x25

if len(sys.argv) != 2:
    print "Usage: %s macho_binary" % sys.argv[0]
    exit(0)

filename = sys.argv[1]
f = open(filename, "r+b", 0)

magic,cputype,_,_,ncmds,sizeofcmds,flags = struct.unpack("<LLLLLLL", f.read(7*4))

assert (magic == 0xfeedface)

for i in xrange(ncmds):
    cmd, cmdsize = struct.unpack("<LL", f.read(8))
    if cmd == LC_VERSION_MIN_IPHONEOS and cmdsize == 16:
        print "Patching LC_VERSION_MIN_IPHONEOS command"
        f.write(struct.pack("<LL", 0x0A0000, 0x0A0000)) #iOS 10 version/sdk
        break
    f.seek(f.tell() + cmdsize - 8)

f.close()
