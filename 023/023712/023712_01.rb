# kがm個
def A(k, m, n)
  n.to_s(4).split('').map(&:to_i).count(k) == m
end

def B(k, m, n)
  ary = []
  s = 0
  while ary.size < n
    ary << s if A(k, m, s)
    s += 1
  end
  ary
end

n = 10000
ary = B(1, 3, n)
(1..ary.size).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}