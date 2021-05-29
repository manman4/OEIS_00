require 'prime'

n = 10
ary = []
(1..n).each{|i|
  (1..i).each{|j|
    if i.gcd(j) == 1
      s = i + j
      while !(s.prime?)
        s += i
      end
      ary << s
      p [i, j, s]
    end
  }
}
(1..ary.size).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}

s = 0
ary = []
(1..n).each{|i|
  (1..i).each{|j|
    if i.gcd(j) == 1
      s += 1
    end
  }
  ary << s
}
p ary
