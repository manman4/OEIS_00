def A028940(n)
  ary = [0]
  a, b = 1r, 0r
  (n - 1).times{
    ary << a.numerator
    c = (b * b - a * a * a) / (a * a)
    d = -1 - b * c / a
    a, b = c, d
  }
  ary
end

def A028944(n)
  ary = A028940(2 * n + 1)
  (0..n).map{|i| ary[2 * i]}
end

n = 150
ary = A028944(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
