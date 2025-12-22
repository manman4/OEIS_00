# a(n) = 2*a(n-1) + a(n-2) + 2*a(n-3) - a(n-4). 
def a(n)
  ary = [1, 3, 6, 17]
  (4..n).each{|i|
    ary << 2*ary[-1] + ary[-2] + 2*ary[-3] - ary[-4]
  }
  ary
end

n = 1000
ary = a(n)
(0..n).each{|i| 
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
} 