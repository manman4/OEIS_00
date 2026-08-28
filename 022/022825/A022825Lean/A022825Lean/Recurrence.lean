import A022825Lean.DivisorChains
import Mathlib.Data.Fintype.BigOperators
import Mathlib.Order.Interval.Finset.Nat

namespace A022825Lean

/-! Counting divisor chains and identifying the A022825 recurrence. -/

/-- A list presentation of a divisor chain.  This is convenient for the
last-element decomposition used in the recurrence proof. -/
def IsChainList (N : ℕ) (ds : List ℕ) : Prop :=
  (∀ d ∈ ds, 3 ≤ d ∧ d ≤ N) ∧
  ds.Pairwise (· < ·) ∧ ds.Pairwise (· ∣ ·)

def ChainList (N : ℕ) := {ds : List ℕ // IsChainList N ds}

namespace ChainList

theorem lower {N : ℕ} (c : ChainList N) {d : ℕ} (hd : d ∈ c.1) : 3 ≤ d :=
  (c.2.1 d hd).1

theorem upper {N : ℕ} (c : ChainList N) {d : ℕ} (hd : d ∈ c.1) : d ≤ N :=
  (c.2.1 d hd).2

theorem pairwise_lt {N : ℕ} (c : ChainList N) : c.1.Pairwise (· < ·) :=
  c.2.2.1

theorem pairwise_dvd {N : ℕ} (c : ChainList N) : c.1.Pairwise (· ∣ ·) :=
  c.2.2.2

end ChainList

/-- Sorted lists and finite-set divisor chains carry exactly the same data. -/
noncomputable def chainListEquivDivisorChain (N : ℕ) :
    ChainList N ≃ DivisorChain N where
  toFun c := divisorChainOfList c.1
    (fun d hd => c.lower hd) c.pairwise_lt c.pairwise_dvd
    (fun d hd => c.upper hd)
  invFun c := ⟨c.values, fun d hd => c.value_bounds hd,
    c.values_pairwise_lt, c.values_pairwise_dvd⟩
  left_inv c := by
    apply Subtype.ext
    exact values_divisorChainOfList c.1
      (fun d hd => c.lower hd) c.pairwise_lt c.pairwise_dvd
      (fun d hd => c.upper hd)
  right_inv c := by
    apply DivisorChain.ext_values
    exact values_divisorChainOfList c.values
      (fun d hd => (c.value_bounds hd).1) c.values_pairwise_lt
      c.values_pairwise_dvd (fun d hd => (c.value_bounds hd).2)

noncomputable instance (N : ℕ) : Fintype (ChainList N) :=
  Fintype.ofEquiv (DivisorChain N) (chainListEquivDivisorChain N).symm

theorem card_chainList_eq_card_divisorChain (N : ℕ) :
    Fintype.card (ChainList N) = Fintype.card (DivisorChain N) :=
  Fintype.card_congr (chainListEquivDivisorChain N)

/-- Multipliers occurring in the A022825 recurrence: `2 ≤ j ≤ N`. -/
def Multiplier (N : ℕ) := {j : Fin (N + 1) // 2 ≤ j.1}

instance (N : ℕ) : Fintype (Multiplier N) := Subtype.fintype _

/-- The disjoint union counted by `sum_{j=2..N} C(floor(N/j))`. -/
def ChainStep (N : ℕ) := Σ j : Multiplier N, ChainList (N / j.1)

noncomputable instance (N : ℕ) : Fintype (ChainStep N) := by
  unfold ChainStep
  infer_instance

namespace ChainList

theorem le_getLast {N : ℕ} (c : ChainList N) (hc : c.1 ≠ [])
    {a : ℕ} (ha : a ∈ c.1) : a ≤ c.1.getLast hc := by
  exact (c.pairwise_lt.imp fun hab => hab.le).rel_getLast ha

theorem dvd_getLast {N : ℕ} (c : ChainList N) (hc : c.1 ≠ [])
    {a : ℕ} (ha : a ∈ c.1) : a ∣ c.1.getLast hc := by
  exact c.pairwise_dvd.rel_getLast ha

end ChainList

/-- Append `j` times the old maximum.  The two empty-list cases encode the
empty chain (`j=2`) and singleton chains (`j≥3`). -/
noncomputable def extendChain {N : ℕ} (x : ChainStep N) : ChainList N := by
  rcases x with ⟨j, c⟩
  have hj2 : 2 ≤ j.1.1 := j.2
  have hjN : j.1.1 ≤ N := Nat.le_of_lt_succ j.1.2
  by_cases hc : c.1 = []
  · by_cases hj : j.1.1 = 2
    · exact ⟨[], by simp [IsChainList]⟩
    · refine ⟨[j.1], ?_⟩
      refine ⟨?_, by simp, by simp⟩
      intro d hd
      simp only [List.mem_singleton] at hd
      subst d
      exact ⟨by omega, hjN⟩
  · let d := c.1.getLast hc
    refine ⟨c.1 ++ [j.1 * d], ?_⟩
    have hdmem : d ∈ c.1 := List.getLast_mem hc
    have hd3 : 3 ≤ d := c.lower hdmem
    have hdupper : d ≤ N / j.1 := c.upper hdmem
    have hjpos : 0 < j.1.1 := lt_of_lt_of_le (by omega) hj2
    have hjdmul : j.1 * d ≤ N := by
      have := (Nat.le_div_iff_mul_le hjpos).mp hdupper
      simpa [Nat.mul_comm] using this
    refine ⟨?_, ?_, ?_⟩
    · intro a ha
      rw [List.mem_append, List.mem_singleton] at ha
      rcases ha with ha | rfl
      · exact ⟨c.lower ha, (c.upper ha).trans (Nat.div_le_self N j.1)⟩
      · have hdle : d ≤ j.1 * d := by
          calc
            d = 1 * d := by simp
            _ ≤ j.1 * d := Nat.mul_le_mul_right d (by omega)
        exact ⟨hd3.trans hdle, hjdmul⟩
    · rw [List.pairwise_append]
      refine ⟨c.pairwise_lt, by simp, ?_⟩
      intro a ha b hb
      simp only [List.mem_singleton] at hb
      subst b
      have had := c.le_getLast hc ha
      have hdlt : d < j.1 * d := by
        calc
          d < 2 * d := by omega
          _ ≤ j.1 * d := Nat.mul_le_mul_right d hj2
      exact lt_of_le_of_lt had hdlt
    · rw [List.pairwise_append]
      refine ⟨c.pairwise_dvd, by simp, ?_⟩
      intro a ha b hb
      simp only [List.mem_singleton] at hb
      subst b
      exact (c.dvd_getLast hc ha).trans (dvd_mul_left d j.1)

/-- Every chain has a preimage under `extendChain`. -/
theorem extendChain_surjective {N : ℕ} (hN : 1 < N) :
    Function.Surjective (extendChain : ChainStep N → ChainList N) := by
  intro c
  classical
  cases hds : c.1 with
  | nil =>
      let jf : Fin (N + 1) := ⟨2, by omega⟩
      let j : Multiplier N := ⟨jf, by change 2 ≤ (2 : ℕ); omega⟩
      let small : ChainList (N / 2) := ⟨[], by simp [IsChainList]⟩
      refine ⟨⟨j, small⟩, ?_⟩
      apply Subtype.ext
      simp [extendChain, j, jf, small, hds]
  | cons a tail =>
      cases htail : tail with
      | nil =>
          have hamem : a ∈ c.1 := by simp [hds]
          have ha3 := c.lower hamem
          have haN := c.upper hamem
          let jf : Fin (N + 1) := ⟨a, by omega⟩
          let j : Multiplier N := ⟨jf, by change 2 ≤ a; omega⟩
          let small : ChainList (N / a) := ⟨[], by simp [IsChainList]⟩
          refine ⟨⟨j, small⟩, ?_⟩
          apply Subtype.ext
          have hane : a ≠ 2 := by omega
          simp [extendChain, j, jf, small, hds, htail, hane]
      | cons b rest =>
          let pre := c.1.dropLast
          have hpre : pre ≠ [] := by simp [pre, hds, htail]
          let d := pre.getLast hpre
          have hdpre : d ∈ pre := List.getLast_mem hpre
          have hdc : d ∈ c.1 := List.dropLast_subset c.1 hdpre
          let m := c.1.getLast (by simp [hds])
          have hmc : m ∈ c.1 := List.getLast_mem (by simp [hds])
          have hmN : m ≤ N := c.upper hmc
          have hdvdm : d ∣ m := c.pairwise_dvd.rel_dropLast_getLast hdpre
          have hdltm : d < m := c.pairwise_lt.rel_dropLast_getLast hdpre
          let q := m / d
          have hqmul : q * d = m := by
            dsimp [q]
            exact Nat.div_mul_cancel hdvdm
          have hqzero : q ≠ 0 := by
            intro hq
            rw [hq] at hqmul
            simp at hqmul
            have := c.lower hmc
            omega
          have hqone : q ≠ 1 := by
            intro hq
            rw [hq] at hqmul
            simp at hqmul
            omega
          have hq2 : 2 ≤ q := (Nat.two_le_iff q).mpr ⟨hqzero, hqone⟩
          have hqN : q ≤ N := (Nat.div_le_self m d).trans hmN
          let jf : Fin (N + 1) := ⟨q, by omega⟩
          let j : Multiplier N := ⟨jf, hq2⟩
          have hpreLt : pre.Pairwise (· < ·) :=
            List.Pairwise.sublist (List.dropLast_sublist c.1) c.pairwise_lt
          have hpreDvd : pre.Pairwise (· ∣ ·) :=
            List.Pairwise.sublist (List.dropLast_sublist c.1) c.pairwise_dvd
          have hpreBound : ∀ x ∈ pre, x ≤ N / q := by
            intro x hx
            have hxd : x ≤ d :=
              (hpreLt.imp fun hxy => hxy.le).rel_getLast hx
            apply hxd.trans
            apply (Nat.le_div_iff_mul_le (by omega : 0 < q)).mpr
            calc
              d * q = q * d := Nat.mul_comm d q
              _ = m := hqmul
              _ ≤ N := hmN
          let small : ChainList (N / q) :=
            ⟨pre, ⟨fun x hx => ⟨c.lower (List.dropLast_subset c.1 hx),
                hpreBound x hx⟩, hpreLt, hpreDvd⟩⟩
          refine ⟨⟨j, small⟩, ?_⟩
          apply Subtype.ext
          have hrestore := List.dropLast_concat_getLast (l := c.1) (by simp [hds])
          simp [extendChain, j, jf, small, hpre]
          simpa [pre, d, m, hqmul] using hrestore

theorem extendChain_injective {N : ℕ} :
    Function.Injective (extendChain : ChainStep N → ChainList N) := by
  rintro ⟨j, c⟩ ⟨k, t⟩ heq
  have hval := congrArg (fun z : ChainList N => z.1) heq
  by_cases hc : c.1 = []
  · by_cases ht : t.1 = []
    · by_cases hj : j.1.1 = 2
      · by_cases hk : k.1.1 = 2
        · have hjkNat : j.1.1 = k.1.1 := by omega
          have hjkFin : j.1 = k.1 := Fin.ext hjkNat
          have hjk : j = k := Subtype.ext hjkFin
          subst k
          have hct : c = t := Subtype.ext (hc.trans ht.symm)
          subst t
          rfl
        · simp [extendChain, hc, ht, hj, hk] at hval
      · by_cases hk : k.1.1 = 2
        · simp [extendChain, hc, ht, hj, hk] at hval
        · have hjkNat : j.1.1 = k.1.1 := by
            simpa [extendChain, hc, ht, hj, hk] using hval
          have hjkFin : j.1 = k.1 := Fin.ext hjkNat
          have hjk : j = k := Subtype.ext hjkFin
          subst k
          have hct : c = t := Subtype.ext (hc.trans ht.symm)
          subst t
          rfl
    · by_cases hj : j.1.1 = 2
      · simp [extendChain, hc, ht, hj] at hval
      · have hlen := congrArg List.length hval
        have htpos : 0 < t.1.length := Nat.pos_of_ne_zero fun hz =>
          ht (List.length_eq_zero_iff.mp hz)
        simp [extendChain, hc, ht, hj] at hlen
  · by_cases ht : t.1 = []
    · by_cases hk : k.1.1 = 2
      · simp [extendChain, hc, ht, hk] at hval
      · have hlen := congrArg List.length hval
        have hcpos : 0 < c.1.length := Nat.pos_of_ne_zero fun hz =>
          hc (List.length_eq_zero_iff.mp hz)
        simp [extendChain, hc, ht, hk] at hlen
    · let d := c.1.getLast hc
      let e := t.1.getLast ht
      have happend : c.1 ++ [j.1.1 * d] = t.1 ++ [k.1.1 * e] := by
        simpa [extendChain, hc, ht, d, e] using hval
      have hpreVal : c.1 = t.1 := by
        have := congrArg List.dropLast happend
        simpa using this
      have hde : d = e := by
        have hlast := congrArg List.getLast? hpreVal
        rw [List.getLast?_eq_getLast_of_ne_nil hc,
          List.getLast?_eq_getLast_of_ne_nil ht] at hlast
        exact Option.some.inj hlast
      have hprod : j.1.1 * d = k.1.1 * e := by
        rw [hpreVal] at happend
        have hsingle := List.append_right_injective t.1 happend
        simpa using hsingle
      rw [hde] at hprod
      have hepos : 0 < e := by
        exact lt_of_lt_of_le (by omega)
          (t.lower (List.getLast_mem ht))
      have hjkNat : j.1.1 = k.1.1 := Nat.mul_right_cancel hepos hprod
      have hjkFin : j.1 = k.1 := Fin.ext hjkNat
      have hjk : j = k := Subtype.ext hjkFin
      subst k
      have hct : c = t := Subtype.ext hpreVal
      subst t
      rfl

noncomputable def chainStepEquivChainList (N : ℕ) (hN : 1 < N) :
    ChainStep N ≃ ChainList N :=
  Equiv.ofBijective extendChain
    ⟨extendChain_injective, extendChain_surjective hN⟩

/-- The sequence counted by divisor chains. -/
noncomputable def a022825 (N : ℕ) : ℕ := Fintype.card (DivisorChain N)

@[simp] theorem a022825_one : a022825 1 = 1 := by
  rw [a022825, ← card_chainList_eq_card_divisorChain]
  apply Fintype.card_eq_one_iff.mpr
  let empty : ChainList 1 := ⟨[], by simp [IsChainList]⟩
  refine ⟨empty, ?_⟩
  intro c
  apply Subtype.ext
  cases hds : c.1 with
  | nil => simp [empty]
  | cons d ds =>
      have hd : d ∈ c.1 := by simp [hds]
      have hd3 := c.lower hd
      have hd1 := c.upper hd
      omega

/-- The recurrence first in its canonical finite-type form. -/
theorem a022825_recurrence_subtype (N : ℕ) (hN : 1 < N) :
    a022825 N = ∑ j : Multiplier N, a022825 (N / j.1.1) := by
  calc
    a022825 N = Fintype.card (ChainList N) := by
      symm
      exact card_chainList_eq_card_divisorChain N
    _ = Fintype.card (ChainStep N) :=
      (Fintype.card_congr (chainStepEquivChainList N hN)).symm
    _ = ∑ j : Multiplier N, Fintype.card (ChainList (N / j.1.1)) :=
      Fintype.card_sigma
    _ = ∑ j : Multiplier N, a022825 (N / j.1.1) := by
      apply Finset.sum_congr rfl
      intro j hj
      exact card_chainList_eq_card_divisorChain (N / j.1.1)

def multiplierEquivIcc (N : ℕ) :
    Multiplier N ≃ {j : ℕ // j ∈ Finset.Icc 2 N} where
  toFun j := ⟨j.1.1, Finset.mem_Icc.mpr
    ⟨j.2, Nat.le_of_lt_succ j.1.2⟩⟩
  invFun j := ⟨⟨j.1, Nat.lt_succ_of_le (Finset.mem_Icc.mp j.2).2⟩,
    (Finset.mem_Icc.mp j.2).1⟩
  left_inv j := by
    apply Subtype.ext
    apply Fin.ext
    rfl
  right_inv j := by
    apply Subtype.ext
    rfl

theorem sum_multiplier_eq_Icc (N : ℕ) (f : ℕ → ℕ) :
    (∑ j : Multiplier N, f j.1.1) = ∑ j ∈ Finset.Icc 2 N, f j := by
  classical
  calc
    (∑ j : Multiplier N, f j.1.1) =
        ∑ j : {j : ℕ // j ∈ Finset.Icc 2 N}, f j.1 := by
      apply Fintype.sum_equiv (multiplierEquivIcc N)
      intro j
      rfl
    _ = ∑ j ∈ Finset.Icc 2 N, f j := by
      simpa only [Finset.univ_eq_attach] using
        (Finset.sum_attach (Finset.Icc 2 N) f)

/-- The defining recurrence of OEIS A022825. -/
theorem a022825_recurrence (N : ℕ) (hN : 1 < N) :
    a022825 N = ∑ j ∈ Finset.Icc 2 N, a022825 (N / j) := by
  rw [a022825_recurrence_subtype N hN]
  exact sum_multiplier_eq_Icc N (fun j => a022825 (N / j))

/-- The initial value and recurrence uniquely determine `a022825` on the
positive natural numbers.  This makes the identification with the recursively
defined OEIS sequence independent of the divisor-chain implementation. -/
theorem eq_a022825_of_one_recurrence (b : ℕ → ℕ)
    (hb_one : b 1 = 1)
    (hb_rec : ∀ N, 1 < N →
      b N = ∑ j ∈ Finset.Icc 2 N, b (N / j)) :
    ∀ N, 0 < N → b N = a022825 N := by
  intro N
  induction N using Nat.strong_induction_on with
  | h N ih =>
      intro hNpos
      by_cases hN_one : N = 1
      · subst N
        simpa using hb_one
      · have hN : 1 < N := by omega
        rw [hb_rec N hN, a022825_recurrence N hN]
        apply Finset.sum_congr rfl
        intro j hj
        have hj_bounds := Finset.mem_Icc.mp hj
        apply ih (N / j)
        · exact Nat.div_lt_self hNpos (by omega)
        · exact Nat.div_pos hj_bounds.2 (by omega)

end A022825Lean
