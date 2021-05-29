require 'prime'

def A(n)
  ary = []
  i = 1
  while ary.size < n
    if i.prime_division.to_a.size == 2
      if (i + 1).prime_division.to_a.size == 2
        ary << i
      end
    end
    i += 1
  end
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
