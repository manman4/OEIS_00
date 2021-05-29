require 'bigdecimal'

def A(m, n)
  d = 500
  ary = [m]
  (n - 1).times{
    m = (BigDecimal.new((2 * m * (m + 1)).to_s).sqrt(d)).to_i
    ary << m
  }
  ary
end

n = 500
ary = A(5, n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
