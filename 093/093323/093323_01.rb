# a(n,k) = s_1 = k で、すべての j について s_j | (s_1 + ... + s_j) を満たす
#          1..n の順列の個数
def a(n, k)
  return 0 unless k.between?(1, n)
  full = (1 << n) - 1
  memo = {}

  count = lambda do |mask, sum|
    return 1 if mask == full
    cached = memo[mask]
    return cached if cached

    total = 0
    1.upto(n) do |x|
      bit = 1 << (x - 1)
      next if mask & bit != 0
      next unless (sum + x) % x == 0   # s_j が部分和を割り切るか
      total += count.call(mask | bit, sum + x)
    end
    memo[mask] = total
  end

  count.call(1 << (k - 1), k)   # j=1 は s_1 | s_1 で常に成立
end

1.upto(26) do |n|
  row = (1..n).map { |k| a(n, k) }
  puts "n=#{n}: #{row.join(' ')}"
end
