require 'prime'

def A(k, n)
  ary = [k + 1]
  i = k + 2
  while ary.size < n
    if i.prime?
      i_ary = (i - k).prime_division.to_a
      s = i_ary.size
      if i_ary.map{|j| j[1]} == Array.new(s, 1)
        ary << i
      end
    end
    i += 1
  end
  ary
end

n = 10000
ary = A(2, n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
