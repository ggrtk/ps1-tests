#!/bin/bash

docker run -it -v $(pwd):/build jaczekanski/psn00bsdk:latest make
