def A(n)
  s = 1
  ary = [1]
  (1..n).each{|i|
    s *= i
    ary << s.to_s.count("1")
  }
  ary
end

n = 10100
m = 10000
ary = A(n)
(0..m).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}