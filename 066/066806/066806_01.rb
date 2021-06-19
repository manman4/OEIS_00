require 'prime'

def T(n)
  return 1 if n < 4 || n.prime?
  s = 1
  ary = []
  (2..n / 2).each{|i|
    ary << i if n % i == 0
  }
  f_ary = ary.map{|i| [i]}
  while !f_ary.empty?
    b_ary = []
    f_ary.each{|i|
      ary.each{|j|
        if i[-1] <= j
          k = i.inject(:*) * j
          if k > n
            break
          elsif k == n
            s += 1
          else
            b_ary << (i + [j])
          end
        end
      }
    }
    f_ary = b_ary
  end
  s
end

def s(f_ary, g_ary, n)
  s = 0
  (1..n).each{|i| s += i * f_ary[i] * g_ary[i] ** (n / i) if n % i == 0}
  s
end

def A(f_ary, g_ary, n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(f_ary, g_ary, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s + a[j] * ary[-j]} / i}
  ary
end

def B(n)
  ary1 = (0..n).map{|i| -T(i)}
  ary4 = Array.new(n + 1, -1)
  A(ary1, ary4, n)
end

n = 10100
ary = B(n)
(0..10000).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}

