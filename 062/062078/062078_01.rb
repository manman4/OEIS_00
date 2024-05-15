def lcm(ary)
  ary.inject(1){|a, b| a.lcm(b)}
end

def A(n)
  lcm(n.to_s.split('').map(&:to_i))
end

n = 10000
(1..n).each{|i|
  j = A(i)
  print i
  print ' '
  puts j
}

