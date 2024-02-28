# If n appears so do 2n, 3n+2, 6n+3.
def A(n)
  a = [1]
  i = 0
  while i <= n
    a << a[i] * 2
    a << a[i] * 3 + 2
    a << a[i] * 6 + 3
    a.sort!.uniq!
    i += 1
  end
  a.sort[0..n - 1]
end

n = 15889
ary = A(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}