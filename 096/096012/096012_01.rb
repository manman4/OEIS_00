require 'prime'

def A(n)
  ary = []
  (1..n).each{|i|
    ary << i if (i ** 2 + 1).prime? && ((i + 2) ** 2 + 1).prime?
  }
  ary
end

ary = A(3 * 10 ** 5)
(1..10 ** 3).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
