def A(a0, a1, n)
  a, b = a0, a1
  ary = [a0]
  n.times{
    a, b = b, a + b
    ary << a
  }
  ary
end

n = 10000
ary = A(20615674205555510, 3794765361567513, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts ary[i]
}
