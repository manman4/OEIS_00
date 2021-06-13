def f(n)
  n.to_s.split('').map(&:to_i).inject(0){|s, i| s + i * i}
end

def A(n)
  m = n
  cnt = 0
  while m != 1 && m != 4
    cnt += 1
    m = f(m)
  end
  cnt
end

n = 80
(1..n).each{|i| 
  print i
  print ' '
  puts A(i)
}
