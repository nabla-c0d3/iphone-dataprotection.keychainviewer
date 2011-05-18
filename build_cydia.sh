#!/bin/bash
IPHONE_HOST="localhost"
SSHPORT=2222

cp -R build/Release-iphoneos/KeychainViewer.app/ cydia/Applications/KeychainViewer.app/
make
mv KeychainViewer cydia/Applications/KeychainViewer.app/KeychainViewer

ssh -p $SSHPORT root@$IPHONE_HOST 'rm -rf /var/root/cydia'
scp -P $SSHPORT -r cydia root@$IPHONE_HOST:/var/root
ssh -p $SSHPORT root@$IPHONE_HOST 'chmod -R 755 cydia/Applications/KeychainViewer.app; chown root:wheel cydia/Applications/KeychainViewer.app/KeychainViewer; chmod +xs cydia/Applications/KeychainViewer.app/KeychainViewer; chmod 755 cydia/DEBIAN ; dpkg-deb -b cydia'
scp -P $SSHPORT root@$IPHONE_HOST:/var/root/cydia.deb keychainviewer.deb