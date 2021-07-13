def s(k, n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % k == 0}
  s
end

def A(ary, n)
  a_ary = [1]
  a = [0] + (1..n).map{|i| ary.inject(0){|s, j| s + j[1] * s(j[0], i)}}
  (1..n).each{|i| a_ary << (1..i).inject(0){|s, j| s - a[j] * a_ary[-j]} / i}
  a_ary
end

def B(k, n)
  A([[2, 5 * k], [1, -2 * k], [4, -2 * k]], n)
end

def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

# ary[0] = 1
def sqrt_a(ary)
  n = ary.size - 1
  a = [1]
  (0..n - 1).each{|i| a << (ary[i + 1] - (1..i).inject(0){|s, j| s + a[j] * a[-j]}) / 2r}
  (0..n).map{|i| (f(i) * a[i]).to_i}
end

n = 500
ary = sqrt_a(B(-21, n))
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
