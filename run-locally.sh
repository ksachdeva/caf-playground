!#/bin/bash

set -x

docker run --rm -it -p 8888:8888 -e JUPYTER_ENABLE_LAB=yes -v ${PWD}/notebooks:/home/jovyan/notebooks ksachdeva17/caf:latest
