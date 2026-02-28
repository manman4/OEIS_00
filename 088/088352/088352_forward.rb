require_relative '088352_series'

class GeneralizedContinuedFraction
  # --- 前向き法: 漸化式 Pk = bk*P(k-1) + ak*P(k-2) で分子・分母を構築 ---
  #   curr = P(k-1), prev = P(k-2)
  def self.compute_forward(max_deg, depth, a_gen, b_gen)
    prev_p, curr_p = [0], a_gen.call(1)
    prev_q, curr_q = [1], b_gen.call(1)

    (2..depth).each{|k|
      ak = a_gen.call(k)
      bk = b_gen.call(k)

      next_p = Series.add(
        Series.poly_mul(bk, curr_p, max_deg),
        Series.poly_mul(ak, prev_p, max_deg),
        max_deg
      )
      next_q = Series.add(
        Series.poly_mul(bk, curr_q, max_deg),
        Series.poly_mul(ak, prev_q, max_deg),
        max_deg
      )

      prev_p, curr_p = curr_p, next_p
      prev_q, curr_q = curr_q, next_q
    }

    Series.div(curr_p, curr_q, max_deg).map(&:to_i)
  end
end

if __FILE__ == $0
  # --- 問題設定 ---
  # A(x) = 1/(1-x - x^2/(1-x^3 - x^4/(1-x^5 - x^6/(1-x^7 - x^8/(...)))))
  # 標準形: a_1/(b_1 + a_2/(b_2 + a_3/(b_3 + ...)))
  #   a_1 = 1,  a_n = -x^(2n-2)  (n >= 2)
  #   b_n = 1 - x^(2n-1)
  MAX_DEG = 1000 # 展開する次数
  DEPTH   = 1005 # 連分数の深さ

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

  result = GeneralizedContinuedFraction.compute_forward(MAX_DEG, DEPTH, a_rule, b_rule)
  result.each_with_index{|v, i| break if v.to_s.size > 1000; puts "#{i} #{v}"}
end
