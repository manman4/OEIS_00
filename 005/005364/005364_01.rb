def f(k, n)
  return 1 if k == 0
  (n + 1..n + k).inject(:*)
end

def S(k, n)
  # (1..n).inject(1){|s, i| s + (0..i - 1).inject(1){|t, j| t * ncr(n + k - 1, k + j) / ncr(n + k - 1, j).to_r}}.to_i
  (1..n).inject(1){|s, i| s + (0..i - 1).inject(1){|t, j| t * f(k, n - 1 - j) / f(k, j).to_r}}.to_i
end

n = 1000
(0..n).each{|i|
  j = S(6, i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
