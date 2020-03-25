#!/bin/sh

# Script to regenerate compile-debug.sh and compile-release.sh
# Requires the jq JSON scripting package
# apt install jq
# Usage: on ubuntu host build natively and run script for debug and release
# cd iotech-c-utils
# ./scripts/linux.sh -broot $PWD
# ./scripts/gen-compile.sh $PWD debug
# ./scripts/gen-compile.sh $PWD release
# git add scripts/compile-debug.sh scripts/compile-release.sh

ROOT=$1
TYPE=$2
CONV="-e s;/usr/bin/cc;gcc; -e s;$ROOT;\$ROOT;g -e s/\"//g -e s;CMakeFiles/;;g "
OUTPUT="scripts/compile-${TYPE}.sh"

cat > ${OUTPUT} <<EOF
#!/bin/sh -x
ROOT=\$1
EOF
find ${TYPE}/c -name link.txt | xargs dirname | sed -e 's;.*CMakeFiles/;;' -e 's/^/mkdir /' >> ${OUTPUT}
jq '.[] | .command' ${TYPE}/compile_commands.json | sed ${CONV} -e 's/D_GNU_SOURCE/D_GNU_SOURCE -D__NO_STRING_INLINES -D_REDHAT_SEAWOLF_/' | grep -v 'c++' >> ${OUTPUT}
find ${TYPE}/c -name link.txt | xargs ls -tr | xargs sed $CONV -e 's;/usr/bin/;;' -e 's; [^ ]*/lib; lib;g' -e 's/\-ldl/-ldl -lrt/' -e 's/D_GNU_SOURCE/D_GNU_SOURCE -D__NO_STRING_INLINES/' >> ${OUTPUT}
