def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# a(0) = 1; a(n) = n * a(n-1) + Sum_{k=1..n} (k-1)! * binomial(n,k) * a(n-k).
def A(n)
  a = [1]
  (1..n).each{|i|
    a << (1..i).inject(i * a[-1]){|s, j| s + f(j - 1) * ncr(i, j) * a[-j]}
  }
  a
end

n = 500
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}