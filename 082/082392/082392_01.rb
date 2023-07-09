# a(0) = 1, a(2*n) = 2^n, a(2*n+1) = a(n).

def A082392(n)
  ary = [1]
  a = 1
  (1..n).each{|i|
    a = i % 2 == 0 ? 2 ** (i / 2) : ary[i / 2]
    ary << a
  }
  ary
end

n = 5000
ary = A082392(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}