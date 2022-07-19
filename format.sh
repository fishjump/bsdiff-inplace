#!env bash

PROJ_DIR=$(dirname $(readlink -f $0))
echo "==== formatig at ${PROJ_DIR} ===="
find include src -iname "*.h" -o -iname "*.c" | xargs clang-format -style=file -i