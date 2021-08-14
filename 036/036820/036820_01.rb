def A(k, n)
  s = 0
  (1..n).each{|i|
    s += i if n % i == 0 && (i % k == 0 || i % k == 1 || i % k == k - 1)
  }
  s
end

def B(k, n)
  (1..n).map{|i| A(k, i)}
end

def C(k, n)
  ary = [1]
  a = [0] + B(k, n)
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s + a[j] * ary[-j]} / i}
  ary
end

n = 55
(5..5).each{|i|
  p i
  p B(i, n)
  p ary = C(i, n)
  # (0..n).each{|j|
  #   print j
  #   print ' '
  #   print ary[j]
  #   print ' |'
  # }
}
