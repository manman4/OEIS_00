import A022825Lean.ReverseDecomposition

namespace A022825Lean

/-! The equivalence between valid permutations and divisor chains. -/

theorem pairwise_rel_of_pairwise_lt_of_mem {R : ℕ → ℕ → Prop}
    {xs : List ℕ} {a b : ℕ} (hlt : xs.Pairwise (· < ·))
    (hR : xs.Pairwise R) (ha : a ∈ xs) (hb : b ∈ xs) (hab : a < b) :
    R a b := by
  have hia : xs.idxOf a < xs.length := List.idxOf_lt_length_iff.mpr ha
  have hib : xs.idxOf b < xs.length := List.idxOf_lt_length_iff.mpr hb
  let ia : Fin xs.length := ⟨xs.idxOf a, hia⟩
  let ib : Fin xs.length := ⟨xs.idxOf b, hib⟩
  have hgeta : xs.get ia = a := by
    exact List.getElem_idxOf hia
  have hgetb : xs.get ib = b := by
    exact List.getElem_idxOf hib
  have hiab : xs.idxOf a < xs.idxOf b := by
    by_contra hnot
    have hba : xs.idxOf b ≤ xs.idxOf a := by omega
    rcases hba.eq_or_lt with heq | hltidx
    · have hieq : ib = ia := Fin.ext heq
      have habEq : b = a := by
        calc
          b = xs.get ib := hgetb.symm
          _ = xs.get ia := congrArg xs.get hieq
          _ = a := hgeta
      omega
    · have hbaVal := hlt.rel_get_of_lt
        (a := ib) (b := ia) hltidx
      have : b < a := by
        rw [hgetb, hgeta] at hbaVal
        exact hbaVal
      omega
  have hrel := hR.rel_get_of_lt
    (a := ia) (b := ib) hiab
  rw [hgeta, hgetb] at hrel
  exact hrel

/-- Package a sorted list satisfying the chain conditions as the finite-set
representation `DivisorChain N`. -/
noncomputable def divisorChainOfList {N : ℕ} (ds : List ℕ)
    (hlower : ∀ d ∈ ds, 3 ≤ d)
    (hlt : ds.Pairwise (· < ·)) (hdvd : ds.Pairwise (· ∣ ·))
    (_hbound : ∀ d ∈ ds, d ≤ N) : DivisorChain N := by
  classical
  refine ⟨Finset.univ.filter (fun d : Fin (N + 1) => d.1 ∈ ds), ?_⟩
  constructor
  · intro d hd
    exact hlower d.1 (Finset.mem_filter.mp hd).2
  · intro a ha b hb hab
    exact pairwise_rel_of_pairwise_lt_of_mem hlt hdvd
      (Finset.mem_filter.mp ha).2 (Finset.mem_filter.mp hb).2 hab

@[simp] theorem values_divisorChainOfList {N : ℕ} (ds : List ℕ)
    (hlower : ∀ d ∈ ds, 3 ≤ d)
    (hlt : ds.Pairwise (· < ·)) (hdvd : ds.Pairwise (· ∣ ·))
    (hbound : ∀ d ∈ ds, d ≤ N) :
    (divisorChainOfList ds hlower hlt hdvd hbound).values = ds := by
  apply List.Pairwise.eq_of_mem_iff
    (DivisorChain.values_pairwise_lt _) hlt
  intro d
  rw [DivisorChain.mem_values_iff]
  constructor
  · rintro ⟨e, he, rfl⟩
    exact (Finset.mem_filter.mp he).2
  · intro hd
    let e : Fin (N + 1) := ⟨d, by have := hbound d hd; omega⟩
    refine ⟨e, ?_, rfl⟩
    exact Finset.mem_filter.mpr ⟨Finset.mem_univ e, hd⟩

/-- Existence half of the converse: every valid remainder permutation is the
canonical permutation of a divisor chain. -/
theorem RemainderPermutation.exists_divisorChain {n : ℕ}
    (p : RemainderPermutation n) :
    ∃ c : DivisorChain (n + 1), p.values = c.permutation := by
  obtain ⟨ds, hblocks, hgap, hdvd, hbound⟩ :=
    p.isValidSuffix.exists_blocks (by omega) (by omega)
  have hgapTail := (List.pairwise_cons.mp hgap).2
  have hlower : ∀ d ∈ ds, 3 ≤ d :=
    (List.pairwise_cons.mp hgap).1
  have hlt : ds.Pairwise (· < ·) :=
    hgapTail.imp (fun hde => by omega)
  have hdvdTail := (List.pairwise_cons.mp hdvd).2
  let c : DivisorChain (n + 1) :=
    divisorChainOfList ds hlower hlt hdvdTail hbound
  have hcvalues : c.values = ds := by
    dsimp [c]
    exact values_divisorChainOfList ds hlower hlt hdvdTail hbound
  refine ⟨c, ?_⟩
  rw [DivisorChain.permutation, endpointPermutation, hcvalues]
  exact hblocks

