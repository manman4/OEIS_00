require 'prime'

def A(n)
  ary = [2]
  Prime.take(n).each{|i|
    ary << i - ary[-1]
  }
  ary
end

n = 10000
m = 10100
ary = A(m)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}