# Leanソースだけから確認できる証明済み事項

## 1. この文書の範囲

この文書は、次のLeanソースだけを読んで作成した監査結果である。

- `RamaLean.lean`
- `RamaLean/**/*.lean`

既存のMarkdown、PDF、Python、数値実験ファイルは根拠にしていない。Leanファイル内の説明コメントは参照したが、証明済みかどうかの最終判定は必ず定理の型（statement）を優先した。

`RamaLean.lean` は117個の `RamaLean` モジュールをimportしている。ソース上には `theorem` または `lemma` で始まる宣言が802個ある。以下は、そのうち数学的な結論を担う主要な定理と、Lean内で証明されていない接続部分の要約である。

監査時に `lake build RamaLean` を再実行し、`Build completed successfully (8591 jobs)` を確認した。出力にはunused variable・unused simp argument・deprecated tactic等のlinter warningがあるが、ビルドエラーはない。

## 2. 先に結論

Lean内で強く、無条件に証明されている中心的結果は次のとおりである。

1. 小さい分割数の値と、それから従う合同式。
2. 置換のサイクル重み平均に対するNewton漸化式とChebyshev閉形式。
3. その閉形式の実零点の区間排除、生成関数、Fibonacci–Lucas恒等式。
4. GCD行列の永久行列式に対する `3`、`4`、`8` および大きな2冪の整除性。
5. SmithのGCD行列式公式と、永久行列式の一般的な群作用・階乗整除補題。
6. 包含排除、被覆の辺数・2-path数、上位係数計算に必要な多項式恒等式。
7. 多数の線形代数、行列、組合せ、解析上の補題。

一方、次の大きな主張は、Lean内ではそのまま無条件には証明されていない。

- `ppent` が全ての `n` で真の分割数 `p(n)` に等しいこと。
- 置換のサイクル重み平均を、実際のランダム置換リフトの期待特性多項式と同一視するグラフ論的部分。
- Hall反例およびsubcubic反例の候補根が、実際に普遍被覆のスペクトル外にあること。
- BAND、GAPCOUNT、unicyclic、feedback vertex、ratio-system関連の最終的なグラフ論的主張の多く。
- Paper 4の完全な係数公式。

これらは「証明が間違っている」という意味ではない。Lean定理の仮定として明示されているか、Leanの定理型まで接続されていない、という意味である。

## 3. Paper 1：分割数

根拠: [`RamaLean/Paper1.lean`](RamaLean/Paper1.lean)

### 3.1 真の分割数について証明されていること

`Paper1.p n` は `Fintype.card (Nat.Partition n)`、すなわち整数分割の組合せ的定義である。

`Paper1.p_values` は次を計算証明する。

$$
p(0),p(1),p(2),p(3),p(4),p(5),p(6),p(7),p(9),p(10),p(11)
=1,1,2,3,5,7,11,15,30,42,56.
$$

これから、以下が証明されている。

- `Paper1.prop_i`: $p(5)\equiv2\pmod5$
- `Paper1.prop_ii`: $p(7)\equiv1\pmod7$
- `Paper1.prop_iii`: $p(11)\equiv1\pmod{11}$
- `Paper1.four_mem`: $4\in S_1$
- `Paper1.seven_mem`: $7\in S_1$
- `Paper1.eleven_mem`: $11\in S_1$
- `Paper1.five_not_mem`: $5\notin S_1$

ここで $S_1=\{n\ge2:n\mid p(n)-1\}$ である。`Paper1.companion_small` は $2\mid p(2)$、$3\mid p(3)$、$p(6)\equiv-1\pmod6$ も証明する。

`Paper1.proposition_of` などの `_of` 定理は、小さい分割数の値を仮定すれば、以後の合同式は通常のkernel証明だけで従うことを分離して示している。

### 3.2 `ppent`について証明されていること

`ppent` はEulerの五角数型の漸化式としてLean内で定義された別の関数である。

- `Paper1.ppent_eq_card_le_11` は `0 ≤ n ≤ 11` の各値について `ppent n = p n` を証明する。
- `Paper1.S₁_members` は、`4, 7, 11, 54, 55, 115, 146, 157, 234, 239, 951` が `ppent n ≡ 1 (mod n)` を満たすことを証明する。
- `Paper1.S₀_members` は `2, 3, 124, 158, 342` に対する `ppent n ≡ 0 (mod n)` を証明する。
- `Paper1.Sneg1_members` は `6, 156, 305, 484` に対する `ppent n ≡ -1 (mod n)` を証明する。

