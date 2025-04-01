def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# a(-1) = 0; a(n) = 1 + (1/3) * (2*a(n-1) + Sum_{k=0..n-1} binomial(n,k) * a(k)) for n > -1.
def A(n)
  ary = [1]
  (1..n).each{|i|
    ary << 1 + (0..i - 1).inject(2 * ary[i - 1]){|s, j| s + ncr(i, j) * ary[j]} / 3
  }
  ary
end

n = 500
ary = [0] + A(n)
(-1..n).each{|i|
  j = ary[i + 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}