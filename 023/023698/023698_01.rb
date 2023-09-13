# kがm個
def A(k, m, n)
  n.to_s(3).split('').map(&:to_i).count(k) == m
end

def B(k, m, n)
  ary = []
  s = 1
  while ary.size < n
    ary << s if A(k, m, s)
    s += 1
  end
  ary
end

n = 10000
ary = B(1, 7, n)
(1..ary.size).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}