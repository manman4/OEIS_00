def plus(f_ary, b_ary)
  s1, s2 = f_ary.size, b_ary.size
  ary = Array.new([s1, s2].max, 0)
  (0..s1 - 1).each{|i| ary[i] += f_ary[i]}
  (0..s2 - 1).each{|i| ary[i] += b_ary[i]}
  ary
end

def sum(ary)
  s = [0]
  ary.each{|i| s = plus(s, i)}
  s
end

# def A(n)
#   ary = [1]
#   (1..n).each{|i|
#     new_ary = [0] + ary
#     (0..i - 1).each{|j| 
#       new_ary[j] += ary[j]
#     }
#     ary = new_ary
#     p ary
#   }
# end

def A(n)
  ary = [[1]]
  a = [[1]]
  (1..n).each{|i|
    new_ary = [[0]] + ary
    (0..i - 1).each{|j| 
      new_ary[j] = plus(new_ary[j], [0] * j + ary[j])
    }
    ary = new_ary
    a << sum(ary)
  }
  a
end

n = 48
ary = A(n).flatten
(0..ary.size - 1).each{|i|
  print i
  print ' '
  puts ary[i]
}