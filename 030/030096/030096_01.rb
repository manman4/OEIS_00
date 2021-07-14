require 'prime'

def A()
  ary = []
  Prime.each(10 ** 7){|i|
    ary << i if i.to_s.split('').map(&:to_i).all?{|i| i % 2 == 1}
  }
  ary
end

ary = A()
(1..10000).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}

