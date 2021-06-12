# �a���q��min�ȏ�max�ȉ��Ō݂��ɈقȂ�
def partition(n, min, max)
  return [[]] if n == 0
  [max, n].min.downto(min).flat_map{|i| partition(n - i, min, i - 1).map{|rest| [i, *rest]}}
end

def A051907(n)
  print 1
  print ' '
  puts 1
  (2..n).each{|m|
    cnt = 0
    partition(m, 2, m).each{|ary|
      cnt += 1 if ary.inject(0){|s, i| s + 1 / i.to_r} == 1
    }
    print m
    print ' '
    puts cnt
  }
end
n = 50
A051907(n)
