def A(k, n)
  (0..n).map{|i| i.to_s(k).split('').reverse.join.to_i(k)}
end

n = 9 ** 4 - 1
ary = A(9, n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
