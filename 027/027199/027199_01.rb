# 和因子はmin以上max以下
def partition(n, min, max)
  return [[]] if n == 0
  [max, n].min.downto(min).flat_map{|i| partition(n - i, min, i).map{|rest| [i, *rest]}}
end

def A(n, k)
  cnt = 0
  partition(n, k, n).each{|i|
    cnt += 1 if i.size % 2 == 1
  }
  cnt
end

def A027199(n)
  (1..n).map{|i| (1..i).map{|j| A(i, j)}}.flatten
end

n = 12
p ary = A027199(n)
(1..ary.size).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
