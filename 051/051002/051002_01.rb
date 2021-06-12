require 'prime'

def divisor_list(n)
  return [] if n <= 0
  return [1] if n == 1
  n.prime_division.map{|e| [*(0..e[-1])].map{|v| e[0] ** v}}.inject{|res, e| res.map{|t| e.map{|v| t * v}}.flatten}.sort
end

n = 10000
(1..n).each{|i|
  s = 0
  divisor_list(i).each{|i| s += i ** 5 if i % 2 == 1}
  print i
  print ' '
  puts s
}
