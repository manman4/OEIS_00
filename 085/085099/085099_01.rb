require 'prime'
def A(n)
  cnt = 0
  flag = true
  while flag
    cnt += 1
    flag = false if (cnt ** 2 + n).prime?
  end
  cnt
end
p (1..100).map{|i| A(i)}
=begin
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
=end
