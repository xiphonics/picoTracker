#!/bin/sh

# builds the html and pdf outputs

bin/picosite -a static -s content -d pdfconfig.yaml

cp output.pdf output/picoTracker-user-manual.pdf