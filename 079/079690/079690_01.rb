def A(k, n)
  m = k.to_s
  s = 1
  ary = [0]
  (1..n).each{|i|
    s *= i
    ary << s.to_s.count(m)
  }
  ary
end

n = 10100
m = 10000
ary = A(5, n)
(0..m).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}