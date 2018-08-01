# Contributing to GEO Project

When contributing to this repository, please consider to discuss the change you wish to make via issue,
or any other method with contributors of this repository before making a change.

Please note we have a [code of conduct](CODE_OF_CONDUCT.md). Try to follow it in all your interactions with the project and contributors.

## Did you find a bug?

* Please, try to ensure the similar bug was not already reported in issues.
* If you're unable to find an open issue addressing the problem, open a [new one](https://github.com/GEO-Project/network-client/issues/new). Be sure to include a title and clear description, as much relevant information as possible, and a code sample or an executable test case demonstrating the expected behavior that is not occurring.

## Did you find a documentation mistake?

* Please, open an issue in __relevant `specs-*` repository__, or fix it directly and submit us a pull request.

## Did you write a patch?

* Open a new GitHub pull request with the patch.
* Ensure the PR description clearly describes the problem and solution. Include the relevant issue number if applicable.
* Wait for approval.
* Enjoy karma change.

# Coding conventions

## Branching

We follow [git-flow branching model](http://nvie.com/posts/a-successful-git-branching-model/).

Please use follow prefixes:
* `feature/`
* `hotfix/`
* `release/`

As feature name use Issue number. Eg `feature/42` when possible.

We suggest you to use [git-flow extensions](https://github.com/nvie/gitflow).

For better understanding, take a look at [git-flow cheatsheet](https://danielkummer.github.io/git-flow-cheatsheet/).


## Commit message
> Describe your changes in imperative mood, e.g. "make xyzzy do frotz" instead of "[This patch] makes xyzzy do frotz" or "[I] changed xyzzy to do frotz", as if you are giving orders to the codebase to change its behavior.

As is described at [Documentation/SubmittingPatches in the Git repo](https://git.kernel.org/pub/scm/git/git.git/tree/Documentation/SubmittingPatches?id=HEAD#n133)

## Tests

For any new programmatic functionality, we like unit/integration/etc tests when possible, so if you can keep your code cleanly isolated, please provide also the tests.

## Pull Request Process

1. Don't forget to actualize a documentation and dependencies.
1. You may merge the Pull Request in once you have the sign-off of one other developer, or if you do not have permission to do that, you may request the second reviewer to merge it for you.
1. If this PR closes the issue, add the line `Fixes #$ISSUE_NUMBER`. Ex. For closing issue 42, include the line `Fixes #42`.
1. If it doesn't close the issue but addresses it partially, just include a reference to the issue number, like `#42`.

For more information about keywords take a look at [Closing issues using keywords](https://help.github.com/articles/closing-issues-using-keywords/)

## Versioning

We are following [SemVer](https://semver.org/) notation for marking our releases. Please, take a note of it when dealing with the releases.

# Public place to talk

* [Telegram EN](https://t.me/geopeopleeng)
* [Telegram UA/RU](http://t.me/geopeoplegroup)
