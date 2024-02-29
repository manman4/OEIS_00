# If n appears so do 2n-2 and 3n-3.
def A(n)
  a = [5]
  i = 0
  while i <= n
    a << a[i] * 2 - 2
    a << a[i] * 3 - 3
    a.sort!.uniq!
    i += 1
  end
  a[0..n - 1]
end

n = 10000
ary = A(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}