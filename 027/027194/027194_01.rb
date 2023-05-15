# 和因子はmin以上max以下
def partition(n, min, max)
  return [[]] if n == 0
  [max, n].min.downto(min).flat_map{|i| partition(n - i, min, i).map{|rest| [i, *rest]}}
end

def A(k, n)
  cnt = 0
  partition(n, k, n).each{|i|
    cnt += 1 if i.size % 2 == 1
  }
  cnt
end

n = 30
b = []
(1..n).each{|i|
  j = A(2, i - 2)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
  b << j
}
p b