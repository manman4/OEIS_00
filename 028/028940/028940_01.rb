def A028940(n)
  ary = [0]
  a, b = 1r, 0r
  (2..n).each{|i|
    ary << a.numerator
    c = (b * b - a * a * a) / (a * a)
    d = -1 - b * c / a
    a, b = c, d
  }
  ary
end

n = 300
ary = A028940(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
