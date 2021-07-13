def A(q, n)
  ary = [1]
  (1..n).each{|i| ary << (0..i - 1).inject(0){|s, j| s + q ** j * ary[j] * ary[i - 1 - j]}}
  ary
end
def A015086(n)
  A(5, n)
end

n = 10
ary = A015086(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
