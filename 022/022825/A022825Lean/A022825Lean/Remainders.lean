import A022825Lean.Definitions

namespace A022825Lean

/-! Basic lemmas about the strictly increasing remainder list. -/

@[simp] theorem length_oneTo (n : ℕ) : (oneTo n).length = n := by
  simp [oneTo]

theorem mem_oneTo {n x : ℕ} : x ∈ oneTo n ↔ 1 ≤ x ∧ x ≤ n := by
  rw [oneTo, List.mem_range']
  constructor
  · rintro ⟨i, hi, rfl⟩
    omega
  · rintro ⟨hx, hxn⟩
    exact ⟨x - 1, by omega, by omega⟩

theorem nodup_oneTo (n : ℕ) : (oneTo n).Nodup := by
  simpa [oneTo] using List.nodup_range' (s := 1) (n := n)

theorem RemainderPermutation.values_mem_permutations {n : ℕ}
    (p : RemainderPermutation n) : p.values ∈ (oneTo n).permutations := by
  exact List.get_mem _ p.1

theorem RemainderPermutation.values_perm {n : ℕ}
    (p : RemainderPermutation n) : p.values.Perm (oneTo n) :=
  List.perm_of_mem_permutations p.values_mem_permutations

@[simp] theorem RemainderPermutation.length_values {n : ℕ}
    (p : RemainderPermutation n) : p.values.length = n := by
  rw [p.values_perm.length_eq, length_oneTo]

theorem RemainderPermutation.mem_values_iff {n x : ℕ}
    (p : RemainderPermutation n) : x ∈ p.values ↔ 1 ≤ x ∧ x ≤ n := by
  rw [p.values_perm.mem_iff, mem_oneTo]

theorem RemainderPermutation.nodup_values {n : ℕ}
    (p : RemainderPermutation n) : p.values.Nodup :=
  p.values_perm.nodup_iff.mpr (nodup_oneTo n)

theorem RemainderPermutation.remainders_pairwise {n : ℕ}
    (p : RemainderPermutation n) : (remainders p.values).Pairwise (· < ·) := by
  exact p.2

@[simp] theorem length_remainders :
    ∀ xs : List ℕ, (remainders xs).length = xs.length - 1
  | [] => rfl
  | [_] => rfl
  | _ :: y :: xs => by
      simp only [remainders, List.length_cons]
      rw [length_remainders (y :: xs)]
      simp

theorem mem_remainders_exists_adjacent {r : ℕ} :
    ∀ {xs : List ℕ}, r ∈ remainders xs →
      ∃ x ∈ xs, ∃ y ∈ xs, r = x % y
  | [], h => by simp [remainders] at h
  | [_], h => by simp [remainders] at h
  | x :: y :: xs, h => by
      simp only [remainders, List.mem_cons] at h
      rcases h with rfl | h
      · exact ⟨x, by simp, y, by simp, rfl⟩
      · rcases mem_remainders_exists_adjacent h with
          ⟨a, ha, b, hb, hab⟩
        exact ⟨a, by simp [ha], b, by simp [hb], hab⟩

theorem RemainderPermutation.remainder_lt_n {n r : ℕ}
    (p : RemainderPermutation n) (hr : r ∈ remainders p.values) : r < n := by
  rcases mem_remainders_exists_adjacent hr with ⟨x, hx, y, hy, rfl⟩
  have hyb := (p.mem_values_iff.mp hy).2
  have hyp := (p.mem_values_iff.mp hy).1
  exact (Nat.mod_lt x hyp).trans_le hyb

/-- The `i`-th entry of a strictly increasing natural-number list is at least `i`. -/
theorem index_le_getElem_of_pairwise_lt {xs : List ℕ}
    (hxs : xs.Pairwise (· < ·)) :
    ∀ (i : ℕ) (hi : i < xs.length), i ≤ xs[i]
  | 0, _ => Nat.zero_le _
  | i + 1, hi => by
      have hi' : i < xs.length := by omega
      have hstep : xs[i] < xs[i + 1] :=
        (List.pairwise_iff_getElem.mp hxs) i (i + 1) hi' hi (by omega)
      have hind := index_le_getElem_of_pairwise_lt hxs i hi'
      omega

theorem RemainderPermutation.index_le_remainder {n i : ℕ}
    (p : RemainderPermutation n) (hi : i < (remainders p.values).length) :
    i ≤ (remainders p.values)[i] :=
  index_le_getElem_of_pairwise_lt p.remainders_pairwise i hi

/-- Remainders really are the adjacent moduli in the original list. -/
theorem getElem_remainders :
    ∀ (xs : List ℕ) (i : ℕ) (hi : i < (remainders xs).length),
      (remainders xs)[i] =
        xs[i]'(by rw [length_remainders] at hi; omega) %
          xs[i + 1]'(by rw [length_remainders] at hi; omega)
  | [], i, hi => by simp [remainders] at hi
  | [_], i, hi => by simp [remainders] at hi
  | _ :: _ :: _, 0, _ => rfl
  | x :: y :: ys, i + 1, hi => by
      simp only [remainders, List.getElem_cons_succ]
      exact getElem_remainders (y :: ys) i (by
        simpa only [remainders, List.length_cons, Nat.add_lt_add_iff_right] using hi)

/-- A strictly increasing list gains at least one at each successive index. -/
theorem getElem_add_le_getElem_of_pairwise_lt {xs : List ℕ}
    (hxs : xs.Pairwise (· < ·)) (i : ℕ) :
    ∀ (d : ℕ) (hid : i + d < xs.length), xs[i] + d ≤ xs[i + d]
  | 0, _ => by simp
  | d + 1, hid => by
      have hprev : i + d < xs.length := by omega
      have hind := getElem_add_le_getElem_of_pairwise_lt hxs i d hprev
      have hstep : xs[i + d] < xs[i + (d + 1)] :=
        (List.pairwise_iff_getElem.mp hxs) (i + d) (i + (d + 1))
          hprev hid (by omega)
      omega

/-- A strictly increasing list of `n-1` values below `n` has entry `i` at most `i+1`. -/
theorem getElem_le_index_add_one {xs : List ℕ} {n i : ℕ}
    (hxs : xs.Pairwise (· < ·)) (hlen : xs.length = n - 1)
    (hbound : ∀ x ∈ xs, x < n) (hi : i < xs.length) : xs[i] ≤ i + 1 := by
  let d := xs.length - 1 - i
  have hid : i + d < xs.length := by
    dsimp [d]
    omega
  have hgrow := getElem_add_le_getElem_of_pairwise_lt hxs i d hid
  have hlast : xs[i + d] < n := hbound _ (List.getElem_mem hid)
  dsimp [d] at hgrow hlast
  omega

theorem RemainderPermutation.remainder_le_index_add_one {n i : ℕ}
    (p : RemainderPermutation n) (hi : i < (remainders p.values).length) :
    (remainders p.values)[i] ≤ i + 1 := by
  apply getElem_le_index_add_one (n := n) p.remainders_pairwise
  · simp
  · exact fun x hx => p.remainder_lt_n hx

theorem RemainderPermutation.remainder_eq_mod {n i : ℕ}
    (p : RemainderPermutation n) (hi : i < (remainders p.values).length) :
    (remainders p.values)[i] =
      p.values[i]'(by rw [length_remainders] at hi; omega) %
        p.values[i + 1]'(by rw [length_remainders] at hi; omega) :=
  getElem_remainders p.values i hi

/-- One-based position of a value in a valid permutation. -/
def RemainderPermutation.position {n : ℕ} (p : RemainderPermutation n)
    (v : ℕ) : ℕ := p.values.idxOf v + 1

theorem RemainderPermutation.position_le_succ {n v : ℕ}
    (p : RemainderPermutation n) (hv : v ∈ p.values) : p.position v ≤ v + 1 := by
  have hj : p.values.idxOf v < p.values.length :=
    List.idxOf_lt_length_iff.mpr hv
  have hvpos : p.values[p.values.idxOf v] = v := List.getElem_idxOf hj
  rcases hidx : p.values.idxOf v with _ | k
  · simp [RemainderPermutation.position, hidx]
  · have hkrem : k < (remainders p.values).length := by
      rw [length_remainders]
      omega
    have hlower := p.index_le_remainder hkrem
    have hmod := p.remainder_eq_mod hkrem
    have hvpos' : p.values[k + 1] = v := by simpa [hidx] using hvpos
    rw [hvpos'] at hmod
    have hvpositive := (p.mem_values_iff.mp hv).1
    have hlt : (remainders p.values)[k] < v := by
      rw [hmod]
      exact Nat.mod_lt _ hvpositive
    simp only [RemainderPermutation.position, hidx]
    omega

end A022825Lean
