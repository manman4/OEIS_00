require 'prime'

def A064549(n)
  s = 1
  n.prime_division.each{|i| s *= i[0]}
  n * s
end

n = 10000
(1..n).each{|i|
  print i
  print ' '
  puts A064549(i)
}
