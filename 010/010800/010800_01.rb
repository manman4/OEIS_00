def f(n)
  return 1 if n == 0
  (1..n).inject(:*)
end

def A(n)
  m = f(2) * f(3) * f(4) * f(5) * f(6)
  return f(n) * f(n + 1) * f(n + 2) * f(n + 3) * f(n + 4) * f(n + 5) / m
end

n = 1000
(1..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}