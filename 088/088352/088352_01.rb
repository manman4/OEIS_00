require_relative '088352_forward'
require_relative '088352_backward'

# --- 問題設定 ---
# A(x) = 1/(1-x - x^2/(1-x^3 - x^4/(1-x^5 - x^6/(1-x^7 - x^8/(...)))))
# 標準形: a_1/(b_1 + a_2/(b_2 + a_3/(b_3 + ...)))
#   a_1 = 1,  a_n = -x^(2n-2)  (n >= 2)
#   b_n = 1 - x^(2n-1)
MAX_DEG = 20 # 展開する次数
DEPTH   = 25 # 連分数の深さ

a_rule = ->(n) {
  if n == 1
    [1]
  else
    coeff = Array.new(2 * n - 1, 0)
    coeff[2 * n - 2] = -1
    coeff
  end
}

b_rule = ->(n) {
  coeff = Array.new(2 * n, 0)
  coeff[0] = 1
  coeff[2 * n - 1] = -1
  coeff
}

# --- 実行 ---
puts "【前向き法】の結果:"
p GeneralizedContinuedFraction.compute_forward(MAX_DEG, DEPTH, a_rule, b_rule)

puts "\n【後ろ向き法】の結果:"
p GeneralizedContinuedFraction.compute_backward(MAX_DEG, DEPTH, a_rule, b_rule)
