def f(k, n)
  return 1 if k == 0
  (n + 1..n + k).inject(:*)
end

def T(n, k)
  (0..k - 1).inject(1){|s, i| s * f(n, i + n) / f(n, i).to_r}.to_i
end

n = 100
(0..n).each{|i|
  j = T(i + 1, i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
