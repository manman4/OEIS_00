# ary[0] = 1
def sqrt_a(ary)
  n = ary.size - 1
  a = [1]
  (0..n - 1).each{|i| a << (ary[i + 1] - (1..i).inject(0){|s, j| s + a[j] * a[-j]}) / 2}
  a
end

def A(n)
  a, b = 1, 8
  cnt = 1
  ary = [1]
  while cnt <= n
    cnt += 1
    a, b = b, (8 * b * (3 * cnt * cnt - 3 * cnt + 1) - 128 * a * (cnt - 1) ** 2) / (cnt * cnt)
    ary << a
  end
  ary
end

n = 900
ary = sqrt_a(A(n))
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
