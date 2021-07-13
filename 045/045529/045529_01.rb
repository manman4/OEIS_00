require 'matrix'

def f(m, n)
  ary0 = Array.new(m, 0)
  ary0[0] = 1
  v = Vector.elements(ary0)
  ary1 = [Array.new(m, 1)]
  (0..m - 2).each{|i|
    ary2 = Array.new(m, 0)
    ary2[i] = 1
    ary1 << ary2
  }
  # �z��������ɓW�J
  a = Matrix[*ary1]
  ((a ** n) * v)[m - 1]
end

n = 5
(0..n).each{|i|
  j =  f(2, 3 ** i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
