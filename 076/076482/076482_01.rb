def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

# Triangle with T(n,k)=n!*(k-1)^k/k! where 1<=k<=n.
def T(n, k)
  f(n) * (k - 1) ** k / f(k)
end

def A076482(n)
  (1..n).map{|i| (1..i).map{|j| T(i, j)}}.flatten
end

n = 140
ary = A076482(n)
(1..ary.size).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}