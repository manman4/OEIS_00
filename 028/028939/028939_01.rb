def A028943(n)
  ary = [1]
  a, b = 1r, 0r
  (2..n).each{|i|
    ary << b.denominator
    c = (b * b - a * a * a) / (a * a)
    d = -1 - b * c / a
    a, b = c, d
  }
  ary
end

def A028939(n)
  ary = A028943(2 * n)
  (1..n).map{|i| ary[2 * i - 1]}
end

n = 100
ary = A028939(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
