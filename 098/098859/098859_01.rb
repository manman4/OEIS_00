class WilfCounter
  def initialize
    # memo[used_mask][p][n] = ways
    @memo = {}
    @result_memo = { 0 => 1 }
  end

  def count(target_n)
    cached = @result_memo[target_n]
    return cached unless cached.nil?

    v = solve(target_n, 1, 0)
    @result_memo[target_n] = v
    v
  end

  private

  def solve(n, p, used_mask)
    return 1 if n == 0
    return 0 if p > n

    by_mask = (@memo[used_mask] ||= {})
    by_p = (by_mask[p] ||= {})
    cached = by_p[n]
    return cached unless cached.nil?

    # 1. パーツ p を使わない場合
    res = solve(n, p + 1, used_mask)

    # 2. パーツ p を m 回使う場合
    m = 1
    max_m = n / p
    while m <= max_m
      bit = (1 << m)
      if (used_mask & bit) == 0
        res += solve(n - p * m, p + 1, used_mask | bit)
      end
      m += 1
    end

    by_p[n] = res
  end
end

n = 100
digit_limit = 1000

counter = WilfCounter.new
(0..n).each{|i|
  j = counter.count(i)
  break if j.to_s.size > digit_limit
  print i
  print ' '
  puts j
}
