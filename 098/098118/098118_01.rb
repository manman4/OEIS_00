def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

n = 1000

s = 0
(1..n).each{|i|
  s += 1r / (2 * i - 1) + 1r / (2 * i)
  s -= 1r / i
  j = (s * f(2 * i) / f(i)).to_i
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
