function get_version {
   VERSION="$(cat ${NAEVDIR}/VERSION)"
   # Get version, negative minors mean betas
   if [[ -n $(echo "${VERSION}" | grep "-") ]]; then
      BASEVER=$(echo "${VERSION}" | sed 's/\.-.*//')
      BETAVER=$(echo "${VERSION}" | sed 's/.*-//')
      VERSION="${BASEVER}.0-beta${BETAVER}"
   fi
   return 0
}

# Install Inetc
mkdir -p ~/temp
wget https://nsis.sourceforge.io/mediawiki/images/c/c9/Inetc.zip -P ~/temp
unzip ~/temp/Inetc.zip -d ~/temp
cp -r ~/temp/Plugins/amd64-unicode/* /mingw64/share/nsis/Plugins/unicode
rm -rf ~/temp

# Move compiled binary to staging folder.
mkdir -p extras/windows/installer/bin
cp src/naev.exe extras/windows/installer/bin/naev-0.8.0-beta1-win64.exe

# Collect DLLs
for fn in `cygcheck src/naev.exe | grep -i "mingw64"`; do
    echo "copying $fn to staging area"
    cp $fn extras/windows/installer/bin
done

makensis -DVERSION=0.8.0 -DVERSION_SUFFIX=-beta1 -DARCH=64 extras/windows/installer/naev.nsi
