import A022825Lean.Remainders

namespace A022825Lean

/-! The canonical block decomposition of a valid permutation. -/

theorem pairwise_lt_of_pairwise_le_of_nodup {α : Type*} [LinearOrder α]
    {xs : List α} (hle : xs.Pairwise (· ≤ ·)) (hn : xs.Nodup) :
    xs.Pairwise (· < ·) := by
  induction xs with
  | nil => simp
  | cons a xs ih =>
      rw [List.pairwise_cons] at hle ⊢
      rw [List.nodup_cons] at hn
      refine ⟨?_, ih hle.2 hn.2⟩
      intro b hb
      exact lt_of_le_of_ne (hle.1 b hb) fun hab => hn.1 (hab ▸ hb)

theorem DivisorChain.values_pairwise_lt {N : ℕ} (c : DivisorChain N) :
    c.values.Pairwise (· < ·) := by
  rw [DivisorChain.values, List.pairwise_map]
  exact pairwise_lt_of_pairwise_le_of_nodup
    (Finset.pairwise_sort c.1 (· ≤ ·)) (Finset.sort_nodup c.1 (· ≤ ·))

theorem DivisorChain.mem_values_iff {N d : ℕ} (c : DivisorChain N) :
    d ∈ c.values ↔ ∃ e : Fin (N + 1), e ∈ c.1 ∧ e.1 = d := by
  simp [DivisorChain.values]

theorem DivisorChain.value_bounds {N d : ℕ} (c : DivisorChain N)
    (hd : d ∈ c.values) : 3 ≤ d ∧ d ≤ N := by
  rcases c.mem_values_iff.mp hd with ⟨e, he, rfl⟩
  exact ⟨c.2.1 e he, Nat.le_of_lt_succ e.2⟩

theorem DivisorChain.values_pairwise_dvd {N : ℕ} (c : DivisorChain N) :
    c.values.Pairwise (· ∣ ·) := by
  rw [DivisorChain.values, List.pairwise_map]
  rw [List.pairwise_iff_getElem]
  intro i j hi hj hij
  let sorted := c.1.sort (· ≤ ·)
  have hai : sorted[i] ∈ c.1 := by
    rw [← Finset.mem_sort (· ≤ ·)]
    exact List.getElem_mem hi
  have hbj : sorted[j] ∈ c.1 := by
    rw [← Finset.mem_sort (· ≤ ·)]
    exact List.getElem_mem hj
  have hlt : sorted[i] < sorted[j] :=
    (pairwise_lt_of_pairwise_le_of_nodup
      (Finset.pairwise_sort c.1 (· ≤ ·))
      (Finset.sort_nodup c.1 (· ≤ ·))).rel_get_of_lt
        (a := ⟨i, hi⟩) (b := ⟨j, hj⟩) hij
  exact c.2.2 _ hai _ hbj hlt

theorem add_two_le_of_dvd_of_lt {a b : ℕ} (ha : 2 ≤ a)
    (hab : a ∣ b) (hlt : a < b) : a + 2 ≤ b := by
  rcases hab with ⟨k, rfl⟩
  have hk0 : k ≠ 0 := by
    intro hk
    simp [hk] at hlt
  have hk1 : k ≠ 1 := by
    intro hk
    simp [hk] at hlt
  have hk2 : 2 ≤ k := by omega
  calc
    a + 2 ≤ a + a := Nat.add_le_add_left ha a
    _ = a * 2 := by omega
    _ ≤ a * k := Nat.mul_le_mul_left a hk2

theorem DivisorChain.values_pairwise_add_two_le {N : ℕ}
    (c : DivisorChain N) : c.values.Pairwise (fun a b => a + 2 ≤ b) := by
  rw [List.pairwise_iff_getElem]
  intro i j hi hj hij
  have hlt := c.values_pairwise_lt.rel_get_of_lt
    (a := ⟨i, hi⟩) (b := ⟨j, hj⟩) hij
  have hdvd := c.values_pairwise_dvd.rel_get_of_lt
    (a := ⟨i, hi⟩) (b := ⟨j, hj⟩) hij
  have ha3 := (c.value_bounds (List.getElem_mem hi)).1
  exact add_two_le_of_dvd_of_lt (by omega) hdvd hlt