ただし、定理の型は列挙された各数の「membership」の連言だけである。`n ≤ 1000` で他に解がないという完全性は定理に含まれない。また、`ppent n = p n` の一般定理はなく、`n > 11` の結果を真の分割数 `p(n)` の結果へ移す橋はLean内では証明されていない。

## 4. Paper 2：置換平均とChebyshev多項式

主な根拠:

- [`RamaLean/Paper2General.lean`](RamaLean/Paper2General.lean)
- [`RamaLean/Paper2ExpFormula.lean`](RamaLean/Paper2ExpFormula.lean)
- [`RamaLean/Paper2Roots.lean`](RamaLean/Paper2Roots.lean)
- [`RamaLean/Paper2GenFun.lean`](RamaLean/Paper2GenFun.lean)
- [`RamaLean/Paper2FibLucas.lean`](RamaLean/Paper2FibLucas.lean)
- [`RamaLean/Paper2SpectralEdge.lean`](RamaLean/Paper2SpectralEdge.lean)
- [`RamaLean/Paper2Unicyclic.lean`](RamaLean/Paper2Unicyclic.lean)

### 4.1 無条件に証明された代数・組合せ部分

`cT`、`cU` はChebyshev型漸化式、`cG` は期待多項式の候補となる閉形式として定義されている。

- `Paper2.cG_newton` は全ての `r` についてNewton型漸化式
  $$
  rG_r=\sum_{k=1}^r f_kG_{r-k}
  $$
  を証明する。
- `Paper2.thm1_of_newton` は、標数0の整域上で初期値と同じNewton漸化式を満たす列が `cG` に一致することを証明する。
- `Paper2.cf_comp` はChebyshevの合成則から、サイクル因子を `2T_{nk}(t)-2` に書き換える。
- `Paper2.thm1_general`、`thm1_closed_form`、`thm1_factored` は、明示されたNewton漸化式を満たす任意の列 `Φ` に対し、
  $$
  \Phi_r=cG(T_n(t),r),\qquad
  \Phi_{k+1}=(2T_n(t)-2)U_k(T_n(t))
  $$
  という閉形式・因数分解を証明する。
- `Paper2.thm1_Sr` は仮定としてNewton漸化式を受け取らず、有限置換のサイクル型に基づく `permAvg` を直接定義し、その平均が `cG(T_n(t),r)` に等しいことを全ての `n,r` で証明する。
- `Paper2.quotient_parity` は `U_k(T_n(t))` の偶奇性
  $$
  U_k(T_n(-t))=(-1)^{nk}U_k(T_n(t))
  $$
  を証明する。

したがって、「置換の各サイクル長 `k` に重み `2T_{nk}(t)-2` を与えた平均」の閉形式はLean内で完結している。

### 4.2 零点・生成関数・特殊値

- `Paper2Roots.Phi_ne_zero_of_two_lt_abs`: 線形順序体で `n,r ≥ 1`、`|x|>2` なら
  $$
  cG(T_n(x/2),r)\ne0.
  $$
  よって、この代数的閉形式の実零点は `[-2,2]` の外にはない。
- `Paper2GenFun.genU_mul`:
  $$
  (1-2YX+X^2)\sum_{d\ge0}U_d(Y)X^d=1.
  $$
- `Paper2GenFun.genPhi_mul`:
  $$
  (1-2YX+X^2)\sum_{r\ge0}cG(Y,r)X^r=(1-X)^2.
  $$
- `Paper2FibLucas.cor2_phi3_general`: `r ≥ 1` で
  $$
  F_{2n}\,\Phi_3(n,r)=(L_{2n}-2)F_{2nr}.
  $$
- `Paper2SpectralEdge.spectral_edge_is_root`: `n>0, r≥2` で
  $$
  U_{r-1}\!\left(T_n\!\left(\cos\frac{\pi}{nr}\right)\right)=0.
  $$

最後の定理は「この値が零点である」ことだけを証明する。最大零点であること、およびgapの漸近式は定理の型に含まれない。また `Phi_ne_zero_of_two_lt_abs` は実数上の区間排除であり、複素零点を含めた「全零点が実数である」という定理ではない。

### 4.3 実際のグラフリフトへの接続

