def A002884(n)
  (0..n).map{|i| (0..i - 1).inject(1){|s, j| s * (2 ** i - 2 ** j)}}
end

n = 11
ary = A002884(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
