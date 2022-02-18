require 'prime'

def A(n)
  return 1 if n == 1
  a = n.prime_division.to_a
  a.map{|i| i[0]}.inject(:+) * a.map{|i| i[1]}.inject(:+)
end

n = 10000
(1..n).each{|i|
  print i
  print ' '
  puts A(i)
}
