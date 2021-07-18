require 'prime'

def A(n)
  ary = []
  s = 1
  while ary.size < n
    s += 1
    ary << s if !s.prime?
  end
  ary
end

def B(n)
  ary = A(n)
  (1..n).map{|i| (1..i).inject(0){|s, j| s + j * ary[i - j]}}
end

n = 10000
ary = B(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
    
