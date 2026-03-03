def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# a(0) = 1; a(n) = Sum_{k=1..n} n^(k-1) * binomial(n,k) * a(n-k).
def a(n)
  ary = [1]
  (1..n).each{|i| ary << (1..i).inject(0){|sum, k| sum + i**(k-1) * ncr(i, k) * ary[-k]}}
  ary
end

n = 500
ary = a(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}