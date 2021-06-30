def s(k, n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % k == 0}
  s
end

def A(k, n)
  (1..n).map{|i| (1..i).inject(0){|s, j| s + j ** k * s(j, i)}}
end

n = 45
ary = A(2, n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
