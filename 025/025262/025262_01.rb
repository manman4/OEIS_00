def A(k, n)
  ary = Array.new(k, 1)
  (k + 1..n).each{|i| ary << (1..i - 1).inject(0){|s, j| s + ary[j - 1] * ary[i - j - 1]}}
  ary[0..n - 1]
end

n = 20
ary = A(3, n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
