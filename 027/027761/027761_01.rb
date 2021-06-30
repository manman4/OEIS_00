require 'prime'

def A(n)
  ary = []
  p_ary = Prime.each(2 * n + 1).to_a
  2.step(2 * n, 2){|i|
    j = 0
    p_ary.each{|k|
      break if k - 1 > i
      j += 1r / k if i % (k - 1) == 0
    }
    ary << j.numerator
  }
  ary
end

n = 10000
ary = A(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
