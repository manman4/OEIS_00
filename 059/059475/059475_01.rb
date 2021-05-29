def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def A(n)
  s = 1
  (0..n - 1).each{|i|
    s *= f(3 * i)
    s *= f(3 * i + 2)
  }
  (0..n - 1).each{|i|
    s /= f(i + n) ** 2
  }
  s
end

n = 100
ary = (0..n).map{|i| A(i)}
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
