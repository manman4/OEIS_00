def A028942(n)
  ary = [0]
  a, b = 1r, 0r
  (2..n).each{|i|
    ary << -b.numerator
    c = (b * b - a * a * a) / (a * a)
    d = -1 - b * c / a
    a, b = c, d
  }
  ary
end

def A028938(n)
  ary = A028942(2 * n)
  (1..n).map{|i| ary[2 * i - 1]}
end

n = 100
ary = A028938(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
