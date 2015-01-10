ARCH?=armv6
HGVERSION:= $(shell hg parents --template '{node|short}' || echo "unknown")
SDK=$(shell xcodebuild -version -sdk iphoneos Path)
CC=clang -Wall -arch $(ARCH)
CFLAGS=-isysroot $(SDK) -DHGVERSION="\"${HGVERSION}\"" -framework IOKit -framework Security -framework Foundation -framework CoreFoundation -framework UIKit  -lsqlite3 -I./Keychain -I./GUI -O3

SRC=main.m GUI/DetailViewController.m GUI/GenericTableViewController.m GUI/KeychainViewerAppDelegate.m GUI/ListViewController.m GUI/RootViewController.m Keychain/IOKit.c Keychain/keychain.c Keychain/keychain3.c Keychain/keychain4.c Keychain/keychain5.c keychain/der_decode_plist.c

all: KeychainViewer

KeychainViewer: $(SRC)
	$(CC) $(CFLAGS) -o $@ $^
	codesign -s - --entitlements Keychain/Entitlements.plist $@

