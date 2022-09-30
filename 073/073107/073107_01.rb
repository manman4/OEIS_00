def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A(k, n)
  m = f(n)
  (0..n).inject(0){|s, j| s + ncr(j, k) * m / f(j)}
end

def B(n)
  ary = (0..n).map{|i| (0..i).map{|j| A(j, i)}}.flatten
end

n = 13
p ary = B(n)
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
