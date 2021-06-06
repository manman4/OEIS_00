def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def g(k, m)
  f(k * m) / (f(k) ** m * f(m))
end

n = 50
ary = (0..n).map{|i| g(i, i)}
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
