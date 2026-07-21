# Repository instructions

## Scope

- This is a public repository for OEIS-related programs. Follow the sequence
  range documented in the repository README.
- Store programs using the repository's existing sequence-directory and
  filename conventions.
- Preserve unrelated user files and changes. Do not reorganize legacy files
  merely to make them match newer conventions.

## Licensing and provenance

- Before adding or modifying source code, identify whether it was written
  independently, copied, translated, or adapted from another work.
- Do not place GPL-, LGPL-, or otherwise copyleft-derived source in this
  repository. Keep such material outside this public repository in a
  non-public location, together with its required notices and license texts.
- Mathematical definitions, algorithms, and numerical output may be used to
  create an independent implementation, but do not copy the expressive source
  code of a copyleft implementation while doing so.
- When the repository has a root license file, new independently written
  source files are covered by that repository-wide license unless stated
  otherwise; per-file SPDX identifiers, copyright notices, and license text
  are not required. If there is no root license, do not assume one: confirm
  the intended repository-wide or file-specific license before publication.
  Add a file-specific notice when its provenance or licensing differs, and do
  not guess or silently relicense code.
- Preserve all existing copyright, attribution, provenance, and license
  notices. If provenance or redistribution rights are uncertain, do not
  publish the code; record the uncertainty and ask the user.
- Comparing independently produced numerical output with another program does
  not make the implementations share a license, provided no source code is
  copied or adapted.

## Generated files

- Do not stage or commit executables, object files, libraries, sanitizer
  binaries, debug bundles, temporary files, or other generated products.
- Keep locally built executables unless the user explicitly requests their
  deletion. Add an appropriate `.gitignore` rule when necessary.
- Inspect `git status` before every commit and verify the exact staged file
  list.

## C and C++ validation

For every new or modified C or C++ source file, perform validation appropriate
to the program before reporting completion:

- compile with strict warnings, including `-Wall -Wextra -Wpedantic`, and
  treat warnings as errors;
- run Clang static analysis when available;
- run AddressSanitizer, UndefinedBehaviorSanitizer, and integer sanitizer on a
  practical representative input;
- define the supported input range, reject values outside it before allocation
  or index/size arithmetic, and test zero, the minimum, practical upper
  boundary values, one-past-the-boundary values, negatives, and malformed
  input as applicable;
- review every array index, pointer offset, shift, narrowing conversion, signed
  and unsigned operation, and allocation-size multiplication at the supported
  boundaries;
- prove mathematically that counters, intermediate expressions, products,
  accumulators, and final output types cannot overflow throughout the declared
  input range; a final-value range check alone is not sufficient;
- compare output with checked OEIS terms or an independent implementation;
- check supported command modes for agreement; and
- verify rejection of malformed and out-of-range arguments when the program
  accepts command-line input.

Routine validation has a three-minute wall-clock budget. This is a workflow
limit, not a promise that every supported input finishes within three minutes.
Do not start a longer validation without the user's explicit approval. If the
largest supported input is impractical, verify its integer and array bounds
mathematically, test the largest practical representative input, and report
both the tested value and what was not executed.

Sanitizers, static analysis, and passing test cases are supporting evidence,
not proofs that undefined behavior or overflow is impossible. Do not describe
a program as safe without both dynamic checks and an explicit boundary and
range argument for the paths not executed.

## Git workflow

- Do not commit, push, or open a pull request unless the user requests it.
- Never include ignored generated files in a commit.
- Before committing, run whitespace checks and inspect the staged diff.