`Paper2.thm1_Sr` の左辺はLeanで定義された置換サイクル重み平均である。置換リフトされたサイクルグラフが長いサイクルへ分解し、その特性多項式がこの積になる、というグラフ論的同一視は定理の型に入っていない。

したがって、Leanだけから無条件に言えるのはサイクル重み平均の公式までであり、「実際のランダム置換リフトの期待特性多項式」との同一視には外部のグラフ論的入力が残る。

### 4.4 unicyclic結果

- `Paper2Unicyclic.subdivision_gap_theorem` はHeilmann–Lieb型の根の下界 `hHL` とsubdivision恒等式 `hid` を仮定して、非零根のgap下界を導く。
- `Paper2Unicyclic.unicyclic_band_theorem` は、実際の多項式を `cV` と同一視する仮定 `hmu` からband不等式を導く。

結論への代数的推論は証明済みだが、`hHL`、`hid`、`hmu` はLean内で導出されていない。

## 5. Paper 3：GCD行列の永久行列式

`gcdMat n` は、添字を1から数えたGCD行列である。

主な根拠:

- [`RamaLean/Paper3Congruence.lean`](RamaLean/Paper3Congruence.lean)
- [`RamaLean/Paper3FourDivides.lean`](RamaLean/Paper3FourDivides.lean)
- [`RamaLean/Paper3EightDivides.lean`](RamaLean/Paper3EightDivides.lean)
- [`RamaLean/Paper3LinearRate.lean`](RamaLean/Paper3LinearRate.lean)
- [`RamaLean/Paper3C1.lean`](RamaLean/Paper3C1.lean)
- [`RamaLean/Paper3Permanent.lean`](RamaLean/Paper3Permanent.lean)
- [`RamaLean/SmithDeterminant.lean`](RamaLean/SmithDeterminant.lean)

### 5.1 最終的な整除定理

- `Paper3.three_dvd_gcd_permanent`: `n ≥ 13` ならGCD行列の永久行列式は `ZMod 3` 上で0。これは整数永久行列式の `3` による合同を表す。
- `Paper3Four.four_dvd_permanent`: `n ≥ 4` なら
  $$
  4\mid\operatorname{per}\!\left(
    \left[\gcd(i,j)\right]_{1\le i,j\le n}
  \right).
  $$
- `Paper3Four.eight_dvd_permanent`: `n ≥ 17` なら同じ永久行列式は `8` で割り切れる。
- `Paper3Linear.two_pow_dvd_permanent`: 全ての `n` について
  $$
  2^{\lfloor(n+1)/2\rfloor-\lfloor\log_2n\rfloor-1}\mid a(n),
  $$
  ただし指数の減算は自然数の切り捨て減算である。
- `Paper3C1.two_pow_dvd_permanent_c1`: 全ての `n` について、さらに強く
  $$
  2^{n-2\lfloor\log_2n\rfloor-2}\mid a(n).
  $$

最後の定理は2進付値が率1の線形下界を持つことを与える。ただし、ソース中に `Tendsto` を使った `v₂(a(n))/n → 1` という別の極限定理はない。Leanで直接証明された最終statementは上の整除式である。

### 5.2 一般的な永久行列式補題

- `Paper3.permanent_eq_zero_of_col_period`: `ZMod p` 上で、位数 `p` の列置換が行列を保つなら永久行列式は0。
- `Paper3.permanent_eq_zero_of_two_cols_eq`: `ZMod 2` 上で2列が等しければ永久行列式は0。
- `factorial_dvd_permanent_of_ones_rows`: `c` 本の全1行を持つ整数行列の永久行列式は `c!` で割り切れる。
- `OddPerm.two_pow_dvd_permanent_odd`: 全成分が奇数の `n×n` 整数行列では
  $$
  2^{n-\lfloor\log_2 n\rfloor-1}\mid\operatorname{per}M.
  $$
- `TwoGroupFactorial`、`ZeroedCorner`、`OddEngine`、`GeneralEngine`、`OrbitSumDivisibility` は、上の整除性を支える群作用・同一行群・zeroed corner・orbit sumの一般補題を証明する。

### 5.3 Smithの行列式公式

- `Smith.det_gcdDivSum`: 任意の $n\in\mathbb{N}$、可換環 $R$、関数 $f:\mathbb{N}\to R$ に対して
  $$
  \det\!\left(
    \left[\sum_{d\mid\gcd(i,j)} f(d)\right]_{1\le i,j\le n}
  \right)
  =\prod_{k=1}^{n} f(k).
  $$
