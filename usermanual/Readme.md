# Picotracker Manual

## Development

While developing and writing content for the manual, you can have a preview of the site by running:
```
./preview.sh
```

If you need to download to run picosite you download a release from GitHub, eg. for a Linux binary:

```
mkdir usermanual/bin; curl -L "https://github.com/maks/picosite/releases/download/0.2.0/picosite-linux" -o "usermanual/bin/picosite"; chmod +x usermanual/bin/picosite
```

When run in preview mode, `picosite` will serve the website on the url:  http://localhost:8080 and will watch the site directory and its subdirectories for file changes and rebuild the output when it detects modifications to the files.

## Building Outputs

Uses picosite and weasyprint to generate HTML and PDF outputs from the markdown source files.


### HTML

To build the html output run:
```
cd usermanual
./build.sh
```

### PDF

TODO: Instructions on building PDF with weasyprint.

## Acknowledgements

Thanks to @squiggythings [WaveTracker documentation website](https://github.com/squiggythings/wavetracker-site) for initial CSS design and example of how to use nice, clean semantic HTML.

