def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# T(n,m) := Sum_{k=0..floor((n-m)/2)} binomial(n-2k,m) * binomial(n-m-k,k).
def A(n, m)
  (0..(n - m) / 2).inject(0){|s, i| s + ncr(n - 2 * i, m) * ncr(n - m - i, i)}
end

def A091594(n)
  (0..n).map{|i| (0..i).map{|j| A(i, j)}}.flatten
end

n = 139
ary = A091594(n)
(0..ary.size - 1).each{|i|
  print i
  print ' '
  puts ary[i]
}