- `Smith.det_gcd_eq_prod_totient`: 任意の $n\in\mathbb{N}$ に対し、整数行列として
  $$
  \det\!\left(
    \left[\gcd(i,j)\right]_{1\le i,j\le n}
  \right)
  =\prod_{k=1}^{n}\varphi(k).
  $$

これらは明示的な外部数学仮定なしに証明されている。

### 5.4 付随する合同・パリティ機構

`Paper3Atom1`、`Paper3CMinus1`、`Paper3DigitSum`、`Paper3Involution`、`Paper3Negation`、`Paper3Translation`、`Paper3Chi4` は、補助量の2進付値、odd/even和、digit-sum、反転、周期性などを証明する。これらは各定理の補助量についての正確な結果であるが、定理の型にない「`a(n)` の正確な2進付値公式」まで自動的に拡張して読むべきではない。

## 6. Paper 4：係数、包含排除、被覆数

主な根拠:

- [`RamaLean/Paper4Coeff.lean`](RamaLean/Paper4Coeff.lean)
- [`RamaLean/ConflictIE.lean`](RamaLean/ConflictIE.lean)
- [`RamaLean/CoverCounts.lean`](RamaLean/CoverCounts.lean)
- [`RamaLean/Transversal.lean`](RamaLean/Transversal.lean)
- [`RamaLean/CoefficientRigidity.lean`](RamaLean/CoefficientRigidity.lean)

証明されている主な内容は次である。

- `Paper4Coeff.L_coeff_top`、`L_coeff_next`: falling-factorial型多項式の上位2係数。
- `Paper4Coeff.binom_two_div_factorial`: 二項係数と階乗の有理数恒等式。
- `Paper4Coeff.top_two_coeff`: Paper 4で必要となる二つの寄与を
  $$
  -\frac{E^{k-2}}{(k-2)!}\left(\frac E2+P\right)
  $$
  にまとめる代数恒等式。
- `ConflictIE.mCount_eq_ie`: conflict-freeな `k` 部分集合数に対する厳密な包含排除公式。
- `CoverCounts.edge_count`: 各fiberの大きさが `d` で次数が保存されるなら、被覆側の辺数は `E d`。
- `CoverCounts.twoPath_count`: 同じ仮定で2-path数も `d` 倍。
- `CoefficientRigidity.four_leading_rigid`: 同じサイズと同じ一次対称和を持つ積多項式の差の次数が下がることから、上位4係数の剛性に必要な多項式核を証明する。
- `Transversal` は微分作用素、trace、pair/triple sumに関する恒等式を証明する。

ただし `Paper4Coeff.top_two_coeff` は有理式の恒等式であり、それ自体は完全な `d`-matching多項式の係数公式ではない。matching polynomialとこれらの量を結ぶ全degree bookkeepingは最終定理として統合されていない。

## 7. Hall反例とsubcubic反例

### 7.1 Hall候補

根拠: [`RamaLean/HallCounterexample.lean`](RamaLean/HallCounterexample.lean)

無条件に証明されているのは次である。

- Lean内で定義された `muG` の明示的因数分解 `HallCounterexample.muG_eq`。
- `HallCounterexample.muG_root_sqrt5`: `√5` は `muG` の零点。
- `HallCounterexample.muG_root_simple`: その根に対応するcofactorは0でない。因数分解と合わせれば通常の代数では単純根と分かるが、定理の型そのものはroot multiplicityではなくcofactorの非消滅を述べる。
- ratio恒等式、非負行列に対するCollatz–Wielandt型評価、明示的residualの正値性。

しかし、次は最終定理に入っていない。

1. Lean内で定義した `muG` が、記述された有限グラフの組合せ的matching polynomialに等しいこと。
2. ratio quotientが実際の普遍被覆のdecay作用素を表すこと。
3. Angel–Friedman–Hoory型の入力から `√5` が普遍被覆スペクトル外にあることをLean内で導くこと。

`HallCounterexample.conj10_false_at_sqrt5` は

```lean
(hspec : Real.sqrt 5 ∉ Spec) →
  ¬ (∀ y, aeval y muG = 0 → y ∈ Spec)
```

という仮定付き定理である。したがって、LeanだけではHallのグラフが無条件の反例だとは結論できない。根の側は証明済みで、スペクトル除外の側が仮定である。

