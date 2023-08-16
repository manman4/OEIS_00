def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# T(n,k) = 11^k*ncr(n,k)
def A(n)
  (0..n).map{|i| (0..i).map{|j| 11 ** j * ncr(i, j)}}.flatten
end

n = 139
ary = A(n)
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}