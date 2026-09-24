# Contributing to ASWUnitTests

Thanks for considering a contribution. This project is a small, dependency-free C++
unit-testing framework, and contributions are expected to keep it that way.

## Before you start

- For anything beyond a small fix, open an issue first to discuss the change. This
  avoids wasted work on something that doesn't fit the project's scope (see
  `AGENTS.md`).
- Coding standards, repository layout, and language/toolchain compatibility rules
  live in [AGENTS.md](AGENTS.md). Read it before making changes; it is the single
  source of truth for how code in this repository should look, and these
  guidelines don't repeat it.
- Base your branch on `develop`, not `main`. `main` tracks released versions;
  `develop` is where in-progress work is integrated before a release.

## Making a change

1. Identify the code path that owns the behavior you're changing.
2. Make the smallest change that addresses the issue. Don't bundle unrelated
   refactoring, formatting, or reordering into the same change.
3. Add or update focused tests for the behavior change, including error and
   platform-specific cases where applicable.
4. Build the affected configuration and run the unit-test executable. See
   [README.md](README.md#how-to-use) for CMake build commands, and
   `rad370/Build_Win64x_Debug.bat` / `Build_Win64x_Release.bat` for the RAD Studio
   build.
5. Check compiler warnings and errors; a change should not introduce new ones.
6. If you touched platform-specific code (`#if defined(_WIN32)` or similar),
   validate both the Windows and the portable path if you can.

## Formatting

This repository uses `uncrustify` for consistent formatting, enforced via a Git
hook. Enable it locally with:

```
git config --local core.hooksPath .githooks/
```

See [README.md](README.md#coding-standards) for how to install `uncrustify`
itself.

## Commit history

Each commit in a branch should focus on one specific task or fix, and the commits
should be in a logical order that tells the story of how the change was built.
Before opening a PR, review your own branch's commit history and confirm each
commit actually does what its message says.

[TortoiseGit](https://tortoisegit.org/) (free) is a good tool for this: it makes
editing a commit message, reordering commits, and squashing an "oops, fix typo"
commit into the commit it belongs with straightforward. Reviewing Anthony West's
own commit history in this repository is a good reference for how commits are
structured here.

Pull requests in this repository are merged with their commit history intact, not
squashed, so a clean history matters: it's what future readers use to see where a
bug was introduced and the reasoning behind a change.

Once a PR is open, one or two additional fixup commits are fine without revisiting
earlier history. Beyond that, edit or squash the fixups into the commits they
belong with and push with `git push --force-with-lease`.

## Submitting a pull request

- Target the `develop` branch.
- Keep the diff focused on the change described in your issue or PR description.
  Review your own diff for accidental file changes, ordering regressions, missing
  includes, and platform assumptions before requesting review.
- Describe what changed and why, not just what the code now does.
- Don't include generated build output (the `build/` and `cmake/build/` directories
  are already git-ignored).

## Reporting bugs

Open a GitHub issue with:

- The platform, compiler, and toolchain (e.g. Windows 11, MSVC 19.40, or Ubuntu
  22.04, GCC 13).
- The command line used to run ASWUnitTests, if relevant.
- The expected behavior versus the actual behavior.
- A minimal reproduction if possible.
