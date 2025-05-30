# OEIS_00

- [日本語](README_ja.md)
- [English](README.md)

## 概要

私（[manman4](https://github.com/manman4)）は2016年からOEISの編集を始めました。

編集の際に使用したコードが溜まってきたので、随時公開していきます。

文字化けしているところがあるかもしれませんがご了承ください。

|  数列番号  |  該当レポジトリ  |
| ---- | ---- |
|  A000001-A099999  |  [OEIS_00](https://github.com/manman4/OEIS_00)  |
|  A100000-A199999  |  [OEIS_01](https://github.com/manman4/OEIS_01)  |
|  A200000-A299999  |  [OEIS_02](https://github.com/manman4/OEIS_02)  |
|  A300000-A399999  |  [OEIS_03](https://github.com/manman4/OEIS_03)  |

## b-file

b-fileをアップロードさせない（サイズが大きいので別リポジトリで管理）ため、テキストファイルもアップロードしない

1000digitsまでだが、符号も含めることにする

```PARI:
\\ A336975
v(n)={x='x+O('x^(n+10)); 1/prod(k=1, n, 1-x^k*(k+x))};
M=1000;
v=v(M);
for(n=0, M, i=polcoef(v, n); if((i<0)+#digits(i)>1000, break); write("/Users/xxx/Desktop/b336975_gp_test.txt", n, " ", i))
```

念のため、計算は多めにしておく　例）10100まで計算し 10000まで表示

## .gpファイル

このリポジトリでは、PARI/GPスクリプトとして保存

## 参考資料

本家OEISのデータに関するリポジトリはこちら

https://github.com/oeis/oeisdata/

テーマ別に自身が作ったリポジトリはこちら

https://github.com/manman4/study_OEIS
