def A(k, m, n)
  s = 0
  (1..n).each{|i|
    s += i if n % i == 0 && (i % k == m || i % k == k - m)
  }
  s
end

def B(k, m, n)
  (1..n).map{|i| A(k, m, i)}
end

def C(k, m, n)
  ary = [1]
  a = [0] + B(k, m, n)
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - a[j] * ary[-j]} / i}
  ary
end

n = 10000
ary = C(4, 1, n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
