# OEIS_00

- [日本語](README_ja.md)
- [English](README.md)

## Overview

I ([manman4](https://github.com/manman4)) started editing OEIS in 2016.

As I have accumulated various pieces of code during the editing process, I plan to release them gradually.

Please note that there may be some garbled text.

|  Sequence Number  |  Relevant Repository  |
| ---- | ---- |
|  A000001-A099999  |  [OEIS_00](https://github.com/manman4/OEIS_00)  |
|  A100000-A199999  |  [OEIS_01](https://github.com/manman4/OEIS_01)  |
|  A200000-A299999  |  [OEIS_02](https://github.com/manman4/OEIS_02)  |
|  A300000-A399999  |  [OEIS_03](https://github.com/manman4/OEIS_03)  |

## b-file

To avoid uploading b-files (due to their large size, which will be managed in a separate repository), text files will also not be uploaded.

The files are limited to 1000 digits, including the sign.

```PARI:
\\ A336975
v(n)={x='x+O('x^(n+10)); 1/prod(k=1, n, 1-x^k*(k+x))};
M=1000;
v=v(M);
for(n=0, M, i=polcoef(v, n); if((i<0)+#digits(i)>1000, break); write("/Users/xxx/Desktop/b336975_gp_test.txt", n, " ", i))
```

As a precaution, the calculations are carried out for a higher range than needed.
For example, calculate up to 10100 but display only up to 10000.

## .gpファイル

In this repository, the scripts are saved as PARI/GP files.

## 参考

The official repository for OEIS data can be found here:

https://github.com/oeis/oeisdata/

Repositories organized by theme, created by me, can be found here:

https://github.com/manman4/study_OEIS
