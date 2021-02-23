def A(n)
  a = 1r
  ary = [1]
  (2..n).each{|i|
    a += 1 / a
    ary << a
  }
  ary
end

n = 20
ary = A(n)
(1..n).each{|i|
  j = ary[i - 1].denominator
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
