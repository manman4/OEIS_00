def phi(n)
  s = 0
  (1..n).each{|i| s += 1 if i.gcd(n) == 1}
  s
end

def A(n)
  a = [0] + (1..n).map{|i| phi(i)}
  (1..n).map{|i| (1..i).inject(0){|s, j| s + j * a[j]}}
end

n = 10000
ary = A(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
