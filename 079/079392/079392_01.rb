def s(n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0}
  s
end
def A000594(n)
  ary = [1]
  a = [0] + (1..n - 1).map{|i| s(i)}
  (1..n - 1).each{|i| ary << (1..i).inject(0){|s, j| s - 24 * a[j] * ary[-j]} / i}
  ary
end


n = 100
m = 2 * n + 100
t_ary = [0] + A000594(m)
ary = (1..m).select{|i| t_ary[i] > 0}
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
