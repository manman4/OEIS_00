# n=90くらいから遅くなる
def a006950(n)
  a = (1..n - 1).map{|i| [i]}
  ary = [[n]]
  while a.size != 0
    b = [] of Array(Int32)
    a.each{|i|
      x = i[-1].to_i
      s = n - i.sum
      t = x % 2
      (x + t..s - 1).each{|j| b << i.clone + [j]}
      ary << i.clone + [s] if s >= x + t
    }
    a = b
  end
  ary.size
end

n = 100
(0..n).each{|i|
  j = a006950(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
