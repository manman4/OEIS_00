import A022825Lean.BlockDecomposition

namespace A022825Lean

/-!
# Reverse block decomposition

This file proves the converse direction: every permutation whose adjacent
remainders are strictly increasing is assembled from the canonical rotated
blocks used in `BlockDecomposition.lean`.
-/

/-- The invariant at the beginning of a block.  The list uses exactly the
still-unused values `s, ..., n`; its remainders are increasing; and its local
remainder at index `i` lies between the two consecutive possible global
values `s+i-1` and `s+i`. -/
def IsValidSuffix (n s : ℕ) (xs : List ℕ) : Prop :=
  xs.Perm (List.range' s (n + 1 - s)) ∧
  (remainders xs).Pairwise (· < ·) ∧
  ∀ (i : ℕ) (hi : i < (remainders xs).length),
    s + i - 1 ≤ (remainders xs)[i] ∧
      (remainders xs)[i] ≤ s + i

theorem IsValidSuffix.length {n s : ℕ} {xs : List ℕ}
    (h : IsValidSuffix n s xs) : xs.length = n + 1 - s := by
  simpa using h.1.length_eq

theorem IsValidSuffix.nodup {n s : ℕ} {xs : List ℕ}
    (h : IsValidSuffix n s xs) : xs.Nodup :=
  h.1.nodup_iff.mpr List.nodup_range'

theorem IsValidSuffix.mem_iff {n s v : ℕ} {xs : List ℕ}
    (h : IsValidSuffix n s xs) : v ∈ xs ↔ s ≤ v ∧ v ≤ n := by
  rw [h.1.mem_iff, List.mem_range']
  constructor
  · rintro ⟨i, hi, rfl⟩
    omega
  · rintro ⟨hsv, hvn⟩
    exact ⟨v - s, by omega, by omega⟩

theorem remainders_drop (xs : List ℕ) (k : ℕ) :
    remainders (xs.drop k) = (remainders xs).drop k := by
  induction k generalizing xs with
  | zero => simp
  | succ k ih =>
      cases xs with
      | nil => simp [remainders]
      | cons x xs =>
          cases xs with
          | nil => simp [remainders]
          | cons y ys =>
              simpa [remainders] using ih (y :: ys)

theorem pairwise_drop {α : Type*} {r : α → α → Prop}
    {xs : List α} (h : xs.Pairwise r) (k : ℕ) : (xs.drop k).Pairwise r := by
  induction k generalizing xs with
  | zero => simpa
  | succ k ih =>
      cases xs with
      | nil => simp
      | cons x xs => exact ih (List.Pairwise.tail h)

/-- The one-based global position bound, in suffix form. -/
theorem IsValidSuffix.idxOf_add_le_succ {n s v : ℕ} {xs : List ℕ}
    (h : IsValidSuffix n s xs) (hv : v ∈ xs) (hs : 1 ≤ s) :
    xs.idxOf v + s ≤ v + 1 := by
  have hj : xs.idxOf v < xs.length := List.idxOf_lt_length_iff.mpr hv
  have hvpos : xs[xs.idxOf v] = v := List.getElem_idxOf hj
  rcases hidx : xs.idxOf v with _ | k
  · have hsv := (h.mem_iff.mp hv).1
    omega
  · have hkrem : k < (remainders xs).length := by
      rw [length_remainders]
      omega
    have hlower := (h.2.2 k hkrem).1
    have hmod := getElem_remainders xs k hkrem
    have hvpos' : xs[k + 1] = v := by simpa [hidx] using hvpos
    rw [hvpos'] at hmod
    have hvpositive : 0 < v := lt_of_lt_of_le (by omega) (h.mem_iff.mp hv).1
    have hlt : (remainders xs)[k] < v := by
      rw [hmod]
      exact Nat.mod_lt _ hvpositive
    omega

theorem RemainderPermutation.isValidSuffix {n : ℕ}
    (p : RemainderPermutation n) : IsValidSuffix n 1 p.values := by
  refine ⟨?_, p.remainders_pairwise, ?_⟩
  · exact p.values_perm
  · intro i hi
    exact ⟨by simpa using p.index_le_remainder hi,
      by have := p.remainder_le_index_add_one hi; omega⟩

/-- Once a nonidentity block starts with `e > s`, the position bound forces
the following entries to be `s, s+1, ..., e-1`. -/
theorem IsValidSuffix.forced_rotated_getElem {n s e : ℕ} {xs : List ℕ}
    (h : IsValidSuffix n s xs) (hs : 1 ≤ s) (hsn : s ≤ n)
    (hhead : xs[0]'(by rw [h.length]; omega) = e) (hse : s < e) :
    ∀ (j : ℕ) (_hjpos : 1 ≤ j) (hj : j ≤ e - s),
      xs[j]'(by
        have hemem : e ∈ xs := hhead ▸ List.getElem_mem (by rw [h.length]; omega)
        have hen := (h.mem_iff.mp hemem).2
        rw [h.length]
        omega) = s + j - 1 := by
  intro j
  induction j using Nat.strong_induction_on with
  | h j ih =>
      intro hjpos hj
      have hemem : e ∈ xs := hhead ▸ List.getElem_mem (by rw [h.length]; omega)
      have hen := (h.mem_iff.mp hemem).2
      let v := s + j - 1
      have hsv : s ≤ v := by dsimp [v]; omega
      have hve : v < e := by dsimp [v]; omega
      have hvn : v ≤ n := hve.le.trans hen
      have hvmem : v ∈ xs := h.mem_iff.mpr ⟨hsv, hvn⟩
      have hidxlt : xs.idxOf v < xs.length := List.idxOf_lt_length_iff.mpr hvmem
      have hvat : xs[xs.idxOf v] = v := List.getElem_idxOf hidxlt
      have hidxle : xs.idxOf v ≤ j := by
        have := h.idxOf_add_le_succ hvmem hs
        have hvone : v + 1 = s + j := by dsimp [v]; omega
        rw [hvone] at this
        omega
      have hidxge : j ≤ xs.idxOf v := by
        by_contra hnot
        have hidxj : xs.idxOf v < j := by omega
        rcases hidx : xs.idxOf v with _ | k
        · have heq : e = v := by
            calc
              e = xs[0] := hhead.symm
              _ = v := by simpa [hidx] using hvat
          omega
        · have hkpos : 1 ≤ k + 1 := by omega
          have hkj : k + 1 < j := by omega
          have hkbound : k + 1 ≤ e - s := hkj.le.trans hj
          have hprev := ih (k + 1) hkj hkpos hkbound
          have heq : s + (k + 1) - 1 = v := by
            calc
              s + (k + 1) - 1 = xs[k + 1] := hprev.symm
              _ = v := by simpa [hidx] using hvat
          dsimp [v] at heq
          omega
      have hidxeq : xs.idxOf v = j := Nat.le_antisymm hidxle hidxge
      simpa [v, hidxeq] using hvat

theorem IsValidSuffix.forced_rotated_take {n s e : ℕ} {xs : List ℕ}
    (h : IsValidSuffix n s xs) (hs : 1 ≤ s) (hsn : s ≤ n)
    (hhead : xs[0]'(by rw [h.length]; omega) = e) (hse : s < e) :
    xs.take (e + 1 - s) = rotatedBlock s (e + 1) := by
  have hemem : e ∈ xs := hhead ▸ List.getElem_mem (by rw [h.length]; omega)
  have hen := (h.mem_iff.mp hemem).2
  apply List.ext_getElem
  · simp [rotatedBlock, h.length]
    omega
  · intro i hitake hiblock
    have hilength : i < xs.length := by
      rw [List.length_take] at hitake
      omega
    rcases i with _ | j
    · simpa [rotatedBlock] using hhead
    · have hjpos : 1 ≤ j + 1 := by omega
      have hjbound : j + 1 ≤ e - s := by
        have heq : e + 1 - s - 1 = e - s := by omega
        simp only [rotatedBlock, List.length_cons, List.length_range'] at hiblock
        rw [heq] at hiblock
        omega
      have hforced := h.forced_rotated_getElem hs hsn hhead hse
        (j + 1) hjpos hjbound
      rw [List.getElem_take]
      simpa [rotatedBlock] using hforced

theorem IsValidSuffix.start_dvd_endpoint {n s e : ℕ} {xs : List ℕ}
    (h : IsValidSuffix n s xs) (hs : 1 ≤ s) (hsn : s ≤ n)
    (hhead : xs[0]'(by rw [h.length]; omega) = e) (hse : s < e) :
    s ∣ e + 1 := by
  have hemem : e ∈ xs := hhead ▸ List.getElem_mem (by rw [h.length]; omega)
  have hen := (h.mem_iff.mp hemem).2
  have hlen2 : 2 ≤ xs.length := by rw [h.length]; omega
  have hsecond := h.forced_rotated_getElem hs hsn hhead hse 1 (by omega) (by omega)
  have hsimp : s + 1 - 1 = s := by omega
  rw [hsimp] at hsecond
  have hi0 : 0 < (remainders xs).length := by rw [length_remainders]; omega
  have hb := h.2.2 0 hi0
  have hmod := getElem_remainders xs 0 hi0
  have hemod : e % s = s - 1 := by
    rw [hhead, hsecond] at hmod
    have hmodlt : e % s < s := Nat.mod_lt _ (by omega)
    omega
  by_cases hsone : s = 1
  · subst s
    exact one_dvd _
  · have hs2 : 2 ≤ s := by omega
    apply Nat.dvd_iff_mod_eq_zero.mpr
    have h1mod : 1 % s = 1 := Nat.mod_eq_of_lt (by omega)
    have hsum : s - 1 + 1 = s := Nat.sub_add_cancel (by omega)
    rw [Nat.add_mod, hemod, h1mod, hsum, Nat.mod_self]

theorem perm_append_left_cancel {α : Type*} (pre xs ys : List α)
    (h : (pre ++ xs).Perm (pre ++ ys)) : xs.Perm ys := by
  induction pre with
  | nil => simpa using h
  | cons a pre ih =>
      apply ih
      exact List.Perm.cons_inv h

/-- Removing a forced nonidentity block preserves the valid-suffix
invariant, with the next block beginning at `e+1`. -/
theorem IsValidSuffix.drop_after_nontrivial {n s e : ℕ} {xs : List ℕ}
    (h : IsValidSuffix n s xs) (hs : 1 ≤ s) (hsn : s ≤ n)
    (hhead : xs[0]'(by rw [h.length]; omega) = e) (hse : s < e) :
    IsValidSuffix n (e + 1) (xs.drop (e + 1 - s)) := by
  let d := e + 1
  let len := d - s
  have hemem : e ∈ xs := hhead ▸ List.getElem_mem (by rw [h.length]; omega)
  have hen := (h.mem_iff.mp hemem).2
  have hd : d ≤ n + 1 := by dsimp [d]; omega
  have hslen : s + len = d := by dsimp [len, d]; omega
  have htake : xs.take len = rotatedBlock s d := by
    dsimp [len, d]
    exact h.forced_rotated_take hs hsn hhead hse
  have hsplit : rotatedBlock s d ++ xs.drop len = xs := by
    rw [← htake]
    exact List.take_append_drop len xs
  have hgap : s + 2 ≤ d := by dsimp [d]; omega
  have hrot : (rotatedBlock s d).Perm (List.range' s len) := by
    convert rotatedBlock_perm hgap using 1
  have hrange :
      List.range' s (n + 1 - s) =
        List.range' s len ++ List.range' d (n + 1 - d) := by
    symm
    calc
      List.range' s len ++ List.range' d (n + 1 - d) =
          List.range' s len ++ List.range' (s + len) (n + 1 - d) := by
            rw [hslen]
      _ = List.range' s (len + (n + 1 - d)) := by
        simpa only [one_mul] using
          (List.range'_append (s := s) (m := len)
            (n := n + 1 - d) (step := 1))
      _ = List.range' s (n + 1 - s) := by
        congr 1
        dsimp [len, d]
        omega
  change IsValidSuffix n d (xs.drop len)
  refine ⟨?_, ?_, ?_⟩
  · apply perm_append_left_cancel (rotatedBlock s d)
      (xs.drop len) (List.range' d (n + 1 - d))
    exact (List.Perm.of_eq hsplit).trans
      (h.1.trans ((List.Perm.of_eq hrange).trans
        (hrot.symm.append_right (List.range' d (n + 1 - d)))))
  · rw [remainders_drop]
    exact pairwise_drop h.2.1 len
  · intro i hi
    have hiorig : len + i < (remainders xs).length := by
      rw [remainders_drop, List.length_drop] at hi
      omega
    have helem :
        (remainders (xs.drop len))[i] = (remainders xs)[len + i] := by
      simpa only [remainders_drop] using
        (List.getElem_drop (xs := remainders xs) (i := len) (j := i))
    rw [helem]
    have hb := h.2.2 (len + i) hiorig
    dsimp [len, d] at hb ⊢
    omega

/-- If the next entry is the fixed point `s`, removing it advances the suffix
invariant to `s+1`. -/
theorem IsValidSuffix.drop_after_identity {n s : ℕ} {xs : List ℕ}
    (h : IsValidSuffix n s xs) (hsn : s ≤ n)
    (hhead : xs[0]'(by rw [h.length]; omega) = s) :
    IsValidSuffix n (s + 1) (xs.drop 1) := by
  have hsplit : s :: xs.drop 1 = xs := by
    have hdrop := List.drop_eq_getElem_cons
      (l := xs) (i := 0) (by rw [h.length]; omega)
    simpa [hhead] using hdrop.symm
  have hrange :
      List.range' s (n + 1 - s) =
        s :: List.range' (s + 1) (n + 1 - (s + 1)) := by
    rw [show n + 1 - s = (n - s) + 1 by omega, List.range'_succ]
    congr 2
    omega
  refine ⟨?_, ?_, ?_⟩
  · exact List.Perm.cons_inv ((List.Perm.of_eq hsplit).trans
      (h.1.trans (List.Perm.of_eq hrange)))
  · rw [remainders_drop]
    exact pairwise_drop h.2.1 1
  · intro i hi
    have hiorig : 1 + i < (remainders xs).length := by
      rw [remainders_drop, List.length_drop] at hi
      omega
    have helem :
        (remainders (xs.drop 1))[i] = (remainders xs)[1 + i] := by
      simpa only [remainders_drop] using
        (List.getElem_drop (xs := remainders xs) (i := 1) (j := i))
    rw [helem]
    have hb := h.2.2 (1 + i) hiorig
    omega

set_option maxHeartbeats 300000 in
/-- A fixed point cannot be followed by the start of a nonidentity block:
the two boundary remainders would be equal. -/
theorem IsValidSuffix.second_eq_succ_of_head_eq {n s : ℕ} {xs : List ℕ}
    (h : IsValidSuffix n s xs) (hs : 1 ≤ s) (hsn : s < n)
    (hhead : xs[0]'(by rw [h.length]; omega) = s) :
    xs[1]'(by rw [h.length]; omega) = s + 1 := by
  have htail := h.drop_after_identity hsn.le hhead
  have hxlen : xs.length = n + 1 - s := h.length
  let e := xs[1]'(by rw [h.length]; omega)
  have htailhead :
      (xs.drop 1)[0]'(by rw [htail.length]; omega) = e := by
    exact List.getElem_drop
  have hemem : e ∈ xs := List.getElem_mem (by rw [h.length]; omega)
  have hse : s + 1 ≤ e := by
    have := (h.mem_iff.mp hemem).1
    have hne : e ≠ s := by
      intro hes
      have heq :
          (⟨1, by rw [h.length]; omega⟩ : Fin xs.length) =
            ⟨0, by rw [h.length]; omega⟩ :=
        h.nodup.get_inj_iff.mp (by simpa [e, hes] using hhead.symm)
      have heqval := congrArg Fin.val heq
      change 1 = 0 at heqval
      omega
    omega
  by_contra hne
  have hgt : s + 1 < e := by omega
  have hen := (h.mem_iff.mp hemem).2
  have hzero : 0 < xs.length := by omega
  have hone : 1 < xs.length := by omega
  have htwo : 2 < xs.length := by omega
  have hthirdTail := htail.forced_rotated_getElem (by omega) (by omega)
    htailhead hgt 1 (by omega) (by omega)
  have hthird : xs[2]'htwo = s + 1 := by
    have hthirdRaw : xs[1 + 1]'(by omega) = s + 1 + 1 - 1 := by
      simpa only [List.getElem_drop] using hthirdTail
    have hsimp : s + 1 + 1 - 1 = s + 1 := by omega
    simpa [hsimp] using hthirdRaw
  have hi0 : 0 < (remainders xs).length := by rw [length_remainders, h.length]; omega
  have hi1 : 1 < (remainders xs).length := by rw [length_remainders, h.length]; omega
  have hmod0 : (remainders xs)[0] = xs[0]'hzero % xs[1]'hone := by
    simpa using getElem_remainders xs 0 hi0
  have hmod1 : (remainders xs)[1] = xs[1]'hone % xs[2]'htwo := by
    simpa using getElem_remainders xs 1 hi1
  have hlt := (List.pairwise_iff_getElem.mp h.2.1) 0 1 hi0 hi1 (by omega)
  rw [hmod0, hmod1, hhead, show xs[1] = e by rfl, hthird] at hlt
  have hfirstMod : s % e = s := Nat.mod_eq_of_lt (by omega)
  have hsecondMod : e % (s + 1) < s + 1 := Nat.mod_lt _ (by omega)
  omega

/-- If a valid suffix begins with its least unused value, the whole suffix is
the identity tail. -/
theorem IsValidSuffix.eq_range_of_head_eq {n s : ℕ} {xs : List ℕ}
    (h : IsValidSuffix n s xs) (hs : 1 ≤ s) (hsn : s ≤ n)
    (hhead : xs[0]'(by rw [h.length]; omega) = s) :
    xs = List.range' s (n + 1 - s) := by
  by_cases hlast : s = n
  · subst n
    apply List.ext_getElem
    · simpa using h.length
    · intro i hix hirange
      have hi : i = 0 := by
        rw [h.length] at hix
        omega
      subst i
      simpa using hhead
  · have hsnlt : s < n := lt_of_le_of_ne hsn hlast
    have hsecond := h.second_eq_succ_of_head_eq hs hsnlt hhead
    have htail := h.drop_after_identity hsn hhead
    have htailhead :
        (xs.drop 1)[0]'(by rw [htail.length]; omega) = s + 1 := by
      simpa only [List.getElem_drop] using hsecond
    have hrec := htail.eq_range_of_head_eq (by omega) (by omega) htailhead
    have hsplit : s :: xs.drop 1 = xs := by
      have hdrop := List.drop_eq_getElem_cons
        (l := xs) (i := 0) (by rw [h.length]; omega)
      simpa [hhead] using hdrop.symm
    have hrange :
        List.range' s (n + 1 - s) =
          s :: List.range' (s + 1) (n + 1 - (s + 1)) := by
      rw [show n + 1 - s = (n - s) + 1 by omega, List.range'_succ]
      congr 2
      omega
    calc
      xs = s :: xs.drop 1 := hsplit.symm
      _ = s :: List.range' (s + 1) (n + 1 - (s + 1)) := by rw [hrec]
      _ = List.range' s (n + 1 - s) := hrange.symm
termination_by n + 1 - s
decreasing_by omega

/-- Every valid suffix has a canonical-block presentation whose endpoints
form a strict divisor chain (with the current start adjoined). -/
theorem IsValidSuffix.exists_blocks {n s : ℕ} {xs : List ℕ}
    (h : IsValidSuffix n s xs) (hs : 1 ≤ s) (hsN : s ≤ n + 1) :
    ∃ ds : List ℕ,
      xs = blocksFrom n s ds ∧
      (s :: ds).Pairwise (fun a b => a + 2 ≤ b) ∧
      (s :: ds).Pairwise (· ∣ ·) ∧
      ∀ d ∈ ds, d ≤ n + 1 := by
  by_cases hsend : s = n + 1
  · subst s
    have hxlen : xs.length = 0 := by simpa using h.length
    have hxs : xs = [] := List.length_eq_zero_iff.mp hxlen
    refine ⟨[], ?_, by simp, by simp, by simp⟩
    simp [hxs, blocksFrom]
  · have hsn : s ≤ n := by omega
    let e := xs[0]'(by rw [h.length]; omega)
    have heq : xs[0]'(by rw [h.length]; omega) = e := rfl
    have hemem : e ∈ xs := heq ▸ List.getElem_mem (by rw [h.length]; omega)
    have hse : s ≤ e := (h.mem_iff.mp hemem).1
    rcases hse.eq_or_lt with hseEq | hseLt
    · have hid := h.eq_range_of_head_eq hs hsn (heq.trans hseEq.symm)
      refine ⟨[], ?_, by simp, by simp, by simp⟩
      simpa [blocksFrom] using hid
    · let d := e + 1
      let len := d - s
      have hd : d ≤ n + 1 := by
        have := (h.mem_iff.mp hemem).2
        dsimp [d]
        omega
      have htail := h.drop_after_nontrivial hs hsn heq hseLt
      change IsValidSuffix n d (xs.drop len) at htail
      obtain ⟨ds, htailEq, hgap, hdvd, hbound⟩ :=
        htail.exists_blocks (by dsimp [d]; omega) hd
      have htake : xs.take len = rotatedBlock s d := by
        dsimp [len, d]
        exact h.forced_rotated_take hs hsn heq hseLt
      have hsplit : rotatedBlock s d ++ xs.drop len = xs := by
        rw [← htake]
        exact List.take_append_drop len xs
      refine ⟨d :: ds, ?_, ?_, ?_, ?_⟩
      · calc
          xs = rotatedBlock s d ++ xs.drop len := hsplit.symm
          _ = rotatedBlock s d ++ blocksFrom n d ds := by rw [htailEq]
          _ = blocksFrom n s (d :: ds) := rfl
      · rw [List.pairwise_cons]
        refine ⟨?_, hgap⟩
        intro x hx
        rw [List.mem_cons] at hx
        rcases hx with rfl | hx
        · dsimp [d]
          omega
        · have hdx := (List.pairwise_cons.mp hgap).1 x hx
          dsimp [d] at hdx ⊢
          omega
      · rw [List.pairwise_cons]
        refine ⟨?_, hdvd⟩
        intro x hx
        rw [List.mem_cons] at hx
        rcases hx with rfl | hx
        · exact h.start_dvd_endpoint hs hsn heq hseLt
        · exact (h.start_dvd_endpoint hs hsn heq hseLt).trans
            ((List.pairwise_cons.mp hdvd).1 x hx)
      · intro x hx
        rw [List.mem_cons] at hx
        rcases hx with rfl | hx
        · exact hd
        · exact hbound x hx
termination_by n + 1 - s
decreasing_by
  omega

/-- A block list satisfying the endpoint gap and bound conditions is uniquely
determined by the permutation it builds. -/
theorem blocksFrom_injective (n s : ℕ) :
    ∀ (ds es : List ℕ),
      (s :: ds).Pairwise (fun a b => a + 2 ≤ b) →
      (s :: es).Pairwise (fun a b => a + 2 ≤ b) →
      (∀ d ∈ ds, d ≤ n + 1) →
      (∀ e ∈ es, e ≤ n + 1) →
      s ≤ n + 1 →
      blocksFrom n s ds = blocksFrom n s es → ds = es
  | [], [], _, _, _, _, _, _ => rfl
  | [], e :: es, _, hgapE, _, hboundE, _, heq => by
      have hse : s + 2 ≤ e :=
        (List.pairwise_cons.mp hgapE).1 e (by simp)
      have hen : e ≤ n + 1 := hboundE e (by simp)
      have hlen : n + 1 - s ≠ 0 := by omega
      have hhead := congrArg List.head? heq
      simp [blocksFrom, rotatedBlock, List.head?_range', hlen] at hhead
      omega
  | d :: ds, [], hgapD, _, hboundD, _, _, heq => by
      have hsd : s + 2 ≤ d :=
        (List.pairwise_cons.mp hgapD).1 d (by simp)
      have hdn : d ≤ n + 1 := hboundD d (by simp)
      have hlen : n + 1 - s ≠ 0 := by omega
      have hhead := congrArg List.head? heq
      simp [blocksFrom, rotatedBlock, List.head?_range', hlen] at hhead
      omega
  | d :: ds, e :: es, hgapD, hgapE, hboundD, hboundE, _, heq => by
      have hsd : s + 2 ≤ d :=
        (List.pairwise_cons.mp hgapD).1 d (by simp)
      have hse : s + 2 ≤ e :=
        (List.pairwise_cons.mp hgapE).1 e (by simp)
      have hhead := congrArg List.head? heq
      have hde : d = e := by
        simp [blocksFrom, rotatedBlock] at hhead
        omega
      subst e
      have htailEq : blocksFrom n d ds = blocksFrom n d es := by
        apply List.append_right_injective (rotatedBlock s d)
        simpa [blocksFrom] using heq
      have hrec := blocksFrom_injective n d ds es
        (List.pairwise_cons.mp hgapD).2
        (List.pairwise_cons.mp hgapE).2
        (fun x hx => hboundD x (by simp [hx]))
        (fun x hx => hboundE x (by simp [hx]))
        (hboundD d (by simp)) htailEq
      rw [hrec]

end A022825Lean
