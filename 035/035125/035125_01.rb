def f(n)
  return false if n % 10 == 0
  m = n.to_s.reverse.to_i
  return (((m ** (1.0 / 3)).round) ** 3 == m)
end

def pl(n)
  n.to_s.reverse.to_i == n
end

ary = []
(1..2 * 10 ** 7).each{|i|
  j = i * i * i
  if !pl(j) && f(j)
    ary << i
p ary
  end
}
p ary
