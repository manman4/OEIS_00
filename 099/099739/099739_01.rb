require 'prime'

def A008683(n)
  ary = n.prime_division
  return (-1) ** (ary.size % 2) if ary.all?{|i| i[1] == 1}
  0
end

def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

n = 500
s = 0
(1..n).each{|i|
  s += A008683(i) * f(i)
  break if s.to_s.size > 1000
  print i
  print ' '
  puts s
}