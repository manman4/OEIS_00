# OEIS_00

* [日本語](README_ja.md)
* [English](README.md)

## 概要

私は [manman4](https://github.com/manman4) と申します。2016年よりOEIS（整数列大辞典）の編集に携わってきました。

編集作業の中で蓄積したさまざまなコードを整理し、順次このリポジトリで公開していきます。
一部文字化けなどがある可能性がありますが、ご理解ください。

## 管理されている整数列番号と対応リポジトリ

| 数列番号範囲      | 対応リポジトリ                                  |
| --------------- | --------------------------------------------- |
| A000001–A099999 | [OEIS_00](https://github.com/manman4/OEIS_00) |
| A100000–A199999 | [OEIS_01](https://github.com/manman4/OEIS_01) |
| A200000–A299999 | [OEIS_02](https://github.com/manman4/OEIS_02) |
| A300000–A399999 | [OEIS_03](https://github.com/manman4/OEIS_03) |
| A400000–A499999 | [OEIS_04](https://github.com/manman4/OEIS_04) |

---

## b-file（バイナリファイル）について

* b-fileは非常にサイズが大きいため、アップロードを控え、別リポジトリで管理しています。
* それに伴い、テキストファイルもアップロードしていません。
* 各数列の数値は1000桁まで（符号込み）を目安としています。

以下はPARI/GPを使った例です：

```PARI:
\\ A336975
v(n)={x='x+O('x^(n+10)); 1/prod(k=1, n, 1-x^k*(k+x))};
M=1000;
v=v(M);
for(n=0, M, i=polcoef(v, n); if((i<0)+#digits(i)>1000, break); write("/Users/xxx/Desktop/b336975_gp_test.txt", n, " ", i))
```

念のため余裕を持って計算を行い、例として10100まで計算しつつ、表示は10000までに制限しています。

---

## .gpファイルについて

* このリポジトリにはPARI/GP用のスクリプトファイルとして保存しています。

---

## 参考リンク

* **公式OEISデータリポジトリ**
  [https://github.com/oeis/oeisdata/](https://github.com/oeis/oeisdata/)

* **私がテーマ別に作成したOEIS関連リポジトリ群**
  [https://github.com/manman4/study_OEIS](https://github.com/manman4/study_OEIS)

  ※以下は **Apple Silicon Mac + Homebrew** 環境での例です。

  A299706 のように OpenMP を使う C / C++ プログラムをコンパイルする際、
  Apple Clang（Mac）では長いオプションが必要です。
  [study_OEIS/src](https://github.com/manman4/study_OEIS/blob/main/src) にある
  以下のエイリアスを `~/.zshrc` に追記して `source ~/.zshrc` を実行しておくと、
  コマンドが短くなり便利です。

  ```sh
  # C用
  alias gcc-omp='clang -O3 -Xpreprocessor -fopenmp -lomp -L/opt/homebrew/opt/libomp/lib -I/opt/homebrew/opt/libomp/include'

  # C++用
  alias g++-omp='clang++ -O3 -Xpreprocessor -fopenmp -std=c++17 -I/opt/homebrew/opt/libomp/include -L/opt/homebrew/opt/libomp/lib -lomp'
  ```

  設定後は以下のように短いコマンドでコンパイルできます。

  ```sh
  # C の場合
  gcc-omp a299706.c -o a299706

  # C++ の場合
  g++-omp a299706.cpp -o a299706
  ```

---