theorem DivisorChain.one_cons_values_pairwise_add_two_le {N : ℕ}
    (c : DivisorChain N) :
    (1 :: c.values).Pairwise (fun a b => a + 2 ≤ b) := by
  rw [List.pairwise_cons]
  exact ⟨fun d hd => c.value_bounds hd |>.1, c.values_pairwise_add_two_le⟩

theorem DivisorChain.one_cons_values_pairwise_dvd {N : ℕ}
    (c : DivisorChain N) : (1 :: c.values).Pairwise (· ∣ ·) := by
  rw [List.pairwise_cons]
  exact ⟨fun _ _ => one_dvd _, c.values_pairwise_dvd⟩

/-- Rotate the interval `[s, d-1]` once to the right. -/
def rotatedBlock (s d : ℕ) : List ℕ :=
  (d - 1) :: List.range' s (d - s - 1)

/-- Build all rotated blocks and then append the identity tail `[s, ..., n]`. -/
def blocksFrom (n : ℕ) : ℕ → List ℕ → List ℕ
  | s, [] => List.range' s (n + 1 - s)
  | s, d :: ds => rotatedBlock s d ++ blocksFrom n d ds

/-- The permutation associated with a list of block endpoints. -/
def endpointPermutation (n : ℕ) (ds : List ℕ) : List ℕ :=
  blocksFrom n 1 ds

/-- The permutation of `[1, ..., n]` associated with a chain bounded by `n+1`. -/
def DivisorChain.permutation {n : ℕ} (c : DivisorChain (n + 1)) : List ℕ :=
  endpointPermutation n c.values

