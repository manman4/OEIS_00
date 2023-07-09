# Using offset 1: a(1) = 1; a(n even) = a(n-1); a(n odd) = a(n-1) + a((n-1)/2). 

def A(n)
  ary = [0, 1]
  (2..n).each{|i|
    a = ary[i - 1]
    a += ary[(i - 1) / 2] if i % 2 == 1
    ary << a
  }
  ary
end

def A040039(n)
  A(n + 1)[1..-1]
end

n = 10100
m = 10000
ary = A040039(n)
(0..m).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}