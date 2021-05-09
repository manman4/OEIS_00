require 'prime'

def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

n = 17
ary = []
a = (1..n).map{|i| f(i)}
(1..n).each{|i|
  a.combination(i){|c|
    ary << c.inject(:+)
  }
}
ary.sort!
b = [0] + ary.select{|i| i.prime?}
(1..1000).each{|i|
  print i
  print ' '
  puts b[i]
}
