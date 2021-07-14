def A(k, n)
  s = 0
  (1..n).each{|i|
    s += i if n % i == 0 && i % k != 0
  }
  s
end

def B(k, n)
  (1..n).map{|i| A(k, i)}
end

def C(k, m, n)
  ary = [1]
  a = [0] + B(k, n)
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - m * a[j] * ary[-j]} / i}
  ary
end

n = 47
ary = C(7, 4, n + 1)
-1.upto(n){|i|
  print i
  print ' '
  puts ary[i + 1]
}
