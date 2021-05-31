require 'prime'

def A(n)
  a, b, c, d = 4, 0, 0, 3
  cnt = 0
  ary = []
  while ary.size < n
    a, b, c, d = b, c, d, a + b
    cnt += 1
    ary << a / cnt if cnt.prime?
  end
  ary
end

n = 1000
ary = A(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
