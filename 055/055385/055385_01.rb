require 'prime'
def A(n)
  return 59649589127497217 if n == 64
  return 2 if n % 2 == 1
  m = n ** n + 1
  i = 3
  while m % i > 0
    i += 2
  end
  i
end

n = 500
(1..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
