def s(k, n)
  m = 2 * k + 3
  k1 = k + 1
  s = 0
  (1..n).each{|i|
    j = i % m
    s += i if (n % i == 0) && (j != 0) && (j != k1) && (j != (m - k1))}
  s
end

def A(k, n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(k, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s + a[j] * ary[-j]} / i}
  ary
end

n = 47
ary = A(5, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
