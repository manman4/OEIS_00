require 'prime'

def A(n)
  i = 0
  ary = []
  while ary.size < n
    j = 4 * i * i + 4 * i + 59
    ary << j if j.prime?
    i += 1
  end
  ary
end

n = 10000
ary = A(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
