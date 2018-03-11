#!/bin/bash

echo Copying test resources
cp -r test/ products/build/test
echo Running ViroRenderer
emrun --no_browser --port 8080 ./products/build/
