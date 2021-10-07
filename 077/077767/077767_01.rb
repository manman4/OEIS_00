require 'prime'

def A(k, n)
  s = 0
  (n ** 2..(n + 1) ** 2).each{|i|
    if i.prime?
      s += 1 if i % 4 == k
    end
  }
  s
end

n = 10000
(1..n).each{|i| 
  print i
  print ' '
  puts A(3, i)
}