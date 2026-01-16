
# Contributing to picoTracker

TL:DR: Join us on [Discord](https://discord.gg/FNnXvmk4Pu), be [courteous](docs/CODE_OF_CONDUCT.md), follow the steps below to set up a development environment to build and contribute code.

## Welcome

There are many ways to contribute, including writing code, filing issues on GitHub, helping people on our Discord, helping to triage, reproduce, or fix bugs that people have filed, adding to our documentation, doing outreach about picoTracker and of course making and publishing music made with picoTracker!

We communicate primarily over GitHub and [Discord](https://discord.gg/FNnXvmk4Pu).


## Developing for picoTracker

If you would prefer to write code, you may wish to start with our [list of good first issues](https://github.com/democloid/picotracker/issues?q=is%3Aopen+is%3Aissue+label%3A%22good+first+issue%22). 

See the [Developer Guide](./DEV.md) for instructions on how to setup your development enviroment and get started building and debugging the picoTracker firmware.

Here are some guidelines for when you are ready to contribute code:

* Always discuss what you plan to work on in the related issue before submitting a PR!
* Issues where we would like contributions have the **help wanted** or **good first issue** labels attached to them
* If a issue doesn't already exist, please create one first
* Always test your changes on a actual picoTracker device and include a message about how it was tested. Or if you don't have access to picoTracker hardware, please make this very clear in the PR so that others can help test your changes.
* Run through the minimal manual "smoke test" plan (see below)
* Please use the clang linter and resist the urge to make *only* cosmetic/code style changes
* Once you are ready submit your PR Or even if you just want to get some initial feedback submit a "Draft PR"

Please note that sometimes a PR won't be accepted for a number of different reasons, please don't take it personally, it's just that not all contributions will meet the goals of the projects author and maintainers. The best thing to do is to start discussion *first* in a related issue to see if what you want to contribute will fit with the goals of the project before you start coding it.


## Quality Assurance

One of the most useful tasks is finding and filing bug reports. Testing beta releases, looking for regressions, creating test cases can really improve the quality of the product alot. 

If this interests you, you can jump in and submit bug reports without needing anyone's permission!

The #beta-testers channel on our Discord server is a good place to talk about what you're doing. We're especially eager for QA testing when we announce a beta release.


### Minimum smoke-test plan

1. Test creating a new project
1. Test loading an existing project
1. Test full project playback
1. Test instrument playback
1. Test MIDI in/out if working on it
1. Always test specifically the feature and/or part of the code base you're working on
    
