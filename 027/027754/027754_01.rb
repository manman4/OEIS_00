require 'prime'

def A(k, n)
  ary = []
  i = 0
  while ary.size < n
    j = i * i + i + k
    ary << i if j.prime?
    i += 1
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
