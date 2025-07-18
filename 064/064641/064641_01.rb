# a(n) = 1/(n+1) * ( (5*n-4)*a(n-1) + 9*(n-1)*a(n-2) + 3*(n-2)*a(n-3) ).
def A(n)
  ary = [1, 2, 7]
  (3..n).each{|i|
    ary << ((5*i-4)*ary[i-1] + 9*(i-1)*ary[i-2] + 3*(i-2)*ary[i-3]) / (i + 1)
  }
  ary
end

n = 1000
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}