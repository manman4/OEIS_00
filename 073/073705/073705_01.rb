def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def s(k, n)
  s = 0
  (1..n).each{|i| s += i ** (k * n / i) if n % i == 0}
  s
end

n = 38
(1..n).each{|i|
  j = s(2, i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
