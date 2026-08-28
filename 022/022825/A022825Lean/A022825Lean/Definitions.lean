import Mathlib.Data.List.Pairwise
import Mathlib.Data.List.Permutation
import Mathlib.Data.List.Range
import Mathlib.Data.Fintype.List
import Mathlib.Data.Fintype.EquivFin
import Mathlib.Data.Fintype.Powerset
import Mathlib.Data.Finset.Sort

/-!
# Definitions for the A022825 remainder-permutation theorem

All permutation values are represented by positive natural numbers.  A
`RemainderPermutation n` contains a list which is a permutation of
`[1, 2, ..., n]` and whose adjacent remainders are strictly increasing.
-/

namespace A022825Lean

/-- The list `[1, 2, ..., n]`. -/
def oneTo (n : ℕ) : List ℕ := List.range' 1 n

/-- Remainders of adjacent entries of a list. -/
def remainders : List ℕ → List ℕ
  | x :: y :: xs => x % y :: remainders (y :: xs)
  | _ => []

/-- A permutation of `[1, ..., n]` with strictly increasing remainders. -/
def IsRemainderPermutation (n : ℕ) (xs : List ℕ) : Prop :=
  xs.Perm (oneTo n) ∧ (remainders xs).Pairwise (· < ·)

/-- The finite type counted on the permutation side of the theorem. -/
def RemainderPermutation (n : ℕ) :=
  {i : Fin (oneTo n).permutations.length //
    (remainders ((oneTo n).permutations.get i)).Pairwise (· < ·)}

noncomputable instance (n : ℕ) : Fintype (RemainderPermutation n) := by
  classical exact Subtype.fintype _

/-- The list represented by a member of `RemainderPermutation n`. -/
def RemainderPermutation.values {n : ℕ} (p : RemainderPermutation n) : List ℕ :=
  (oneTo n).permutations.get p.1

/-- A finite set which, in increasing order, is a strict divisor chain. -/
def IsDivisorChain (N : ℕ) (s : Finset (Fin (N + 1))) : Prop :=
  (∀ d ∈ s, 3 ≤ d.1) ∧
  ∀ a ∈ s, ∀ b ∈ s, a.1 < b.1 → a.1 ∣ b.1

/-- The finite type of strict divisor chains bounded by `N`. -/
def DivisorChain (N : ℕ) :=
  {s : Finset (Fin (N + 1)) // IsDivisorChain N s}

noncomputable instance (N : ℕ) : Fintype (DivisorChain N) := by
  classical exact Subtype.fintype _

/-- The endpoint list represented by a member of `DivisorChain N`. -/
def DivisorChain.values {N : ℕ} (c : DivisorChain N) : List ℕ :=
  (c.1.sort (· ≤ ·)).map Fin.val

end A022825Lean
