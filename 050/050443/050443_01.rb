def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A(k, n)
  return k + 1 if n == 0
  (n * (1..n / k).inject(0){|s, i| s + ncr(i, n - k * i) / i.to_r}).to_i
end

def B(k, n)
  (0..n).map{|i| A(k, i)}
end

n = 1000
ary = B(3, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
