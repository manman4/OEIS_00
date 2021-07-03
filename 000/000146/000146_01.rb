require 'prime'

def c(n, r)
  return 1 if r == 0
  return c(n, n - r) if r > n - r
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# 0, 2, 4�Ԗ�
def b2(n)
  a = [1r]
  (1..n).each{|i| a << 1r / 2 - (0..i - 1).inject(0){|s, j| s + c(2 * i + 1, 2 * j) * a[j]} / (2 * i + 1)}
  a
end

def A000146(n)
  ary = []
  p_ary = Prime.each(2 * n + 1).to_a
  b_ary = b2(n)
  2.step(2 * n, 2){|i|
    j = b_ary[i / 2]
    p_ary.each{|k|
      break if k - 1 > i
      j += 1r / k if i % (k - 1) == 0
    }
    ary << j.numerator
  }
  ary
end

n = 24
ary = A000146(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
