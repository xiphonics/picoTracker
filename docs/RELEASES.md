## How to make a new release 

We create a new release branch for each new stable release version, eg. `v2.1`, `v2.2`, etc. All releases are built by CI from this branch and tags for each patch version release (`2.1.1`, `2.1.2` etc) are applied to commits on this branch.

While all releases are built on CI using GitHub Actions on the release branch, eg. `2.2-release` branch, some manual steps are still required to create a new release:

1. Update the build number (`PROJECT_NUMBER` define) in `sources/Application/Model/Project.h`
1. If required update the version number in `firmware_info` in `sources/Adapters/adv/Core/Src/main.c`
1. Create a PR to merge into the current stable release branch and have it merged
1. Wait for the build to finish and then download the artifacts, note they will be zip files containing the actual firmware files
1. [Draft a new release on GitHub](https://github.com/xiphonics/picoTracker/releases/new)
1. When you do "Choose a tag", make sure you select to "create a tag on publish" with the name of the release, eg. `v2.0.0`
1. Title the release with the version number, eg. `v2.0.0`
1. For the Advance, rename to `pT-Advance-versionnumber.bin` and the ILI build to `picoTracker-legacy-versionnumber.uf2` and upload them to the release
1. Click the "Generate release notes" button
1. Edit the release notes if required for a description of the changes in this release
1. Publish the release
1. Notify users in the Discord channel #announcements of the new release