### 7.2 subcubic候補

根拠: [`RamaLean/SubcubicCounterexample.lean`](RamaLean/SubcubicCounterexample.lean)

- 再帰的に定義した次数31の多項式 `A3` に `X⁴-7X²+8` が割り切れる。
- `θ²=(7-√17)/2` を満たす正の実数 `θ` が存在する。
- その `θ` は `A3` の零点である。

一方、`θ` が対応する普遍被覆スペクトルの内部gapにあることはLean定理になっていない。従って、こちらも多項式の根の候補までは厳密だが、無条件のグラフ反例までは到達していない。

### 7.3 Xuのadditive-product主張

根拠: [`RamaLean/XuAdditiveProduct.lean`](RamaLean/XuAdditiveProduct.lean)

`xu_conj21_false`、`xu_conj24_false` は、edge-atom系の多項式がHallの `muG` であるという仮定 `hedge` と、`√5` のスペクトル除外 `hspec` から反例を導く。論理的な反駁の組立ては証明済みだが、二つの入力を含む条件付き定理である。

## 8. Conjecture 10、BAND、biregular関連

この部分には、有用な無条件の核と、外部入力を明示した条件付き最終定理が混在する。

### 8.1 無条件の核

- `BiregularBlocking.no_tree_of_two_le_degree`: 全頂点の次数が2以上の有限非空単純グラフは木でない。
- `BiregularBlocking.root_sep`、`RootSeparation.cauchy_lower`: Lipschitz/Cauchy型の零点分離評価。
- `RootSeparation.matching_root_sep`: 定理型に与えられたmatching-number係数方程式から、非零根の下界を導く。
- `OrderFourSolvable.order_four_solvable`: cokernelが定数ベクトルの直線で、obstructionの座標和が0なら線形方程式が解ける。
- `PathTreeInduction.no_vanishing`: path-tree再帰、子数、減少条件、閉性不等式を全て仮定すれば、全ratioが非零である。
- `AbelianCover.root_mem_of_average`: 連結なパラメータ空間上の連続な実関数族の平均が0なら、族のどこかで値が0になる。

これらの含意そのものには未証明の途中ステップはない。ただし、具体的なmatching polynomial、普遍被覆、torus族へ適用するための同一視が仮定に残る場合がある。

### 8.2 条件付きの最終定理

- `BiregularBlocking.biregular_reduction` は、biregular treeのスペクトル表示 `hspec` とHall–Puder–Sawin型区間評価 `hHPS` を仮定して、Conjecture 10を内側gapの非存在へ同値変形する。
- `BandTheorem.band_of` は固有値枝の連続性 `hcont` とinterlacing squeeze `hsqueeze` を仮定して、各根がbandに入ることを示す。
- `BandTheorem.gapcount_ab_of` も同じ仮定からgap count公式を導く。
- `AbelianCover.root_mem_of_average` の実際のグラフ適用には、matching polynomialをtorus上の行列式平均と同一視する式が必要である。
- `FeedbackGapCount`、`FeedbackVertex`、`FeedbackTwo` の最終結果は、inertia、path-tree、definiteness等を仮定として受け取る。
- `RatioCertificate` のスペクトル除外はAngel–Friedman–Hoory型入力を仮定として受け取る。
- `PathTreeInduction.no_vanishing` は強い抽象定理だが、具体的な全グラフ族についてその全仮定を満たすインスタンスを構成する定理ではない。

したがって、このLean群は「仮定から結論までの論理鎖」をかなり細かく検証しているが、BANDやbiregular問題全体の無条件解決を証明してはいない。

## 9. その他の証明済み基礎部品

117モジュール全ての802宣言をここで逐語的に列挙はしないが、次の分野の一般補題がLeanで証明されている。

- 行列・線形代数: `AdjugatePSD`、`GramDet`、`Plucker`、`TracePSD`、`MixedDiscriminant`、`Interlacing`、`Congruence`、`CokernelRank`、`SpanRank`。
- matchingと組合せ: `MatchingRecursion`、`BipartiteMatchingPoly`、`SDRCount`、`SDRMatching`、`ConflictIE`、`PathCount`、`CotreeCycle`。
- 永久行列式と群作用: `OrbitSumDivisibility`、`PermanentFactorial`、`TwoGroupFactorial`、`OddEngine`、`GeneralEngine`、`ZeroedCorner`。
- 多項式・解析評価: `ProductBound`、`ShellBound`、`RootSeparation`、`BandLipschitz`、`MomentTransfer`、`MomentLadder`。
- cavity/path-treeの代数的機構: `Cavity`、`CavityThreshold`、`RatioRoute`、`PathTreeRoute`、`PathTreeInduction`。

