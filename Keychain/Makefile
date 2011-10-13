SDKVER?=4.3
CC=/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin/arm-apple-darwin10-gcc-4.0.1 
SDK=/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS$(SDKVER).sdk/
CFLAGS=-Wall -isysroot $(SDK) -framework IOKit -framework CoreFoundation -framework Security -O3 -I.

all: IOKit keychain_dump

IOKit:
	ln -s /System/Library/Frameworks/IOKit.framework/Versions/Current/Headers IOKit

sqlite3.c:
	curl -O http://www.sqlite.org/sqlite-amalgamation-3070800.zip
	unzip sqlite-amalgamation-3070800.zip sqlite-amalgamation-3070800/sqlite3.c sqlite-amalgamation-3070800/sqlite3.h 
	mv sqlite-amalgamation-3070800/* .
	rmdir sqlite-amalgamation-3070800

keychain_dump: keychain.c keychain_dump.c IOKit.c keychain3.c keychain4.c keychain5.c
	$(CC) $(CFLAGS) -lsqlite3 -o $@ $^
	ldid -SEntitlements.plist $@

keychain_dump_static: keychain.c keychain_dump.c IOKit.c keychain3.c keychain4.c keychain5.c sqlite3.c
	$(CC) $(CFLAGS) -o $@ $^
	ldid -SEntitlements.plist $@

clean:
	rm -f keychain_dump keychain_dump_static

