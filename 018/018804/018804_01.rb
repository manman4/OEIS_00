def A(n)
  (1..n).map{|i| (1..i).inject(0){|s, j| s + i.gcd(j)}}
end

n = 60
ary = A(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
