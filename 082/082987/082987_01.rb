# Fibonacci number.
def f(n)
  a, b = 0, 1
  ary = [0]
  cnt = 0
  while cnt < n
    cnt += 1
    a, b = b, a + b
    ary << a
  end
  ary
end

def A(k, n)
  ary = f(n)
  a = []
  s = 0
  (0..n).each{|i|
    s += k ** i * ary[i]
    a << s
  }
  a
end

n = 1000
ary = A(3, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}