これらも、抽象定理としては正確に証明されている。具体的なグラフへの適用に追加仮定がある場合は、その定理の引数として現れている。

## 10. trust baseと検証方法

### 10.1 proof holeと独自公理

Leanソースの宣言位置を検索した限り、実際の `axiom`、`opaque`、`sorry`、`admit` 宣言はない。該当語が説明コメント中に現れる箇所はあるが、proof holeではない。

代表的な主要定理に `#print axioms` を実行すると、通常の定理は次のMathlib/Lean標準公理だけに依存する。

- `propext`
- `Classical.choice`
- `Quot.sound`

確認対象には `Paper1.proposition_of`、`Paper2.thm1_Sr`、`Paper2Roots.Phi_ne_zero_of_two_lt_abs`、Paper 3の主要整除定理、Smith公式、`Paper4Coeff.top_two_coeff`、Hallの多項式根、`PathTreeInduction.no_vanishing` を含めた。

### 10.2 `native_decide`

`Paper1.lean`、`Paper2.lean`、`Paper2FibLucas.lean` の一部の有限計算は `native_decide` を使う。例えば `Paper1.p_values` の `#print axioms` には通常の三公理に加え、各native計算に対応する `native_decide` axiomが表示される。

これは、これらの有限計算がLean kernelによる項の正規化だけではなく、Leanのnative compiler/runtimeの正しさもtrust baseに含めることを意味する。対して `Paper1.proposition_of` のような計算結果を仮定として受け取る論理部分は標準三公理だけである。

## 11. 最終判定表

| 主張 | Leanだけでの状態 |
|---|---|
| 小さい真の分割数と合同式 | 証明済み（有限計算は `native_decide`） |
| `ppent` の列挙値 | `ppent`について証明済み |
| 全ての `n` で `ppent=p` | 未証明 |
| 置換サイクル重み平均のChebyshev閉形式 | 証明済み |
| 実際のランダム置換リフトとの同一視 | Lean定理として未接続 |
| 閉形式の実零点が `[-2,2]` 外にない | 証明済み |
| `2cos(π/(nr))` が零点 | 証明済み |
| それが最大零点、gapの漸近式 | 未証明 |
| GCD永久行列式の `3,4,8` 整除性 | 証明済み |
| `2^(n-2 log₂ n-2)` の整除下界 | 証明済み |
| `v₂(a(n))/n → 1` という明示的な極限定理 | 定理としては未実装 |
| SmithのGCD行列式公式 | 証明済み |
| Paper 4の係数計算に必要な代数恒等式 | 証明済み |
| Paper 4の完全な係数定理 | 未統合 |
| Hall候補多項式で `√5` が零点、対応cofactorが非零 | 証明済み |
| `√5` が実際の普遍被覆スペクトル外 | 仮定 |
| Hallグラフが無条件の反例 | Leanだけからは未証明 |
| subcubic候補多項式の明示的根 | 証明済み |
| subcubic候補のスペクトル除外 | 未証明 |
| BAND/GAPCOUNTの抽象的含意 | 仮定付きで証明済み |
| BAND/GAPCOUNTの一般グラフ定理 | Leanだけからは未証明 |

## 12. 総括

このLeanコードは、特にPaper 3のGCD永久行列式の整除性、Smith公式、Paper 2の置換サイクル重み平均、そして多数の抽象的な行列・多項式・組合せ補題について、強い機械検証済み結果を含む。

最も注意すべき点は、Leanファイルの見出しや説明コメントが、しばしば研究上の最終目標を表している一方、実際の定理型では古典的グラフ定理、スペクトル同一視、計算証明書などが仮定として残ることである。したがって正確な読み方は、次の三段階に分けることである。

1. 代数的・組合せ的kernel：多くが無条件に証明済み。
2. 外部の古典定理や具体的グラフとの同一視から結論への推論：仮定付きで証明済み。
3. その外部入力自体：一部はLean内で未証明。

この区別を守れば、何がLeanによって保証され、何が今後のformalization対象かは明確である。
