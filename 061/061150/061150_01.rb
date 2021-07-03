require 'prime'

def A(n)
  p_ary = [0] + Prime.take(n).to_a
  ary = []
  (1..n).each{|i|
    s = 0
    (1..i).each{|j|
      s += j * p_ary[j] if i % j == 0
    }
    ary << s
  }
  ary
end

n = 10000
ary = A(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
