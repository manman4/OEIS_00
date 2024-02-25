# a(0)=a(1)=1, a(2)=a(3)=0, a(n) = (n+1)*a(n-1) - (n-2)*a(n-2) - (n-5)*a(n-3) + (n-3)*a(n-4).
def A002464(n)
  a = [1, 1, 0, 0]
  (4..n).each{|i| a << (i + 1) * a[-1] - (i - 2) * a[-2] - (i - 5) * a[-3] + (i - 3) * a[-4]}
  a
end

n = 1000
ary = A002464(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}