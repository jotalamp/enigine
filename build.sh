#!/bin/bash

# TODO: variable user/channel
#conan create . user/dev
conan create . user/dev -s compiler.libcxx=libstdc++11
