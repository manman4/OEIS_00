def A(k, n)
(0..n).map{|i| 
  i.to_s(k).split('').sort.join.to_i(k)
}
end

n = 8191
ary = A(2, n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