theorem rotatedBlock_perm {s d : ℕ} (h : s + 2 ≤ d) :
    List.Perm (rotatedBlock s d) (List.range' s (d - s)) := by
  have hrange :
      List.range' s (d - s - 1) ++ [d - 1] = List.range' s (d - s) := by
    rw [show d - s = (d - s - 1) + 1 by omega, List.range'_concat]
    congr 2
    omega
  simpa [rotatedBlock, hrange] using
    (List.perm_append_comm :
      List.Perm ([d - 1] ++ List.range' s (d - s - 1))
        (List.range' s (d - s - 1) ++ [d - 1]))

theorem blocksFrom_perm (n : ℕ) :
    ∀ (s : ℕ) (ds : List ℕ),
      (s :: ds).Pairwise (fun a b => a + 2 ≤ b) →
      (∀ d ∈ ds, d ≤ n + 1) → s ≤ n + 1 →
      List.Perm (blocksFrom n s ds) (List.range' s (n + 1 - s))
  | _, [], _, _, _ => List.Perm.refl _
  | s, d :: ds, hgap, hbound, _ => by
      rw [List.pairwise_cons] at hgap
      have hsd : s + 2 ≤ d := hgap.1 d (by simp)
      have hd : d ≤ n + 1 := hbound d (by simp)
      have htail : (d :: ds).Pairwise (fun a b => a + 2 ≤ b) := hgap.2
      have htailBound : ∀ e ∈ ds, e ≤ n + 1 :=
        fun e he => hbound e (by simp [he])
      have hblock := rotatedBlock_perm hsd
      have hrest := blocksFrom_perm n d ds htail htailBound hd
      have happ := hblock.append hrest
      have hrange :
          List.range' s (d - s) ++ List.range' d (n + 1 - d) =
            List.range' s (n + 1 - s) := by
        have hstart : s + (d - s) = d := by
          rw [Nat.add_comm, Nat.sub_add_cancel (by omega)]
        have hlen : (d - s) + (n + 1 - d) = n + 1 - s := by
          rw [Nat.add_comm]
          exact Nat.sub_add_sub_cancel hd (by omega)
        simpa only [one_mul, hstart, hlen] using
          (List.range'_append (s := s) (m := d - s)
            (n := n + 1 - d) (step := 1))
      simpa [blocksFrom, hrange] using happ

theorem DivisorChain.permutation_perm {n : ℕ} (c : DivisorChain (n + 1)) :
    c.permutation.Perm (oneTo n) := by
  simpa [DivisorChain.permutation, endpointPermutation, oneTo] using
    blocksFrom_perm n 1 c.values c.one_cons_values_pairwise_add_two_le
      (fun d hd => (c.value_bounds hd).2) (by omega)

theorem pred_mod_of_dvd {s d : ℕ} (hs : 0 < s) (hd : s ∣ d)
    (hdpos : 0 < d) : (d - 1) % s = s - 1 := by
  have hle : s ≤ d := Nat.le_of_dvd hdpos hd
  have heq : d - 1 = (d - s) + (s - 1) := by omega
  have hzero : (d - s) % s = 0 :=
    Nat.dvd_iff_mod_eq_zero.mp (Nat.dvd_sub hd (dvd_refl s))
  have hpred : (s - 1) % s = s - 1 := Nat.mod_eq_of_lt (by omega)
  rw [heq, Nat.add_mod, hzero, zero_add, hpred, hpred]

theorem remainders_range' :
    ∀ (s len : ℕ), remainders (List.range' s len) = List.range' s (len - 1)
  | _, 0 => rfl
  | _, 1 => rfl
  | s, len + 2 => by
      simp only [List.range'_succ, remainders, Nat.reduceSubDiff]
      change s % (s + 1) :: remainders (List.range' (s + 1) (len + 1)) =
        s :: List.range' (s + 1) len
      rw [remainders_range' (s + 1) (len + 1)]
      rw [Nat.mod_eq_of_lt (by omega)]
      simp

theorem remainders_rotatedBlock {s d : ℕ} (hs : 0 < s)
    (hgap : s + 2 ≤ d) (hdvd : s ∣ d) :
    remainders (rotatedBlock s d) = List.range' (s - 1) (d - s - 1) := by
  have hlen : d - s - 1 = (d - s - 2) + 1 := by omega
  have hrange :
      List.range' s (d - s - 1) = s :: List.range' (s + 1) (d - s - 2) := by
    rw [hlen, List.range'_succ]
  rw [rotatedBlock, hrange]
  simp only [remainders]
  rw [pred_mod_of_dvd hs hdvd (by omega)]
  rw [show remainders (s :: List.range' (s + 1) (d - s - 2)) =
      remainders (List.range' s (d - s - 1)) by rw [hrange]]
  rw [remainders_range', hlen, List.range'_succ]
  have hs' : s - 1 + 1 = s := Nat.sub_add_cancel hs
  simp only [hs']
  simp

theorem remainders_append {xs ys : List ℕ} (hx : xs ≠ []) (hy : ys ≠ []) :
    remainders (xs ++ ys) = remainders xs ++
      [xs.getLast hx % ys.head hy] ++ remainders ys := by
  induction xs with
  | nil => contradiction
  | cons x xs ih =>
      cases xs with
      | nil =>
          obtain ⟨y, ys, rfl⟩ := List.exists_cons_of_ne_nil hy
          rfl
      | cons z zs =>
          simpa only [List.cons_append, remainders] using
            congrArg (List.cons (x % z)) (ih (by simp))

theorem mem_range'_bounds {s len x : ℕ}
    (hx : x ∈ List.range' s len) : s ≤ x ∧ x < s + len := by
  rw [List.mem_range'] at hx
  rcases hx with ⟨i, hi, rfl⟩
  omega

theorem getLast_rotatedBlock {s d : ℕ} (hgap : s + 2 ≤ d) :
    (rotatedBlock s d).getLast (by simp [rotatedBlock]) = d - 2 := by
  have htail : List.range' s (d - s - 1) ≠ [] := by
    simp
    omega
  change ((d - 1) :: List.range' s (d - s - 1)).getLast _ = d - 2
  rw [List.getLast_cons htail, List.getLast_range']
  omega

/-- Remainders of a valid block construction are increasing and stay in range. -/
theorem blocksFrom_remainders (n : ℕ) :
    ∀ (s : ℕ) (ds : List ℕ),
      (s :: ds).Pairwise (fun a b => a + 2 ≤ b) →
      (s :: ds).Pairwise (· ∣ ·) →
      (∀ d ∈ ds, d ≤ n + 1) → 1 ≤ s → s ≤ n + 1 →
      let rs := remainders (blocksFrom n s ds)
      rs.Pairwise (· < ·) ∧
        (∀ r ∈ rs, s - 1 ≤ r) ∧ (∀ r ∈ rs, r < n)
  | s, [], _, _, _, _, _ => by
      simp only [blocksFrom, remainders_range']
      refine ⟨List.pairwise_lt_range', ?_, ?_⟩
      · intro r hr
        have hb := mem_range'_bounds hr
        omega
      · intro r hr
        have hb := mem_range'_bounds hr
        omega
  | s, d :: ds, hgap, hdvd, hbound, hs, _ => by
      rw [List.pairwise_cons] at hgap hdvd
      have hsd : s + 2 ≤ d := hgap.1 d (by simp)
      have hsdvd : s ∣ d := hdvd.1 d (by simp)
      have hd : d ≤ n + 1 := hbound d (by simp)
      have htailGap : (d :: ds).Pairwise (fun a b => a + 2 ≤ b) := hgap.2
      have htailDvd : (d :: ds).Pairwise (· ∣ ·) := hdvd.2
      have htailBound : ∀ e ∈ ds, e ≤ n + 1 :=
        fun e he => hbound e (by simp [he])
      have ih := blocksFrom_remainders n d ds htailGap htailDvd
        htailBound (by omega) hd
      let rest := blocksFrom n d ds
      by_cases hrest : rest = []
      · have hwhole : blocksFrom n s (d :: ds) = rotatedBlock s d := by
          simp [blocksFrom, rest, hrest]
        rw [hwhole, remainders_rotatedBlock hs hsd hsdvd]
        refine ⟨List.pairwise_lt_range', ?_, ?_⟩
        · intro r hr
          have hb := mem_range'_bounds hr
          omega
        · intro r hr
          have hb := mem_range'_bounds hr
          omega
      · have hrestPerm :
            List.Perm rest (List.range' d (n + 1 - d)) := by
          exact blocksFrom_perm n d ds htailGap htailBound hd
        have hheadMem : rest.head hrest ∈ rest := List.head_mem hrest
        have hheadRange : rest.head hrest ∈ List.range' d (n + 1 - d) :=
          hrestPerm.mem_iff.mp hheadMem
        have hheadLower : d ≤ rest.head hrest :=
          (mem_range'_bounds hheadRange).1
        have hlast :
            (rotatedBlock s d).getLast (by simp [rotatedBlock]) = d - 2 :=
          getLast_rotatedBlock hsd
        have hboundary :
            (rotatedBlock s d).getLast (by simp [rotatedBlock]) %
              rest.head hrest = d - 2 := by
          rw [hlast, Nat.mod_eq_of_lt (by omega)]
        have happ := remainders_append
          (xs := rotatedBlock s d) (ys := rest) (by simp [rotatedBlock]) hrest
        have hprefix :
            List.range' (s - 1) (d - s - 1) ++ [d - 2] =
              List.range' (s - 1) (d - s) := by
          rw [show d - s = (d - s - 1) + 1 by omega, List.range'_concat]
          congr 2
          omega
        have hrem :
            remainders (blocksFrom n s (d :: ds)) =
              List.range' (s - 1) (d - s) ++ remainders rest := by
          rw [blocksFrom, show blocksFrom n d ds = rest by rfl, happ,
            remainders_rotatedBlock hs hsd hsdvd, hboundary, hprefix]
        rw [hrem]
        refine ⟨List.pairwise_append.mpr ⟨List.pairwise_lt_range', ih.1, ?_⟩, ?_, ?_⟩
        · intro a ha b hb
          have habound := mem_range'_bounds ha
          have hblower := ih.2.1 b hb
          omega
        · intro r hr
          rw [List.mem_append] at hr
          rcases hr with hr | hr
          · exact (mem_range'_bounds hr).1
          · have := ih.2.1 r hr
            omega
        · intro r hr
          rw [List.mem_append] at hr
          rcases hr with hr | hr
          · have hrb := mem_range'_bounds hr
            omega
          · exact ih.2.2 r hr

theorem DivisorChain.permutation_remainders_pairwise {n : ℕ}
    (c : DivisorChain (n + 1)) :
    (remainders c.permutation).Pairwise (· < ·) := by
  simpa [DivisorChain.permutation, endpointPermutation] using
    (blocksFrom_remainders n 1 c.values
      c.one_cons_values_pairwise_add_two_le c.one_cons_values_pairwise_dvd
      (fun d hd => (c.value_bounds hd).2) (by omega) (by omega)).1

end A022825Lean
