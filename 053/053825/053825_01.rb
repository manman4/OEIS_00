require 'prime'

def A008683(n)
  ary = n.prime_division
  return (-1) ** (ary.size % 2) if ary.all?{|i| i[1] == 1}
  0
end

def A(k, n)
  m_ary = [0] + (1..n).map{|i| A008683(i)}
  ary = [0]
  (1..n).each{|i|
    s = 0
    (1..i).each{|j|
      s += m_ary[j] * m_ary[i / j] * j ** k if i % j == 0
    }
    ary << s
  }
  ary
end

n = 42
ary = A(3, n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
