# x^2 + y^3 = n 
def A(n)
  s = 0
  (1..n).each{|i|
    break if i * i > n
    (1..n).each{|j|
      k = i * i + j * j * j
      break if k > n
      s += 1 if k == n
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


