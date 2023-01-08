# a(0) = 0
def A(n)
  ary = [0, 1]
  (2..n).each{|i|
    ary << (1..i - 1).inject(0){|s, j| s + [ary[j], ary[i - j]].max}
  }
  ary
end

n = 2100
m = 2000
ary = A(n)
(1..m).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}