def all_digits?(n)
  n.to_s.split('').uniq.size == 10
end

def A(n)
  return 1023456789 if n == 1
  k = 2
  while !all_digits?(k ** n)
    k += 1
  end
  k ** n
end

n = 10000
(1..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
