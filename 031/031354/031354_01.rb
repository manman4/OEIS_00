def A(k, n)
  s = 1
  n.to_s.split('').each{|i|
    s *= i.to_i ** k
  }
  s
end

def B(k, n)
  cnt = 0
  while n > 1
    cnt += 1
    n = A(k, n)
  end
  cnt
end

n = 10000
(1..n).each{|i|
  j = B(8, i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
