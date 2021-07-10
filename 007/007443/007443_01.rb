require 'prime'

def A007443(n)
  a = Prime.take(n).to_a
  ary = [2]
  (n - 1).times{|i|
    a = (0..n - i - 2).map{|i| a[i + 1] + a[i]}
    ary << a[0]
  }
  ary
end

n = 3000
ary = A007443(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
