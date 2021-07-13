require 'prime'

def A(n)
  a = Prime.each(n).to_a
  (a.map{|i| i + 1} + a.map{|i| i - 1}).uniq.sort
end

n = 10000
ary = A(60000)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
