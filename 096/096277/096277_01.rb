require 'prime'

def A(k, n)
  a = Prime.take(n + k).to_a
  k.times{|i|
    a = (0..n + k - i - 2).map{|i| a[i + 1] + a[i]}
  }
  a
end

n = 10100
ary = A(2, n)
(1..10000).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
