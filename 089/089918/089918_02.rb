def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# a(0) = 1; a(n) = (1/n) * Sum_{k=1..n} n^k * binomial(n,k) * a(n-k).
def a(n)
  ary = [1]
  (1..n).each{|i| ary << (1..i).inject(0){|sum, k| sum + i**k * ncr(i, k) * ary[-k]} / i}
  ary
end

def b_t(ary)
  # Sum_{k=0..n} binomial(n, k) * a(k).
  (0..ary.size - 1).map{|i| (0..i).inject(0){|sum, k| sum + ncr(i, k) * ary[k]}}
end

n = 295
ary = a(n)
(0..n).each{|i|
  j = ary[i] / (i + 1)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
  ary = b_t(ary)
}