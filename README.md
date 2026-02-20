# OEIS_00

* [日本語 (Japanese)](README_ja.md)
* [English](README.md)

---

## Overview

I’m [manman4](https://github.com/manman4), and I have been actively editing the OEIS (Online Encyclopedia of Integer Sequences) since 2016.

Over the years, I have accumulated various scripts and tools used in the editing and analysis process. This repository is part of a series where I will gradually release these resources to the community.

> **Note:** Some text files or documents may contain garbled characters due to encoding issues.

---

## Repository Structure and Sequence Coverage

| Sequence Range    | Corresponding Repository                      |
| ----------------- | --------------------------------------------- |
| A000001 – A099999 | [OEIS_00](https://github.com/manman4/OEIS_00) |
| A100000 – A199999 | [OEIS_01](https://github.com/manman4/OEIS_01) |
| A200000 – A299999 | [OEIS_02](https://github.com/manman4/OEIS_02) |
| A300000 – A399999 | [OEIS_03](https://github.com/manman4/OEIS_03) |
| A400000 – A499999 | [OEIS_04](https://github.com/manman4/OEIS_04) |

---

## b-file Management

Due to the large size of b-files, this repository **does not** include b-files or corresponding text files. These files will be managed separately in a dedicated repository.

The numerical data is limited to 1000 digits, including the sign, to maintain manageable file sizes.

An example PARI/GP snippet used for generating sequence data is shown below:

```PARI:
\\ A336975
v(n)={x='x+O('x^(n+10)); 1/prod(k=1, n, 1-x^k*(k+x))};
M=1000;
v=v(M);
for(n=0, M, i=polcoef(v, n); if((i<0)+#digits(i)>1000, break); write("/Users/xxx/Desktop/b336975_gp_test.txt", n, " ", i))
```

*Note:* Calculations are performed for a slightly larger range than required (e.g., calculating up to 10,100 terms but displaying only 10,000) to ensure accuracy.

---

## .gp files

In this repository, the scripts are saved as PARI/GP files.

---

## Reference Materials and Resources

* The **official OEIS data repository** is maintained here:
  [https://github.com/oeis/oeisdata/](https://github.com/oeis/oeisdata/)

* My personal thematic repositories related to OEIS studies can be found here:
  [https://github.com/manman4/study_OEIS](https://github.com/manman4/study_OEIS)

  > **Note:** The following is specific to **Apple Silicon Mac + Homebrew** environments.

  When compiling C / C++ programs that use OpenMP (such as A299706),
  Apple Clang requires a lengthy set of flags.
  The aliases available in [study_OEIS/src](https://github.com/manman4/study_OEIS/blob/main/src)
  shorten the commands significantly. Add them to `~/.zshrc` and run `source ~/.zshrc`:

  ```sh
  # For C
  alias gcc-omp='clang -O3 -Xpreprocessor -fopenmp -lomp -L/opt/homebrew/opt/libomp/lib -I/opt/homebrew/opt/libomp/include'

  # For C++
  alias g++-omp='clang++ -O3 -Xpreprocessor -fopenmp -std=c++17 -I/opt/homebrew/opt/libomp/include -L/opt/homebrew/opt/libomp/lib -lomp'
  ```

  Once set up, you can compile with short commands like:

  ```sh
  # C
  gcc-omp a299706.c -o a299706

  # C++
  g++-omp a299706.cpp -o a299706
  ```

---


