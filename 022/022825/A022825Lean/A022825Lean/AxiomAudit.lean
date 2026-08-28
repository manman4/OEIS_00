import A022825Lean.Main

/-!
This module contains `#print axioms` commands for the public declarations.
It is intentionally imported only during the final audit.
-/

namespace A022825Lean

#print axioms IsValidSuffix.exists_blocks
#print axioms blocksFrom_injective
#print axioms RemainderPermutation.exists_divisorChain
#print axioms DivisorChain.toRemainderPermutation_injective
#print axioms DivisorChain.toRemainderPermutation_surjective
#print axioms divisorChainEquivRemainderPermutation
#print axioms chainStepEquivChainList
#print axioms a022825_one
#print axioms a022825_recurrence
#print axioms eq_a022825_of_one_recurrence
#print axioms remainderPermutation_card_eq_a022825

end A022825Lean
