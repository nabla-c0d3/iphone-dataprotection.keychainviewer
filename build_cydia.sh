#!/bin/bash

BINARY="./cydia/Applications/KeychainViewer.app/KeychainViewer"
HGREV=`hg parents --template '{node|short}'`
#http://stackoverflow.com/questions/2708380/xcodebuild-how-to-define-preprocessor-macro
defines=("HGVERSION=\"${HGREV}\"")

rm -rf cydia/Applications/*
rm -rf build/

xcodebuild -arch armv6 \
           CODE_SIGNING_REQUIRED=NO \
           CODE_SIGN_IDENTITY=""  \
           CONFIGURATION_BUILD_DIR=cydia/Applications/ \
           DEBUG_INFORMATION_FORMAT=dwarf \
           OTHER_LDFLAGS="-weak_library /usr/lib/libSystem.B.dylib" \
           GCC_PREPROCESSOR_DEFINITIONS='$GCC_PREPROCESSOR_DEFINITIONS '"$(printf '%q ' "${defines[@]}")"

./patch_lc_version_min_iphoneos.py $BINARY

codesign -s - --entitlements Keychain/Entitlements.plist $BINARY

plutil -replace CFBundleExecutable -string rootstrap.sh cydia/Applications/KeychainViewer.app/Info.plist 

echo "Setting KeychainViewer setuid root"
sudo chown root:wheel $BINARY
sudo chmod +xs $BINARY

./dpkg-deb-fat -b cydia keychainviewer-${HGREV}.deb
