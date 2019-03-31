!#/bin/bash

set -x

docker run --rm -it -p 8888:8888 -v ${PWD}/notebooks:/home/jovyan/notebooks ksachdeva17/caf:latest
