CC=/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin/arm-apple-darwin10-gcc-4.2.1 
CFLAGS=-isysroot /Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS4.2.sdk/
CFLAGS_IOKIT=$(CFLAGS) -I/usr/local/include -L./lib -lIOKit -framework Security -O3 -lsqlite3 -lobjc -framework Foundation -framework CoreFoundation -framework UIKit -I./FMDB -I./Keychain -I./GUI

SRC=main.m FMDB/FMDatabase.m FMDB/FMResultSet.m FMDB/FMDatabaseAdditions.m GUI/DetailViewController.m GUI/GenericTableViewController.m GUI/KeychainViewerAppDelegate.m GUI/ListViewController.m GUI/RootViewController.m Keychain/IOKit.c Keychain/keychain.c Keychain/Keychain.m

all: KeychainViewer

KeychainViewer: $(SRC)
	$(CC) $(CFLAGS_IOKIT) -o $@ $^
	ldid -SKeychain/Entitlements.plist $@

