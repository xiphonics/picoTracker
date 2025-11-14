#!/bin/sh

# builds the html and pdf outputs

# first clear out any out generated content
rm -rf web

# pico edition first
bin/picosite -a static -s pico-edition/content -d pdfconfig.yaml

mkdir -p web/pico
mv output/* web/pico/

# now advance edition
rm -rf output
bin/picosite -a static -s advance-edition/content -d pdfconfig.yaml

mkdir -p web/advance
mv output/* web/advance/

cp landing/* web/

# cp output.pdf output/picoTracker-user-manual.pdf