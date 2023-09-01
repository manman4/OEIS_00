def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def A(k, n)
  f((k + 1) * n) / f(k * n + 1)
end

n = 1000
(0..n).each{|i|
  j = A(5, i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}