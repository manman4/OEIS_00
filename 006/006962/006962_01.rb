require 'prime'

def A006962(n)
  ary = []
  cnt = 1
  Prime.each(10 ** 7){|p|
    a = Array.new(p, 0)
    (0..p - 1).each{|i| a[(i * i) % p] += 1}
    s = 0
    (0..p - 1).each{|i|
      s += a[(i * i * i - 4 * i * i + 16) % p]
      break if s > p
    }
    if p == s
      ary << p
      cnt += 1
      return ary if cnt > n
    end
  }
end

n = 60
ary = A006962(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
