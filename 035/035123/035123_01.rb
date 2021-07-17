def f(n)
  return false if n % 10 == 0
  m = n.to_s.reverse.to_i
  return ((Math.sqrt(m).round) ** 2 == m)
end

def pl(n)
  n.to_s.reverse.to_i == n
end

ary = []
(1..10 ** 7).each{|i|
  j = i * i
  if !pl(j) && f(j)
    ary << i
  end
}
(1..ary.size).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
