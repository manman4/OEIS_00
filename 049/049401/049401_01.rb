def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

# a(n) = 6 * n! * Sum_{k=0..floor(n/2)} (2*k+2)!/((n-2*k)!*k!*(k+1)!*(k+2)!*(k+3)!).
def a(n)
  (6 * f(n) * (0..n / 2).inject(0){|s, k| s + f(2 * k + 2) / (f(n - 2 * k) * f(k) * f(k + 1) * f(k + 2) * f(k + 3).to_r)}).to_i
end

n = 1000
(0..n).each{|i|
  j = a(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}