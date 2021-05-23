require 'prime'

def A(n)
  p_ary = [0] + Prime.take(n).to_a
  ary = [1, 2, 4, 7]
  s = 5
  (4..n).each{|i|
    s += p_ary[i]
    ary << s
  }
  ary
end

m = 10100
n = 10000
ary = A(m)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
