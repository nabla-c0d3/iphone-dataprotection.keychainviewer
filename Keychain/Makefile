SDKVER?=5.1
ARCH?=armv6
CC=clang -arch $(ARCH)
SDK=/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS$(SDKVER).sdk/
CFLAGS=-Wall -isysroot $(SDK) -framework IOKit -framework CoreFoundation -framework Security -I.

all: keychain_dump

IOKit:
	ln -s /System/Library/Frameworks/IOKit.framework/Versions/Current/Headers IOKit

sqlite3.c:
	curl -O http://www.sqlite.org/sqlite-amalgamation-3070800.zip
	unzip sqlite-amalgamation-3070800.zip sqlite-amalgamation-3070800/sqlite3.c sqlite-amalgamation-3070800/sqlite3.h 
	mv sqlite-amalgamation-3070800/* .
	rmdir sqlite-amalgamation-3070800

keychain_dump: keychain.c keychain_dump.c IOKit.c keychain3.c keychain4.c keychain5.c der_decode_plist.c
	$(CC) $(CFLAGS) -lsqlite3 -o $@ $^
	codesign -s - --entitlements Entitlements.plist $@

keychain_dump_static: keychain.c keychain_dump.c IOKit.c keychain3.c keychain4.c keychain5.c der_decode_plist.c sqlite3.c
	$(CC) $(CFLAGS) -o $@ $^
	codesign -s - --entitlements Entitlements.plist $@

clean:
	rm -f keychain_dump keychain_dump_static

