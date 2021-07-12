# ary�̃T�C�Y��n
def A(ary, n)
  a = [0] + ary
  b = []
  (1..n).each{|i|
    s = 0
    (1..i).each{|j|
      s += a[j] if i % j == 0
    }
    b << s
  }
  b
end

n = 68
ary = Array.new(n, 1)
5.times{
  ary = A(ary, n)
}
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
