def partition(n, min, max)
  return [[]] if n == 0
  [max, n].min.downto(min).flat_map{|i| partition(n - i, min, i).map{|rest| [i, *rest]}}
end
def A051908(n)
  ary = [1]
  (2..n).each{|m|
    cnt = 0
    partition(m, 2, m).each{|ary|
      cnt += 1 if ary.inject(0){|s, i| s + 1 / i.to_r} == 1
    }
    ary << cnt
  }
  ary
end
p A051908(100)
