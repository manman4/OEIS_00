def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A(n)
  (0..n).map{|i| (0..i - 1).inject(0){|s, j| s + (-1) ** (i + j + 1) * ncr(i, j) * f(i - j - 1)}}
end

def B(n)
  a = A(n)
  ary = [1]
  (1..n).each{|i|
    ary <<(1..i).inject(0){|s, j| s + a[j] * ncr(i - 1, j - 1) * ary[-j]}
  }
  ary
end

n = 500
ary = B(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}