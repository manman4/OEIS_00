import A022825Lean.Recurrence

namespace A022825Lean

/-! # The A022825 remainder-permutation theorem -/

/-- The number of permutations `sigma` of `[1, ..., n]` whose adjacent
remainders are strictly increasing is `a022825 (n + 1)`.

Here `a022825` is the divisor-chain counting function; `a022825_one` and
`a022825_recurrence` prove that it has the initial value and recurrence that
define OEIS A022825. -/
theorem remainderPermutation_card_eq_a022825 (n : ℕ) :
    Fintype.card (RemainderPermutation n) = a022825 (n + 1) := by
  exact (Fintype.card_congr
    (divisorChainEquivRemainderPermutation n)).symm

end A022825Lean
