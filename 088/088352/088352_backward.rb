require_relative '088352_series'

class GeneralizedContinuedFraction
  # --- 後ろ向き法: 末尾から折りたたんで連分数を評価 ---
  # tail を p/q として保持し、各ステップは poly_mul のみ使用。
  # div は最後に1回だけ呼ぶことで O(depth*max_deg²) → O(depth*max_deg) に改善。
  def self.compute_backward(max_deg, depth, a_gen, b_gen)
    # 初期値: tail = 0 = 0/1
    tail_p = [0]
    tail_q = [1]

    depth.downto(2){|k|
      ak = a_gen.call(k)
      bk = b_gen.call(k)
      # new_tail = ak / (bk + tail_p/tail_q)
      #          = (ak * tail_q) / (bk * tail_q + tail_p)
      new_p = Series.poly_mul(ak, tail_q, max_deg)
      new_q = Series.add(Series.poly_mul(bk, tail_q, max_deg), tail_p, max_deg)
      tail_p, tail_q = new_p, new_q
    }

    a1 = a_gen.call(1)
    b1 = b_gen.call(1)
    # result = a1 / (b1 + tail_p/tail_q) = (a1 * tail_q) / (b1 * tail_q + tail_p)
    final_p = Series.poly_mul(a1, tail_q, max_deg)
    final_q = Series.add(Series.poly_mul(b1, tail_q, max_deg), tail_p, max_deg)
    Series.div(final_p, final_q, max_deg).map(&:to_i)
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

  result = GeneralizedContinuedFraction.compute_backward(MAX_DEG, DEPTH, a_rule, b_rule)
  result.each_with_index{|v, i| break if v.to_s.size > 1000; puts "#{i} #{v}"}
end
