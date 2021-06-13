require 'prime'

def A080790(n)
  Prime.each(5 * 10 ** 5).select{|i| (j = i.to_s(2).reverse.to_i(2)) != i && j.prime?}[0..n - 1]
end

n = 10000
ary = A080790(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
