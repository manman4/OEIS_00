require 'prime'

def A(n)
  a = [1]
  (2..10 ** 7).each{|i|
    break if a.size == n
    b = i.prime_division
    a << i if b.size == 1
  }
  ps = [1] + [0] * n
  a.each{|num|
    (num..n).each{|i|
      ps[i] += ps[i - num]
    }
  }
  ps
end

n = 10000
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts ary[i]
}
