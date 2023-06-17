# x^2 + y^3 = n 
def A(n)
  s = 0
  (1..n).each{|i|
    break if i * i > n
    (1..n).each{|j|
      s += 1 if i * i + j * j * j == n
    }
  }
  s
end

n = 10000
(1..n).each{|i| 
  print i
  print ' '
  puts A(i)
}