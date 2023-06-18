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

n = 100000
b=[0]
(1..n).each{|i|
  j = A(i)
  b<<j
}

p (1..5).map{|i| b.index(i)}

