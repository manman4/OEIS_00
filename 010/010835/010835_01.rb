def s(n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0}
  s
end

def A(m, n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - m * a[j] * ary[-j]} / i}
  ary
end

n = 10100
m = 10000
ary = A(30, n)
(0..m).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}