/-- Turn a list satisfying the two defining properties into the corresponding
member of the finite type `RemainderPermutation n`. -/
noncomputable def RemainderPermutation.ofList {n : ℕ} (xs : List ℕ)
    (hperm : xs.Perm (oneTo n))
    (hrem : (remainders xs).Pairwise (· < ·)) : RemainderPermutation n := by
  classical
  let all := (oneTo n).permutations
  have hmem : xs ∈ all := List.mem_permutations.mpr hperm
  have hidx : all.idxOf xs < all.length := List.idxOf_lt_length_iff.mpr hmem
  let i : Fin all.length := ⟨all.idxOf xs, hidx⟩
  refine ⟨i, ?_⟩
  have hget : all.get i = xs := by
    exact List.getElem_idxOf hidx
  change (remainders (all.get i)).Pairwise (· < ·)
  rw [hget]
  exact hrem

@[simp] theorem RemainderPermutation.values_ofList {n : ℕ} (xs : List ℕ)
    (hperm : xs.Perm (oneTo n))
    (hrem : (remainders xs).Pairwise (· < ·)) :
    (RemainderPermutation.ofList xs hperm hrem).values = xs := by
  classical
  unfold RemainderPermutation.ofList RemainderPermutation.values
  simp only
  apply List.getElem_idxOf

/-- The forward map from a divisor chain to its remainder permutation. -/
noncomputable def DivisorChain.toRemainderPermutation {n : ℕ}
    (c : DivisorChain (n + 1)) : RemainderPermutation n :=
  RemainderPermutation.ofList c.permutation c.permutation_perm
    c.permutation_remainders_pairwise

@[simp] theorem DivisorChain.values_toRemainderPermutation {n : ℕ}
    (c : DivisorChain (n + 1)) :
    c.toRemainderPermutation.values = c.permutation := by
  simp [DivisorChain.toRemainderPermutation]

theorem RemainderPermutation.ext_values {n : ℕ}
    {p q : RemainderPermutation n} (h : p.values = q.values) : p = q := by
  apply Subtype.ext
  apply (List.nodup_permutations (oneTo n) (nodup_oneTo n)).get_inj_iff.mp
  exact h

theorem DivisorChain.ext_values {N : ℕ} {c d : DivisorChain N}
    (h : c.values = d.values) : c = d := by
  apply Subtype.ext
  apply Finset.ext
  intro x
  have mem_values (a : DivisorChain N) : x.1 ∈ a.values ↔ x ∈ a.1 := by
    rw [a.mem_values_iff]
    constructor
    · rintro ⟨y, hy, hval⟩
      have hyx : y = x := Fin.ext hval
      simpa [hyx] using hy
    · intro hx
      exact ⟨x, hx, rfl⟩
  rw [← mem_values c, ← mem_values d, h]

theorem DivisorChain.toRemainderPermutation_injective {n : ℕ} :
    Function.Injective
      (DivisorChain.toRemainderPermutation :
        DivisorChain (n + 1) → RemainderPermutation n) := by
  intro c d hcd
  apply DivisorChain.ext_values
  have hperm := congrArg RemainderPermutation.values hcd
  rw [c.values_toRemainderPermutation, d.values_toRemainderPermutation] at hperm
  change blocksFrom n 1 c.values = blocksFrom n 1 d.values at hperm
  exact blocksFrom_injective n 1 c.values d.values
    c.one_cons_values_pairwise_add_two_le
    d.one_cons_values_pairwise_add_two_le
    (fun x hx => (c.value_bounds hx).2)
    (fun x hx => (d.value_bounds hx).2) (by omega) hperm

theorem DivisorChain.toRemainderPermutation_surjective {n : ℕ} :
    Function.Surjective
      (DivisorChain.toRemainderPermutation :
        DivisorChain (n + 1) → RemainderPermutation n) := by
  intro p
  obtain ⟨c, hpc⟩ := p.exists_divisorChain
  refine ⟨c, RemainderPermutation.ext_values ?_⟩
  rw [c.values_toRemainderPermutation]
  exact hpc.symm

/-- The full bijection used by the counting theorem. -/
noncomputable def divisorChainEquivRemainderPermutation (n : ℕ) :
    DivisorChain (n + 1) ≃ RemainderPermutation n :=
  Equiv.ofBijective DivisorChain.toRemainderPermutation
    ⟨DivisorChain.toRemainderPermutation_injective,
      DivisorChain.toRemainderPermutation_surjective⟩

end A022825Lean
