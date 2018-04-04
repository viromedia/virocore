#!/bin/bash

echo Creating preload directory
rm -rf products/preload && mkdir -p products/preload
cp -r test/$1/preload/* products/preload
cp -r preload/* products/preload
