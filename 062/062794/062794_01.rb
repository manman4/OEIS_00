require 'prime'

def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def A008683(n)
  ary = n.prime_division
  return (-1) ** (ary.size % 2) if ary.all?{|i| i[1] == 1}
  0
end

def A(n)
  a = (0..n).map{|i| f(i)}
  m_ary = [0] + (1..n).map{|i| A008683(i)}
  ary = []
  (1..n).each{|i|
    s = 0
    (1..i).each{|j|
      s += m_ary[i / j] * a[j] if i % j == 0
    }
    ary << s
  }
  ary
end

n = 600
ary = A(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
