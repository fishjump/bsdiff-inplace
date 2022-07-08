#!env bash

PROJ_DIR=$(dirname $(dirname $(readlink -f $0)))

# cmake
echo "==== Generating Makefile Project at ${PROJ_DIR}/build ===="
cd ${PROJ_DIR}
cmake -B cd ${PROJ_DIR}/build ${PROJ_DIR}

# make
echo "==== Compiling at ${PROJ_DIR}/build ===="
make -C ${PROJ_DIR}/build

# demo
echo "==== Compiling Demo at ${PROJ_DIR}/demo ===="
touch ${PROJ_DIR}/demo/hello1 ${PROJ_DIR}/demo/hello2 ${PROJ_DIR}/demo/hello3 ${PROJ_DIR}/demo/hello_patch
rm ${PROJ_DIR}/demo/hello1 ${PROJ_DIR}/demo/hello2 ${PROJ_DIR}/demo/hello3 ${PROJ_DIR}/demo/hello_patch
gcc ${PROJ_DIR}/demo/hello1.c -o ${PROJ_DIR}/demo/hello1
gcc ${PROJ_DIR}/demo/hello2.c -o ${PROJ_DIR}/demo/hello2

${PROJ_DIR}/build/src/bin/bsdiff_bin ${PROJ_DIR}/demo/hello1 ${PROJ_DIR}/demo/hello2 ${PROJ_DIR}/demo/hello_patch
${PROJ_DIR}/build/src/bin/bspatch_bin ${PROJ_DIR}/demo/hello1 ${PROJ_DIR}/demo/hello3 ${PROJ_DIR}/demo/hello_patch

# run
echo ""
echo "==== Output of hello1 ===="
${PROJ_DIR}/demo/hello1
echo "==== Output of hello1 end ===="
echo ""

echo ""
echo "==== Output of hello2 ===="
${PROJ_DIR}/demo/hello2
echo "==== Output of hello2 end ===="
echo ""

echo ""
echo "==== Output of hello3(in-place patched) ===="
${PROJ_DIR}/demo/hello3
echo "==== Output of hello3 end ===="
echo ""

# show patch sz
ls -la ${PROJ_DIR}/demo/hello1 ${PROJ_DIR}/demo/hello2 ${PROJ_DIR}/demo/hello3 ${PROJ_DIR}/demo/hello_patch
