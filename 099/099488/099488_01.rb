# a(n) = 4*a(n-1) - 2*a(n-2) + 4*a(n-3) - a(n-4).
def A(n)
  ary = [1, 2, 7, 28]
  (4..n).each{|i|
    ary << 4 * ary[-1] - 2 * ary[-2] + 4 * ary[-3] - ary[-4]
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
