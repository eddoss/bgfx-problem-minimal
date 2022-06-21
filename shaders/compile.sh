#!/usr/bin/bash

THIS_DIR=$( dirname -- "$0"; )

shaderc -f "$THIS_DIR/mesh/vert.sc" -o "$THIS_DIR/mesh/vert.bin" --type v --platform linux --profile spirv --varyingdef "$THIS_DIR/mesh/varying.def.sc" -i $THIS_DIR
shaderc -f "$THIS_DIR/mesh/frag.sc" -o "$THIS_DIR/mesh/frag.bin" --type f --platform linux --profile spirv --varyingdef "$THIS_DIR/mesh/varying.def.sc" -i $THIS_DIR