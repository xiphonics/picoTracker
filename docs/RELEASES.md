## How to make a new release 

While all releases are built on CI using GitHub Actions on the `master` branch, some manual steps are still required to create a new release:

1. Update the build number (`PROJECT_NUMBER` define) in `sources/Application/Model/Project.h`
1. Create a PR to merge into `master` branch and have it merged
1. Wait for the build to finish and then download the artifacts, note they will be zip files containing the actual firmware uf2 files
1. [Draft a new release on GitHub](https://github.com/xiphonics/picoTracker/releases/new)
1. When you do "Choose a tag", make sure you select to "create a tag on publish" with the name of the release, eg. `v2.0.0`
1. Title the release with the version number, eg. `v2.0.0`
1. Rename the artifacts to `picoTracker_ili9341.uf2` and `picoTracker_st7789.uf2` and upload them to the release
1. Click the "Generate release notes" button
1. Publish the release
1. Notify users in the Discord channel #announcements of the new release

