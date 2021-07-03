require 'prime'

def A(n)
  ary = []
  cnt = 0
  while ary.size < n
    ary << cnt if (3 * cnt + 1).prime? && (3 * cnt + 5).prime? && (3 * cnt + 7).prime?
    cnt += 1
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
