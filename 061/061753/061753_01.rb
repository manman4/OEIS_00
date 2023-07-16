# Numbers k such that k! is divisible by (k+1)^m.
def A(k, n)
  ary = []
  cnt = 1
  s = 1
  while ary.size < n
    ary << cnt if s % (cnt + 1) ** k == 0
    cnt += 1
    s *= cnt
  end
  ary
end

n = 10000
ary = A(5, n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
