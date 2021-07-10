require 'prime'

def f(n)
  ((1..n - 1).inject(0){|s, i| s + i ** (n - 1)} + 1) / n
end

n = 100
p_ary = Prime.take(n).to_a
(1..n).each{|i|
  j = f(p_ary[i - 1])
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
