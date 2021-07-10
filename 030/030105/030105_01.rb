def A(k, n)
  (0..n).map{|i| i.to_s(k).split('').reverse.join.to_i(k)}
end

n = 6 ** 5 - 1
ary = A(6, n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
