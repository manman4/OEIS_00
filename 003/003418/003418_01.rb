def A003418(n)
  s = 1
  ary = [1]
  (1..n).each{|i|
    s = (s * i) / s.gcd(i)
    ary << s
  }
  ary
end

n = 2500
ary = A003418(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
